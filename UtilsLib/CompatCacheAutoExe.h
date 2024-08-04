#pragma once
#include "Utils.h"
class CompatCacheAutoExe
{
public:
    CompatCacheAutoExe();
    ~CompatCacheAutoExe();
    NTSTATUS Init(const wchar_t* Path, HANDLE ExistingHandle);
    NTSTATUS AslFileOpen(const wchar_t* Path);
    NTSTATUS AslPathBuildSignatureLongpath(const wchar_t* Path);
    HANDLE m_Handle;
    BOOL m_ShouldCloseHandle;
    PVOID m_HeapBuffer;
    UNICODE_STRING m_UnicodeStr;
};