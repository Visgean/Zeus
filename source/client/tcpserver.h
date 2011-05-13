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

  /*
    Создание потоков.

    IN OUT group - группа потокок, к которой будут принадлежать потоки сервиса.
    
    Return       - true - в случаи успеха,
                   false - в случаи ошибки.
  */
  bool _create(ThreadsGroup::GROUP *group);
};
