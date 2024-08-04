#pragma once
#include "RpcClient.h"
#ifdef _WIN64
#include "AppVIsvVirtualization_x64_h.h"
#else
#include "AppVIsvVirtualization_x86_h.h"
#endif

class AppVIsvClient final : public RpcClient
{
public:
	AppVIsvClient();
	void NotifyNewProcess(int Pid) const;
	void NotifyNewChildProcess(int Pid, const wstring& Path) const;

private:
	static wstring GetEndpoint();
};