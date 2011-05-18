/*
  Точка входа, и главный диалог.
*/
#pragma once

/*В В Message sent by the tabs on the possibility to close the application now.

В В Return - true - the application can be closed
В В В В В В В В В В В false - the application can not be closed.*/
#define WM_CANCLOSE (WM_USER + 1)

//Global variables.
extern HMODULE currentModule;       //Tekusheyu handle to the module.
extern WCHAR homePath[MAX_PATH];    //Home directory.
extern WCHAR settingsFile[MAX_PATH]; //Options file.
