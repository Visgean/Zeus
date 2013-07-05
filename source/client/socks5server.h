/*
  Socks5 IPv4/IPv6 сервер без поддержки аутентификации.

  Примечание: при команде BIND входящее соединение ожидается SOCKS5_BIND_TIMEOUT миллисекунд.
*/
#pragma once

namespace Socks5Server
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
    Запуск сессии Socks 4 (версия читается из сокета до вызова).
    
    IN s       - сокет, после окончания сессиии сокет НЕ будет закрыт.
    IN timeout - таймаут сокета в миллисекундах, 0 для неограниченого ожидания.

    Return     - true - в случаи проведения сессии без ошибок протокола и обрывов связи,
                 false - в случаи возникновения ошибок в протоколе или обрывов связи.
  */
  bool _start5(SOCKET s, DWORD timeout);

  /*
    Запуск сессии Socks 5 (версия читается из сокета до вызова).
    
    IN s       - сокет, после окончания сессиии сокет НЕ будет закрыт.
    IN timeout - таймаут сокета в миллисекундах, 0 для неограниченого ожидания.

    Return     - true - в случаи проведения сессии без ошибок протокола и обрывов связи,
                 false - в случаи возникновения ошибок в протоколе или обрывов связи.
  */
  bool _start4(SOCKET s, DWORD timeout);

  /*В В В В Starting the session, with auto determine a Socks.
В В В В 
В В В В IN s - the socket after the session socket will be closed.
В В В В IN timeout - timeout of the socket, in milliseconds, 0 for unlimited expectations.

В В В В Return - true - in the case of an error-free session protocol, and disconnected,
В В В В В В В В В В В В В В В В В false - if there are errors in the protocol or connection failure.
В В */
  bool _start(SOCKET s, DWORD timeout);
};
