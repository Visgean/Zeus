/*
  Работа с консолью.
*/
#pragma once

namespace Console
{
  /*
    Инициализация.

    Return - true - в случаи успеха,
             false - в случаи ошибки.
  */
  bool init(void);

  /*
    Деинициализация.
  */
  void uninit(void);

  /*
    Запись данных в консоль.

    IN data - данные для записи.
    IN size - количество байт для записи.

    Return  - количество записанных байт.
  */
  DWORD writeData(void *data, DWORD size);

  /*
    Запись Unicode строки в консоль.

    IN string - строка для записи.
    IN size   - количество символов для записи, или (DWORD)-1 если строка оканчивается на \0.

    Return    - количество записанных символов.
  */
  DWORD writeStringW(LPWSTR string, DWORD size);

  /*В В В В Analogue of wprintf.

В В В В IN format - line-format output.
В В В В IN ... - Arguments for the row.

В В В В Return - number of characters written.
В В */
  DWORD writeFormatW(LPWSTR format, ...);
};
