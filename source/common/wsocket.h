/*
  Дополнение для WinSock.
*/
#pragma once

//Данные о количестве байт в IP.
#define IPv4_SIZE     4
#define IPv6_SIZE    16
#define IP_PORT_SIZE  2

//Временная функция для очитски не известных мне данных. Когда я их узнаю, необходимо профиксить.
#define HZ_IPV6_CLEAR(s) {if(((SOCKADDR_IN6 *)(s))->sin6_family == AF_INET6){((SOCKADDR_IN6 *)(s))->sin6_flowinfo = ((SOCKADDR_IN6 *)(s))->sin6_scope_id = 0;}}

namespace WSocket
{
  /*
    Инициализация.
    
    Return - true - в случаи успеха,
             false - в случаи ошибки.
  */
  bool init(void);

  /*
    Деинициализация.
  */
  void uninit(void);

  /*
    Проверяет присутвует ли сокет в списке fd_set.

    IN s   - сокет для проверки.
    IN pfs - указатель на fd_set.

    Return - true - если присутвует,
             false - если отсутсвует.
  */
  bool _fdIsSet(SOCKET s, fd_set *pfs);
  
  /*
    Чтение данных из сокета.

    IN s       - сокет.
    OUT buf    - буфер для данных.
    IN bufSize - размер буфера.
    IN timeout - количество миллесекунд ожидания получения данных, или 0 для бесконечного
                 ожидания на блокируемом сокете. Или INFINITE для бесконечного ожидания на не
                 блокируемом сокете.

    Return     - количетсво прочитаных байт или SOCKET_ERROR.
  */
  int tcpRecv(SOCKET s, void *buf, int bufSize, DWORD timeout);

  /*
    Чтение данных из сокета до полного запаолнения буфера.

    IN s       - сокет.
    OUT buf    - буфер для данных.
    IN bufSize - размер буфера.
    IN timeout - количество миллесекунд ожидания получения данных,  или 0 для бесконечного
                 ожидания на блокируемом сокете. Или INFINITE для бесконечного ожидания на не
                 блокируемом сокете.

    Return     - true - в случаи успеха,
                 false - в случаи ошибки.
  */
  bool tcpRecvAll(SOCKET s, void *buf, int bufSize, DWORD timeout);

  /*
    Чтение всех данных из сокета в NULL.

    IN s       - сокет.
    IN size    - размер данных.
    IN timeout - количество миллесекунд ожидания получения данных, или 0 для бесконечного
                   ожидания.

    Return     - true - в случаи успеха,
                 false - в случаи ошибки.
  */
  bool tcpRecvAllToNull(SOCKET s, int size, DWORD timeout);

  /*
    Запись данных в сокет.

    IN s       - сокет.
    IN buf     - буфер с данными.
    IN bufSize - размер буфера.

    Return     - true - в случаи успеха,
                 false - в случаи ошибки.
  */
  bool tcpSend(SOCKET s, const void *buf, int bufSize);

  /*
    Создание TCP соединения.
    
    IN sockAddr - данные сервера, к которому нужно подключится.

    Return      - ID сокета - в случаи успешного соединения, после окончания сессии сокет должен
                  быть закрыт через tcpClose,
                  INVALID_SOCKET - в случаи ошибки.
  */
  SOCKET tcpConnect(const SOCKADDR_STORAGE *sockAddr);
  
  /*
    Создание TCP соединения.
    
    IN host - домен/IPv4/IPv6.
    IN port - порт.

    Return  - ID сокета - в случаи успешного соединения, после окончания сессии сокет должен
              быть закрыт через tcpClose,
              INVALID_SOCKET - в случаи ошибки.
  */
  SOCKET tcpConnectA(const LPSTR host, const WORD port);

  /*
    Создает TCP порт для приема соединений.
    
    IN sockAddr - данные сервера.
    IN backlog  - максимальное количетсво ожидлающих соединений или SOMAXCONN.
    
    Return      - ID сокета - в случаи успешного создания порта, когда порт более не нужен,
                  необходимо закрыть через tcpClose,
                  INVALID_SOCKET - в случаи ошибки.
  */
  SOCKET tcpListen(const SOCKADDR_STORAGE *sockAddr, int backlog);
  
  /*
    Создает TCP порт для приема соединений на произвольном порту.
    
    IN family  - тип протокола, AF_INET или AF_INET6.
    IN backlog - максимальное количетсво ожидлающих соединений или SOMAXCONN.
    IN portMin - минимальный порт в машинном порядке байт.
    IN portMax - максимальный порт в машинном порядке байт.

    Return     - ID сокета - в случаи успешного создания порта, когда порт более не нужен,
                 необходимо закрыть через tcpClose,
                 INVALID_SOCKET - в случаи ошибки.
    
    Примечание: Требует инициализации Crypt.
  */
  SOCKET tcpListenRandom(int family, int backlog, WORD portMin, WORD portMax);

  /*
    Насдтройка для TCP_Listen для простого создания порта.
    
    IN family  - тип протокола, AF_INET или AF_INET6.
    IN port    - порт в машинном порядке байт.
    IN backlog - максимальное количетсво ожидлающих соединений или SOMAXCONN.

    Return     - ID сокета - в случаи успешного создания порта, когда порт более не нужен,
                 необходимо закрыть через tcpClose,
                 INVALID_SOCKET - в случаи ошибки.
  */
  SOCKET tcpListenEx(int family, WORD port, int backlog);
  
  /*
    Организация тунеля между двумя соединениями.
    
    IN s1 - первый сокет.
    IN s2 - второй сокет.

    Примичание: выход из функции будет произведен только после перевания соединения или ошибки.
  */
  void tcpTunnel(SOCKET s1, SOCKET s2);
  
  /*
    Организация тунеля между двумя соединениями, с возможностью прерывания по событию.
    
    IN s1          - первый сокет.
    IN s2          - второй сокет.
    IN eventHandle - событие, при срабатывание которого тунель будет закрыт.
  */
  void tcpTunnelAndWaitForWinEvent(SOCKET s1, SOCKET s2, HANDLE eventHandle);

  /*
    Ожидание события read на сокетах.

    IN sockets         - массив сокетов на которых ожидается событие.
    IN socketsCount    - количество элементов в массиве sockets.
    IN timeout         - количество миллесекунд ожидания, или 0 для бесконечного ожидания.
    IN advSockets      - массив дополнительных сокетов для слежения на событие read.
    IN advSocketsCount - количество элементов в массиве advSockets.

    Return             - ID сокета на котором произошло событие,
                         INVALID_SOCKET - в случаи ошибки, таймауа, или срабатывания события
                         на любом сокете sAdvSockets.

    Прмечание: В случаи таймаута, WSAGetLastError() будет возращать WSAETIMEDOUT.
  */
  SOCKET tcpWaitForEvent(const SOCKET *sockets, DWORD socketsCount, DWORD timeout, const SOCKET *advSockets, DWORD advSocketsCount);

  /*
    Ожидание входящего соединения и получение нового сокета.
    
    IN sockets         - массив сокетов на которых ожидается соединение.
    IN socketsCount    - количество элементов в массиве sockets.
    IN timeout         - количество миллесекунд ожидания, или 0 для бесконечного ожидания.
    OUT sockAddr       - буфер для информации о клиенте, или NULL если информация не нужна.
    OUT index          - индекст в массиве sockets, для которого сработало событие. Может быть
                         NULL.
    IN advSockets      - массив дополнительных сокетов для слежения на событие read.
    IN advSocketsCount - количество элементов в массиве advSockets.
    
    Return             - Cокет - в случаи успешного соединения, когда сокет более не нужен,
                         необходимо закрыть через tcpClose,
                         INVALID_SOCKET - в случаи ошибки, таймауа, или срабатывания события
                         на любом сокете advSockets.

    Прмечание: В случаи таймаута, WSAGetLastError() будет возращать WSAETIMEDOUT.
  */
  SOCKET tcpWaitForIncomingAndAccept(SOCKET *sockets, DWORD socketsCount, DWORD timeout, SOCKADDR_STORAGE *sockAddr, LPDWORD index, SOCKET *advSockets, DWORD advSocketsCount);

  /*
    Закрытие и освобождение сокета.

    IN s - сокет для закрытия. Возможно значение INVALID_SOCKET.
  */
  void tcpClose(SOCKET s);

  /*
    Включает или выключает алгоритм Nagle(TCP_NODELAY). ПО умолчанию включен для всех сокетов.

    IN s       - сокет.
    IN disable - true - отключить алгоритм.
                 false - включить алгоритм,
    
    Return     - true - в случаи спешного изменения состояния,
                 false - в случаи ошибки.
  */
  bool tcpDisableDelay(SOCKET s, bool disable);

  /*
    Включает или выключает TCP keepalive.

    IN s        - сокет.
    IN enable   - true - включить.
                 false - отключить,
    IN time     - переуд неактивности в мс, через которое отслыается пакет.
    IN interval - интервал отсылки пакетов, если не дошел предыдущий.
                
               
    Return      - true - в случаи спешного изменения состояния,
                  false - в случаи ошибки.
  */
  bool tcpSetKeepAlive(SOCKET s, bool enable, DWORD time, DWORD interval);

  /*
    Выбор режима блокируемого, неблокируемого сокета.

    IN s       - сокет.
    IN disable - true -  неблокируемый,
                 false - блокируемый.
    
    Return     - true - в случаи спешного изменения состояния,
                 false - в случаи ошибки.

  */
  bool setNonBlockingMode(SOCKET s, bool enable);
  
  /*
    Создает UDP порт для приема соединений.

    IN sockAddr - данные сервера.

    Return      - ID сокета - в случаи успешного создания порта, когда порт более не нужен,
                  необходимо закрыть через udpClose,
                  INVALID_SOCKET - в случаи ошибки.
  */
  SOCKET udpListen(const SOCKADDR_STORAGE *sockAddr);

  /*
    Закрытие и освобождение сокета.

    IN s - сокет для закрытия. Возможно значение INVALID_SOCKET.
  */
  void udpClose(SOCKET s);

  /*
    Преобразование IP в строку ip:port.

    IN sockAddr - данные IP.
    OUT buffer  - буфер для строки, должен быть размером не менее MAX_PATH.
    
    Return      - true - в случаи успешной конвертации,
                  false - в случи ошибки, при этом string будет ранвятся "0:0".
  */
  bool ipToStringW(const SOCKADDR_STORAGE *sockAddr, LPWSTR buffer);

  /*
    Преобразование строки в IP.

    OUT sockAddr - данные IP.
    IN buffer    - строка.

    Return       - true - в случаи успешной конвертации,
                   false - в случви ошибки, при этом sockAddr будет обнулен.
  */
  bool stringToIpW(SOCKADDR_STORAGE *sockAddr, const LPWSTR string);

  /*
    Проверяет, является ли IP локальным.
    
    IN sockAddr - данные IP.

    Return      - true - если локальный,
                  false - если не локальный.
  */
  bool _isLocalIp(const SOCKADDR_STORAGE *sockAddr);

  /*
    Получение списка IP-адресов всех интерфейсов.

    IN family - AF_INET или AF_INET6.

    Return    - NULL - в случаи ошибки,
                или адрес списка в случаи успеха (нужно освободить через Mem).
  */
  SOCKET_ADDRESS_LIST *getListOfIpAddresses(int family);

  /*
    Получение типа сокета AF_*.

    IN s   - сокет.
    
    Return - AF_*.
  */
  USHORT getFamily(SOCKET s);
};
