#include <windows.h>

#include "defines.h"
#include "resources\resources.h"
#include "main.h"
#include "languages.h"

/*
  Обработка вкладки.
*/
INT_PTR CALLBACK toolSettingsProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch(msg)
  {
    case WM_INITDIALOG:
    {
      CWA(user32, SetDlgItemTextW)(hwnd, IDC_SETTINGS_LANGUAGE_TITLE, Languages::get(Languages::tool_settings_language_title));
      CWA(user32, SetDlgItemTextW)(hwnd, IDC_SETTINGS_APPLY, Languages::get(Languages::tool_settings_apply));

      //Заполняем языки.
      {
        const Languages::LANGINFO *langInfo;
        const Languages::LANGINFO *curLangInfo = Languages::getCurLangInfo();
        HWND cb = CWA(user32, GetDlgItem)(hwnd, IDC_SETTINGS_LANGUAGE);
        
        for(WORD i = 0; (langInfo = Languages::getLangInfo(i)) != NULL; i++)
        {
          int index = (int)CWA(user32, SendMessageW)(cb, CB_ADDSTRING, 0, (LPARAM)langInfo->name);
          if(index >= 0)
          {
            CWA(user32, SendMessageW)(cb, CB_SETITEMDATA, (WPARAM)index, (LPARAM)i);
            if(curLangInfo->id == langInfo->id)CWA(user32, SendMessageW)(cb, CB_SETCURSEL, (WPARAM)index, 0);
          }
        }
      }
      break;
    }
    
    case WM_CANCLOSE:
    {
      CWA(user32, SetWindowLongW)(hwnd, DWL_MSGRESULT, true);
      break;
    }
    
    case WM_COMMAND:
    {
      switch(LOWORD(wParam))
      {
        //Применяем настройки.
        case IDC_SETTINGS_APPLY:
        {
          bool ok = true;
          
          //Изменяем язык.
          {
            HWND cb = CWA(user32, GetDlgItem)(hwnd, IDC_SETTINGS_LANGUAGE);
            int index = (int)CWA(user32, SendMessageW)(cb, CB_GETCURSEL, 0, 0);
            if(index >= 0)
            {
              const Languages::LANGINFO *langInfo = Languages::getLangInfo(CWA(user32, SendMessageW)(cb, CB_GETITEMDATA, (WPARAM)index, 0));
              if(langInfo != NULL)
              {
                if(!Languages::setDefaultLangId(langInfo->id))ok = false;
                else
                {
                  const Languages::LANGINFO *curLangInfo = Languages::getCurLangInfo();
                  if(langInfo->id != curLangInfo->id)
                  {
                    CWA(user32, MessageBoxW)(NULL, Languages::get(Languages::tool_settings_language_saved), Languages::get(Languages::main_title), MB_ICONINFORMATION | MB_OK);
                  }
                }
              }
            }
          }
          
          if(ok == false)CWA(user32, MessageBoxW)(NULL, Languages::get(Languages::tool_settings_apply_failed), NULL, MB_ICONERROR | MB_OK);
          break;
        }

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
