/*
  Сборка бота.
*/
#pragma once

#include "..\common\config0.h"

namespace BuildBot
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
    Запуск сборки бота.

    IN owner      - оснвоное окно.
    IN output     - окно для вывода статуса.
    IN config     - конфигурация.
    IN destFolder - папка назначения для конечного файла.

    Return        - true - в случаи успеха,
                    false - в случаи ошибки.
  */
  bool _run(HWND owner, HWND output, Config0::CFGDATA *config, LPWSTR destFolder);
};
