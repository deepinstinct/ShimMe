#include "Utils.h"

LPVOID LocateResource(const PDWORD Size, const LPCSTR Name, const LPCSTR Type)
{
	const HMODULE moduleHandle = GetModuleHandle(nullptr);
	const HRSRC resourceHandle = FindResourceA(moduleHandle, Name, Type);
	if (nullptr == resourceHandle)
		ThrowException("FindResourceA failed", GetLastError());

	const HGLOBAL globalHandle = LoadResource(moduleHandle, resourceHandle);
	if (nullptr == globalHandle)
		ThrowException("LoadResource failed", GetLastError());

	const LPVOID resourcePointer = LockResource(globalHandle);
	if (nullptr == resourcePointer)
		ThrowException("LockResource failed", GetLastError());

	*Size = SizeofResource(nullptr, resourceHandle);
	return resourcePointer;
}

void DropResource(const wstring& DirectoryPath, const wstring& FileBaseName, const LPCSTR Name, const LPCSTR Type)
{
	BOOL success = CreateDirectoryW(DirectoryPath.c_str(), nullptr);
	if (!success)
	{
		const DWORD error = GetLastError();
		if (ERROR_ALREADY_EXISTS != error)
			ThrowException("[DropResource] - CreateDirectoryW failed", error);
	}
	DWORD resourceSize = 0;
	const PVOID resource = LocateResource(&resourceSize, Name, Type);
	wstring fileFullPath(DirectoryPath);
	fileFullPath.append(L"\\");
	fileFullPath.append(FileBaseName);
	const HANDLE fileHandle = CreateFileW(fileFullPath.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (INVALID_HANDLE_VALUE == fileHandle)
		ThrowException("[DropResource] - CreateFileW failed", GetLastError());

	success = WriteFile(fileHandle, resource, resourceSize, nullptr, nullptr);
	CloseHandle(fileHandle);
	if (!success)
		ThrowException("[DropResource] - WriteFile failed", GetLastError());
}

void ThrowException(const char* Message, const DWORD ErrorCode)
{
	stringstream messageStream;
	messageStream << Message;
	messageStream << " 0x";
	messageStream << std::hex << ErrorCode << std::dec;
	const string errorMessage = messageStream.str();
	throw runtime_error(errorMessage.c_str());
}

bool EnablePrivilege(const HANDLE TokenHandle, const LPCWSTR Privilege)
{
	TOKEN_PRIVILEGES tokenPrivileges = {};

	if (!::LookupPrivilegeValueW(nullptr, Privilege, &tokenPrivileges.Privileges[0].Luid))
		return false;

	tokenPrivileges.PrivilegeCount = 1;
	tokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	return ::AdjustTokenPrivileges(TokenHandle, 0, &tokenPrivileges, sizeof(TOKEN_PRIVILEGES), nullptr, nullptr);
}

bool EnablePrivilegeCurrentProcess(const LPCWSTR Privilege)
{
	HANDLE tokenHandle = nullptr;
	if (::OpenProcessToken(::GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &tokenHandle))
	{
		return EnablePrivilege(tokenHandle, Privilege);
	}
	return false;
}

bool EnableDebugPrivilegeCurrentProcess()
{
	return EnablePrivilegeCurrentProcess(SE_DEBUG_NAME);
}

void NtCreateUserProcessWrap(wstring& Path,
	wstring& Commandline,
	wstring& CurrentDirectory,
	HANDLE* ProcessHandle,
	HANDLE* ThreadHandle,
	ACCESS_MASK ProcessDesiredAccess,
	ACCESS_MASK ThreadDesiredAccess,
	ULONG ProcessFlags,
	ULONG ThreadFlags)
{
	// Path to the image file from which the process will be created
	UNICODE_STRING NtImagePath;
	NTSTATUS status = RtlDosPathNameToNtPathName_U_WithStatus(Path.c_str(), &NtImagePath, nullptr, nullptr);
	if (!NT_SUCCESS(status))
		ThrowException("[NtCreateUserProcessWrap] RtlInitUnicodeString failed", status);

	// Create the process parameters
	PRTL_USER_PROCESS_PARAMETERS ProcessParameters = nullptr;
	UNICODE_STRING commandlineVar = {};
	PUNICODE_STRING commandlinePtr = nullptr;
	if (!Commandline.empty())
	{
		RtlInitUnicodeString(&commandlineVar, Commandline.c_str());
		commandlinePtr = &commandlineVar;
	}

	UNICODE_STRING currentDirectoryVar = {};
	PUNICODE_STRING currentDirectoryPtr = nullptr;
	if (!CurrentDirectory.empty())
	{
		RtlInitUnicodeString(&currentDirectoryVar, CurrentDirectory.c_str());
		currentDirectoryPtr = &currentDirectoryVar;
	}
	RtlCreateProcessParametersEx(&ProcessParameters, &NtImagePath, nullptr, currentDirectoryPtr, commandlinePtr, nullptr, nullptr, nullptr, nullptr, nullptr, RTL_USER_PROCESS_PARAMETERS_NORMALIZED);
	if (!NT_SUCCESS(status))
		ThrowException("[NtCreateUserProcessWrap] RtlCreateProcessParametersEx failed", status);

	// Initialize the PS_CREATE_INFO structure
	PS_CREATE_INFO CreateInfo = { 0 };
	CreateInfo.Size = sizeof(CreateInfo);
	CreateInfo.State = PsCreateInitialState;

	// Initialize the PS_ATTRIBUTE_LIST structure
	auto attributeList = static_cast<PPS_ATTRIBUTE_LIST>(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(PS_ATTRIBUTE_LIST)));
	attributeList->TotalLength = sizeof(PS_ATTRIBUTE_LIST);
	attributeList->Attributes[0].Attribute = PS_ATTRIBUTE_IMAGE_NAME;
	attributeList->Attributes[0].Size = NtImagePath.Length;
	attributeList->Attributes[0].Value = reinterpret_cast<ULONG_PTR>(NtImagePath.Buffer);

	// Create the process
	HANDLE hProcess = nullptr, hThread = nullptr;
	status = NtCreateUserProcess(&hProcess, 
		&hThread,
		ProcessDesiredAccess,
		ThreadDesiredAccess,
		nullptr,
		nullptr,
		ProcessFlags,
		ThreadFlags,
		ProcessParameters,
		&CreateInfo,
		attributeList);
	
	// Clean up
	RtlFreeUnicodeString(&NtImagePath);
	HeapFree(GetProcessHeap(), 0, attributeList);
	RtlDestroyProcessParameters(ProcessParameters);

	if (!NT_SUCCESS(status))
		ThrowException("[NtCreateUserProcessWrap] NtCreateUserProcess failed", status);
	*ProcessHandle = hProcess;
	*ThreadHandle = hThread;
}