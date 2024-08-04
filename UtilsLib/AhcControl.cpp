#include "AhcControl.h"

void CompatCachepSetFlags(wstring& FilePath, DWORD Flag)
{
	CompatCacheAutoExe compatCache;
	compatCache.Init(FilePath.c_str(), nullptr);
	DWORD buffer = Flag;
	AHC_SERVICE_DATA serviceData = {};
	serviceData.Update.FileHandle = compatCache.m_Handle;
	serviceData.Update.Data = &buffer;
	serviceData.Update.InfoClass = AhcInfoClassInstallDetect;
	serviceData.Update.ExeSignature = compatCache.m_UnicodeStr;
	serviceData.Update.DataSize = sizeof(buffer);
	NTSTATUS status = NtApphelpCacheControl(ApphelpCacheServiceUpdate, &serviceData);
	if (status)
		ThrowException("[CompatCachepSetFlags] - NtApphelpCacheControl failed", status);
}

DWORD CompatCachepGetFlags(wstring& FilePath)
{
	CompatCacheAutoExe compatCache;
	compatCache.Init(FilePath.c_str(), nullptr);
	DWORD buffer[3] = {};
	AHC_SERVICE_DATA serviceData = {};
	serviceData.Lookup.FileHandle = compatCache.m_Handle;
	serviceData.ParamsOut = buffer;
	serviceData.Lookup.InfoClass = AhcInfoClassInstallDetect;
	serviceData.Lookup.ExeSignature = compatCache.m_UnicodeStr;
	serviceData.ParamsOutSize = sizeof(buffer);
	NTSTATUS status = NtApphelpCacheControl(ApphelpCacheServiceLookup, &serviceData);
	if (status)
		ThrowException("[CompatCachepSetFlags] - NtApphelpCacheControl failed", status);
	return buffer[2];
}

void CompatCacheRemoveExeManual(wstring& FilePath)
{
	// An example for calling NtApphelpCacheControl manually, for research purposes
	CompatCacheAutoExe compatCache;
	compatCache.Init(FilePath.c_str(), nullptr);
	AHC_SERVICE_DATA serviceData = {};
	serviceData.Remove.FileHandle = compatCache.m_Handle;
	serviceData.Remove.ExeSignature = compatCache.m_UnicodeStr;
	constexpr auto infoClass = static_cast<AHC_INFO_CLASS>(
		AhcInfoClassMaskAllExeTypesForQueryResults |
		AhcInfoClassDxType |
		AhcInfoClassInstallDetect |
		AhcInfoClassSdbInstallerFlags |
		AhcInfoClassSdbFusionFlags |
		AhcInfoClassSdbRunlevelFlags |
		AhcInfoClassSdbSxsOverrideManifest |
		AhcInfoClassSdbQueryResult);
	serviceData.Remove.InfoClass = infoClass;
	NTSTATUS status = NtApphelpCacheControl(ApphelpCacheServiceRemove, &serviceData);
	if (status)
		ThrowException("[CompatCacheRemoveExeManual] - NtApphelpCacheControl failed", status);
}

void InitProcessData(HANDLE ProcessHandle, PSHIM_DATA ShimData)
{
	AHC_SERVICE_DATA serviceData = {};
	serviceData.InitData.ProcessHandle = ProcessHandle;
	serviceData.InitData.Data = ShimData;
	serviceData.InitData.DataSize = sizeof(SHIM_DATA); //0x000011C0
	NTSTATUS status = NtApphelpCacheControl(ApphelpCacheServiceInitProcessData, &serviceData);
	if (!NT_SUCCESS(status))
		ThrowException("[InitProcessData] NtApphelpCacheControl failed", status);
}