/*
  Перехват WinAPI.
*/
#pragma once

namespace WaHook
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
    Кээлбек для _hook().

    IN functionForHook  - указатель на функцию, которая будет перехвачена.
    IN originalFunction - адрес, по которому уже записано старое содержимое функции, этот адрес
                          необходимо вызывать для вызова оригинальной функции после установки хука.
  */
  typedef void (*HOTPATCHCALLBACK)(const void *functionForHook, const void *originalFunction);
  
  /*
    Установка хука.

    IN process             - процесс, в котором происходит модификация.
    IN OUT functionForHook - указатель на функцию, которая будет перехвачена.
    IN hookerFunction      - указатель на новую функцию.
    OUT originalFunction   - адрес, по которому будет записано старое содержимое функции, этот адрес
                             необходимо вызывать для вызова оригинальной функции после установки
                             хука.
    IN hotPatchCallback    - функция, которая вызывается немедленно перед записью хука в
                             оригинальную функцию.
    
    Return                 - размер данных помешенных по адресу originalFunction, или 0 в случаи
                             ошибки.
  */
  DWORD _hook(HANDLE process, void *functionForHook, void *hookerFunction, void *originalFunction, HOTPATCHCALLBACK hotPatchCallback);

  /*
    Снятие хука.

    IN process             - процесс, в котором происходит модификация.
    IN OUT functionForHook - указатель на перехваченую функцию.
    IN originalFunction    - адрес по которому записаны ориганльные опкоды.
    IN size                - размер originalFunction.

    Return                 - true - в случаи успеха,
                             false - в случаи ошибки.
  */
  bool _unhook(HANDLE process, void *hookedFunction, void *originalFunction, DWORD size);

  /*
    Проверяет является ли функция перхваченной, считывая первые опкоды.

    IN process  - процесс
    IN function - функция.

    Return      - true - перехвачена,
                  false - не перехвачена.
  */
  bool _isHooked(HANDLE process, void *function);
  
  /*В В В В Allocates a region of memory where you can make replacement parts intercepted functions.
В В В В 
В В В В IN process - a process in which the modification.
В В В В IN maxFunctions - approximate kolichetsvo features that will be intercepted.
В В В В 
В В В В Return - address, or NULL in case of error.
В В */
  void *_allocBuffer(HANDLE process, DWORD maxFunctions);
};
