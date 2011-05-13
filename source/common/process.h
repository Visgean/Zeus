/*
  Работа с процессами.
*/
#pragma once

//ID текущего процесса.
#define CURRENT_PROCESS ((HANDLE)-1)

namespace Process
{
  //Уровень приложения.
  enum
  {
    INTEGRITY_UNKNOWN,
    INTEGRITY_LOW,     //Untrusted (Process can only write to low integrity locations, such as the Temporary Internet Files\Low folder or the HKEY_CURRENT_USER\Software\LowRegistry key)
    INTEGRITY_MEDIUM,  //User (Process can create and modify files in the user's Documents folder and write to user-specific areas of the registry, such as HKEY_CURRENT_USER.)
    INTEGRITY_HIGH     //Administrative (Process can install files to the Program Files folder and write to sensitive registry areas like HKEY_LOCAL_MACHINE.)
  };
  
  //Флаги для showShellRunDialog().
  enum
  {
    RFD_NOBROWSE        = 0x00000001, //Не отображать кнопку "Обзор".
    RFD_NODEFFILE       = 0x00000002, //Не выбирать файл по умолчанию.
    RFD_USEFULLPATHDIR  = 0x00000004, //Использолвать рабочию директорию из пути запускаемого файла (игнарируется при workingDir != NULL).
    RFD_NOSHOWOPEN      = 0x00000008  //Не отображать метку для Edit'а.
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
    Получение хэндла модуля в процессе.

    IN process    - процесс. Нужны права PROCESS_QUERY_INFORMATION | PROCESS_VM_READ
    IN moduleName - имя модуля для поиска. Можеть быть как базовым именим, так и полным.

    Return        - хэндл модуля, или NULL если не найден.
  */
  HMODULE _getModuleHandle(HANDLE process, LPWSTR moduleName);

  /*
    Получение SID для процесса.

    IN process    - хэндл процесса с правами PROCESS_QUERY_INFORMATION.
    OUT sessionId - ID сессии, или NULL если она не нужна.
    
    Return        - данные SID (необходимо освободить через Mem), или NULL в случаи ошибки.
  */
  TOKEN_USER *_getUserByProcessHandle(HANDLE process, LPDWORD sessionId);

  /*
    Получение SID для процесса.

    IN id         - ID процесса.
    OUT sessionId - ID сессии, или NULL если она не нужна.

    Return        - данные SID (необходимо освободить через Mem), или NULL в случаи ошибки.
  */
  TOKEN_USER *_getUserByProcessId(DWORD id, LPDWORD sessionId);

  /*
    Получение кол. потоков в процессе.

    IN id  - ID процесса.
    
    Return - кол. потоков, или (DWORD)-1 в случаи ошибки.
  */
  DWORD _getCountOfThreadsByProcessId(DWORD id);

  /*
    Установка привелегии для текущего потока или процесса (как повезет).

    IN privilegeName - имя привелегии.
    IN enable        - true - включить, false - выключить.

    Return           - true - в случаи успешного изменения привелегии,
                       false - в случаи ошибки, или если привелегия уже имеет выбарнный статус.
  */
  bool _enablePrivilege(LPWSTR privilegeName, bool enable);

  /*
    Получение уровня процесс.

    IN procrss - хэндл процесса (нужно право PROCESS_QUERY_INFORMATION), или -1 для
                 текущего процесса.

    Return     - INTEGRITY_*.
  */
  BYTE _getIntegrityLevel(HANDLE process);

  /*
    Проверяет, находиться ли процесс под WOW64.

    IN process - процесс с правом PROCESS_QUERY_INFORMATION или PROCESS_QUERY_LIMITED_INFORMATION.

    Return     - true - процесс под WOW64,
                 false - процесс не под WOW64.
  */
#if !defined _WIN64
  bool _isWow64(HANDLE process);
#endif

  /*
    Создание нового процесса.

    IN module      - имя модуля, может быть NULL.
    IN commandLine - командная строка, может быть NULL. Указанный адрес не может быть константой!
    IN workDir     - рабочая директория нвого процесса, может быть NULL.
    IN starupInfo  - STARTUPINFO, моежт быть NULL.
    OUT pi         - PROCESS_INFORMATION, моежт быть NULL.

    Return         - ID процесса - в случаи успеха,
                     0 - в случаи ошибки.
  */
  DWORD _create(const LPWSTR module, const LPWSTR commandLine, const LPWSTR workDir, const STARTUPINFOW *starupInfo, PROCESS_INFORMATION *pi);

  /*
    Расширение для _create(), котрое создает процесс с командной строкой, где первый агрумент имя
    модуля. Т.е. в самой часто используемой формы запуска процессов, рекомендуется использовать
    именно эту функцию вместо _create(), это связано с тупостью некотрых кодеров, которые всегда
    ждут в первом аргументе имя процесса.

    IN module      - имя модуля, не может быть NULL.
    IN commandLine - командная строка, может быть NULL.
    IN workDir     - рабочая директория нвого процесса, может быть NULL.
    IN starupInfo  - STARTUPINFO, моежт быть NULL.
    OUT pi         - PROCESS_INFORMATION, моежт быть NULL.

    Return         - ID процесса - в случаи успеха,
                     0 - в случаи ошибки.
  */
  DWORD _createEx(const LPWSTR module, const LPWSTR commandLine, const LPWSTR workDir, const STARTUPINFOW *starupInfo,  PROCESS_INFORMATION *pi);


  /*
    Создание нового процесса.

    IN token       - примарный токен пользователя, для которого создается процесс. Нужны права:
                     TOKEN_QUERY, TOKEN_DUPLICATE, TOKEN_ASSIGN_PRIMARY.
    IN desktop     - Десктоп в формате "станцния\десктоп". Может быть NULL. Не имеет смысла, если
                     starupInfo != NULL.
    IN module      - имя модуля, может быть NULL.
    IN commandLine - командная строка, может быть NULL. Указанный адрес не может быть константой!
    IN workDir     - рабочая директория нвого процесса, может быть NULL.
    IN starupInfo  - STARTUPINFO, моежт быть NULL.
    OUT pi         - PROCESS_INFORMATION, моежт быть NULL.

    Return         - ID процесса - в случаи успеха,
                     0 - в случаи ошибки.
  */
  DWORD _createAsUser(HANDLE token, const LPWSTR desktop, const LPWSTR module, const LPWSTR commandLine, const LPWSTR workDir, const STARTUPINFOW *starupInfo, PROCESS_INFORMATION *pi);

  /*
    Расширение для _createAsUser(), котрое создает процесс с командной строкой, где первый агрумент имя
    модуля. Т.е. в самой часто используемой формы запуска процессов, рекомендуется использовать
    именно эту функцию вместо _createAsUser(), это связано с тупостью некотрых кодеров, которые всегда
    ждут в первом аргументе имя процесса.

    IN token       - примарный токен пользователя, для которого создается процесс. Нужны права:
                     TOKEN_QUERY, TOKEN_DUPLICATE, TOKEN_ASSIGN_PRIMARY.
    IN desktop     - Десктоп в формате "станцния\десктоп". Может быть NULL. Не имеет смысла, если
                     starupInfo != NULL.
    IN module      - имя модуля, не может быть NULL.
    IN commandLine - командная строка, может быть NULL.
    IN workDir     - рабочая директория нвого процесса, может быть NULL.
    IN starupInfo  - STARTUPINFO, моежт быть NULL.
    OUT pi         - PROCESS_INFORMATION, моежт быть NULL.

    Return         - ID процесса - в случаи успеха,
                     0 - в случаи ошибки.
  */
  DWORD _createAsUserEx(HANDLE token, const LPWSTR desktop, const LPWSTR module, const LPWSTR commandLine, const LPWSTR workDir, const STARTUPINFOW *starupInfo, PROCESS_INFORMATION *pi);

  /*
    Закрытие хэндлов и обнуление PROCESS_INFORMATION.

    IN OUT pi - PROCESS_INFORMATION.
  */
  void _closeProcessInformation(PROCESS_INFORMATION *pi);
  
  /*
    Настройка над CreateThread().

    IN stackSize    - размер стека.
    IN startAddress - точка входа. 
    IN parameter    - параметр.

    Return          - ID потока, или 0 в случаи ошибки.
  */
  DWORD _createThread(SIZE_T stackSize, LPTHREAD_START_ROUTINE startAddress, LPVOID parameter);

  /*
    Вызов стандартного дилалога запуска (RunFileDlg).

    IN owner      - родитель, может быть NULL.
    IN workingDir - рабочая директория, может быть NULL.
    IN title      - заголовок диалога, может быть NULL для значения по умолчанию.
    IN promt      - текст диалога, может быть NULL для значения по умолчанию.
    IN flags      - флаги RFD_*.
  */
  void _showShellRunDialog(HWND owner, const LPWSTR workingDir, const LPWSTR title, const LPWSTR promt, DWORD flags);

  /*
    Удаление файла через батник.

    IN fileForRemoving - полное имя файла для удаления.

    Return             - true - в случаи успеха,
                         false - в случаи ошибки.
  */
  bool _runBatchForRemoveFile(const LPWSTR fileForRemoving);

  /*
    Запуск bat-файла с произволным содержимым, и его последущее удаление.

    IN context - содержимое файла в кодировке OEM, будет добавлены дополнительные перфиксы и
                 постфиксы.
    
    Return     - true - в случаи успеха,
                 false - в случаи ошибки.
  */
  bool _runTempBatch(const LPSTR context);
};
