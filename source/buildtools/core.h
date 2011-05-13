/*
  Ядро.
*/
#pragma once

//Структура для хранения основных данных процесса.
typedef struct
{
  DWORD exitCode;           //Код выхода
  struct
  {
    int argsCount;          //Количество аргументов в командной строке.
    LPWSTR *args;           //Массив командной строки.
  }commandLine;
  WCHAR fileName[MAX_PATH]; //Текущие имя файла.
}COREDATA;
extern COREDATA coreData;

namespace Core
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
    Вывод логотипа.
  */
  void showLogo(void);
};
