/*
  Интсрументы для обработки данных HTTP.
*/
#pragma once

namespace HttpTools
{
  enum
  {
    GMH_HTTP_METHOD,                                      //Тип HTTP-запроса.
    GMH_HTTP_URI,                                         //HTTP-URI
    GMH_REQUEST_HTTP_VERSION,                             //HTTP-версия.
    GMH_RESPONSE_HTTP_VERSION = GMH_HTTP_METHOD,          //HTTP-версия.
    GMH_RESPONSE_STATUS       = GMH_HTTP_URI,             //HTTP-статус.
    GMH_RESPONSE_STATUSTEXT   = GMH_REQUEST_HTTP_VERSION, //Тексотвый HTTP-статус.
    GMH_DATA,                                             //Данные идушие после заголовков.
    GMH_COUNT
  };

  //Тип соединения для URLDATA.
  enum
  {
    UDS_UNKNOWN,
    UDS_HTTP,
    UDS_HTTPS
  };

  //Описание URL.
  typedef struct
  {
    LPSTR host;           //Хост.
    LPSTR uri;            //URI. Всегда будет начинаться с '/'.
    DWORD uriExtraOffset; //Позиция начала до данных в uri. (? или #).
    WORD port;            //Порт.
    BYTE scheme;          //Тип UDS_*.
  }URLDATA;

  /*
    Инициализация.
  */
  void init(void);

  /*
    Деинициализация.
  */
  void uninit(void);

  /*
    Получение значение переменной из "x-www-form-urlencoded" или URI (смешение до ? не происходит).

    IN list     - список переменных.
    IN listSize - размер списка в байтах
    IN varable  - переменная для поиска, в URL-кодировке.
    OUT valSize - размер значения перменной в байтах..

    Return      - адрес переменной, или NULL если переменная не найдена.
  */
  LPSTR _findValueInUrlEncoded(const void *list, DWORD listSize, const LPSTR varable, LPDWORD valSize);
  
  /*
    Парсинг URL. После использования необходимо освободить через _freeUrlData().

    IN url - URL строка для парсинга.
    OUT ud - результат парсига.

    Return - true - в случаи успеха,
             false - в случаи ошибки.
  */
  bool _parseUrl(const LPSTR url, URLDATA *ud);

  /*
    Освобождение URLDATA.

    IN ud - URLDATA для освобождения.
  */
  void _freeUrlData(URLDATA *ud);

  /*
    Получение имени файла из URL с раскодированием %XX и конрвертацией из UTF8.

    IN url - URL.

    Return - NULL - в случаи ошибки,
             имя файла в случаи успеха (нужно освободить через Mem).
  */
  LPWSTR _getFileNameFromUrl(const LPSTR url);

  /*
    Сравнивает имя тега со строкой. 

    IN tag     - имя тэга дял сраванения.
    IN tagSize - размер pTag.
    IN string  - строка для сравнения. Строка может начинаться как с символа '<' или '</', так и
                 сразу с имени тега.

    Return    - true - если тэг совпадает со строкой.
                false - если тэг не совпадает со строкой.
  */
  bool _compareTagNameA(const LPSTR tag, DWORD tagSize, const LPSTR string);

  /*
    Удаляет тэги из буфера.

    IN OUT string - буфер для обработки.
    IN size       - размер буфера.

    Return        - количетсво байт оставленных в буфере, исключая конечный символ '\0'. 
  */
  DWORD _removeTagsA(LPSTR string, DWORD size);
  
  /*
    FIXME:UrlUnescape
    Кодирует данные согласно URL кодировки.

    IN input     - данные для кодирования.
    IN inputSize - размер данных для кодирования.
                     символом '\0'.
    OUT output   - буфер для кодированых данных, размер буфера должен быть не менее
                   (dwInputSize * 3) + 1, для предотврашения переполнения.

    Return: количетсво байт скопированных в output, исключая конечный символ '\0'.
  */
  DWORD _urlEncodeA(LPSTR input, DWORD inputSize, LPSTR output);

  /*
    Кодирует данные согласно URL кодировки.

    IN input     - данные для кодирования.
    IN inputSize - размер данных для кодирования.
                     символом '\0'.
    OUT output   - буфер для кодированых данных, размер буфера должен быть не менее
                   (dwInputSize * 3) + 1, для предотврашения переполнения.

    Return: количетсво байт скопированных в output, исключая конечный символ '\0'.
  */
  DWORD _urlEncodeW(LPWSTR input, DWORD inputSize, LPWSTR output);

  /*
    Кодирует данные согласно URL кодировки, вызывает URLEncodeA.

    IN input     - данные для кодирования.
    IN inputSize - размер данных для кодирования.

    Return: кодированные данные (нужно совободжидить через Mem::free()),
            NULL - в случаи ошибки.
  */
  LPSTR _urlEncodeExA(LPSTR input, DWORD inputSize);

  /*
    FIXME:UrlUnescape
    Кодирует данные согласно URL кодировки, вызывает URLEncodeW.

    IN input     - данные для кодирования.
    IN inputSize - размер данных для кодирования.

    Return: кодированные данные (нужно совободжидить через Mem::free()),
            NULL - в случаи ошибки.
  */
  LPWSTR _urlEncodeExW(LPWSTR input, DWORD inputSize);

  /*
    Парсер Mime-заголовков.

    IN mimeData - данные с Mime-заголовком.
    IN mimeSize - размер mimeData.
    IN header   - значение для получения, может принимать значения GMH_*.
    OUT size    - размер возрашемого значения, может быть 0, если содержимое не определено.

    Return      - содержимое для pstrHeader, на выходе будет NULL, если значения не сущетвует
                  или произошла ошибка (*pdwSize также будет равна 0).
  */
  LPSTR _getMimeHeader(const void *mimeData, DWORD mimeSize, const LPSTR header, SIZE_T *size);

  /*
    Удаление заголовка из MIME-заголовка.
    
    IN OUT mimeData - данные с Mime-заголовком.
    IN mimeSize     - размер mimeData.
    IN header       - значение для получения, НЕ может быть GMH_*.

    Return          - новый размер MIME-заголовка.(Данные идущее после \r\n\r\n сохраняются.)
  */
  DWORD _removeMimeHeader(void *mimeData, DWORD mimeSize, LPSTR header);

  /*
    Модицикация заголовка.

    IN OUT mimeData - данные с Mime-заголовком.
    IN mimeSize     - размер mimeData.
    IN header       - значение для получения, НЕ может быть GMH_*.
    IN context      - значение заголовк. Может быть NULL.
    
    Return          - новый размер MIME-заголовка.(Данные идущее после \r\n\r\n сохраняются.)
  */
  DWORD _modifyMimeHeader(void *mimeData, DWORD mimeSize, const LPSTR header, const LPSTR context);

  /*
    Чтение одного chunk-блока.

    IN data             - начала блока.
    IN dataSize         - размер известных байт.
    OUT chunkedData     - начало данных. NULL - если данные достпуны не полностью.
    OUT chunkedDataSize - размер данных.

    Return              - NULL - ошибка в формате,
                          указатель на следующий байт после чтекущего chunk'а.
  */
  void *_readChunkedData(const void *data, DWORD dataSize, void **chunkedData, LPDWORD chunkedDataSize);

  /*
    Добавление URL-параметров в другоую URL.

    IN source - исходная URL, параметры которой будут скопированы.
    IN dest   - конечная URL, в конец которой параметры будут добавлены. (Параметры могут уже
                существовать).
    
    Return    - новая URL (нужно освободить через Mem), или NULL в случаи ошибки.
  */
  LPSTR _catExtraInfoFromUrlToUrlA(const LPSTR source, const LPSTR dest);

  /*
    Декодирование строки HTTP-авторизации.

    IN source     - исходная строка.
    IN sourceSize - размер source.
    OUT userName  - имя пользователя, нужно освободить через Mem.
    OUT password  - пароль, нужно освободить через Mem.

    Return        - true - в случаи успешного декодировния,
                    false - в случаи ошибки.
  */
  bool _parseAuthorization(const LPSTR source, DWORD sourceSize, LPWSTR *userName, LPWSTR *password);
};
