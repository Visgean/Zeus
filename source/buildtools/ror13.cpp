/*
  ROR13 хэш, основное предназначение - поулчение хэша WinAPI-функций.
*/
#include <windows.h>

#include "defines.h"
#include "core.h"
#include "language.h"

#include "..\common\console.h"
#include "..\common\cui.h"

void commandRor13(LPWSTR *switches, DWORD switchesCount)
{
  LPWSTR inputString = Cui::_getSwitchValue(switches, switchesCount, lng_switch_inputstring);
  if(inputString == NULL || inputString == (LPWSTR)1)
  {
    Console::writeStringW(lng_ror13_no_inputstring, sizeof(lng_ror13_no_inputstring) / sizeof(WCHAR) - 1);
    return;
  }

  {
    DWORD hash = 0;

    while(*inputString)
    {
      hash = (hash << (0x20 - 13)) | (hash >> 13);
      hash += *inputString;
      inputString++;
    }

    if(Cui::_getSwitchValue(switches, switchesCount, lng_switch_nologo) != NULL)
    {
      Console::writeFormatW(L"0x%08X", hash);
    }
    else
    {
      Console::writeFormatW(L"ROR13 hash: 0x%08X\n", hash);
    }
  }
  coreData.exitCode = Cui::EXITCODE_SUCCESSED;
}
