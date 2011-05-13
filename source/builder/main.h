/*
  Точка входа, и главный диалог.
*/
#pragma once

/*
  Сообщение рассылаемое вкладкам, о возможности закрыть приложение сейчас.

  Return - true - приложение можно закрыть,
           false - приложение нельзя закрыть.
*/
#define WM_CANCLOSE (WM_USER + 1)

//Глобальные переменные.
extern HMODULE currentModule;       //Хэндл текушего модуля.
extern WCHAR homePath[MAX_PATH];    //Домашняя директория.
extern WCHAR settingsFile[MAX_PATH]; //Файл опций.
