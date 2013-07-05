/*
  Таблицы перехвата WinAPI.

  Данный модуль подчинается Core, и не имеет каких либо защит от повторных перехватов.
  По соображениям стабилности unhook не возможен.
*/
#pragma once

namespace WinApiTables
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
    Установка для процесса пользователя. 

    Return - true - в случаи успеха,
             false - в случаи ошибки.
  */
  bool _setUserHooks(void);

  /*
    Снятие хуков для процесса пользователя.

    Return - true - в случаи успеха,
             false - в случаи ошибки.
  */
  bool _removeUserHooks(void);

  /*
    Попытка установить перехват для nspr4.dll. 

    Return - true - в случаи успеха,
             false - в случаи ошибки.
  */
#if(BO_NSPR4 > 0)
  bool _trySetNspr4Hooks(void);
#endif

  /*
    Попытка установить перехват для nspr4.dll. 

    IN moduleName   - базове имя предпалогоемого модуля.
    IN moduleHandle - хэндл предпалогоемого модуля.
    
    Return          - true - в случаи успеха,
                      false - в случаи ошибки.
  */
#if(BO_NSPR4 > 0)
  bool _trySetNspr4HooksEx(LPWSTR moduleName, HMODULE moduleHandle);
#endif

  /*В В В В Installation for nspr4.dll.

В В В В IN nspr4Handle - handle nspr4.dll.
В В В В 
В В В В Return - true - if successful,
В В В В В В В В В В В В В В В В В В В В В false - if an error occurs.
В В */
#if(BO_NSPR4 > 0)
  bool _setNspr4Hooks(HMODULE nspr4Handle);
#endif
};
