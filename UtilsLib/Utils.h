#pragma once
#include <Windows.h>
#include <iostream>
#include <sstream>
#include <taskschd.h>
#include <comdef.h>
#include <ntstatus.h>
#include "ahcache.h"

using std::endl;
using std::wcout;
using std::string;
using std::wstring;
using std::stringstream;
using std::runtime_error;

void DropResource(const wstring& DirectoryPath, const wstring& FileBaseName, LPCSTR Name, LPCSTR Type);
void ThrowException(const char* Message, const DWORD ErrorCode);
bool EnableDebugPrivilegeCurrentProcess();
void NtCreateUserProcessWrap(wstring& Path,
	wstring& Commandline,
	wstring& CurrentDirectory,
	HANDLE* ProcessHandle,
	HANDLE* ThreadHandle,
	ACCESS_MASK ProcessDesiredAccess = PROCESS_ALL_ACCESS,
	ACCESS_MASK ThreadDesiredAccess = THREAD_ALL_ACCESS,
	ULONG ProcessFlags = 0,
	ULONG ThreadFlags = 0);