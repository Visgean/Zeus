/*
  Р СћР С•РЎвЂЎР С”Р В° Р Р†РЎвЂ¦Р С•Р Т‘Р В°, Р С‘ Р С–Р В»Р В°Р Р†Р Р…РЎвЂ№Р в„– Р Т‘Р С‘Р В°Р В»Р С•Р С–.
*/
#pragma once

/*Р вЂ™Р’В Р вЂ™Р’В Message sent by the tabs on the possibility to close the application now.

Р вЂ™Р’В Р вЂ™Р’В Return - true - the application can be closed
Р вЂ™Р’В Р вЂ™Р’В Р вЂ™Р’В Р вЂ™Р’В Р вЂ™Р’В Р вЂ™Р’В Р вЂ™Р’В Р вЂ™Р’В Р вЂ™Р’В Р вЂ™Р’В Р вЂ™Р’В false - the application can not be closed.*/
#define WM_CANCLOSE (WM_USER + 1)

//Global variables.
extern HMODULE currentModule;       //Tekusheyu handle to the module.
extern WCHAR homePath[MAX_PATH];    //Home directory.
extern WCHAR settingsFile[MAX_PATH]; //Options file.
