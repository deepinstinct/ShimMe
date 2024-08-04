#pragma once
#include "Utils.h"

class RpcClient
{
public:
    RpcClient(RPC_IF_HANDLE Interface, const wchar_t* Endpoint);
    ~RpcClient();

protected:
    RPC_BINDING_HANDLE m_BindingHandle;

private:
    static RPC_BINDING_HANDLE CreateBindingHandle(const wchar_t* Endpoint);
    static RPC_BINDING_HANDLE ConnectToRpcServer(RPC_IF_HANDLE Interface, const wchar_t* Endpoint);
    
};