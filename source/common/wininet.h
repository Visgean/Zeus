#pragma once
/*
  Инструменты для работы с Wininet.
*/

//The maximum amount of data allocated for skachvanie in memory.
#define WININET_MAXBYTES_TO_MEM  (10 * 1024 * 1024)

#include "mem.h"

namespace Wininet
{
  //Structure for CallURL.
  typedef struct
  {
    bool bAutoProxy;                   //Make attempts to connect with WICF_USE_IE_PROXY, then without it, for each attempt.
    BYTE bTryCount;                    //The number of attempts, at a minimum should be 1.
    DWORD dwRetryDelay;                //Delay Mezhuyev connections.
    HANDLE hStopEvent;                 //Signal priryvaniya.
    LPSTR pstrUserAgent;               //UserAgent.
    LPSTR pstrURL;                     //URL.

    DWORD Connect_dwFlags;             //Flags WICF_ *.

    LPSTR SendRequest_pstrReferer;     //Referel
    void *SendRequest_pPostData;       //Post-data.
    DWORD SendRequest_dwPostDataSize;  //The size of the settlement data.
    DWORD SendRequest_dwFlags;         //Flags WISRF_ *.

    DWORD DownloadData_dwSizeLimit;    //Limit on downloaded data.
    LPWSTR DownloadData_pstrFileName;  //If not NULL, then the downloading of data occurs in this file.
  }CALLURLDATA;

  //Flags for _Connect.
  enum
  {
    WICF_USE_IE_PROXY = 0x1 //Whether to use when connecting from the proxy settings IE.
  };

  //Flags for SendRequest.
  enum
  {
    WISRF_METHOD_GET      = 0x0, //Use the method GET. (DEFAULT)
    WISRF_METHOD_POST     = 0x1, //Use the method POST.
    WISRF_KEEP_CONNECTION = 0x2, //Maintain the connection after vypoleneniya request. This option is not
                                 //guarantees the preservation of the connection, it depends on the server.
    WISRF_IS_HTTPS        = 0x4 //This query is performed over HTTPS protocol.
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
    Подключение к серверу по HTTP/HTTPS протоколу.

    IN pstrUserAgent - User-Agent, если NULL, то используюется значение по умолчанию.
    IN pstrHost      - адрес сервера, IP или домен.
    IN wPort         - порт сервера.
    IN dwFlags       - флаги WICF_*.

    Return           - хэндл соединения или NULL.
  */
  HINTERNET _Connect(LPSTR pstrUserAgent, LPSTR pstrHost, WORD wPort, DWORD dwFlags);

  /*
    Закрытие соединения, открытого при помощи _Connect.

    IN hConnect - хэндл соединения.
  */
  void _CloseConnection(HINTERNET hConnect);

  /*
    Закрытие хэндла, и всех его родителей.

    IN OUT handle - хэндл.
  */
  void _closeWithParents(HINTERNET handle);

  /*
    Вызов InternetStatusCallback.

    ... -  согласно InternetStatusCallback.

    Return - true - функция вызвана,
             false - функция не вызвана.
  */
  bool callCallback(HINTERNET handle, DWORD_PTR context, DWORD internetStatus, LPVOID statusInformation, DWORD statusInformationLength);

  /*
    Отправка запроса.

    IN hConnect       - хэндл соединения.
    IN pstrURI        - запрашаваемый объект.
    IN pstrReferer    - реферел или NULL.
    IN pPostData      - пост данные или NULL.
    IN dwPostDataSize - размер pPostData.
    IN dwFlags        - Flags WISRF_ *.

    Return            - в случаи успеха хэнло полученый от HttpOpenRequest, или NULL в случаи ошибки.
  */
  HINTERNET _SendRequest(HINTERNET hConnect, LPSTR pstrURI, LPSTR pstrReferer, void *pPostData, DWORD dwPostDataSize, DWORD dwFlags);

  /*
    Скачивание данных.

    IN hRequest    - хэндл запроса.
    OUT pBuf       - буффер для данных, после использование необходимо удалить через Mem. Может
                     быть NULL.
    IN dwSizeLimit - лимит скачиваемых байт, если равно 0 или больше WININET_MAXBYTES_TO_MEM, то
                     dwSizeLimit =  WININET_MAXBYTES_TO_MEM;
    IN hStopEvent  - хэндл события для прерывания скачивания или NULL.

    Return         - true - в случаи успеха,
                     false - в случаи ошибки, в том числе срабатываении hStopEvent и привышения
                     dwSizeLimit. Если возникла ошибка, то соединение необходимо закрыть.
  */
  bool _DownloadData(HINTERNET hRequest, MEMDATA *pBuf, DWORD dwSizeLimit, HANDLE hStopEvent);
  
  /*
    Скачивание данных в файл.

    IN hRequest     - хэндл запроса.
    IN pstrFileName - имя файла, в который будут сохранены данные.
    IN dwSizeLimit  - лимит скачиваемых байт, если равно 0, то ограничения нет.
    IN hStopEvent   - хэндл события для прерывания скачивания или NULL.

    Return          - true - в случаи успеха,
                      false - в случаи ошибки, в том числе срабатываении hStopEvent или привышения
                      dwSizeLimit. Если возникла ошибка, то соединение необходимо закрыть.
  */
  bool _DownloadDataToFile(HINTERNET hRequest, LPWSTR pstrFileName, DWORD dwSizeLimit, HANDLE hStopEvent);

  /*
    Оболочка для _Connect + _SendRequest + _DownloadData(ToFile).

    IN pcud  - параметры URL;
    OUT pBuf - буффер для данных, после использование необходимо удалить через Mem. Может быть NULL.
               При pcud->DownloadData_pstrFileName != NULL не имеет смысла.

    Return   - true - в случаи успеха,
               false - в случаи ошибки.
  */
  bool _CallURL(CALLURLDATA *pcud, MEMDATA *pBuf);

  /*
    Получение User-Agent от Internet Explorer.

    Return - User-Agent, или NULL в случаи оишбки. Память должна быть освобождена через Mem.
  */
  LPSTR _GetIEUserAgent(void);

  /*
    Тест сокрости загрузки URL.

    IN url       - URL.
    IN stopEvent - событие остановки или NULL.

    Return       - затраченое время в ms, или 0 - в случаи ошибки.
  */
  DWORD _testDownloadDelay(LPSTR url, HANDLE stopEvent);

  /*
    Настройка над InternetQueryOptionA с автовыледением памяти.

    IN internet - хэндл.
    IN option   - опция.
    OUT lenght  - размер данных. Если это строка, размер будет возрашен без нулеового символа.

    Return      - данные (нужно освободить через Mem), если это строка, то он будет с окончанием на
                  нулевой символ. Или NULL в случаи ошибки.
  */
  void *_queryOptionExA(HINTERNET internet, DWORD option, LPDWORD lenght);

  /*
    Настройка над InternetQueryOptionW с автовыледением памяти.

    IN internet - хэндл.
    IN option   - опция.
    OUT lenght  - размер данных в БАЙТАХ. Если это строка, размер будет возрашен без нулеового символа.

    Return      - данные (нужно освободить через Mem), если это строка, то он будет с окончанием на
                  нулевой символ. Или NULL в случаи ошибки.
  */
  void *_queryOptionExW(HINTERNET internet, DWORD option, LPDWORD lenght);

  /*В В В В Customize over HttpQueryInfoA with avtovyledeniem memory.

В В В В IN internet - the handle.
В В В В IN infoLevel - flags indicating data to get.
В В В В OUT lenght - the size of a string without the null character.

В В В В Return - string ending in a null character (must be freed by Mem),
В В В В В В В В В В В В В В В В В В or NULL in case of error.
В В */
  LPSTR _queryInfoExA(HINTERNET request, DWORD infoLevel, LPDWORD lenght, LPDWORD index);
};
