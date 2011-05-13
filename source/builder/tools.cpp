#include <windows.h>

#include "defines.h"
#include "languages.h"

#include "..\common\mem.h"
#include "..\common\str.h"

void writeOutput(HWND hwnd, LPWSTR format, ...)
{
  LPWSTR buf = NULL;
  va_list list;
  va_start(list, format);
  Str::_vsprintfExW(&buf, format, list);
  va_end(list);
  
  if(buf != NULL)
  {
    if(Str::_CatExW(&buf, L"\r\n", 2))
    {
      int size = CWA(user32, GetWindowTextLengthW)(hwnd);
      CWA(user32, SendMessageW)(hwnd, EM_SETSEL, size, size);
      CWA(user32, SendMessageW)(hwnd, EM_REPLACESEL, FALSE, (LPARAM)buf);
    }
    Mem::free(buf);
  }
}

void writeOutputError(HWND hwnd, LPWSTR format, ...)
{
  LPWSTR buf = NULL;
  va_list list;
  va_start(list, format);
  Str::_vsprintfExW(&buf, format, list);
  va_end(list);

  writeOutput(hwnd, Languages::get(Languages::error_output_template), buf);
  Mem::free(buf);
}

void closeThreadIfFinsinhed(HANDLE *subThread)
{
  if(*subThread != NULL && CWA(kernel32, WaitForSingleObject)(*subThread, 0) == WAIT_OBJECT_0)
  {
    CWA(kerenl32, CloseHandle)(*subThread);
    *subThread = NULL;
  }
}
