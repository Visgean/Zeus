/*
  Работа с COM.
*/
#pragma once

namespace ComLibrary
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
    Инициализация библиотеки для потока.

    OUT result - результат инициализации, не можут быть NULL. Должен быть передан в парную
                 _uninitThread().

    Return     - true - в случаи успеха,
                 false - в случаи ошибки.
  */
  bool _initThread(HRESULT *result);

  /*
    Деинициализация библиотеки для потока. Функцию можно вызывать (необязательно) даже в случаи 
    ошибки _initThread(), но с верным значением initResult.
    
    IN result - результат инициализации от _initThread().
  */
  void _uninitThread(HRESULT initResult);

  /*В В В В Creating an interface add-on CoCreateInstance.

В В В В IN clsid - CLSID.
В В В В IN iid - IID.

В В В В Return - the interface pointer, or NULL - otherwise.
В В */
  void *_createInterface(REFCLSID clsid, REFIID iid);
};
