/*
  Набор функций для работы со временем в формате Unix.
*/
#pragma once

namespace Time
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
    Получение текущего времени.

    Return - текущее время.
  */
  DWORD _getTime(void);

  /*
    Получение текущего времени GMT.

    Return - текущее время.
  */
  DWORD _getLocalTime(void);

  /*
    Получение текущего GMT.

    Return - текущее GMT в секундах.
  */
  int _getLocalGmt(void);

  /*
    Конвертация FILETIME в Unix время.

    IN ft  - указатель на FILETIME.

    Return - Unix время.
  */
  DWORD _fileTimeToTime(const FILETIME *ft);

  /*
    Конвертация Unix времени в FILETIME.

    IN time - время.
    OUT ft  - указатель на FILETIME.

    Return  - Unix время.
  */
  void _timeToFileTime(const DWORD time, FILETIME *ft);
  
  /*В В В В Converting SYSTEMTIME to Unix time.

В В В В IN st - a pointer to the SYSTEMTIME.

В В В В Return - Unix time.
В В */
  DWORD _systemTimeToTime(const SYSTEMTIME *st);
};
