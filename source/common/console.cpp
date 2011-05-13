#include <windows.h>
#include <shlwapi.h>

#include "console.h"

static HANDLE handleOutput;
static bool isWineBug;

bool Console::init(void)
{
  register DWORD mode;
  handleOutput = CWA(kernel32, GetStdHandle(STD_OUTPUT_HANDLE));

  //Wine баг: http://www.winehq.org/pipermail/wine-bugs/2008-January/088451.html
  isWineBug = (CWA(kernel32, GetFileType)(handleOutput) != FILE_TYPE_CHAR || CWA(kernel32, GetConsoleMode)(handleOutput, &mode) == FALSE);
  return (handleOutput != INVALID_HANDLE_VALUE);
}

void Console::uninit(void)
{

}

DWORD Console::writeData(void *data, DWORD size)
{
  register DWORD result;
  CWA(kernel32, WriteFile)(handleOutput, data, size, &result, NULL);
  return result;
}

DWORD Console::writeStringW(LPWSTR string, DWORD size)
{
  if(size == (DWORD)-1)size = Str::_LengthW(string);
  if(isWineBug)return writeData(string, size * sizeof(WCHAR)); //WINE WARNING: Вывод почему-то происходит абсолютно номарльно.

  register DWORD result;
  CWA(kernel32, WriteConsoleW)(handleOutput, string, size, &result, NULL);
  return result;
}

DWORD Console::writeFormatW(LPWSTR format, ...)
{
  LPWSTR buf = NULL;
  int size;

  va_list list;
  va_start(list, format);
  size = Str::_vsprintfExW(&buf, format, list);
  va_end(list);
  
  if(size == -1)
  {
    //WriteStringW(L"\n\n-- Not enough memory! --\n\n", -1);
    return 0;
  }

  writeStringW(buf, (DWORD)size);
  Mem::free(buf);
  return (DWORD)size;
}
