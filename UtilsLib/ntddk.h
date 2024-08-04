#pragma once

template <class T>
struct UNICODE_STRING_T
{
    union
    {
        struct
        {
            USHORT Length;
            USHORT MaximumLength;
        } u;
        T dummy;
    };
    T _Buffer;
};

template <class T, class NGF, int A>
struct _PEB_T
{
    union
    {
        struct
        {
            char InheritedAddressSpace;
            char ReadImageFileExecOptions;
            char BeingDebugged;
            char _SYSTEM_DEPENDENT_01;
        } u;
        T dummy01;
    };
    T Mutant;
    T ImageBaseAddress;
    T Ldr;
    T ProcessParameters;
    T SubSystemData;
    T ProcessHeap;
    T FastPebLock;
    T _SYSTEM_DEPENDENT_02;
    T _SYSTEM_DEPENDENT_03;
    T _SYSTEM_DEPENDENT_04;
    union
    {
        T KernelCallbackTable;
        T UserSharedInfoPtr;
    };
    ULONG SystemReserved;
    ULONG _SYSTEM_DEPENDENT_05;
    T _SYSTEM_DEPENDENT_06;
    T TlsExpansionCounter;
    T TlsBitmap;
    ULONG TlsBitmapBits[2];
    T ReadOnlySharedMemoryBase;
    T _SYSTEM_DEPENDENT_07;
    T ReadOnlyStaticServerData;
    T AnsiCodePageData;
    T OemCodePageData;
    T UnicodeCaseTableData;
    ULONG NumberOfProcessors;
    union
    {
        ULONG NtGlobalFlag;
        NGF dummy02;
    };
    LARGE_INTEGER CriticalSectionTimeout;
    T HeapSegmentReserve;
    T HeapSegmentCommit;
    T HeapDeCommitTotalFreeThreshold;
    T HeapDeCommitFreeBlockThreshold;
    ULONG NumberOfHeaps;
    ULONG MaximumNumberOfHeaps;
    T ProcessHeaps;
    T GdiSharedHandleTable;
    T ProcessStarterHelper;
    T GdiDCAttributeList;
    T LoaderLock;
    ULONG OSMajorVersion;
    ULONG OSMinorVersion;
    USHORT OSBuildNumber;
    USHORT OSCSDVersion;
    ULONG OSPlatformId;
    ULONG ImageSubsystem;
    ULONG ImageSubsystemMajorVersion;
    T ImageSubsystemMinorVersion;
    union
    {
        T ImageProcessAffinityMask;
        T ActiveProcessAffinityMask;
    };
    T GdiHandleBuffer[A];
    T PostProcessInitRoutine;
    T TlsExpansionBitmap;
    ULONG TlsExpansionBitmapBits[32];
    T SessionId;
    ULARGE_INTEGER AppCompatFlags;
    ULARGE_INTEGER AppCompatFlagsUser;
    T pShimData;
    T AppCompatInfo;
    UNICODE_STRING_T<T> CSDVersion;
    T ActivationContextData;
    T ProcessAssemblyStorageMap;
    T SystemDefaultActivationContextData;
    T SystemAssemblyStorageMap;
    T MinimumStackCommit;
};

typedef _PEB_T<ULONG, DWORD64, 34> PEB32;
typedef _PEB_T<DWORD64, ULONG, 30> PEB64;

#ifndef __NTDLL_H__
#define __NTDLL_H__

#ifdef __cplusplus
extern "C" {
#endif
#include <Windows.h>

#ifdef _NTDDK_
#error This header cannot be compiled together with NTDDK
#endif


#ifndef _NTDLL_SELF_                            // Auto-insert the library
#pragma comment(lib, "Ntdll.lib")
#endif

#pragma warning(disable: 4201)                  // nonstandard extension used : nameless struct/union

#pragma warning(push)
#pragma warning(disable:4005)
#include <ntstatus.h>
#pragma warning(pop)

    //------------------------------------------------------------------------------
    // Defines for NTSTATUS

    typedef long NTSTATUS;

#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)
#endif

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS       ((NTSTATUS)0x00000000L)
#endif

#ifndef STATUS_UNSUCCESSFUL
#define STATUS_UNSUCCESSFUL ((NTSTATUS)0xC0000001L)
#endif

#ifndef ASSERT
#ifdef _DEBUG
#define ASSERT(x) assert(x)
#else
#define ASSERT(x) /* x */
#endif
#endif

    typedef enum _RTL_PATH_TYPE
    {
        RtlPathTypeUnknown,
        RtlPathTypeUncAbsolute,
        RtlPathTypeDriveAbsolute,
        RtlPathTypeDriveRelative,
        RtlPathTypeRooted,
        RtlPathTypeRelative,
        RtlPathTypeLocalDevice,
        RtlPathTypeRootLocalDevice,
    } RTL_PATH_TYPE;

    typedef struct _UNICODE_STRING
    {
        USHORT Length;
        USHORT MaximumLength;
        PWSTR  Buffer;

    } UNICODE_STRING, * PUNICODE_STRING;

#ifndef OBJ_CASE_INSENSITIVE
#define OBJ_INHERIT             0x00000002L
#define OBJ_PERMANENT           0x00000010L
#define OBJ_EXCLUSIVE           0x00000020L
#define OBJ_CASE_INSENSITIVE    0x00000040L
#define OBJ_OPENIF              0x00000080L
#define OBJ_OPENLINK            0x00000100L
#define OBJ_KERNEL_HANDLE       0x00000200L
#define OBJ_FORCE_ACCESS_CHECK  0x00000400L
#define OBJ_VALID_ATTRIBUTES    0x000007F2L

    typedef struct _OBJECT_ATTRIBUTES {
        ULONG           Length;
        HANDLE          RootDirectory;
        PUNICODE_STRING ObjectName;
        ULONG           Attributes;
        PVOID           SecurityDescriptor;
        PVOID           SecurityQualityOfService;
    } OBJECT_ATTRIBUTES, * POBJECT_ATTRIBUTES;
#endif

    typedef struct _CURDIR
    {
        UNICODE_STRING DosPath;
        HANDLE Handle;
    } CURDIR, * PCURDIR;

    typedef struct _RTL_DRIVE_LETTER_CURDIR
    {
        USHORT Flags;
        USHORT Length;
        ULONG TimeStamp;
        UNICODE_STRING DosPath;
    } RTL_DRIVE_LETTER_CURDIR, * PRTL_DRIVE_LETTER_CURDIR;

#define RTL_MAX_DRIVE_LETTERS 32

    typedef struct _RTL_USER_PROCESS_PARAMETERS
    {
        ULONG MaximumLength;
        ULONG Length;

        ULONG Flags;
        ULONG DebugFlags;

        HANDLE ConsoleHandle;
        ULONG ConsoleFlags;
        HANDLE StandardInput;
        HANDLE StandardOutput;
        HANDLE StandardError;

        CURDIR CurrentDirectory;
        UNICODE_STRING DllPath;
        UNICODE_STRING ImagePathName;
        UNICODE_STRING CommandLine;
        PWCHAR Environment;

        ULONG StartingX;
        ULONG StartingY;
        ULONG CountX;
        ULONG CountY;
        ULONG CountCharsX;
        ULONG CountCharsY;
        ULONG FillAttribute;

        ULONG WindowFlags;
        ULONG ShowWindowFlags;
        UNICODE_STRING WindowTitle;
        UNICODE_STRING DesktopInfo;
        UNICODE_STRING ShellInfo;
        UNICODE_STRING RuntimeData;
        RTL_DRIVE_LETTER_CURDIR CurrentDirectories[RTL_MAX_DRIVE_LETTERS];

        ULONG_PTR EnvironmentSize;
        ULONG_PTR EnvironmentVersion;
        PVOID PackageDependencyData;
        ULONG ProcessGroupId;
        ULONG LoaderThreads;
    } RTL_USER_PROCESS_PARAMETERS, * PRTL_USER_PROCESS_PARAMETERS;

    typedef enum _PS_CREATE_STATE
    {
        PsCreateInitialState,
        PsCreateFailOnFileOpen,
        PsCreateFailOnSectionCreate,
        PsCreateFailExeFormat,
        PsCreateFailMachineMismatch,
        PsCreateFailExeName, // Debugger specified
        PsCreateSuccess,
        PsCreateMaximumStates
    } PS_CREATE_STATE;

    typedef struct _PS_CREATE_INFO
    {
        SIZE_T Size;
        PS_CREATE_STATE State;
        union
        {
            // PsCreateInitialState
            struct
            {
                union
                {
                    ULONG InitFlags;
                    struct
                    {
                        UCHAR WriteOutputOnExit : 1;
                        UCHAR DetectManifest : 1;
                        UCHAR IFEOSkipDebugger : 1;
                        UCHAR IFEODoNotPropagateKeyState : 1;
                        UCHAR SpareBits1 : 4;
                        UCHAR SpareBits2 : 8;
                        USHORT ProhibitedImageCharacteristics : 16;
                    } s1;
                } u1;
                ACCESS_MASK AdditionalFileAccess;
            } InitState;

            // PsCreateFailOnSectionCreate
            struct
            {
                HANDLE FileHandle;
            } FailSection;

            // PsCreateFailExeFormat
            struct
            {
                USHORT DllCharacteristics;
            } ExeFormat;

            // PsCreateFailExeName
            struct
            {
                HANDLE IFEOKey;
            } ExeName;

            // PsCreateSuccess
            struct
            {
                union
                {
                    ULONG OutputFlags;
                    struct
                    {
                        UCHAR ProtectedProcess : 1;
                        UCHAR AddressSpaceOverride : 1;
                        UCHAR DevOverrideEnabled : 1; // From Image File Execution Options
                        UCHAR ManifestDetected : 1;
                        UCHAR ProtectedProcessLight : 1;
                        UCHAR SpareBits1 : 3;
                        UCHAR SpareBits2 : 8;
                        USHORT SpareBits3 : 16;
                    } s2;
                } u2;
                HANDLE FileHandle;
                HANDLE SectionHandle;
                ULONGLONG UserProcessParametersNative;
                ULONG UserProcessParametersWow64;
                ULONG CurrentParameterFlags;
                ULONGLONG PebAddressNative;
                ULONG PebAddressWow64;
                ULONGLONG ManifestAddress;
                ULONG ManifestSize;
            } SuccessState;
        };
    } PS_CREATE_INFO, * PPS_CREATE_INFO;

    typedef struct _PS_ATTRIBUTE
    {
        ULONG_PTR Attribute;                // PROC_THREAD_ATTRIBUTE_XXX | PROC_THREAD_ATTRIBUTE_XXX modifiers, see ProcThreadAttributeValue macro and Windows Internals 6 (372)
        SIZE_T Size;                        // Size of Value or *ValuePtr
        union
        {
            ULONG_PTR Value;                // Reserve 8 bytes for data (such as a Handle or a data pointer)
            PVOID ValuePtr;                 // data pointer
        };
        PSIZE_T ReturnLength;               // Either 0 or specifies size of data returned to caller via "ValuePtr"
    } PS_ATTRIBUTE, * PPS_ATTRIBUTE;

    typedef struct _PS_ATTRIBUTE_LIST
    {
        SIZE_T TotalLength;                 // sizeof(PS_ATTRIBUTE_LIST)
        PS_ATTRIBUTE Attributes[1];         // Depends on how many attribute entries should be supplied to NtCreateUserProcess
    } PS_ATTRIBUTE_LIST, * PPS_ATTRIBUTE_LIST;

#define PS_ATTRIBUTE_NUMBER_MASK    0x0000ffff
#define PS_ATTRIBUTE_THREAD         0x00010000 // Attribute may be used with thread creation
#define PS_ATTRIBUTE_INPUT          0x00020000 // Attribute is input only
#define PS_ATTRIBUTE_ADDITIVE       0x00040000 // Attribute may be "accumulated", e.g. bitmasks, counters, etc.

    typedef enum _PS_ATTRIBUTE_NUM
    {
        PsAttributeParentProcess,                   // in HANDLE
        PsAttributeDebugPort,                       // in HANDLE
        PsAttributeToken,                           // in HANDLE
        PsAttributeClientId,                        // out PCLIENT_ID
        PsAttributeTebAddress,                      // out PTEB
        PsAttributeImageName,                       // in PWSTR
        PsAttributeImageInfo,                       // out PSECTION_IMAGE_INFORMATION
        PsAttributeMemoryReserve,                   // in PPS_MEMORY_RESERVE
        PsAttributePriorityClass,                   // in UCHAR
        PsAttributeErrorMode,                       // in ULONG
        PsAttributeStdHandleInfo,                   // in PPS_STD_HANDLE_INFO
        PsAttributeHandleList,                      // in PHANDLE
        PsAttributeGroupAffinity,                   // in PGROUP_AFFINITY
        PsAttributePreferredNode,                   // in PUSHORT
        PsAttributeIdealProcessor,                  // in PPROCESSOR_NUMBER
        PsAttributeUmsThread,                       // see MSDN UpdateProceThreadAttributeList (CreateProcessW) - in PUMS_CREATE_THREAD_ATTRIBUTES
        PsAttributeMitigationOptions,               // in UCHAR
        PsAttributeProtectionLevel,                 // in ULONG
        PsAttributeSecureProcess,                   // since THRESHOLD (Virtual Secure Mode, Device Guard)
        PsAttributeJobList,
        PsAttributeChildProcessPolicy,              // since THRESHOLD2
        PsAttributeAllApplicationPackagesPolicy,    // since REDSTONE
        PsAttributeWin32kFilter,
        PsAttributeSafeOpenPromptOriginClaim,
        PsAttributeBnoIsolation,
        PsAttributeDesktopAppPolicy,
        PsAttributeMax
    } PS_ATTRIBUTE_NUM;

#define PsAttributeValue(Number, Thread, Input, Additive) \
    (((Number) & PS_ATTRIBUTE_NUMBER_MASK) | \
    ((Thread) ? PS_ATTRIBUTE_THREAD : 0) | \
    ((Input) ? PS_ATTRIBUTE_INPUT : 0) | \
    ((Additive) ? PS_ATTRIBUTE_ADDITIVE : 0))

#define PS_ATTRIBUTE_PARENT_PROCESS \
    PsAttributeValue(PsAttributeParentProcess, FALSE, TRUE, TRUE) // 0x60000
#define PS_ATTRIBUTE_DEBUG_PORT \
    PsAttributeValue(PsAttributeDebugPort, FALSE, TRUE, TRUE) // 0x60001
#define PS_ATTRIBUTE_TOKEN \
    PsAttributeValue(PsAttributeToken, FALSE, TRUE, TRUE) // 0x60002
#define PS_ATTRIBUTE_CLIENT_ID \
    PsAttributeValue(PsAttributeClientId, TRUE, FALSE, FALSE) // 0x10003
#define PS_ATTRIBUTE_TEB_ADDRESS \
    PsAttributeValue(PsAttributeTebAddress, TRUE, FALSE, FALSE) // 0x10004
#define PS_ATTRIBUTE_IMAGE_NAME \
    PsAttributeValue(PsAttributeImageName, FALSE, TRUE, FALSE) // 0x20005
#define PS_ATTRIBUTE_IMAGE_INFO \
    PsAttributeValue(PsAttributeImageInfo, FALSE, FALSE, FALSE) // 0x6
#define PS_ATTRIBUTE_MEMORY_RESERVE \
    PsAttributeValue(PsAttributeMemoryReserve, FALSE, TRUE, FALSE) // 0x20007
#define PS_ATTRIBUTE_PRIORITY_CLASS \
    PsAttributeValue(PsAttributePriorityClass, FALSE, TRUE, FALSE) // 0x20008
#define PS_ATTRIBUTE_ERROR_MODE \
    PsAttributeValue(PsAttributeErrorMode, FALSE, TRUE, FALSE) // 0x20009
#define PS_ATTRIBUTE_STD_HANDLE_INFO \
    PsAttributeValue(PsAttributeStdHandleInfo, FALSE, TRUE, FALSE) // 0x2000A
#define PS_ATTRIBUTE_HANDLE_LIST \
    PsAttributeValue(PsAttributeHandleList, FALSE, TRUE, FALSE) // 0x2000B
#define PS_ATTRIBUTE_GROUP_AFFINITY \
    PsAttributeValue(PsAttributeGroupAffinity, TRUE, TRUE, FALSE) // 0x2000C
#define PS_ATTRIBUTE_PREFERRED_NODE \
    PsAttributeValue(PsAttributePreferredNode, FALSE, TRUE, FALSE) // 0x2000D
#define PS_ATTRIBUTE_IDEAL_PROCESSOR \
    PsAttributeValue(PsAttributeIdealProcessor, TRUE, TRUE, FALSE) // 0x2000E
#define PS_ATTRIBUTE_MITIGATION_OPTIONS \
    PsAttributeValue(PsAttributeMitigationOptions, FALSE, TRUE, TRUE) // 0x60010
#define PS_ATTRIBUTE_PROTECTION_LEVEL \
    PsAttributeValue(PsAttributeProtectionLevel, FALSE, TRUE, FALSE) // 0x20011

#define PROCESS_CREATE_FLAGS_BREAKAWAY 0x00000001 // NtCreateProcessEx & NtCreateUserProcess
#define PROCESS_CREATE_FLAGS_NO_DEBUG_INHERIT 0x00000002 // NtCreateProcessEx & NtCreateUserProcess
#define PROCESS_CREATE_FLAGS_INHERIT_HANDLES 0x00000004 // NtCreateProcessEx & NtCreateUserProcess
#define PROCESS_CREATE_FLAGS_OVERRIDE_ADDRESS_SPACE 0x00000008 // NtCreateProcessEx only
#define PROCESS_CREATE_FLAGS_LARGE_PAGES 0x00000010 // NtCreateProcessEx only, requires SeLockMemory
#define PROCESS_CREATE_FLAGS_LARGE_PAGE_SYSTEM_DLL 0x00000020 // NtCreateProcessEx only, requires SeLockMemory
#define PROCESS_CREATE_FLAGS_PROTECTED_PROCESS 0x00000040 // NtCreateUserProcess only
#define PROCESS_CREATE_FLAGS_CREATE_SESSION 0x00000080 // NtCreateProcessEx & NtCreateUserProcess, requires SeLoadDriver
#define PROCESS_CREATE_FLAGS_INHERIT_FROM_PARENT 0x00000100 // NtCreateProcessEx & NtCreateUserProcess
#define PROCESS_CREATE_FLAGS_SUSPENDED 0x00000200 // NtCreateProcessEx & NtCreateUserProcess
#define PROCESS_CREATE_FLAGS_FORCE_BREAKAWAY 0x00000400 // NtCreateProcessEx & NtCreateUserProcess, requires SeTcb
#define PROCESS_CREATE_FLAGS_MINIMAL_PROCESS 0x00000800 // NtCreateProcessEx only
#define PROCESS_CREATE_FLAGS_RELEASE_SECTION 0x00001000 // NtCreateProcessEx & NtCreateUserProcess
#define PROCESS_CREATE_FLAGS_CLONE_MINIMAL 0x00002000 // NtCreateProcessEx only
#define PROCESS_CREATE_FLAGS_CLONE_MINIMAL_REDUCED_COMMIT 0x00004000 //
#define PROCESS_CREATE_FLAGS_AUXILIARY_PROCESS 0x00008000 // NtCreateProcessEx & NtCreateUserProcess, requires SeTcb
#define PROCESS_CREATE_FLAGS_CREATE_STORE 0x00020000 // NtCreateProcessEx only
#define PROCESS_CREATE_FLAGS_USE_PROTECTED_ENVIRONMENT 0x00040000 // NtCreateProcessEx & NtCreateUserProces

#define THREAD_CREATE_FLAGS_CREATE_SUSPENDED        0x00000001
#define THREAD_CREATE_FLAGS_SUPPRESS_DLLMAINS       0x00000002
#define THREAD_CREATE_FLAGS_HIDE_FROM_DEBUGGER      0x00000004
#define THREAD_CREATE_FLAGS_HAS_SECURITY_DESCRIPTOR 0x00000010 // ?
#define THREAD_CREATE_FLAGS_ACCESS_CHECK_IN_TARGET  0x00000020 // ?
#define THREAD_CREATE_FLAGS_INITIAL_THREAD          0x00000080

#define RTL_USER_PROCESS_PARAMETERS_NORMALIZED              0x01
#define RTL_USER_PROCESS_PARAMETERS_PROFILE_USER            0x02
#define RTL_USER_PROCESS_PARAMETERS_PROFILE_KERNEL          0x04
#define RTL_USER_PROCESS_PARAMETERS_PROFILE_SERVER          0x08
#define RTL_USER_PROCESS_PARAMETERS_RESERVE_1MB             0x20
#define RTL_USER_PROCESS_PARAMETERS_RESERVE_16MB            0x40
#define RTL_USER_PROCESS_PARAMETERS_CASE_SENSITIVE          0x80
#define RTL_USER_PROCESS_PARAMETERS_DISABLE_HEAP_DECOMMIT   0x100
#define RTL_USER_PROCESS_PARAMETERS_DLL_REDIRECTION_LOCAL   0x1000
#define RTL_USER_PROCESS_PARAMETERS_APP_MANIFEST_PRESENT    0x2000
#define RTL_USER_PROCESS_PARAMETERS_IMAGE_KEY_MISSING       0x4000
#define RTL_USER_PROCESS_PARAMETERS_NX_OPTIN                0x20000

#ifndef FILE_SUPERSEDE
#define FILE_SUPERSEDE                  0x00000000
#define FILE_OPEN                       0x00000001
#define FILE_CREATE                     0x00000002
#define FILE_OPEN_IF                    0x00000003
#define FILE_OVERWRITE                  0x00000004
#define FILE_OVERWRITE_IF               0x00000005
#define FILE_MAXIMUM_DISPOSITION        0x00000005
#endif  // File create flags

    // Define the create/open option flags
#ifndef FILE_DIRECTORY_FILE
#define FILE_DIRECTORY_FILE                     0x00000001
#define FILE_WRITE_THROUGH                      0x00000002
#define FILE_SEQUENTIAL_ONLY                    0x00000004
#define FILE_NO_INTERMEDIATE_BUFFERING          0x00000008
#define FILE_SYNCHRONOUS_IO_ALERT               0x00000010
#define FILE_SYNCHRONOUS_IO_NONALERT            0x00000020
#define FILE_NON_DIRECTORY_FILE                 0x00000040
#define FILE_CREATE_TREE_CONNECTION             0x00000080
#define FILE_COMPLETE_IF_OPLOCKED               0x00000100
#define FILE_NO_EA_KNOWLEDGE                    0x00000200
#define FILE_OPEN_FOR_RECOVERY                  0x00000400
#define FILE_RANDOM_ACCESS                      0x00000800
#define FILE_DELETE_ON_CLOSE                    0x00001000
#define FILE_OPEN_BY_FILE_ID                    0x00002000
#define FILE_OPEN_FOR_BACKUP_INTENT             0x00004000
#define FILE_NO_COMPRESSION                     0x00008000
#define FILE_RESERVE_OPFILTER                   0x00100000
#define FILE_OPEN_REPARSE_POINT                 0x00200000
#define FILE_OPEN_NO_RECALL                     0x00400000
#define FILE_OPEN_FOR_FREE_SPACE_QUERY          0x00800000
#endif // FILE_DIRECTORY_FILE

    typedef struct _IO_STATUS_BLOCK
    {
        union
        {
            NTSTATUS Status;
            PVOID Pointer;
        };

        ULONG_PTR Information;

    } IO_STATUS_BLOCK, * PIO_STATUS_BLOCK;

    typedef LONG KPRIORITY;

    typedef struct _PROCESS_BASIC_INFORMATION {
        NTSTATUS ExitStatus;
        ULONG_PTR PebBaseAddress;
        ULONG_PTR AffinityMask;
        KPRIORITY BasePriority;
        ULONG_PTR UniqueProcessId;
        ULONG_PTR InheritedFromUniqueProcessId;
    } PROCESS_BASIC_INFORMATION;

    typedef enum _PROCESSINFOCLASS {
        ProcessBasicInformation,
        ProcessQuotaLimits,
        ProcessIoCounters,
        ProcessVmCounters,
        ProcessTimes,
        ProcessBasePriority,
        ProcessRaisePriority,
        ProcessDebugPort,
        ProcessExceptionPort,
        ProcessAccessToken,
        ProcessLdtInformation,
        ProcessLdtSize,
        ProcessDefaultHardErrorMode,
        ProcessIoPortHandlers,          // Note: this is kernel mode only
        ProcessPooledUsageAndLimits,
        ProcessWorkingSetWatch,
        ProcessUserModeIOPL,
        ProcessEnableAlignmentFaultFixup,
        ProcessPriorityClass,
        ProcessWx86Information,
        ProcessHandleCount,
        ProcessAffinityMask,
        ProcessPriorityBoost,
        ProcessDeviceMap,
        ProcessSessionInformation,
        ProcessForegroundInformation,
        ProcessWow64Information,
        ProcessImageFileName,
        ProcessLUIDDeviceMapsEnabled,
        ProcessBreakOnTermination,
        ProcessDebugObjectHandle,
        ProcessDebugFlags,
        ProcessHandleTracing,
        MaxProcessInfoClass                             // MaxProcessInfoClass should always be the last enum
    } PROCESSINFOCLASS;

    NTSYSAPI
        NTSTATUS
        NTAPI
        RtlInitUnicodeString(
            PUNICODE_STRING DestinationString,
            PCWSTR SourceString
        );

    NTSYSAPI
        NTSTATUS
        NTAPI
        RtlInitUnicodeStringEx(
            PUNICODE_STRING DestinationString,
            PCWSTR SourceString
        );

    NTSYSAPI
        VOID
        NTAPI
        RtlFreeUnicodeString(
            IN  PUNICODE_STRING UnicodeString
        );

    NTSYSAPI
        NTSTATUS
        NTAPI
        RtlGetFullPathName_UEx(
            PCWSTR FileName,
            ULONG BufferLength,
            PWSTR Buffer,
            PWSTR* FilePart,
            DWORD* BytesNeeded
        );

    NTSYSAPI
        NTSTATUS
        NTAPI
        RtlDosPathNameToNtPathName_U_WithStatus(
            __in PCWSTR DosFileName,
            __out PUNICODE_STRING NtFileName,
            __deref_opt_out_opt PWSTR* FilePart,
            __reserved PVOID Reserved
        );

    NTSYSAPI
        NTSTATUS
        NTAPI
        NtCreateFile(
            OUT PHANDLE FileHandle,
            IN  ACCESS_MASK DesiredAccess,
            IN  POBJECT_ATTRIBUTES ObjectAttributes,
            OUT PIO_STATUS_BLOCK IoStatusBlock,
            IN  PLARGE_INTEGER AllocationSize,
            IN  ULONG FileAttributes,
            IN  ULONG ShareAccess,
            IN  ULONG CreateDisposition,
            IN  ULONG CreateOptions,
            IN  PVOID EaBuffer,
            IN  ULONG EaLength);

    NTSYSAPI
        NTSTATUS
        NTAPI
        NtQueryInformationProcess(
            IN            HANDLE           ProcessHandle,
            IN            PROCESSINFOCLASS ProcessInformationClass,
            OUT           PVOID            ProcessInformation,
            IN            ULONG            ProcessInformationLength,
            OUT OPTIONAL  PULONG           ReturnLength);

    NTSYSAPI
        NTSTATUS
        NTAPI
        NtCreateUserProcess(
            _Out_ PHANDLE ProcessHandle,
            _Out_ PHANDLE ThreadHandle,
            _In_ ACCESS_MASK ProcessDesiredAccess,
            _In_ ACCESS_MASK ThreadDesiredAccess,
            _In_opt_ POBJECT_ATTRIBUTES ProcessObjectAttributes,
            _In_opt_ POBJECT_ATTRIBUTES ThreadObjectAttributes,
            _In_ ULONG ProcessFlags,
            _In_ ULONG ThreadFlags,
            _In_ PRTL_USER_PROCESS_PARAMETERS ProcessParameters,
            _Inout_ PPS_CREATE_INFO CreateInfo,
            _In_ PPS_ATTRIBUTE_LIST AttributeList);

    NTSYSAPI
        NTSTATUS
        NTAPI
        RtlCreateProcessParametersEx(
            _Out_ PRTL_USER_PROCESS_PARAMETERS* pProcessParameters,
            _In_ PUNICODE_STRING ImagePathName,
            _In_opt_ PUNICODE_STRING DllPath,
            _In_opt_ PUNICODE_STRING CurrentDirectory,
            _In_opt_ PUNICODE_STRING CommandLine,
            _In_opt_ PVOID Environment,
            _In_opt_ PUNICODE_STRING WindowTitle,
            _In_opt_ PUNICODE_STRING DesktopInfo,
            _In_opt_ PUNICODE_STRING ShellInfo,
            _In_opt_ PUNICODE_STRING RuntimeData,
            _In_ ULONG Flags // Pass RTL_USER_PROCESS_PARAMETERS_NORMALIZED to keep parameters normalized
        );

    NTSYSAPI
        NTSTATUS
        NTAPI
        RtlDestroyProcessParameters(
            _In_ PRTL_USER_PROCESS_PARAMETERS ProcessParameters
        );

    typedef enum _EVENT_TYPE
    {
        NotificationEvent,
        SynchronizationEvent
    } EVENT_TYPE;

    NTSYSAPI
        NTSTATUS
        NTAPI
        NtCreateEvent(
            _Out_           PHANDLE            EventHandle,
            _In_            ACCESS_MASK        DesiredAccess,
            _In_opt_        POBJECT_ATTRIBUTES ObjectAttributes,
            _In_            EVENT_TYPE         EventType,
            _In_            BOOLEAN            InitialState
        );

    typedef VOID(NTAPI* PIO_APC_ROUTINE)(
        _In_ PVOID ApcContext,
        _In_ PIO_STATUS_BLOCK IoStatusBlock,
        _In_ ULONG Reserved
        );

    NTSYSAPI
        NTSTATUS
        NTAPI
        NtFsControlFile(
            _In_            HANDLE           FileHandle,
            _In_opt_        HANDLE           Event,
            _In_opt_        PIO_APC_ROUTINE  ApcRoutine,
            _In_opt_        PVOID            ApcContext,
            _Out_           PIO_STATUS_BLOCK IoStatusBlock,
            _In_            ULONG            FsControlCode,
            _In_opt_        PVOID            InputBuffer,
            _In_            ULONG            InputBufferLength,
            _Out_opt_       PVOID            OutputBuffer,
            _In_            ULONG            OutputBufferLength
        );

    NTSYSCALLAPI
        NTSTATUS
        NTAPI
        NtWaitForSingleObject(
            _In_ HANDLE Handle,
            _In_ BOOLEAN Alertable,
            _In_opt_ PLARGE_INTEGER Timeout
        );

#ifdef __cplusplus
} // extern "C"
#endif

#endif // __NTDLL_H__
