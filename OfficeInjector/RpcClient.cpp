#include "RpcClient.h"

RpcClient::RpcClient(RPC_IF_HANDLE Interface, const wchar_t* Endpoint)
{
	m_BindingHandle = ConnectToRpcServer(Interface, Endpoint);
}

RpcClient::~RpcClient()
{
	RpcBindingFree(&m_BindingHandle);
}

RPC_BINDING_HANDLE RpcClient::CreateBindingHandle(const wchar_t* Endpoint)
{
	RPC_WSTR stringBinding = nullptr;
	RPC_BINDING_HANDLE bindingHandle = nullptr;
	auto status = RpcStringBindingComposeW(
		nullptr,
		reinterpret_cast<RPC_WSTR>(const_cast<wchar_t*>(L"ncalrpc")),
		nullptr,
		reinterpret_cast<RPC_WSTR>(const_cast<wchar_t*>(Endpoint)),
		nullptr,
		&stringBinding);
	if (status)
		ThrowException("[RpcClient::CreateBindingHandle] - RpcStringBindingComposeW failed", status);

	status = RpcBindingFromStringBindingW(stringBinding, &bindingHandle);
	RpcStringFreeW(&stringBinding);
	if (status)
		ThrowException("[RpcClient::CreateBindingHandle] - RpcBindingFromStringBindingW failed", status);

	return bindingHandle;
}

RPC_BINDING_HANDLE RpcClient::ConnectToRpcServer(const RPC_IF_HANDLE Interface, const wchar_t* Endpoint)
{
	RPC_BINDING_HANDLE bindingHandle = CreateBindingHandle(Endpoint);
	auto status = RpcEpResolveBinding(bindingHandle, Interface);
	if (status)
	{
		RpcBindingFree(&bindingHandle);
		ThrowException("[RpcClient::ConnectToRpcServer] - RpcEpResolveBinding failed", status);
	}

	return bindingHandle;
}

void __RPC_FAR* __RPC_USER midl_user_allocate(size_t cBytes)
{
	return((void __RPC_FAR*) malloc(cBytes));
}

void __RPC_USER midl_user_free(void __RPC_FAR* p)
{
	free(p);
}