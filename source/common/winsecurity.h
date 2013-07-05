/*
  Работа с дескрипторами безопасности Windows.
*/
#pragma once

namespace WinSecurity
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
    Создание SECURITY_ATTRIBUTES и SECURITY_DESCRIPTOR с полным доступом.

    OUT sa - указатель SECURITY_ATTRIBUTES, может быть NULL, если значение не нужно.
    OUT sd - указатель SECURITY_DESCRIPTOR.

    Return - в случаи успеха: адрес памяти который нужно освободить через LocalFree, или  (void *)-1.
             в случаи ошибки: NULL.

    Примечание: sa и sd связаны между собой.
  */
  void *_getFullAccessDescriptors(SECURITY_ATTRIBUTES *sa, SECURITY_DESCRIPTOR *sd);

  /*
    Установка на объект прав на полный досут для Low Integrity процессов.

    IN name       - имя объекта.
    IN objectType - тип объекта SE_OBJECT.
    IN inherit    - влючить наследования дочерным объектам.
    
    Return        - true - в случаи успеха,
                    false - в случаи ошибки.
  */
  bool _setLowIntegrityLevelLabel(LPWSTR name, DWORD objectType, bool inherit);

  /*В В В В Getting TOKEN_USER of the token.

В В В В IN token - the token, you need the right TOKEN_QUERY.

В В В В Return - NULL - in case of error,
В В В В В В В В В В В В В В В address of the structure in case of success, must be released through the Mem.
В В */
  TOKEN_USER *_getUserByToken(HANDLE token);
};
