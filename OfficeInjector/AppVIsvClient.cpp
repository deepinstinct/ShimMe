#include "AppVIsvClient.h"

AppVIsvClient::AppVIsvClient() : RpcClient(AppVIsvVirtualization_v1_0_c_ifspec, GetEndpoint().c_str()) 
{

}

void AppVIsvClient::NotifyNewProcess(const int Pid) const
{
	try
	{
		virtman_notification_server_notify_new_process(m_BindingHandle, Pid);
	}
	catch (...)
	{
		ThrowException("[RpcClient::NotifyNewProcess] - RPC call failed", RPC_S_CALL_FAILED);
	}
}

void AppVIsvClient::NotifyNewChildProcess(const int Pid, const wstring& Path) const
{
	PATH_STRUCT pathStruct = {};
	pathStruct.Path = Path.c_str();
	pathStruct.PathLength = Path.length() + 1;
	try
	{
		virtman_notification_server_notify_new_child_process(m_BindingHandle, Pid, &pathStruct);
	}
	catch (...)
	{
		ThrowException("[RpcClient::NotifyNewChildProcess] - RPC call failed", RPC_S_CALL_FAILED);
	}
}

wstring AppVIsvClient::GetEndpoint()
{
	const auto regHive = HKEY_LOCAL_MACHINE;
	const auto subKey = L"SOFTWARE\\Microsoft\\AppVISV";
	const auto valueName = L"c:\\program files\\microsoft office";
	DWORD access = KEY_QUERY_VALUE;
#ifndef _WIN64
	access |= KEY_WOW64_64KEY;
#endif

	DWORD regValueSize = 0;
	HKEY keyHandle = nullptr;
	LSTATUS error = RegOpenKeyExW(regHive, subKey, 0, access, &keyHandle);
	if (ERROR_SUCCESS != error)
	{
		ThrowException("[RpcClient::GetEndpoint] - RegOpenKeyExW failed to open the sub key HKLM\\SOFTWARE\\Microsoft\\AppVISV", error);
	}

	error = RegQueryValueExW(keyHandle, valueName, nullptr, nullptr, nullptr, &regValueSize);
	if (ERROR_SUCCESS != error)
	{
		RegCloseKey(keyHandle);
		ThrowException("[RpcClient::GetEndpoint] - RegQueryValueExW failed to query the size of the value \"c:\\program files\\microsoft office\" under HKLM\\SOFTWARE\\Microsoft\\AppVISV", error);
	}

	const auto regValue = static_cast<PBYTE>(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, regValueSize));
	error = RegQueryValueExW(keyHandle, valueName, nullptr, nullptr, regValue, &regValueSize);
	if (ERROR_SUCCESS != error)
	{
		RegCloseKey(keyHandle);
		HeapFree(GetProcessHeap(), 0, regValue);
		ThrowException("[RpcClient::GetEndpoint] - RegQueryValueExW failed to retrieve the value \"c:\\program files\\microsoft office\" under HKLM\\SOFTWARE\\Microsoft\\AppVISV", error);
	}

	wstring path = L"\\RPC Control\\AppV-ISV-";
	path.append(reinterpret_cast<wchar_t*>(regValue));
	path.append(L"APPV-VIRTMAN-NOTIFICATIONS");
	RegCloseKey(keyHandle);
	HeapFree(GetProcessHeap(), 0, regValue);
	return path;
}