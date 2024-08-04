#pragma once
#include <Windows.h>
#include <string>

using std::wstring;

const wstring PIPE_NAME(L"\\\\.\\pipe\\ElevationPipe");

#ifdef INJECTED_EXPORTS
#define INJECTED_API __declspec(dllexport)
#else
#define INJECTED_API __declspec(dllimport)
#endif

extern "C" INJECTED_API int Function(void);
