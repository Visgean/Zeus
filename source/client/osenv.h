/*
  Данные OC.
*/
#pragma once

namespace OsEnv
{
  //Версии OS.
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
  
  //Краткая информация об OC.
# pragma pack(push, 1)
  typedef struct
  {
    BYTE version;      //VERSION_*.
    BYTE sp;           //Service Pack.
    WORD build;        //Номер билда.
    WORD architecture; //Архитиктура процессора.
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

    Return - VERSION_*.
  */
  DWORD _getVersion(void);

  /*
    Получение текущей версии Windows.

    OUT oi - версия Windows.
  */
  void _getVersionEx(OSINFO *oi);

  /*
    Получение директории профиля пользователя по SID.

    IN sid     - SID.
    OUT buffer - буфер для пути, должен быть не менее MAX_PATH.

    Return     - true - путь успешн получен,
                 false - путь не получен.
  */
  bool _getUserProfileDirectoryhBySid(PSID sid, LPWSTR buffer);
};
