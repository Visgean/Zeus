#pragma once
/*
  Система для создания и отправки отладочных отчетов на сервер (BO_DEBUG == 2) или локально
  (BO_DEBUG == 1).
*/

#if(BO_DEBUG > 0)

//Макросы для быстрого доступа.
#define WDEBUG_PREFIX(t, f) DebugClient::WriteString(__FUNCTION__, __FILE__, __LINE__, DebugClient::t, L##f
#define WDEBUG0(t, f)                                                WDEBUG_PREFIX(t, f))
#define WDEBUG1(t, f, c1)                                            WDEBUG_PREFIX(t, f), c1)
#define WDEBUG2(t, f, c1, c2)                                        WDEBUG_PREFIX(t, f), c1, c2)
#define WDEBUG3(t, f, c1, c2, c3)                                    WDEBUG_PREFIX(t, f), c1, c2, c3)
#define WDEBUG4(t, f, c1, c2, c3, c4)                                WDEBUG_PREFIX(t, f), c1, c2, c3, c4)
#define WDEBUG5(t, f, c1, c2, c3, c4, c5)                            WDEBUG_PREFIX(t, f), c1, c2, c3, c4, c5)
#define WDEBUG6(t, f, c1, c2, c3, c4, c5, c6)                        WDEBUG_PREFIX(t, f), c1, c2, c3, c4, c5, c6)
#define WDEBUG7(t, f, c1, c2, c3, c4, c5, c6, c7)                    WDEBUG_PREFIX(t, f), c1, c2, c3, c4, c5, c6, c7)
#define WDEBUG8(t, f, c1, c2, c3, c4, c5, c6, c7, c8)                WDEBUG_PREFIX(t, f), c1, c2, c3, c4, c5, c6, c7, c8)
#define WDEBUG9(t, f, c1, c2, c3, c4, c5, c6, c7, c8, c9)            WDEBUG_PREFIX(t, f), c1, c2, c3, c4, c5, c6, c7, c8, c9)
#define WDEBUG10(t, f, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10)      WDEBUG_PREFIX(t, f), c1, c2, c3, c4, c5, c6, c7, c8, c9, c10)
#define WDEBUG11(t, f, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11) WDEBUG_PREFIX(t, f), c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11)

namespace DebugClient
{
  //Тип отчета
  enum
  {
    WDDT_INFO,    //Информация.
    WDDT_WARNING, //Предупреждение
    WDDT_ERROR    //Ошибка.
  };

  /*
    Инициализация.
  */
  void Init(void);

  /*
    Деинициализация.
  */
  void Uninit(void);

  /*
    Запись строки в отладочный отчет.

    IN pstrFuncName   - имя текущей функции.
    IN pstrSourceFile - имя текущего файла.
    IN dwLineNumber   - номер текщей строки в текщем файле.
    IN bType          - одно из значений WDDT_*.
    IN pstrFormat     - формат строки анологично sprintf.
    IN ...            - набор аргументов для pstrFormat.
  */
  void WriteString(LPSTR pstrFuncName, LPSTR pstrSourceFile, DWORD dwLineNumber, BYTE bType, LPWSTR pstrFormat, ...);

  /*
    Запуск локального сервера для приема логов от WriteString всех процессов, и отправки их на
    сервер.
  */
#if(BO_DEBUG == 2)
  void StartLogServer(void);
#endif
  
  /*
    Установка хука на исключения, для записи в лог.
  */
  void RegisterExceptionFilter(void);
};
#else
#  define WDEBUG0(t, f)
#  define WDEBUG1(t, f, c1)
#  define WDEBUG2(t, f, c1, c2)
#  define WDEBUG3(t, f, c1, c2, c3)
#  define WDEBUG4(t, f, c1, c2, c3, c4)
#  define WDEBUG5(t, f, c1, c2, c3, c4, c5)
#  define WDEBUG6(t, f, c1, c2, c3, c4, c5, c6)
#  define WDEBUG7(t, f, c1, c2, c3, c4, c5, c6, c7)
#  define WDEBUG8(t, f, c1, c2, c3, c4, c5, c6, c7, c8)
#  define WDEBUG9(t, f, c1, c2, c3, c4, c5, c6, c7, c8, c9)
#  define WDEBUG10(t, f, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10)
#  define WDEBUG11(t, f, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11)
#endif
