/*
  Данные OC.
*/
#pragma once

namespace OsEnv
{
  //Version of OS.
  enum
  {
    VERSION_UNKNOWN,
    VERSION_2000,
    VERSION_XP,
    VERSION_S2003,
    VERSION_VISTA,
    VERSION_S2008,
    VERSION_SEVEN,
    VERSION_S2008R2
  };
  
  //Summary of the OC.
# pragma pack(push, 1)
  typedef struct
  {
    BYTE version;      //VERSION_ *.
    BYTE sp;           //Service Pack.
    WORD build;        //Build number.
    WORD architecture; //Arhitiktura processor.
  }OSINFO;
# pragma pack(pop)  
  /*
    Инициализация.
  */
  void init(void);

  /*
    Деинициализация.
  */
  void uninit(void);

  /*
    Получение текущей версии Windows.

    Return - VERSION_ *.
  */
  DWORD _getVersion(void);

  /*
    Получение текущей версии Windows.

    OUT oi - версия Windows.
  */
  void _getVersionEx(OSINFO *oi);

  /*В В В В Getting the user profile directory on the SID.

В В В В IN sid - SID.
В В В В OUT buffer - a buffer for the path must be at least MAX_PATH.

В В В В Return - true - the path is successfully received,
В В В В В В В В В В В В В В В В В false - the path has not been received.
В В */
  bool _getUserProfileDirectoryhBySid(PSID sid, LPWSTR buffer);
};
