#include <windows.h>
#include <commdlg.h>
#include <commctrl.h>

#include "gui.h"
#include "mem.h"

void Gui::init(void)
{

}

void Gui::uninit(void)
{

}

void Gui::_enumWindows(HWND owner, bool topToDown, ENUMWINDOWSPROC proc, void *param)
{
  HWND currentWindow = CWA(user32, GetTopWindow)(owner);
  if(currentWindow != NULL)
  {
    UINT type;

    if(topToDown == false)
    {
      //Переходим вниз.
      if((currentWindow = CWA(user32, GetWindow)(currentWindow, GW_HWNDLAST)) == NULL)return; //Невозможно.
      type = GW_HWNDPREV;
    }
    else
    {
      //Переходим вверх.
      type = GW_HWNDNEXT;
    }

    //Обзор.
    while(proc(currentWindow, param) && (currentWindow = CWA(user32, GetWindow)(currentWindow, type)) != NULL);
  }
}

HWND Gui::_windowFromPoint(POINT point, DWORD timeout, DWORD *hitTest)
{
  DWORD_PTR result;
  HWND window = CWA(user32, WindowFromPoint)(point);

  if(window != NULL)
  {
    if(CWA(user32, SendMessageTimeoutW)(window, WM_NCHITTEST, 0, MAKELPARAM(point.x, point.y), SMTO_ABORTIFHUNG | SMTO_NORMAL, timeout, &result) == 0)
    {
      window = NULL;
    }
    else if(result == HTTRANSPARENT)
    {
      HWND curWindow = window;

      CWA(user32, SetWindowLongPtrW)(curWindow, GWL_STYLE, CWA(user32, GetWindowLongPtrW)(curWindow, GWL_STYLE) | WS_DISABLED /*WS_HIDDEN ?*/);
      window = _windowFromPoint(point, timeout, hitTest);
      CWA(user32, SetWindowLongPtrW)(curWindow, GWL_STYLE, CWA(user32, GetWindowLongPtrW)(curWindow, GWL_STYLE) & (~WS_DISABLED));
    }
    else
    {
      if(hitTest != NULL)*hitTest = (DWORD)result;
    }
  }
  return window;
}

bool Gui::_styleHaveSizeBorders(DWORD style)
{
  return ((style & WS_CAPTION) == WS_CAPTION || (style & (WS_POPUP | WS_THICKFRAME)) != 0);
}

LPWSTR Gui::_getWindowText(HWND window, LPDWORD size)
{
  LPWSTR buf = NULL;
  int len = CWA(user32, GetWindowTextLengthW)(window);
  if(len > 0 && (buf = (LPWSTR)Mem::alloc(len * sizeof(WCHAR) + sizeof(WCHAR))) != NULL)
  {
    if((len = CWA(user32, GetWindowTextW)(window, buf, len + 1)) > 0)
    {
      if(size)*size = len;
    }
    else 
    {
      Mem::free(buf);
      buf = NULL;
    }
  }
  return buf;
}

HICON Gui::_loadSharedIcon(HMODULE module, const LPWSTR id)
{
  return (HICON)CWA(user32, LoadImageW)(module, id, IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
}

HCURSOR Gui::_loadSharedCursor(HMODULE module, const LPWSTR id)
{
  return (HCURSOR)CWA(user32, LoadImageW)(module, id, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED); 
}

HFONT Gui::_createDialogFont(BYTE pointSize)
{
  LOGFONTW lf;

  LONG height;
  {
    HDC dc = CWA(user32, GetDC)(NULL);
    height = -CWA(kernel32, MulDiv)(pointSize, CWA(gdi32, GetDeviceCaps)(dc, LOGPIXELSY), 72);
    CWA(user32, ReleaseDC)(NULL, dc);
  }

  lf.lfHeight         = height;
  lf.lfWidth          = 0;
  lf.lfEscapement     = 0;
  lf.lfOrientation    = 0;
  lf.lfWeight         = FW_NORMAL;
  lf.lfItalic         = FALSE;
  lf.lfUnderline      = FALSE;
  lf.lfStrikeOut      = FALSE;
  lf.lfCharSet        = DEFAULT_CHARSET;
  lf.lfOutPrecision   = OUT_DEFAULT_PRECIS;
  lf.lfClipPrecision  = CLIP_DEFAULT_PRECIS;
  lf.lfQuality        = DEFAULT_QUALITY;
  lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
  
  Mem::_copy(lf.lfFaceName, FONT_DIALOG, sizeof(FONT_DIALOG));
 
  return CWA(gdi32, CreateFontIndirectW)(&lf);
}

bool Gui::_browseForFile(HWND owner, LPWSTR initialDir, LPWSTR fileBuffer)
{
  OPENFILENAMEW ofn;
  Mem::_zero(&ofn, sizeof(OPENFILENAMEW));

  ofn.lStructSize     = sizeof(OPENFILENAMEW);
  ofn.hwndOwner       = owner;
  ofn.lpstrFile       = fileBuffer;
  ofn.nMaxFile        = MAX_PATH;
  ofn.lpstrInitialDir = initialDir;
  ofn.Flags           = OFN_DONTADDTORECENT | OFN_ENABLESIZING | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;
  
  return CWA(comdlg32, GetOpenFileNameW)(&ofn) ? true : false;
}

bool Gui::_browseForSaveFile(HWND owner, LPWSTR initialDir, LPWSTR fileBuffer, LPWSTR defaultExtension, LPWSTR filter, DWORD filterIndex)
{
  OPENFILENAMEW ofn;
  Mem::_zero(&ofn, sizeof(OPENFILENAMEW));

  ofn.lStructSize     = sizeof(OPENFILENAMEW);
  ofn.hwndOwner       = owner;
  ofn.lpstrFilter     = filter;
  ofn.nFilterIndex    = filterIndex;
  ofn.lpstrFile       = fileBuffer;
  ofn.nMaxFile        = MAX_PATH;
  ofn.lpstrInitialDir = initialDir;
  ofn.Flags           = OFN_DONTADDTORECENT | OFN_ENABLESIZING | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
  ofn.lpstrDefExt     = defaultExtension;
  
  return CWA(comdlg32, GetSaveFileNameW)(&ofn) ? true : false;
}

bool Gui::_loadCommonControl(DWORD classes)
{
  INITCOMMONCONTROLSEX cc;

  cc.dwSize = sizeof(INITCOMMONCONTROLSEX);
  cc.dwICC  = classes;

  return (CWA(comctl32, InitCommonControlsEx)(&cc) == TRUE);
}
