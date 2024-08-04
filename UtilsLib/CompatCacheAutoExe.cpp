#include "CompatCacheAutoExe.h"

CompatCacheAutoExe::CompatCacheAutoExe() : m_Handle(nullptr), m_ShouldCloseHandle(false), m_HeapBuffer(nullptr)
{
	m_UnicodeStr = {};
}

CompatCacheAutoExe::~CompatCacheAutoExe()
{
	if (m_ShouldCloseHandle)
		CloseHandle(m_Handle);
	if (m_HeapBuffer)
		HeapFree(GetProcessHeap(), 0, m_HeapBuffer);
}

NTSTATUS CompatCacheAutoExe::Init(const wchar_t* Path, HANDLE ExistingHandle)
{
	HANDLE tmpHandle = nullptr;
	bool tmpShouldClose;
	NTSTATUS result;
	if (INVALID_HANDLE_VALUE != ExistingHandle)
		tmpHandle = ExistingHandle;
	if (tmpHandle)
	{
		m_Handle = tmpHandle;
		tmpShouldClose = false;
	}
	else
	{
		result = AslFileOpen(Path);
		if (!NT_SUCCESS(result))
			return result;
		tmpHandle = m_Handle;
		tmpShouldClose = m_Handle != nullptr;
	}
	m_ShouldCloseHandle = tmpShouldClose;
	if (!tmpHandle)
		return STATUS_NOT_FOUND;
	result = AslPathBuildSignatureLongpath(Path);
	if (NT_SUCCESS(result))
	{
		result = RtlInitUnicodeStringEx(&m_UnicodeStr, static_cast<PCWSTR>(m_HeapBuffer));
		if (NT_SUCCESS(result))
			return STATUS_SUCCESS;
	}
	return result;
}

NTSTATUS CompatCacheAutoExe::AslFileOpen(const wchar_t* Path)
{
	m_Handle = nullptr;
	bool v3 = false;
	UNICODE_STRING DestinationString = {};
	RtlInitUnicodeString(&DestinationString, Path);
	ULONG bufferSize = MAX_PATH * sizeof(wchar_t);
	NTSTATUS status = STATUS_SUCCESS;
	auto buffer = static_cast<wchar_t*>(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, bufferSize));
	DWORD bytesNeeded = GetFullPathNameW(Path, MAX_PATH, buffer, nullptr);
	if (0 == bytesNeeded || bytesNeeded > bufferSize)
	{
		RtlFreeUnicodeString(&DestinationString);
		HeapFree(GetProcessHeap(), 0, buffer);
		printf("[CompatCacheAutoExe::AslFileOpen] - GetFullPathNameW failed %d\n", GetLastError());
		return status;
	}
	wstring systemRoot = L"\\SystemRoot\\";
	if (_wcsnicmp(buffer, systemRoot.c_str(), systemRoot.size()))
	{
		status = RtlDosPathNameToNtPathName_U_WithStatus(buffer, &DestinationString, nullptr, nullptr);
		if (!NT_SUCCESS(status))
		{
			RtlFreeUnicodeString(&DestinationString);
			HeapFree(GetProcessHeap(), 0, buffer);
			printf("[CompatCacheAutoExe::AslFileOpen] - RtlDosPathNameToNtPathName_U_WithStatus failed 0x%x", status);
			return status;
		}

		IO_STATUS_BLOCK statusBlock = {};
		OBJECT_ATTRIBUTES objectAttributes = {};
		objectAttributes.Length = sizeof(OBJECT_ATTRIBUTES);
		objectAttributes.Attributes = OBJ_CASE_INSENSITIVE;
		objectAttributes.ObjectName = &DestinationString;
		objectAttributes.SecurityDescriptor = nullptr;
		status = NtCreateFile(&m_Handle, GENERIC_READ | SYNCHRONIZE | FILE_READ_ATTRIBUTES, &objectAttributes, &statusBlock, 0, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ, FILE_OPEN, FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT, nullptr, 0);
		if (!NT_SUCCESS(status))
			printf("[CompatCacheAutoExe::AslFileOpen] - NtCreateFile failed 0x%x", status);
		RtlFreeUnicodeString(&DestinationString);
		HeapFree(GetProcessHeap(), 0, buffer);
	}
	return status;
}

NTSTATUS CompatCacheAutoExe::AslPathBuildSignatureLongpath(const wchar_t* Path)
{
	int pathLength = wcslen(Path);
	DWORD bufferSize = (pathLength + 1) * sizeof(wchar_t);
	m_HeapBuffer = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, bufferSize);
	memcpy_s(m_HeapBuffer, bufferSize, Path, bufferSize);
	return STATUS_SUCCESS;
}