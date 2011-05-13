#pragma once
/*
  Работа с MS CAB файлами через библиотеку cabinet.dll. Надстройка полнсотью обходит ограничения 
  ANSI кодировки.
  
  Система отптимизирована на минимальный размер.

  Примичание: функции не безопасны при многопоточном использовании, поэтому необходиму использовать
              функции синхронизации.

  TODO:
    Добавить поддержку нескольких томов, т.е. привышение лимита CB_MAX_DISK.
*/

#if !defined XLIB_MSCAB_FCI
  #error XLIB_MSCAB_FCI not defined!
#endif

#if !defined XLIB_MSCAB_FDI
  #error XLIB_MSCAB_FDI not defined!
#endif

namespace MsCab
{
  enum
  {
    CFF_RECURSE = 0x1, //Поиск в поддиректориях.
  };

  /*
    Инициализация.
  */
  void Init(void);

  /*
    Деинициализация.
  */
  void Uninit(void);

#if(XLIB_MSCAB_FCI > 0)
  /*
    Создание нового архива.

    IN pstrPath     - путь к архиву.
    IN pstrFile     - имя архива. Может быть NULL, тогда имя должно быть включено в pstrPath.
    IN pstrTempPath - директория для временных файлов. Если указано NULL, то используеся значение 
                      GetTempPath

    Return          - хэндл архива, хэндл должен быть закрыт через FCIClose,
                      или NULL в случаи ошибки.
  */
  void *FCICreate(LPWSTR pstrPath, LPWSTR pstrFile, LPWSTR pstrTempPath);

  /*
    Добавление файла в архив.
    
    IN pFCI           - хэндл.
    IN pstrSourceFile - исходный файл.
    IN pstrDestFile   - конечный файл в архиве, возможно указания путей. Будет конвертироваться в
                        UTF-8, но если после конвертации размер строки привышает 256 - 1 символов,
                        он будет сохранен в архиве согласно текушей кодировки ОС.

    Return            - true - в случаи успеха,
                        false - в случаи ошибки.
  */
  bool FCIAddFile(void *pFCI, LPWSTR pstrSourceFile, LPWSTR pstrDestFile);
  
  /*
    Закрытие архива.

    IN pFCI - хэндл.

    Return  - true - в случаи успеха,
              false - в случаи ошибки.
  */
  bool FCIFlush(void *pFCI);
  
  /*
    Закрытие хэндла созданного через FCICreate.

    IN pFCI - хэндл.
  */
  void FCIClose(void *pFCI);

  /*
    Создание архива из папки.

    IN outputFile    - создаваемый файл архива.
    IN sourceFolder  - исходная папка. Может кончатся на слеш.
    IN tempFolder    - временная папка или NULL.
    IN fileMask      - массив масок файлов.
    IN fileMaskCount - кол. элементов в fileMask.
    IN flags         - флаги CFF_*.

    Return           - true - в случаи успешного создания архива и архивированя хотябы одного
                       файла, false - в случаи ошибки или отсутвия файлов.
 */
  bool createFromFolder(LPWSTR outputFile, LPWSTR sourceFolder, LPWSTR tempFolder, LPWSTR *fileMask, DWORD fileMaskCount, DWORD flags);
#endif

#if(XLIB_MSCAB_FDI > 0)
  
  /*
    Функция вызываваемая для запроса на копирование файла.

    IN sourceFile - исходный файл в архиве.
    OUT destFile  - файл назначения. Файл и весь путь будет создан автоматически.
    IN destSize   - размер файла.
    IN data       - данные.

    Return        -  1 - для извелчения файла,
                     0 - для пропуска файла,
                    -1 - для прерывания обзора архива.
  */
  typedef int (* FDICOPYFILECALLBACK)(LPCWSTR sourceFile, LPWSTR destFile, DWORD destSize, void *data);

  /*
    Открытие архива.
    
    IN copyCallback - функция вызываваемая для запроса на копирование файла. Может быть NULL, для
                      извлечения всех файлов.
    IN data         - данные для copyCallback. Если copyCallback == NULL, то переменная указывает
                      на адрес папки, в которую распоковываются файлы.

    Return  - хэндл архива, хэндл должен быть закрыт через FDIClose,
              или NULL в случаи ошибки.
  */
  void *FDICreate(FDICOPYFILECALLBACK copyCallback, void *data);
  
  /*
    Обзор и распоковка архива.

    IN pFDI     - хэндл.
    IN pstrPath - путь к архиву.
    IN pstrFile - имя архива. Может быть NULL.

    Return      - true - в случаи успеха
                  false - в случаи ошибки.
  */
  bool FDICopy(void *pFDI, LPWSTR pstrPath, LPWSTR pstrFile);
  
  /*
    Закрытие хэндла созданного через FCICreate.

    IN pFDI - хэндл.
  */
  void FDIClose(void *pFDI);

  /*
    Обертка для FDI.

    IN pstrPath     - путь к архиву.
    IN pstrFile     - имя архива. Может быть NULL.
    IN copyCallback - функция вызываваемая для запроса на копирование файла. Может быть NULL, для
                      извлечения всех файлов.
    IN data         - данные для copyCallback. Если copyCallback == NULL, то переменная указывает
                      на адрес папки, в которую распоковываются файлы.

    Return          - true - в случаи успеха,
                      false - в случаи ошибки.
  */
  bool extractFiles(LPWSTR pstrPath, LPWSTR pstrFile, FDICOPYFILECALLBACK copyCallback, void *data);
#endif
};
