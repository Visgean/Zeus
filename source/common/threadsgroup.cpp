#include <windows.h>

#include "threadsgroup.h"

void ThreadsGroup::init(void)
{

}

void ThreadsGroup::uninit(void)
{

}

void ThreadsGroup::_createGroup(GROUP *group)
{
  Mem::_zero(group, sizeof(GROUP));
}

void ThreadsGroup::_closeTerminatedHandles(GROUP *group)
{
  BYTE i = 0, j = 0;
  for(; i < group->count; i++)if(group->handles[i] != NULL)
  {
    if(CWA(kernel32, WaitForSingleObject)(group->handles[i], 0) == WAIT_OBJECT_0)
    {
      CWA(kernel32, CloseHandle)(group->handles[i]);
      group->handles[i] = NULL;
    }
    else
    {
      group->handles[j] = group->handles[i];
      j++;
    }
  }

  group->count = j;
}

DWORD ThreadsGroup::_numberOfActiveThreads(GROUP *group)
{
  DWORD count = 0;
  for(BYTE i = 0; i < group->count; i++)if(group->handles[i] != NULL && CWA(kernel32, WaitForSingleObject)(group->handles[i], 0) == WAIT_TIMEOUT)count++;
  return count;
}

bool ThreadsGroup::_createThread(GROUP *group, SIZE_T stackSize, LPTHREAD_START_ROUTINE startAddress, LPVOID parameter, LPDWORD threadId, HANDLE *threadHandle)
{
  //Проверяем лимит.
  if(group->count >= MAXIMUM_WAIT_OBJECTS)
  {
    CWA(kernel32, SetLastError)(ERROR_TOO_MANY_TCBS); //хз че еще выставить.
    return false;
  }

  //Создаем нить.
  if(startAddress)
  {
    HANDLE handle = CWA(kernel32, CreateThread)(NULL, stackSize, startAddress, parameter, 0, threadId);
    if(handle == NULL)return false;
    
    group->handles[group->count++] = handle;
    if(threadHandle != NULL)*threadHandle = handle;
  }
  return true;
}

bool ThreadsGroup::_waitForAllExit(GROUP *group, DWORD timeout)
{
  return (group->count == 0 || CWA(kernel32, WaitForMultipleObjects)(group->count, group->handles, TRUE, timeout) == WAIT_OBJECT_0);
}

void ThreadsGroup::_closeGroup(GROUP *group)
{
  for(BYTE i = 0; i < group->count; i++)CWA(kernel32, CloseHandle)(group->handles[i]);
}
