#include <windows.h>

#include "sync.h"

void Sync::init(void)
{

}

void Sync::uninit(void)
{

}

HANDLE Sync::_waitForMutex(SECURITY_ATTRIBUTES *mutexAttributes, LPWSTR name)
{
  HANDLE mutexHandle = CWA(kernel32, CreateMutexW)(mutexAttributes, FALSE, name);
  if(mutexHandle != NULL)
  {
    DWORD r = CWA(kernel32, WaitForSingleObject)(mutexHandle, INFINITE);
    if(r == WAIT_OBJECT_0 || r == WAIT_ABANDONED)return mutexHandle;
    CWA(kernel32, CloseHandle)(mutexHandle);
  }
  return NULL;
}

DWORD Sync::_waitForMultipleObjectsAndDispatchMessages(DWORD count, const HANDLE* handles, bool waitAll, DWORD milliseconds)
{
  DWORD retVal;
  DWORD message = WAIT_OBJECT_0 + count;
  MSG msg;

  while((retVal = CWA(user32, MsgWaitForMultipleObjects)(count, handles, waitAll, milliseconds, QS_ALLINPUT)) == message)
  {
    while(CWA(user32, PeekMessageW)(&msg, NULL, 0, 0, PM_REMOVE) != FALSE)
    {
      if(msg.message == WM_QUIT)goto END;
      CWA(user32, TranslateMessage)(&msg);
      CWA(user32, DispatchMessageW)(&msg);
    }
  }

END:
  return retVal;
}

void Sync::_freeMutex(HANDLE mutex)
{
  CWA(kernel32, ReleaseMutex)(mutex);
  CWA(kernel32, CloseHandle)(mutex);
}

HANDLE Sync::_createUniqueMutex(SECURITY_ATTRIBUTES *mutexAttributes, LPWSTR name)
{
  HANDLE mutexHandle = CWA(kernel32, CreateMutexW)(mutexAttributes, TRUE, name);
  if(mutexHandle != NULL && CWA(kernel32, GetLastError)() == ERROR_ALREADY_EXISTS)
  {
    CWA(kernel32, CloseHandle)(mutexHandle);
    mutexHandle = NULL;
  }
  
  return mutexHandle;  
}

bool Sync::_mutexExists(LPWSTR name)
{
  HANDLE mutexHandle = CWA(kernel32, OpenMutexW)(SYNCHRONIZE, FALSE, name);
  if(mutexHandle != NULL)
  {
    CWA(kernel32, CloseHandle)(mutexHandle);
    return true;
  }

  return false;  
}
