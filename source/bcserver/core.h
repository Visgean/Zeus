/*
  ����.
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
    �������������.
  */
  void init(void);

  /*
    ���������������.
  */
  void uninit(void);

  /*    Display of the logo.
  */
  void showLogo(void);
};
