/*
  Ядро.
*/
#pragma once

//Structure for storing basic data processes.
typedef struct
{
  DWORD exitCode;  //Output Code
  struct
  {
    LPWSTR *args;  //An array of command line.
    int argsCount; //The number of arguments on the command line.
  }commandLine;
  WCHAR fileName[MAX_PATH]; //Current file name.
}COREDATA;
extern COREDATA coreData;

namespace Core
{
  /*
    Инициализация.
  */
  void init(void);

  /*
    Деинициализация.
  */
  void uninit(void);

  /*В В В В Display of the logo.
В В */
  void showLogo(void);
};
