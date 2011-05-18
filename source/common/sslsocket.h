/*
  SSL-сокет на SSPI.

  Примичания:
    1. За приоритет взят минимальный расход памяти, т.к. в скорости смысла нет.
    2. Для работы могут использоваться только блокируемые сокеты.
*/
#pragma once

#include <security.h>
#include <schnlsp.h>

namespace SslSocket
{
  enum
  {
    SDF_STATUS_HANDSHAKE = 0x1
  };
  
  typedef struct
  {
    DWORD flags;
    CredHandle credHandle;
    CtxtHandle ctxtHandle;
  }SERVERDATA;
  
  
  typedef struct
  {
    SOCKET socket;
    SecHandle sh;
    CtxtHandle ch;
    SecPkgContext_StreamSizes streamSizes;
    void *extraBuffer;
    DWORD extraBufferSize;
    void *ioBuffer;
    DWORD ioBufferSize;
    void *pendingData;
    DWORD pendingDataSize;
  }SOCKETDATA;
  
  /*
    Инициализация.
  */
  void init(void);

  /*
    Деинициализация.
  */
  void uninit(void);

  SERVERDATA *_createServerData(PCCERT_CONTEXT context, DWORD enabledProtocols);
  void _freeServerData(SERVERDATA *sd);
  
  bool _serverHandshake(SERVERDATA *sd, void *recv, DWORD recvSize, void *send, DWORD *sendSize);  
  /*
    Производить SSL handshake со стороны клиента.

    IN socket           - сокет.
    IN serverName       - имя сервера (для работы с сертификатом).
    OUT sd              - данные SSL-сокета.
    IN enabledProtocols - список активных протоколов SP_PROT_*_CLIENT.
    IN timeout          - количество миллесекунд ожидания получения данных, или 0 для бесконечного
                          ожидания. 

    Return              - true - в случаи успешного установления соединения,
                          false - в случаи ошибки.
  */
  bool _startClientEncryption(SOCKET socket, LPWSTR serverName, SOCKETDATA *sd, DWORD enabledProtocols, DWORD timeout);
  
  /*
    Закрывает SSL-сокет (не закрывает соединение).

    IN sd - данные SSL-сокета.
  */
  void _close(SOCKETDATA *sd);
  
  /*
    Запись данных в сокет.

    IN sd      - данные SSL-сокета.
    IN buf     - буфер с данными.
    IN bufSize - размер буфера.

    Return      - true - в случаи успеха,
                  false - в случаи ошибки.
  */
  bool _send(SOCKETDATA *sd, void *buf, int bufSize);
  
  /*
    Чтение данных из сокета.

    IN sd             - данные SSL-сокета.
    OUT buf           - буфер для данных.
    IN bufSize        - размер буфера.
    IN timeout        - количество миллесекунд ожидания получения данных, или 0 для бесконечного
                        ожидания.
    OUT extraAvalible - сообщает достпны ли излишниеданные, еоторые не вместились в буфер.
    
    Return            - количетсво прочитаных байт или SOCKET_ERROR.
  */
  int _recv(SOCKETDATA *sd, void *buf, int bufSize, DWORD timeout, bool *extraAvalible);

  /*В В В В Reading data from the socket until the buffer is full.

В В В В IN sd - data SSL-socket.
В В В В OUT buf - buffer for data.
В В В В IN bufSize - the size of the buffer.
В В В В IN timeout - the number of millesekund are waiting for data, or 0 for an infinite
В В В В В В В В В В В В В В В В В В В В В В В В expectations.
В В В В OUT extraAvalible - tells whether dostpny izlishniedannye, eotorye not fit into the buffer.

В В В В Return - true - if successful,
В В В В В В В В В В В В В В В В В В В В В В В В false - if an error occurs.
В В */
  bool _recvAll(SOCKETDATA *sd, void *buf, int bufSize, DWORD timeout, bool *extraAvalible);
};
