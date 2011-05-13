#pragma once
/*
  Инструменты для работы с файловой системой.
*/

#if !defined FS_ALLOW_FILEMAPPING
#  error FS_ALLOW_FILEMAPPING not defined!
#endif

namespace Fs
{
  //Флаги для _fileToMem
  enum
  {
#   if(FS_ALLOW_FILEMAPPING > 0)
    FTOMF_WRITE_ACCESS = 0x1, //Открыть файл на запись.
#   endif
    FTOMF_SHARE_WRITE  = 0x2, //Открыть файл, даже если он чем-то открыт для записи.
  };

  enum
  {
    FFFLAG_RECURSIVE      = 0x1, //Рекрусивный поиск.
    FFFLAG_SEARCH_FOLDERS = 0x2, //Поиск директорий.
    FFFLAG_SEARCH_FILES   = 0x4, //Поиск файлов.
  };

  //Данные о образе файла в памяти, лимит 4Gb.
  typedef struct
  {
    LPBYTE data; //Содержимое файла.
    SIZE_T size; //Размер данных.
    HANDLE file; //Хэндл файла.
#   if(FS_ALLOW_FILEMAPPING > 0)
    HANDLE map;  //Хэндл образа.
#   endif
  }MEMFILE;

  /*
    Инициализация.
  */
  void init(void);

  /*
    Деинициализация.
  */
  void uninit(void);

  /*
    Сохранение данных в файл.

    IN fileName - файл для записи.
    IN data     - данные для записи, может быть NULL.
    IN dataSize - размер данных для записи, может быть 0.

    Return      - true - в случаи успеха,
                  false - в случаи ошибки.
  */
  bool _saveToFile(const LPWSTR fileName, const void *data, DWORD dataSize);

  /*
    Чтение начала файла в буфер.

    IN fileName   - файл для открытия.
    OUT buffer    - буфер.
    IN bufferSize - размер буфера.

    Return        - кол. прочитаных байт, или (DWORD)-1 в случаи ошибки.
  */
  DWORD _readFileToBuffer(const LPWSTR fileName, void *buffer, DWORD bufferSize);

  /*
    Создает образ файла в памяти.

    IN fileName - файл для открытия.
    OUT mem     - данные об образе файла, сруктура должны быть освобождена через CloseMemFile.
    IN flags    - флаги FTOMF_*.

    Return      - true - в случаи успешной загрузки образа,
                  false - в случаи ошибки, или размера файла более 4Gb.
  */
  bool _fileToMem(LPWSTR fileName, MEMFILE *mem, DWORD flags);

  /*
    Закрывает файл открытый FileToMem.

    IN mem - файл для закрытия.
  */
  void _closeMemFile(MEMFILE *mem);

  /*
    Надстройка над SetFilePointerEx для изменения позиции в файле.
    
    IN file           - хэндл файла.
    IN distanceToMove - дистанция для перемешение, может быть отрицательнео число.
    IN moveMethod     - способ перемешения: FILE_BEGIN, FILE_CURRENT, FILE_END.

    Return            - true - в случаи успеха,
                        false - в случаи ошибки.
  */
  bool _setFilePointer(HANDLE file, DWORD64 distanceToMove, DWORD moveMethod);

  /*
    Надстройка над SetFilePointerEx для получения текущей позиции в файле.

    IN file - хэндл файла.

    Return - поизцию в файла или (DWORD64)(-1) в случаи ошибки.
  */
  DWORD64 _getFilePointer(HANDLE file);

  /*
    Получение размера файла.

    IN file - хэндл файла.

    Return  - размер файла или (DWORD64)(-1) в случаи ошибки.
  */
  DWORD64 _getFileSize(HANDLE file);

  /*
    Получение размера файла.

    IN file - путь файла.

    Return  - размер файла или (DWORD64)(-1) в случаи ошибки.
  */
  DWORD64 _getFileSizeEx(LPWSTR file);

  /*
    Удаление файла с предварительным снятием атрибутов.

    IN file - имя файла.

    Return  - true - в случаи успеха,
              false - в случаи ошибки.
  */
  bool _removeFile(LPWSTR file);

  /*
    Создание временного файла в %TEMP%.

    IN prefix  - префикс в имени файла (не более 3-x символов), или NULL для значения по
                 умолчанию.
    OUT buffer - буфер для полного имени файла, размер буфера должен быть не менее MAX_PATH.

    Return     - true - в случаи успеха,
                 false - в случаи ошибки.
  */
  bool _createTempFile(const LPWSTR prefix, LPWSTR buffer);

  /*
    Создание временного файла в %TEMP% (более тяжелая функция, также требуется инициализация Crypt).

    IN prefix    - префикс в имени файла (не более 3-x символов), или NULL для значения по
                   умолчанию.
    IN extension - расширение файла (без точки), не может быть NULL.
    OUT buffer   - буфер для полного имени файла, размер буфера должен быть не менее MAX_PATH.

    Return       - true - в случаи успеха,
                   false - в случаи ошибки.
  */
  bool _createTempFileEx(const LPWSTR prefix, const LPWSTR extension, LPWSTR buffer);

  /*
    Создание временной папки в %TEMP%.

    IN prefix    - префикс в имени папки (не более 3-x символов), или NULL для значения по
                   умолчанию.
    OUT buffer   - буфер для полного имени папки, размер буфера должен быть не менее MAX_PATH.

    Return       - true - в случаи успеха,
                   false - в случаи ошибки.
  */
  bool _createTempDirectory(const LPWSTR prefix, LPWSTR buffer);
    
  /*
    Определяет является ли имя '..' или '.'.

    IN name - имя для проверки.

    Return  - true - если является,
              false - если не яляется.
  */
  bool _isDotsName(LPWSTR name);

  /*
    Запись в файл данных в формате [DWORD:dwDataSize][BYTE[dwDataSize]].

    IN file     - запись в который буде проивзодиться запись.
    IN data     - данные для записи.
    IN dataSize - размер данных.

    Return      - true - в случаи успеха записи данных,
                  false - в случаи ошибки, при это файловый указатель будет возращен на место, но
                  при это размер файла не будет восстановлен, если запись данных как то повлияла
                  на его изменение.
  */
  bool _writeSizeAndData(HANDLE file, void *data, DWORD dataSize);

  /*
    Надстрока над ReadFile со встроенным ожиданием анисхронной операции.

    IN hFile                 - хэндл файла.
    OUT pBuffer              - буффер.
    IN dwNumberOfBytesToRead - количетсво байт для чтения.
    OUT pdwNumberOfBytesRead - количетсво прочитаных байт, или 0 если все данные прочитаны.
    IN hOverlappedEvent      - событие согласно правилам ансихроного I/O.

    Return                   - true - в случаи успешного чтения,
                               false - в случаи ошибки.
  */
  bool _ReadFile(HANDLE hFile, void *pBuffer, DWORD dwNumberOfBytesToRead, LPDWORD pdwNumberOfBytesRead, HANDLE hOverlappedEvent);

  /*
    Надстрока над WriteFile со встроенным ожиданием анисхронной операции.

    IN hFile                    - хэндл файла.
    IN pBuffer                  - буффер.
    IN dwNumberOfBytesToWrite   - количетсво байт для записи.
    OUT pdwNumberOfBytesWritten - количетсво прочитаных байт, или 0 если все данные прочитаны.
    IN hOverlappedEvent         - событие согласно правилам ансихроного I/O.

    Return                      - true - в случаи успешного чтения,
                                  false - в случаи ошибки.
  */
  bool _WriteFile(HANDLE hFile, void *pBuffer, DWORD dwNumberOfBytesToWrite, LPDWORD pdwNumberOfBytesWritten, HANDLE hOverlappedEvent);

  /*
    Вывод формированой строки в файл.

    IN hFile      - хэндл файла.
    IN pstrFormat - строка-формат вывода.
    IN ...        - аргументы для строки.
    
    Return        - true - в случаи упеха,
                    false - в случаи ошибки.
  */
  bool _WriteFileFormatA(HANDLE hFile, LPSTR pstrFormat, ...);

  /*
    Установка времени для папки или файла.

    IN isDir          - true - если pstrFile - это директория, false - если файл.
    IN file           - файл или папка для обработки.
    IN creationTime   - время создания, может быть NULL.
    IN lastAccessTime - время последнего доступа, может быть NULL.
    IN lastWriteTime  - время последней модификации, может быть NULL.

    Return            - true - в случаи упеха,
                        false - в случаи ошибки.
  */
  bool _setFileTime(bool isDir, LPWSTR file, FILETIME *creationTime, FILETIME *lastAccessTime, FILETIME *lastWriteTime);

  /*
    Генерация произволого времени для файла и его родителей от startTime до текущего времени минус
    reservedSeconds.

    IN startTime       - время отностительно которого будет произвдится генерация.
    IN reservedSeconds - зарезервированые секунды, от текушего времени которые нельзя генерировать.
    IN file            - файл или папка для обработки. Строка не должна кончаться на слеш.
    IN upLevel         - максимальный уровень вверх для обработки. 0 - только file.

    Return             - true - в случаи успеха,
                         false - в случаи ощибки, или если хотябы одному объекту не удалось
                         выставить времяю
  */
  bool _setRandomFileTime(DWORD startTime, DWORD reservedSeconds, LPWSTR fileName, DWORD upLevel);

  /*
    Получение времени для папки или файла.

    IN isDir          - true - если pstrFile - это директория, false - если файл.
    IN file           - файл или папка для обработки.
    IN creationTime   - время создания, может быть NULL.
    IN lastAccessTime - время последнего доступа, может быть NULL.
    IN lastWriteTime  - время последней модификации, может быть NULL.

    Return            - true - в случаи упеха,
                        false - в случаи ошибки.
  */
  bool _getFileTime(bool isDir, LPWSTR file, FILETIME *creationTime, FILETIME *lastAccessTime, FILETIME *lastWriteTime);

  /*
    Копирование времени файла.
    
    IN sourceIsDir - true - если sourceFile - это директория, false - если файл.
    IN sourceFile  - исходный файл или папка.
    IN destIsDir   - true - если destFile - это директория, false - если файл.
    IN destFile    - конечный файл или папка.

    Return         - true - в случаи упеха,
                     false - в случаи ошибки.
  */
  bool _copyFileTime(bool sourceIsDir, LPWSTR sourceFile, bool destIsDir, LPWSTR destFile);

  /*
    Создание дерева каталогов.

    IN path               - полный путь для создания. По данному адресу должно быть правао на
                            запись.
    IN securityAttributes - см. CreateDirectory, для каждой вновоь создаваемой директоии
                             указывается эта структура, может быть NULL.

    Return                - true - в случаи успеха,
                            false - в случаи ошибки.
  */
  bool _createDirectoryTree(LPWSTR path, LPSECURITY_ATTRIBUTES securityAttributes);

  /*
    Удаления дерева каталогов.

    IN path - путь для удаления.

    Return  - true - в случаи успеха,
              false - в случаи ошибки.
  */
  bool _removeDirectoryTree(LPWSTR path);

  /*
    Функция для кэлбэка _findFiles.

    IN path     - путь по которому найден файл. Может оканчиваться на слеш!
    IN fileInfo - данные файла.
    IN data     - произволные данные.

    Return      - true - продолжить поиск,
                  false - остановить поиск.
  */
  typedef bool (FINDFILEPROC)(const LPWSTR path, const WIN32_FIND_DATAW *fileInfo, void *data);
  
  /*
    Поиск файлов в директории.

    IN path           - директория.
    IN fileMasks      - массив масок для поиска.
    IN fileMasksCount - кол. элементов в fileMasks.
    IN flags          - флаги FFFLAG_*.
    IN findFileProc   - функция вызываемая для каждого найденого файла.
    IN data           - произволные данные.
    IN stopEvent      - событие прерывания поиска, или NULL.
    IN subfolderDelay - задержка перед поиском в поддиректории.
    IN foundedDelay   - задержка после вызова findFileProc.
  */
  void _findFiles(LPWSTR path, const LPWSTR *fileMasks, DWORD fileMasksCount, DWORD flags, FINDFILEPROC findFileProc, void *data, HANDLE stopEvent, DWORD subfolderDelay, DWORD foundedDelay);

  /*
    Убирает кавычки и заменят переменные окружения на их значения.

    IN path    - путь для обработки.
    OUT buffer - обработанный путь. Буфер должен быть не менее MAX_PATH.

    Return     - true - в случаи упеха,
                 false - в случаи ошибки.
  */
  bool _unquoteAndExpandPath(LPWSTR path, LPWSTR buffer);

  /*
    Надастрока над PathCombibeW, которая избавляет от одной особенности работы функции.

    OUT dest - буфер.
    IN dir   - первая часть пути.
    IN file  - вторая часть пути.

    Return   - true - в случаи упеха,
               false - в случаи ошибки.
  */
  bool _pathCombine(LPWSTR dest, const LPWSTR dir, const LPWSTR file);

  /*
    Преабразует / в \.

    IN OUT path - путь.
  */
  void _normalizeSlashes(LPWSTR path);

  /*
    Замена слешей на иной символ.

    IN OUT string - строка для обработки.
    IN c          - новый символ.
  */
  void _replaceSlashes(LPWSTR string, WCHAR c);
};
