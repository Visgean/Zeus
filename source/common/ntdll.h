#pragma once
/*
  Íàáîð îïðåäåëåíèé äëÿ ntdll.dll
*/

#if !defined NTDLL_IMPORT
  #error NTDLL_IMPORT not defined!
#endif

//////////////////////////////////////////////////// ////////////////////////////////////////////////
//Â NT status.
//////////////////////////////////////////////////// ////////////////////////////////////////////////

typedef LONG NTSTATUS;

//In the Generic test for success on any status value (non-negative numbers indicate success).
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)

//In the Generic test for information on any status value.
#define NT_INFORMATION(Status) ((((ULONG)(Status)) >> 30) == 1)

//In the Generic test for warning on any status value.
#define NT_WARNING(Status) ((((ULONG)(Status)) >> 30) == 2)

//In the Generic test for error on any status value.
#define NT_ERROR(Status) ((((ULONG)(Status)) >> 30) == 3)

//////////////////////////////////////////////////// ////////////////////////////////////////////////

//The success status codes.
#define STATUS_SUCCESS                   ((NTSTATUS)0x00000000L)

//The file% hs does not exist.
#define STATUS_NO_SUCH_FILE              ((NTSTATUS)0xC000000FL)

//////////////////////////////////////////////////// ////////////////////////////////////////////////
//Â Determine a different.
//////////////////////////////////////////////////// ////////////////////////////////////////////////
typedef LONG KPRIORITY;

//Flags for NtCreateUserProcess.CreateProcessFlags
#define CREATE_PROCESS_INHERIT_HANDLES 0x0004
#define CREATE_PROCESS_PROTECTED       0x0040

//Flags for NtCreateUserProcess.CreateThreadFlags
#define CREATE_THREAD_SUSPENDED 0x0001

//File mode creation.
#define FILE_SUPERSEDE           0x00000000
#define FILE_OPEN                0x00000001
#define FILE_CREATE              0x00000002
#define FILE_OPEN_IF             0x00000003
#define FILE_OVERWRITE           0x00000004
#define FILE_OVERWRITE_IF        0x00000005
#define FILE_MAXIMUM_DISPOSITION 0x00000005

//Option to create the file.
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
  ProcessIoPortHandlers,          //In Note: this is kernel mode only
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
  MaxProcessInfoClass             //In MaxProcessInfoClass should always be the last enum
}PROCESSINFOCLASS;

typedef enum
{
  FileDirectoryInformation         = 1,
  FileFullDirectoryInformation,   //In 2
  FileBothDirectoryInformation,   //The 3
  FileBasicInformation,           //In 4
  FileStandardInformation,        //At 5
  FileInternalInformation,        //In 6
  FileEaInformation,              //In 7
  FileAccessInformation,          //At 8
  FileNameInformation,            //At 9
  FileRenameInformation,          //In 10
  FileLinkInformation,            //In 11
  FileNamesInformation,           //In 12
  FileDispositionInformation,     //In 13
  FilePositionInformation,        //In 14
  FileFullEaInformation,          //In 15
  FileModeInformation,            //In 16
  FileAlignmentInformation,       //In 17
  FileAllInformation,             //In 18
  FileAllocationInformation,      //In 19
  FileEndOfFileInformation,       //In 20
  FileAlternateNameInformation,   //In 21
  FileStreamInformation,          //In 22
  FilePipeInformation,            //In 23
  FilePipeLocalInformation,       //In 24
  FilePipeRemoteInformation,      //In 25
  FileMailslotQueryInformation,   //In 26
  FileMailslotSetInformation,     //In 27
  FileCompressionInformation,     //In 28
  FileObjectIdInformation,        //In 29
  FileCompletionInformation,      //The 30
  FileMoveClusterInformation,     //The 31
  FileQuotaInformation,           //The 32
  FileReparsePointInformation,    //The 33
  FileNetworkOpenInformation,     //The 34
  FileAttributeTagInformation,    //The 35
  FileTrackingInformation,        //The 36
  FileIdBothDirectoryInformation, //The 37
  FileIdFullDirectoryInformation, //The 38
  FileValidDataLengthInformation, //The 39
  FileShortNameInformation,       //In 40
  FileIoCompletionNotificationInformation, //In 41
  FileIoStatusBlockRangeInformation,       //In 42
  FileIoPriorityHintInformation,           //In 43
  FileSfioReserveInformation,              //In 44
  FileSfioVolumeInformation,               //In 45
  FileHardLinkInformation,                 //In 46
  FileProcessIdsUsingFileInformation,      //In 47
  FileNormalizedNameInformation,           //In 48
  FileNetworkPhysicalNameInformation,      //In 49
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
  SystemPowerInformation1, //Konfilikt with winnt.h
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

//////////////////////////////////////////////////// ////////////////////////////////////////////////
//Â Structure.
//////////////////////////////////////////////////// ////////////////////////////////////////////////
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
  PVOID SecurityDescriptor;        //In the Points to type SECURITY_DESCRIPTOR
  PVOID SecurityQualityOfService;  //In the Points to type SECURITY_QUALITY_OF_SERVICE
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

//////////////////////////////////////////////////// ////////////////////////////////////////////////
//Â Function.
//////////////////////////////////////////////////// ////////////////////////////////////////////////
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

