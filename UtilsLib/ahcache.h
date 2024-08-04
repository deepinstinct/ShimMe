/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

Module Name:

    ahcache.h

Abstract:

    This file defines the user mode visible portions of the application
    compatibility cache support.

    The cache associates meta-data with a file such that it can be retrieved
    very rapidly.

    The cache serves several customers:

      1. A fast lookup for the result of database queries performed by the
         compat infrastructure. This is used to fast path certain
         performance-critical APIs, e.g. CreateProcess, CheckElevation, etc.

      2. A front end to the AppCompat Lookup service. Since the fixes applied
         to an application govern how it runs, they have security implications.
         Therefore, administrator rights are required to update the cache
         state. Since most callers are restricted, a request is made to a
         service to do the update. The service is communicated with via ALPC.

      3. A fast lookup for Fusion/Side-by-side state. Several Fusion functions
         are required during process launch and are costly, e.g. locating the
         manifests.

      4. Application Impact Telemetry state. The telemetry shim uses the cache
         to determine if an application is a candidate for telemetry, i.e. if
         it is opt-ed in, as well as using it to mark when the shim has already
         run (so that it doesn't get sampled again).

      5. An event is sent whenever a launched application is shimmed. This
         event should only be sent once. The cache allows tracking of this.

    The cache is persistent across reboots. It is stored in the registry.

Author:

    VadimB      created

Revision History:

    Qian Zhou (qianzh) 08-01-2009   refactored

--*/
#include "ntddk.h"
#ifndef _AHCACHE_H_
#define _AHCACHE_H_

#ifdef __cplusplus
extern "C" {
#endif

    typedef enum _AHC_INFO_CLASS {
        AhcInfoClassSdbQueryResult = 0x1,
        AhcInfoClassSdbSxsOverrideManifest = 0x2,
        AhcInfoClassSdbRunlevelFlags = 0x4,
        AhcInfoClassSdbFusionFlags = 0x8,
        AhcInfoClassSdbInstallerFlags = 0x10,
        AhcInfoClassFusionFlags = 0x20,
        AhcInfoClassTelemetryFlags = 0x40,
        AhcInfoClassInstallDetect = 0x80,
        AhcInfoClassRacEventSent = 0x100,
        AhcInfoClassIsSystemFile = 0x200,
        AhcInfoClassMonitoringFlags = 0x400,
        AhcInfoClassExeType = 0x800,
        AhcInfoClassDxType = 0x1000,
        AhcInfoClassExeTypesArray = 0x2000,
        AhcInfoClassMaskAllExeTypesForQueryResults = 0xFFFF0000,
    } AHC_INFO_CLASS, * PAHC_INFO_CLASS;

#define AHC_INFO_CLASS_FILTER_ON_FILETIME_CHANGE            \
    (AHC_INFO_CLASS)(AhcInfoClassSdbQueryResult |           \
                     AhcInfoClassSdbSxsOverrideManifest |   \
                     AhcInfoClassSdbRunlevelFlags |         \
                     AhcInfoClassSdbFusionFlags |           \
                     AhcInfoClassSdbInstallerFlags |        \
                     AhcInfoClassFusionFlags |              \
                     AhcInfoClassRacEventSent)

#define AHC_INFO_CLASS_FILTER_ON_SDB_CHANGE                 \
    (AHC_INFO_CLASS)(AhcInfoClassSdbQueryResult |           \
                     AhcInfoClassSdbSxsOverrideManifest |   \
                     AhcInfoClassSdbRunlevelFlags |         \
                     AhcInfoClassSdbFusionFlags |           \
                     AhcInfoClassSdbInstallerFlags |        \
                     AhcInfoClassInstallDetect)

#define AHC_INFO_CLASS_ALL                                  \
    (AHC_INFO_CLASS)(AhcInfoClassSdbQueryResult |           \
                     AhcInfoClassSdbSxsOverrideManifest |   \
                     AhcInfoClassSdbRunlevelFlags |         \
                     AhcInfoClassSdbFusionFlags |           \
                     AhcInfoClassSdbInstallerFlags |        \
                     AhcInfoClassFusionFlags |              \
                     AhcInfoClassTelemetryFlags |           \
                     AhcInfoClassInstallDetect |            \
                     AhcInfoClassRacEventSent |             \
                     AhcInfoClassIsSystemFile |             \
                     AhcInfoClassMonitoringFlags |          \
                     AhcInfoClassExeType)

#define AHC_INFO_CLASS_INTERNALLY_COMPUTED                  \
    (AHC_INFO_CLASS)(AhcInfoClassSdbQueryResult |           \
                     AhcInfoClassSdbSxsOverrideManifest |   \
                     AhcInfoClassSdbRunlevelFlags |         \
                     AhcInfoClassSdbFusionFlags |           \
                     AhcInfoClassSdbInstallerFlags |        \
                     AhcInfoClassTelemetryFlags |           \
                     AhcInfoClassIsSystemFile |             \
                     AhcInfoClassMonitoringFlags |          \
                     AhcInfoClassExeType)

#define AHC_INFO_CLASS_SAFE_FOR_UNPRIVILEGED_UPDATE         \
    (AHC_INFO_CLASS)(AhcInfoClassInstallDetect |            \
                     AhcInfoClassRacEventSent |             \
                     AhcInfoClassTelemetryFlags |           \
                     AhcInfoClassMonitoringFlags)

//
// AHC_HINT_* flags are used in the HintFlags variable.
//

#define AHC_HINT_FORCE_BYPASS                           0x00000001
#define AHC_HINT_REMOVABLE_MEDIA                        0x00000002
#define AHC_HINT_TEMPORARY_DIRECTORY                    0x00000004
#define AHC_HINT_USER_PERM_LAYER                        0x00000008
#define AHC_HINT_CREATE_PROCESS                         0x00000010
#define AHC_HINT_NATIVE_EXE                             0x00000020

#define SHIM_CACHE_MAIN_DATABASE_PATH32                 L"\\AppPatch\\sysmain.sdb"
#define SHIM_CACHE_MAIN_DATABASE_PATH64                 L"\\AppPatch\\AppPatch64\\sysmain.sdb"

//
// Flag definitions for various flag-type information in cache.
//

#define AHC_CACHE_FLAG_MONITORING_IS_CANDIDATE          0x00000001 // Candidate for monitoring.
#define AHC_CACHE_FLAG_MONITORING_IS_COMPLETE           0x00000002 // Monitoring has completed.
#define AHC_CACHE_FLAG_MONITORING_VALID_MASK            (AHC_CACHE_FLAG_MONITORING_IS_CANDIDATE | \
                                                         AHC_CACHE_FLAG_MONITORING_IS_COMPLETE)

#define AHC_CACHE_FLAG_TELEMETRY_IS_CANDIDATE           0x00000001 // Candidate for telemetry.
#define AHC_CACHE_FLAG_TELEMETRY_HAS_SAMPLED            0x00000002 // Telemetry has run.
#define AHC_CACHE_FLAG_TELEMETRY_VALID_MASK             (AHC_CACHE_FLAG_TELEMETRY_IS_CANDIDATE | \
                                                         AHC_CACHE_FLAG_TELEMETRY_HAS_SAMPLED)

#define AHC_CACHE_FLAG_FUSION_HASDOTLOCAL               0x00000001 // Dot local file exists.
#define AHC_CACHE_FLAG_FUSION_HASMANIFESTFILE           0x00000002 // Fusion manifest exists.
#define AHC_CACHE_FLAG_FUSION_HASMANIFESTRESOURCE       0x00000004 // Fusion manifest resource exists.
#define AHC_CACHE_FLAG_FUSION_VALID_MASK                (AHC_CACHE_FLAG_FUSION_HASDOTLOCAL | \
                                                         AHC_CACHE_FLAG_FUSION_HASMANIFESTFILE | \
                                                         AHC_CACHE_FLAG_FUSION_HASMANIFESTRESOURCE)

#define AHC_CACHE_FLAG_RAC_EVENTSENT                    0x00000001 // Rac event has been sent.
#define AHC_CACHE_FLAG_RAC_VALID_MASK                   (AHC_CACHE_FLAG_RAC_EVENTSENT)

#define AHC_CACHE_FLAG_INSTALLDETECT_CLAIMED            0x00000001 // InstallDetect claimed.
#define AHC_CACHE_FLAG_INSTALLDETECT_VALID_MASK         (AHC_CACHE_FLAG_RAC_EVENTSENT)

//
// Cache structures and APIs.
//

    typedef struct _AHC_SERVICE_LOOKUP {
        AHC_INFO_CLASS InfoClass;                   // Information to lookup.
        UINT HintFlags;                             // Hint flags about cache query.
        UNICODE_STRING PackageAlias;                // Aliased package moniker in a packed string.
        HANDLE FileHandle;                          // User space handle to file.
        HANDLE ProcessHandle;                       // User space process handle.
        USHORT ExeType;                             // Executable bitness.
        USHORT Padding;                             // Padding to even USHORTs.
        UNICODE_STRING ExeSignature;                // Executable file signature.
        PCZZWSTR Environment;                       // Environment block.
        UINT EnvironmentSize;                       // Size of environment block in bytes.
        UNICODE_STRING ProcessCommandLine;
    } AHC_SERVICE_LOOKUP, * PAHC_SERVICE_LOOKUP;

    typedef struct _AHC_SERVICE_UPDATE {
        AHC_INFO_CLASS InfoClass;
        UNICODE_STRING PackageAlias;
        HANDLE FileHandle;
        UNICODE_STRING ExeSignature;
        PVOID Data;
        ULONG DataSize;
    } AHC_SERVICE_UPDATE, * PAHC_SERVICE_UPDATE;

    typedef struct _AHC_SERVICE_DATACACHE {
        HANDLE FileHandle;                          // User space handle to file.
        USHORT ExeType;                             // Executable bitness.
        USHORT Padding;                             // Padding to even USHORTs.
        UINT HintFlags;                             // Metadata flags about cache query.
        HANDLE ProcessHandle;                       // User space process handle.
        UNICODE_STRING FileName;                    // Executable file name.
        UNICODE_STRING Environment;                 // Environment block.
        UNICODE_STRING PackageAlias;                // Aliased package moniker in a packed string.
        ULONG CustomDataSize;                       // Size of the custom data to cache.
        PBYTE CustomData;                           // Pointer to the custom data.
    } AHC_SERVICE_DATACACHE, * PAHC_SERVICE_DATACACHE;

    typedef struct _AHC_SERVICE_LOOKUP_CDB {
        UNICODE_STRING Name;
    } AHC_SERVICE_LOOKUP_CDB, * PAHC_SERVICE_LOOKUP_CDB;

    typedef struct _AHC_SERVICE_CLEAR {
        AHC_INFO_CLASS InfoClass;
    } AHC_SERVICE_CLEAR, * PAHC_SERVICE_CLEAR;

    typedef struct _AHC_SERVICE_REMOVE {
        AHC_INFO_CLASS InfoClass;
        UNICODE_STRING PackageAlias;
        HANDLE FileHandle;
        UNICODE_STRING ExeSignature;
    } AHC_SERVICE_REMOVE, * PAHC_SERVICE_REMOVE;

    typedef struct _AHC_SERVICE_HWID_QUERY {
        BOOLEAN QueryResult;                        // Query result
        UNICODE_STRING HwId;                        // Query HwId; can contain wildcards
    } AHC_SERVICE_HWID_QUERY, * PAHC_SERVICE_HWID_QUERY;

#define SHIMDATA_PLATFORM_X64 0x8664
#define SHIMDATA_PLATFORM_X86 0x14c
#define SHIMDATA_MAGIC 0xAC0DEDAB

typedef struct _SHIM_DATA
{
    DWORD Unknown1[130];
    DWORD Size;         // Always 0x11C0
    DWORD Magic;        // Always 0xAC0DEDAB
    DWORD UpBit;        // Always 0x1
    DWORD ExeType;      // 0x8664 (64-bit), 0x14c (32-bit)
    DWORD ExeTag;       // shim "id", pointing to the address in the sdb
    DWORD Unknown2[20];
    DWORD Tag2;         // exe or layer tag (optional)
    DWORD Tag3;         // exe or layer tag (optional)
    DWORD Unknown3[19];
    DWORD ExeCount;     // number of shims to apply
    DWORD LayerCount;   // number of layers to apply
    DWORD Unknown4[958];
} SHIM_DATA, * PSHIM_DATA;

typedef struct _AHC_SERVICE_INIT_PROCESS_DATA
{
    HANDLE ProcessHandle;
    PSHIM_DATA Data;
    ULONG DataSize;
} AHC_SERVICE_INIT_PROCESS_DATA, * PAHC_SERVICE_INIT_PROCESS_DATA;

//
// Statistics.
//

    typedef struct _AHC_MAIN_STATISTICS {
        ULONG Lookup;                               // Count of lookup calls.
        ULONG Remove;                               // Count of remove calls.
        ULONG Update;                               // Count of update calls.
        ULONG Clear;                                // Count of clear calls.
        ULONG SnapStatistics;                       // Count of snap statistics calls.
        ULONG SnapCache;                            // Count of snap store calls.
    } AHC_MAIN_STATISTICS, * PAHC_MAIN_STATISTICS;

    typedef struct _AHC_STORE_STATISTICS {
        ULONG LookupHits;                           // Count of lookup hits.
        ULONG LookupMisses;                         // Count of lookup misses.
        ULONG Inserted;                             // Count of inserted.
        ULONG Replaced;                             // Count of replaced.
        ULONG Updated;                              // Count of updates.
    } AHC_STORE_STATISTICS, * PAHC_STORE_STATISTICS;

    typedef struct _AHC_STATISTICS {
        ULONG Size;                                 // Size of the structure.
        AHC_MAIN_STATISTICS Main;                   // Main statistics.
        AHC_STORE_STATISTICS Store;                 // Store statistics.
    } AHC_STATISTICS, * PAHC_STATISTICS;

    typedef struct _AHC_SERVICE_DATAQUERY {
        AHC_STATISTICS Stats;                       // Statistics.
        ULONG DataSize;                             // Size of data.
        PBYTE Data;                                 // Data.
    } AHC_SERVICE_DATAQUERY, * PAHC_SERVICE_DATAQUERY;

    typedef struct _AHC_SERVICE_DATA {
        AHC_SERVICE_LOOKUP Lookup;                  // Lookup EXE/Package.
        AHC_SERVICE_UPDATE Update;                  // Updating flags for a given exe/package.
        AHC_SERVICE_DATACACHE Cache;                // For cache operations.
        AHC_SERVICE_LOOKUP_CDB LookupCdb;           // Lookup cdb.
        AHC_SERVICE_CLEAR Clear;                    // Clear flags for all exes/packages.
        AHC_SERVICE_REMOVE Remove;                  // Remove EXE/Package.
        AHC_SERVICE_HWID_QUERY HwIdQuery;           // For HWID cache queries.
        AHC_SERVICE_INIT_PROCESS_DATA InitData;
        NTSTATUS DriverStatus;                      // Receive the status from the cache driver. Set error code in IoStatus block causes driver verifier violation.
        PVOID ParamsOut;                            // Parameters out data.
        ULONG ParamsOutSize;                        // Parameters out size.
    } AHC_SERVICE_DATA, * PAHC_SERVICE_DATA;

    typedef enum _AHC_SERVICE_CLASS
    {
        ApphelpCacheServiceLookup = 0x0,
        ApphelpCacheServiceRemove = 0x1,
        ApphelpCacheServiceUpdate = 0x2,
        ApphelpCacheServiceClear = 0x3,
        ApphelpCacheServiceSnapStatistics = 0x4,
        ApphelpCacheServiceSnapCache = 0x5,
        ApphelpCacheServiceLookupCdb = 0x6,
        ApphelpCacheServiceRefreshCdb = 0x7,
        ApphelpCacheServiceMapQuirks = 0x8,
        ApphelpCacheServiceHwIdQuery = 0x9,
        ApphelpCacheServiceInitProcessData = 0xA,
        ApphelpCacheServiceLookupProcess = 0xB,
        ApphelpCacheServiceMax = 0xC
    } AHC_SERVICE_CLASS;

    NTSYSAPI
        NTSTATUS
        NTAPI
        NtApphelpCacheControl(
            IN AHC_SERVICE_CLASS ServiceClass,
            IN PAHC_SERVICE_DATA ServiceData
        );

#ifdef __cplusplus
}
#endif

#endif //_AHCACHE_H_