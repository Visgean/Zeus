/*
  SSL-сокет на SSPI.

  ѕримичани€:
    1. «а приоритет вз€т минимальный расход пам€ти, т.к. в скорости смысла нет.
    2. ƒл€ работы могут использоватьс€ только блокируемые сокеты.
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
    »нициализаци€.
  */
  void init(void);

  /*
    ƒеинициализаци€.
  */
  void uninit(void);

  SERVERDATA *_createServerData(PCCERT_CONTEXT context, DWORD enabledProtocols);
  void _freeServerData(SERVERDATA *sd);
  
  bool _serverHandshake(SERVERDATA *sd, void *recv, DWORD recvSize, void *send, DWORD *sendSize);  
  /*
    ѕроизводить SSL handshake со стороны клиента.

    IN socket           - сокет.
    IN serverName       - им€ сервера (дл€ работы с сертификатом).
    OUT sd              - данные SSL-сокета.
    IN enabledProtocols - список активных протоколов SP_PROT_*_CLIENT.
    IN timeout          - количество миллесекунд ожидани€ получени€ данных, или 0 дл€ бесконечного
                          ожидани€. 

    Return              - true - в случаи успешного установлени€ соединени€,
                          false - в случаи ошибки.
  */
  bool _startClientEncryption(SOCKET socket, LPWSTR serverName, SOCKETDATA *sd, DWORD enabledProtocols, DWORD timeout);
  
  /*
    «акрывает SSL-сокет (не закрывает соединение).

    IN sd - данные SSL-сокета.
  */
  void _close(SOCKETDATA *sd);
  
  /*
    «апись данных в сокет.

    IN sd      - данные SSL-сокета.
    IN buf     - буфер с данными.
    IN bufSize - размер буфера.

    Return      - true - в случаи успеха,
                  false - в случаи ошибки.
  */
  bool _send(SOCKETDATA *sd, void *buf, int bufSize);
  
  /*
    „тение данных из сокета.

    IN sd             - данные SSL-сокета.
    OUT buf           - буфер дл€ данных.
    IN bufSize        - размер буфера.
    IN timeout        - количество миллесекунд ожидани€ получени€ данных, или 0 дл€ бесконечного
                        ожидани€.
    OUT extraAvalible - сообщает достпны ли излишниеданные, еоторые не вместились в буфер.
    
    Return            - количетсво прочитаных байт или SOCKET_ERROR.
  */
  int _recv(SOCKETDATA *sd, void *buf, int bufSize, DWORD timeout, bool *extraAvalible);

  /*
    „тение данных из сокета до полного заполнени€ буфера.

    IN sd             - данные SSL-сокета.
    OUT buf           - буфер дл€ данных.
    IN bufSize        - размер буфера.
    IN timeout        - количество миллесекунд ожидани€ получени€ данных, или 0 дл€ бесконечного
                        ожидани€.
    OUT extraAvalible - сообщает достпны ли излишниеданные, еоторые не вместились в буфер.

    Return            - true - в случаи успеха,
                        false - в случаи ошибки.
  */
  bool _recvAll(SOCKETDATA *sd, void *buf, int bufSize, DWORD timeout, bool *extraAvalible);
};
