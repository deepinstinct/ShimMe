#include "ScheduledTasks.h"

void RunScheduledTask(wstring& TaskName, wstring& TaskFolder)
{
	HRESULT hr = S_OK;
	hr = CoInitialize(nullptr);
	if (FAILED(hr))
		ThrowException("CoInitialize failed", hr);

	ITaskService* taskService = nullptr;
	hr = CoCreateInstance(CLSID_TaskScheduler,
		nullptr,
		CLSCTX_INPROC_SERVER,
		IID_ITaskService,
		reinterpret_cast<void**>(&taskService));

	if (FAILED(hr))
	{
		CoUninitialize();
		ThrowException("CoCreateInstance for IID_ITaskService failed", hr);
	}

	hr = taskService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
	if (FAILED(hr))
	{
		taskService->Release();
		CoUninitialize();
		ThrowException("ITaskService::Connect failed", hr);
	}

	ITaskFolder* taskFolder = nullptr;

	hr = taskService->GetFolder(TaskFolder.data(), &taskFolder);
	taskService->Release();
	if (FAILED(hr))
	{
		CoUninitialize();
		ThrowException("ITaskService::GetFolder failed", hr);
	}

	IRegisteredTask* registeredTask = nullptr;
	hr = taskFolder->GetTask(TaskName.data(), &registeredTask);
	taskFolder->Release();
	if (FAILED(hr))
	{
		CoUninitialize();
		ThrowException("ITaskFolder::GetTask failed", hr);
	}

	IRunningTask* runningTask = nullptr;
	hr = registeredTask->Run(_variant_t(), &runningTask);
	registeredTask->Release();
	if (FAILED(hr))
	{
		CoUninitialize();
		ThrowException("IRegisteredTask::Run failed", hr);
	}

	runningTask->Release();
	CoUninitialize();
}

wstring GetScheduledTaskAction(wstring& TaskName, wstring& TaskFolder)
{
	HRESULT hr = S_OK;
	hr = CoInitialize(nullptr);
	if (FAILED(hr))
		ThrowException("CoInitialize failed", hr);

	ITaskService* taskService = nullptr;
	hr = CoCreateInstance(CLSID_TaskScheduler,
		nullptr,
		CLSCTX_INPROC_SERVER,
		IID_ITaskService,
		reinterpret_cast<void**>(&taskService));

	if (FAILED(hr))
	{
		CoUninitialize();
		ThrowException("CoCreateInstance for IID_ITaskService failed", hr);
	}

	hr = taskService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
	if (FAILED(hr))
	{
		taskService->Release();
		CoUninitialize();
		ThrowException("ITaskService::Connect failed", hr);
	}

	ITaskFolder* taskFolder = nullptr;

	hr = taskService->GetFolder(TaskFolder.data(), &taskFolder);
	taskService->Release();
	if (FAILED(hr))
	{
		CoUninitialize();
		ThrowException("ITaskService::GetFolder failed", hr);
	}

	IRegisteredTask* registeredTask = nullptr;
	hr = taskFolder->GetTask(TaskName.data(), &registeredTask);
	taskFolder->Release();
	if (FAILED(hr))
	{
		CoUninitialize();
		ThrowException("ITaskFolder::GetTask failed", hr);
	}

	ITaskDefinition* taskDefinition = nullptr;
	hr = registeredTask->get_Definition(&taskDefinition);
	registeredTask->Release();
	if (FAILED(hr))
	{
		CoUninitialize();
		ThrowException("IRegisteredTask::get_Definition failed", hr);
	}

	IActionCollection* actionCollection = nullptr;
	hr = taskDefinition->get_Actions(&actionCollection);
	taskDefinition->Release();
	if (FAILED(hr))
	{
		CoUninitialize();
		ThrowException("ITaskDefinition::get_Actions failed", hr);
	}

	IAction* action = nullptr;
	hr = actionCollection->get_Item(1, &action);
	actionCollection->Release();
	if (FAILED(hr))
	{
		CoUninitialize();
		ThrowException("IActionCollection::get_Item failed", hr);
	}

	IExecAction* execAction = nullptr;
	hr = action->QueryInterface(IID_IExecAction, reinterpret_cast<void**>(&execAction));
	if (FAILED(hr))
	{
		CoUninitialize();
		ThrowException("IAction::QueryInterface failed", hr);
	}

	BSTR actionPath = nullptr;
	hr = execAction->get_Path(&actionPath);
	action->Release();
	if (FAILED(hr))
	{
		CoUninitialize();
		ThrowException("IExecAction::get_Path failed", hr);
	}

	wstring path(actionPath);
	SysFreeString(actionPath);
	CoUninitialize();
	return path;
}