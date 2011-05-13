#pragma once
/*
  Набор определений для ntdll.dll
*/

#if !defined NTDLL_IMPORT
  #error NTDLL_IMPORT not defined!
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
// NT статусы.
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef LONG NTSTATUS;

// Generic test for success on any status value (non-negative numbers indicate success).
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)

// Generic test for information on any status value.
#define NT_INFORMATION(Status) ((((ULONG)(Status)) >> 30) == 1)

// Generic test for warning on any status value.
#define NT_WARNING(Status) ((((ULONG)(Status)) >> 30) == 2)

// Generic test for error on any status value.
#define NT_ERROR(Status) ((((ULONG)(Status)) >> 30) == 3)

////////////////////////////////////////////////////////////////////////////////////////////////////

//The success status codes.
#define STATUS_SUCCESS                   ((NTSTATUS)0x00000000L)

//The file %hs does not exist.
#define STATUS_NO_SUCH_FILE              ((NTSTATUS)0xC000000FL)

////////////////////////////////////////////////////////////////////////////////////////////////////
// Разные определния.
////////////////////////////////////////////////////////////////////////////////////////////////////
typedef LONG KPRIORITY;

//Флаги для NtCreateUserProcess.CreateProcessFlags
#define CREATE_PROCESS_INHERIT_HANDLES 0x0004
#define CREATE_PROCESS_PROTECTED       0x0040

//Флаги для NtCreateUserProcess.CreateThreadFlags
#define CREATE_THREAD_SUSPENDED 0x0001

//Режимы создания файла.
#define FILE_SUPERSEDE           0x00000000
#define FILE_OPEN                0x00000001
#define FILE_CREATE              0x00000002
#define FILE_OPEN_IF             0x00000003
#define FILE_OVERWRITE           0x00000004
#define FILE_OVERWRITE_IF        0x00000005
#define FILE_MAXIMUM_DISPOSITION 0x00000005

//Опции создания файла.
#define FILE_DIRECTORY_FILE            0x00000001
#define FILE_WRITE_THROUGH             0x00000002
#define FILE_SEQUENTIAL_ONLY           0x00000004
#define FILE_NO_INTERMEDIATE_BUFFERING 0x00000008

#define FILE_SYNCHRONOUS_IO_ALERT      0x00000010
#define FILE_SYNCHRONOUS_IO_NONALERT   0x00000020
#define FILE_NON_DIRECTORY_FILE        0x00000040
#define FILE_CREATE_TREE_CONNECTION    0x00000080

#define FILE_COMPLETE_IF_OPLOCKED      0x00000100
#define FILE_NO_EA_KNOWLEDGE           0x00000200
#define FILE_OPEN_REMOTE_INSTANCE      0x00000400
#define FILE_RANDOM_ACCESS             0x00000800

#define FILE_DELETE_ON_CLOSE           0x00001000
#define FILE_OPEN_BY_FILE_ID           0x00002000
#define FILE_OPEN_FOR_BACKUP_INTENT    0x00004000
#define FILE_NO_COMPRESSION            0x00008000

#define FILE_RESERVE_OPFILTER          0x00100000
#define FILE_OPEN_REPARSE_POINT        0x00200000
#define FILE_OPEN_NO_RECALL            0x00400000
#define FILE_OPEN_FOR_FREE_SPACE_QUERY 0x00800000

typedef enum
{
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
  ProcessIoPriority,
  ProcessExecuteFlags,
  ProcessTlsInformation,
  ProcessCookie,
  ProcessImageInformation,
  ProcessCycleTime,
  ProcessPagePriority,
  ProcessInstrumentationCallback,
  ProcessThreadStackAllocation,
  ProcessWorkingSetWatchEx,
  ProcessImageFileNameWin32,
  ProcessImageFileMapping,
  MaxProcessInfoClass             // MaxProcessInfoClass should always be the last enum
}PROCESSINFOCLASS;

typedef enum
{
  FileDirectoryInformation         = 1,
  FileFullDirectoryInformation,   // 2
  FileBothDirectoryInformation,   // 3
  FileBasicInformation,           // 4
  FileStandardInformation,        // 5
  FileInternalInformation,        // 6
  FileEaInformation,              // 7
  FileAccessInformation,          // 8
  FileNameInformation,            // 9
  FileRenameInformation,          // 10
  FileLinkInformation,            // 11
  FileNamesInformation,           // 12
  FileDispositionInformation,     // 13
  FilePositionInformation,        // 14
  FileFullEaInformation,          // 15
  FileModeInformation,            // 16
  FileAlignmentInformation,       // 17
  FileAllInformation,             // 18
  FileAllocationInformation,      // 19
  FileEndOfFileInformation,       // 20
  FileAlternateNameInformation,   // 21
  FileStreamInformation,          // 22
  FilePipeInformation,            // 23
  FilePipeLocalInformation,       // 24
  FilePipeRemoteInformation,      // 25
  FileMailslotQueryInformation,   // 26
  FileMailslotSetInformation,     // 27
  FileCompressionInformation,     // 28
  FileObjectIdInformation,        // 29
  FileCompletionInformation,      // 30
  FileMoveClusterInformation,     // 31
  FileQuotaInformation,           // 32
  FileReparsePointInformation,    // 33
  FileNetworkOpenInformation,     // 34
  FileAttributeTagInformation,    // 35
  FileTrackingInformation,        // 36
  FileIdBothDirectoryInformation, // 37
  FileIdFullDirectoryInformation, // 38
  FileValidDataLengthInformation, // 39
  FileShortNameInformation,       // 40
  FileIoCompletionNotificationInformation, // 41
  FileIoStatusBlockRangeInformation,       // 42
  FileIoPriorityHintInformation,           // 43
  FileSfioReserveInformation,              // 44
  FileSfioVolumeInformation,               // 45
  FileHardLinkInformation,                 // 46
  FileProcessIdsUsingFileInformation,      // 47
  FileNormalizedNameInformation,           // 48
  FileNetworkPhysicalNameInformation,      // 49
  FileMaximumInformation
}FILE_INFORMATION_CLASS, *PFILE_INFORMATION_CLASS;

typedef enum
{
  SystemBasicInformation,
  SystemProcessorInformation,
  SystemPerformanceInformation,
  SystemTimeOfDayInformation,
  SystemPathInformation,
  SystemProcessInformation,
  SystemCallCountInformation,
  SystemDeviceInformation,
  SystemProcessorPerformanceInformation,
  SystemFlagsInformation,
  SystemCallTimeInformation,
  SystemModuleInformation, 
  SystemLocksInformation,
  SystemStackTraceInformation,
  SystemPagedPoolInformation,
  SystemNonPagedPoolInformation,
  SystemHandleInformation,
  SystemObjectInformation,
  SystemPageFileInformation,
  SystemVdmInstemulInformation,
  SystemVdmBopInformation,
  SystemFileCacheInformation,
  SystemPoolTagInformation,
  SystemInterruptInformation,
  SystemDpcBehaviorInformation,
  SystemFullMemoryInformation,
  SystemLoadGdiDriverInformation,
  SystemUnloadGdiDriverInformation,
  SystemTimeAdjustmentInformation,
  SystemSummaryMemoryInformation,
  SystemNextEventIdInformation,
  SystemEventIdsInformation,
  SystemCrashDumpInformation,
  SystemExceptionInformation,
  SystemCrashDumpStateInformation,
  SystemKernelDebuggerInformation,
  SystemContextSwitchInformation,
  SystemRegistryQuotaInformation,
  SystemExtendServiceTableInformation,
  SystemPrioritySeperation,
  SystemPlugPlayBusInformation,
  SystemDockInformation,
  SystemPowerInformation1, //Конфиликт с winnt.h
  SystemProcessorSpeedInformation,
  SystemCurrentTimeZoneInformation,
  SystemLookasideInformation
}SYSTEM_INFORMATION_CLASS, *PSYSTEM_INFORMATION_CLASS;

typedef enum
{
  ObjectBasicInformation,
  ObjectNameInformation,
  ObjectTypeInformation,
  ObjectAllInformation,
  ObjectDataInformation
}OBJECT_INFORMATION_CLASS, *POBJECT_INFORMATION_CLASS;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Структуры.
////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
  USHORT Length;
  USHORT MaximumLength;
  LPWSTR Buffer;
}UNICODE_STRING, *PUNICODE_STRING;

typedef struct
{
  USHORT Length;
  USHORT MaximumLength;
  PCHAR  Buffer;
}ANSI_STRING, *PANSI_STRING;

typedef struct
{
  ULONG Length;
  HANDLE RootDirectory;
  PUNICODE_STRING ObjectName;
  ULONG Attributes;
  PVOID SecurityDescriptor;        // Points to type SECURITY_DESCRIPTOR
  PVOID SecurityQualityOfService;  // Points to type SECURITY_QUALITY_OF_SERVICE
}OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;

typedef struct
{
  HANDLE UniqueProcess;
  HANDLE UniqueThread;
}CLIENT_ID, *PCLIENT_ID;

typedef struct
{
  PVOID StackBase;
  PVOID StackLimit;
  PVOID StackCommit;
  PVOID StackCommitMax;
  PVOID StackReserved;
}INITIAL_TEB, *PINITIAL_TEB;

typedef struct
{
  union
  {
    NTSTATUS Status;
    PVOID Pointer;
  };
  ULONG_PTR Information;
}IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

typedef struct
{
  UNICODE_STRING Name;
}OBJECT_NAME_INFORMATION, *POBJECT_NAME_INFORMATION;

typedef struct
{
  NTSTATUS ExitStatus;
  void *PebBaseAddress;
  ULONG_PTR AffinityMask;
  KPRIORITY BasePriority;
  ULONG_PTR UniqueProcessId;
  ULONG_PTR InheritedFromUniqueProcessId;
}PROCESS_BASIC_INFORMATION, *PPROCESS_BASIC_INFORMATION;

typedef struct
{
  ULONG NextEntryOffset;
  ULONG FileIndex;
  ULONG FileNameLength;
  WCHAR FileName[1];
}FILE_NAMES_INFORMATION, *PFILE_NAMES_INFORMATION;

typedef struct
{
  ULONG NextEntryOffset;
  ULONG FileIndex;
  LARGE_INTEGER CreationTime;
  LARGE_INTEGER LastAccessTime;
  LARGE_INTEGER LastWriteTime;
  LARGE_INTEGER ChangeTime;
  LARGE_INTEGER EndOfFile;
  LARGE_INTEGER AllocationSize;
  ULONG FileAttributes;
  ULONG FileNameLength;
  WCHAR FileName[1];
}FILE_DIRECTORY_INFORMATION, *PFILE_DIRECTORY_INFORMATION;

typedef struct
{
  ULONG NextEntryOffset;
  ULONG FileIndex;
  LARGE_INTEGER CreationTime;
  LARGE_INTEGER LastAccessTime;
  LARGE_INTEGER LastWriteTime;
  LARGE_INTEGER ChangeTime;
  LARGE_INTEGER EndOfFile;
  LARGE_INTEGER AllocationSize;
  ULONG FileAttributes;
  ULONG FileNameLength;
  ULONG EaSize;
  WCHAR FileName[1];
}FILE_FULL_DIR_INFORMATION, *PFILE_FULL_DIR_INFORMATION;

typedef struct
{
  ULONG NextEntryOffset;
  ULONG FileIndex;
  LARGE_INTEGER CreationTime;
  LARGE_INTEGER LastAccessTime;
  LARGE_INTEGER LastWriteTime;
  LARGE_INTEGER ChangeTime;
  LARGE_INTEGER EndOfFile;
  LARGE_INTEGER AllocationSize;
  ULONG FileAttributes;
  ULONG FileNameLength;
  ULONG EaSize;
  LARGE_INTEGER FileId;
  WCHAR FileName[1];
}FILE_ID_FULL_DIR_INFORMATION, *PFILE_ID_FULL_DIR_INFORMATION;

typedef struct
{
  ULONG NextEntryOffset;
  ULONG FileIndex;
  LARGE_INTEGER CreationTime;
  LARGE_INTEGER LastAccessTime;
  LARGE_INTEGER LastWriteTime;
  LARGE_INTEGER ChangeTime;
  LARGE_INTEGER EndOfFile;
  LARGE_INTEGER AllocationSize;
  ULONG FileAttributes;
  ULONG FileNameLength;
  ULONG EaSize;
  CCHAR ShortNameLength;
  WCHAR ShortName[12];
  WCHAR FileName[1];
}FILE_BOTH_DIR_INFORMATION, *PFILE_BOTH_DIR_INFORMATION;

typedef struct
{
  ULONG NextEntryOffset;
  ULONG FileIndex;
  LARGE_INTEGER CreationTime;
  LARGE_INTEGER LastAccessTime;
  LARGE_INTEGER LastWriteTime;
  LARGE_INTEGER ChangeTime;
  LARGE_INTEGER EndOfFile;
  LARGE_INTEGER AllocationSize;
  ULONG FileAttributes;
  ULONG FileNameLength;
  ULONG EaSize;
  CCHAR ShortNameLength;
  WCHAR ShortName[12];
  LARGE_INTEGER FileId;
  WCHAR FileName[1];
}FILE_ID_BOTH_DIR_INFORMATION, *PFILE_ID_BOTH_DIR_INFORMATION;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Функции.
////////////////////////////////////////////////////////////////////////////////////////////////////
typedef VOID (NTAPI *PIO_APC_ROUTINE)
(
 PVOID ApcContext,
 PIO_STATUS_BLOCK IoStatusBlock,
 ULONG Reserved
 );

#if(NTDLL_IMPORT == 1)
  #define DECLARE_NTAPI(ret, name) __declspec(dllimport) ret NTAPI name
#else
  #define DECLARE_NTAPI(ret, name) typedef ret (NTAPI *ntdll##name)
#endif

#if(NTDLL_IMPORT == 1)
extern "C"
{
#endif

  DECLARE_NTAPI(NTSTATUS, NtCreateThread)
  (
    PHANDLE ThreadHandle,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes,
    HANDLE ProcessHandle,
    PCLIENT_ID ClientId,
    PCONTEXT ThreadContext,
    PINITIAL_TEB InitialTeb,
    BOOLEAN CreateSuspended
  );

  DECLARE_NTAPI(NTSTATUS, NtQueryInformationProcess)
  (
    HANDLE ProcessHandle,
    PROCESSINFOCLASS ProcessInformationClass, 
    PVOID ProcessInformation,
    ULONG ProcessInformationLength,
    PULONG ReturnLength
  );

  DECLARE_NTAPI(NTSTATUS, LdrLoadDll)
  (
    PWCHAR PathToFile,
    ULONG Flags,
    PUNICODE_STRING ModuleFileName,
    PHANDLE ModuleHandle
  );

  DECLARE_NTAPI(NTSTATUS, LdrGetDllHandle)
  (
    PWCHAR PathToFile,
    PVOID Unused,
    PUNICODE_STRING ModuleFileName,
    PHANDLE ModuleHandle
  );

  DECLARE_NTAPI(NTSTATUS, LdrGetProcedureAddress)
  (
    HMODULE ModuleHandle,
    PANSI_STRING FunctionName,
    WORD Oridinal,
    PVOID *FunctionAddress
  );

  DECLARE_NTAPI(NTSTATUS, NtQueryDirectoryFile)
  (
    HANDLE FileHandle,
    HANDLE Event,
    PIO_APC_ROUTINE ApcRoutine,
    PVOID ApcContext,
    PIO_STATUS_BLOCK IoStatusBlock,
    PVOID FileInformation,
    ULONG Length,
    FILE_INFORMATION_CLASS FileInformationClass,
    BOOLEAN ReturnSingleEntry,
    PUNICODE_STRING FileName,
    BOOLEAN RestartScan
  );

  DECLARE_NTAPI(NTSTATUS, NtQuerySystemInformation)
  (
    SYSTEM_INFORMATION_CLASS SystemInformationClass,
    PVOID SystemInformation,
    ULONG SystemInformationLength,
    PULONG ReturnLength
  );

  DECLARE_NTAPI(NTSTATUS, NtQueryObject)
  (
    HANDLE Handle,
    OBJECT_INFORMATION_CLASS ObjectInformationClass,
    PVOID ObjectInformation,
    ULONG ObjectInformationLength,
    PULONG ReturnLength
  );

  DECLARE_NTAPI(NTSTATUS, NtCreateFile)
  (
    PHANDLE FileHandle,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes,
    PIO_STATUS_BLOCK IoStatusBlock,
    PLARGE_INTEGER AllocationSize,
    ULONG FileAttributes,
    ULONG ShareAccess,
    ULONG CreateDisposition,
    ULONG CreateOptions,
    PVOID EaBuffer,
    ULONG EaLength
  );

  DECLARE_NTAPI(NTSTATUS, RtlCreateUserThread)
  (
    HANDLE ProcessHandle,
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    BOOLEAN CreateSuspended,
    ULONG StackZeroBits,
    PULONG StackReserved,
    PULONG StackCommit,
    PVOID StartAddress,
    PVOID StartParameter,
    PHANDLE ThreadHandle,
    PVOID ClientID
  );

  DECLARE_NTAPI(NTSTATUS, NtCreateUserProcess)
  (
    PHANDLE ProcessHandle,
    PHANDLE ThreadHandle,
    ACCESS_MASK ProcessDesiredAccess,
    ACCESS_MASK ThreadDesiredAccess,
    POBJECT_ATTRIBUTES ProcessObjectAttributes,
    POBJECT_ATTRIBUTES ThreadObjectAttributes,
    ULONG CreateProcessFlags,
    ULONG CreateThreadFlags,
    PVOID /*PRTL_USER_PROCESS_PARAMETERS*/ ProcessParameters,
    PVOID Parameter9,
    PVOID /*PNT_PROC_THREAD_ATTRIBUTE_LIST*/ AttributeList
  );

  DECLARE_NTAPI(NTSTATUS, NtResumeThread)
  (
    HANDLE ThreadHandle,
    PULONG SuspendCount
  );

  DECLARE_NTAPI(NTSTATUS, NtCreateFile)
  (
    PHANDLE FileHandle,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes,
    PIO_STATUS_BLOCK IoStatusBlock,
    PLARGE_INTEGER AllocationSize,
    ULONG FileAttributes,
    ULONG ShareAccess,
    ULONG CreateDisposition,
    ULONG CreateOptions,
    PVOID EaBuffer,
    ULONG EaLength
  );

#if(NTDLL_IMPORT == 1)
};
#endif

#undef DECLARE_NTAPI

