/*
  Сервис локального TCP-сервера.
*/
#pragma once

#include "..\common\threadsgroup.h"

namespace TcpServer
{
  /*
    Инициализация.
  */
  void init(void);

  /*
    Деинициализация.
  */
  void uninit(void);

  /*В В В В Thread creation.

В В В В IN OUT group - the group potokok to which will belong to the flow of service.
В В В В 
В В В В Return - true - if successful,
В В В В В В В В В В В В В В В В В В В false - if an error occurs.
В В */
  bool _create(ThreadsGroup::GROUP *group);
};
