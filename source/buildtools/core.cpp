#include <windows.h>
#include <shellapi.h>
#include <shlwapi.h>

#include "defines.h"
#include "core.h"
#include "language.h"

#include "..\common\mem.h"
#include "..\common\str.h"
#include "..\common\console.h"
#include "..\common\cui.h"

COREDATA coreData;

//ROR 13 Опции
static const Cui::SWITCH ror13Switches[] =
{
  {lng_switch_nologo,                   lng_switch_nologo_help},
  {lng_switch_inputstring L":[string]", lng_switch_inputstring_help}
};

//PEINFO Опции
static const Cui::SWITCH peInfoSwitches[] =
{
  {lng_switch_nologo,                    lng_switch_nologo_help},
  {lng_switch_file L":[file]",           lng_switch_file_help},
  {lng_switch_as_va,                     lng_switch_as_va_help},
  {lng_switch_datadirectory L":[index]", lng_switch_datadirectory_help},
  {lng_switch_patch_dw L":[rva,dword]",  lng_switch_patch_dw_help}
};

//Команды
void commandRor13(LPWSTR *switches, DWORD switchesCount);
void commandPeInfo(LPWSTR *switches, DWORD switchesCount);

static const Cui::COMMAND commands[] =
{
  {lng_command_ror13,  lng_command_ror13_help,  commandRor13,  (Cui::SWITCH *)ror13Switches, sizeof(ror13Switches) / sizeof(Cui::SWITCH)},
  {lng_command_peinfo, lng_command_peinfo_help, commandPeInfo, (Cui::SWITCH *)peInfoSwitches, sizeof(peInfoSwitches) / sizeof(Cui::SWITCH)},
};

////////////////////////////////////////////////////////////////////////////////////////////////////

void Core::init(void)
{
  Mem::_zero(&coreData, sizeof(COREDATA));
  CWA(kernel32, GetModuleFileNameW)(NULL, coreData.fileName, MAX_PATH);

  coreData.exitCode         = Cui::EXITCODE_ERROR_BAD_COMMAND_LINE;
  coreData.commandLine.args = CWA(kernel32, CommandLineToArgvW)(CWA(kernel32, GetCommandLineW()), &coreData.commandLine.argsCount);
}

void Core::uninit(void)
{
  if(coreData.commandLine.args != NULL)CWA(kernel32, LocalFree)(coreData.commandLine.args);
}

void Core::showLogo(void)
{
  Console::writeStringW(lng_logo, sizeof(lng_logo) / sizeof(WCHAR) - 1);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Точка входа.
////////////////////////////////////////////////////////////////////////////////////////////////////
void WINAPI entryPoint(void)
{
  Mem::init();  
  Console::init();  
  Core::init();
  
  CUI_DEFAULT_COMMANDLINE_HELPER;

  Core::uninit();
  Console::uninit();
  Mem::uninit();
  
  CWA(kernel32, ExitProcess)(coreData.exitCode);
}
