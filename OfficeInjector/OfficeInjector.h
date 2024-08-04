#pragma once
#include "../Injected/Injected.h"
#include "resource.h"
#include "AppVIsvClient.h"
#include "FileOpLock.h"
#include "ScheduledTasks.h"
#include <psapi.h>
#include <sddl.h>

typedef NTSTATUS(NTAPI* BaseUpdateAppcompatCacheWorker_func)
(
	IN  const wchar_t* Filename,
	IN  HANDLE    FileHandle,
	IN  BOOL    Remove
);