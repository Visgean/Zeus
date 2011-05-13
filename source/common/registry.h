/*
  Работа с реестром Windows.
*/
#pragma once

#define REGISTRY_MAX_VALUE_DATA_SIZE (1 * 1024 * 1204)

namespace Registry
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
    Получение значения из реестра как строки. Также самостоятельно обрабатывает REG_EXPAND_SZ.

    IN key        - ключ.
    IN subKey     - подключ ключа key, если значение равно NULL, то запрашивается value для
                    текущего key.
    IN value      - имя значения для получения.
    IN buffer     - буффер.
    IN bufferSize - размер буфера в символах включая нулевой символ.

    Return        - размер строки, в символах, исключая нулевой символ,
                    (DWORD)-1 - в случаи ошибки.
  */
  DWORD _getValueAsString(HKEY key, const LPWSTR subKey, const LPWSTR value, LPWSTR buffer, DWORD bufferSize);
  
  /*
    Получение значения из реестра как строки.

    IN key        - ключ.
    IN subKey     - подключ ключа key, если значение равно NULL, то запрашивается value для
                    текущего key.
    IN value      - имя значения для установки.
    IN buffer     - буффер.
    IN bufferSize - на входе размер буфера, в символах исключая нулевой символ,

    Return        - true - в случаи успеха,
                    false - в случаи ошибки.
  */
  bool _setValueAsString(HKEY key, const LPWSTR subKey, const LPWSTR value, const LPWSTR buffer, DWORD bufferSize);

  /*
    Получение значения из реестра как DWORD.

    IN key    - ключ.
    IN subKey - подключ ключа key, если значение равно NULL, то запрашивается value для текущего\
                key.
    IN value  - имя значения для получения.

    Return    - значение value, или 0 в случаи ошибки.
  */
  DWORD _getValueAsDword(HKEY key, const LPWSTR subKey, LPWSTR value);

  /*
    Запись значения в реестр как DWORD.
  
    IN key    - ключ.
    IN subKey - подключ ключа key, если значение равно NULL, то запрашивается value для текущего
                key.
    IN value  - имя значения для установки.
    IN data   - значение.

    Return    - true - в случаи успеха,
                false - в случаи ошибки.
  */
  bool _setValueAsDword(HKEY key, const LPWSTR subKey, const LPWSTR value, DWORD data);

  /*
    Получение значения из реестра как данных.

    IN key        - ключ.
    IN subKey     - подключ ключа hKey, если значение равно NULL, то запрашивается pstrValue для
                    текущего hKey.
    IN value      - имя значения для получения.
    OUT type      - тип значения REG_*, може быть NULL.
    OUT buffer    - буффер для значения.
    IN bufferSize - размер буффера pBuf.

    Return        - размер данных, или (DWORD)-1 - в случаи ошибки.
  */
  DWORD _getValueAsBinary(HKEY key, const LPWSTR subKey, const LPWSTR value, LPDWORD type, void *buffer, DWORD bufferSize);

  /*
    Запись значения в реестр как данных.
  
    IN key        - ключ.
    IN subKey     - подключ ключа key, если значение равно NULL, то запрашивается value для
                    текущего key.
    IN value      - имя значения для установки.
    IN type       - тип значения REG_*, може быть NULL.
    IN buffer     - буффер.
    IN bufferSize - количетсво байт для записи.

    Return        - true - в случаи успеха,
                    false - в случаи ошибки.
  */
  bool _setValueAsBinary(HKEY key, const LPWSTR subKey, const LPWSTR value, DWORD type, const void *buffer, DWORD bufferSize);
    
  /*
    Получение значения из реестра как данных с выделением памяти.

    IN key     - ключ.
    IN subKey  - подключ ключа key, если значение равно NULL, то запрашивается value для
                 текущего key.
    IN value   - имя значения для получения.
    OUT type   - тип значения REG_*, може быть NULL.
    OUT buffer - буффер для значения, должен быть освобожден через Mem.

    Return     - размер данных, или (DWORD)-1 - в случаи ошибки.
  */
  DWORD _getValueAsBinaryEx(HKEY key, const LPWSTR subKey, const LPWSTR value, LPDWORD type, void **buffer);
  
  /*
    Получение значения из реестра в виде CRC32. Требует Crypt.

    IN key    - ключ.
    IN subKey - подключ ключа key, если значение равно NULL, то запрашивается value для текущего
                key.
    IN value  - имя значения для получения.

    Return   - CRC32, или 0 в случаи ошибки или пустой строки.
  */
  DWORD _getsCrc32OfValue(HKEY key, const LPWSTR subKey, const LPWSTR value);

  /*
    Удаление ключа со всеми подключами.
    
    IN key    - ключ.
    IN subKey - подключ ключа key для удаления (не может быть NULL).

    Return    - true - в случаи успеха,
                false - в случаи ошибки.
  */
  bool _deleteKey(HKEY key, const LPWSTR subKey);

  /*
    Удаление значения в реестре.
    
    IN key    - ключ.
    IN subKey - подключ ключа key, если значение равно NULL, то запрашивается value для текущего
                key.
    IN value  - имя значения для удаления.

    Return    - true - в случаи успеха,
                false - в случаи ошибки.
  */
  bool _deleteValue(HKEY key, const LPWSTR subKey, const LPWSTR value);

  /*
    Проверяет сущtcтвует ли значение

    IN key    - ключ.
    IN subKey - подключ ключа key, если значение равно NULL, то запрашивается value для текущего
                key.
    IN value  - имя значения.

    Return    - true - в случаи успеха,
                false - в случаи ошибки.
  */
  bool _valueExists(HKEY key, const LPWSTR subKey, const LPWSTR value);
};
