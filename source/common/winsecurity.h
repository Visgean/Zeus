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

  /*
    Получение TOKEN_USER из токена.

    IN token - токен, нужны права TOKEN_QUERY.

    Return   - NULL - в случаи ошибки,
               адрес структуры в случаи успеха, необходимо освободить через Mem.
  */
  TOKEN_USER *_getUserByToken(HANDLE token);
};
