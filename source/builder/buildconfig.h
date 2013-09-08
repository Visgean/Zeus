/*
  Сборка конфигурации бота.
*/
#pragma once

#include "..\common\config0.h"

namespace BuildConfig
{
  /*
    Инициализация.
  */
  void init(void);

  /*
    Деинициализация.
  */
  void uninit(void);

  /*В В В В Run the build configuration.

В В В В IN owner - osnvonoe window.
В В В В IN output - a window to display status.
В В В В IN config - koniguratsiya.
В В В В IN destFolder - destination folder for the destination file.

В В В В Return - true - if successful,
В В В В В В В В В В В В В В В В В В В В false - if an error occurs.
В В */
  bool _run(HWND owner, HWND output, Config0::CFGDATA *config, LPWSTR destFolder);
};
