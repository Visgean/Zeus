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

  /*
    Создание интерфейса, надстройка над CoCreateInstance.

    IN clsid - CLSID.
    IN iid   - IID.

    Return   - указатель на интерфейс, или NULL - в случаи ошибки.
  */
  void *_createInterface(REFCLSID clsid, REFIID iid);
};
