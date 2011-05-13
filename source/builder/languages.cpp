#include <windows.h>

#include "defines.h"
#include "languages.h"
#include "main.h"

#include "..\common\str.h"

//Английский язык.
static const LPWSTR languageEn[] =
{
  L"Not enough memory",
  L"Failed to write output file.",
  L"\r\nERROR: %s",

  BO_NAME L" Builder",

  L"Information",
#if(BO_CLIENT_PLATFORMS > 0)
  L"Builder",
#endif
  L"Settings",

  L"Current version",
  L"Version: %u.%u.%u.%u\r\nBuild time: %s\r\nSignature: %S",
  L"Information about active bot",
  L"Encryption key:",
  L"Information:",
  L"Refresh",
  L"Remove bot",
  L"Removing bot...",
  L"Bot removed! Please restart computer for full removing.",
  L"Failed to remove bot! Try again or restart computer.",
  L"-- default --",
  L"Version: %u.%u.%u.%u\r\nBotnet: %s\r\nFile of loader: ~\\%s",
  L"Bot not founded.",

#if(BO_CLIENT_PLATFORMS > 0)
  L"Source configuartion file:",
  L"Browse...",
  L"Edit...",
  L"Actions",
  L"Build the bot configuration",
  L"Build the bot executable",
  L"-- file not selected --",
  L"Failed to execute external editor.",
  L"Failed to read configuration file.",

  L"Building the bot...",
  L"Loading configuration...",
  L"Creating executable file...",
  L"Size of output file is %u bytes.",
  L"\r\nBUILD SUCCEEDED!",
  
  L"Building configuration of bot...\r\n",
  L"\r\nBUILD SUCCEEDED!",
  L"Failed to find \"StaticConfig\".",
  L"Maximum length of value \"StaticConfig/botnet\" can not be more than %u characters long.",
  L"Failed to find \"StaticConfig/url_config\".",
  L"Maximum length of value \"StaticConfig/url_config\" can not be more than %u characters long.",
  L"Failed to find \"StaticConfig/encryption_key\".",
  L"Source executable file corrupted.",
  L"Failed to find \"DynamicConfig\".",
  L"Failed to find \"DynamicConfig/url_loader\".",
  L"Failed to find \"DynamicConfig/url_server\".",
  L"Error founded in \"DynamicConfig/AdvancedConfigs\".",
#if(BO_WININET > 0)
  L"Error founded in \"DynamicConfig/WebFilters\".",
  L"Error founded in \"DynamicConfig/WebDataFilters\".",
#endif  
  L"Error founded in \"DynamicConfig/WebFakes\".",
  L"Error founded in \"DynamicConfig/file_webinjects\".",
  L"Building the HTTP injects...",
  L"Failed to open file of HTTP injects.",
  L"Bad HTTP-inject founded.",
#endif

  L"Language:",
  L"Apply",
  L"For apply the language you should restart application.",
  L"Failed to save all settings.",
};

//Русский язык.
static const LPWSTR languageRu[] =
{
  L"Недостаточно памяти.",
  L"Не удалось сохранить файл.",
  L"\r\nОШИБКА: %s",

  BO_NAME L" Builder",

  L"Общая информация",
#if(BO_CLIENT_PLATFORMS > 0)
  L"Сборка",
#endif
  L"Опции",

  L"Текущая версия",
  L"Версия: %u.%u.%u.%u\r\nВремя сборки: %s\r\nСигнатура сборки: %S",
  L"Информация о запущенном боте",
  L"Ключ шифрования:",
  L"Информация:",
  L"Обновить",
  L"Удалить бота",
  L"Удаление бота...",
  L"Бот удален! Пожалуйста, перезагрузите компьютер для полного удаления бота.",
  L"Не удалось удалить бота! Попробуйте снова, или перезагрузите компьютер.",
  L"-- default --",
  L"Версия: %u.%u.%u.%u\r\nБотнет: %s\r\nФайл лоадера: ~\\%s",
  L"Бот не найден.",

#if(BO_CLIENT_PLATFORMS > 0)
  L"Исходный файл конфигурации:",
  L"Обзор...",
  L"Правка...",
  L"Действия",
  L"Собрать конфигурацию бота",
  L"Собрать бота",
  L"-- файл не выбран --",
  L"Не удалось запустить внешний редактор.",
  L"Не удалось прочитать файл конфигурации.",

  L"Сборка бота...",
  L"Загрузка конфигурации...",
  L"Создание EXE-файла...",
  L"Размер файла %u байт.",
  L"\r\nСБОРКА УСПЕШНО ЗАВЕРШЕНА!",
  
  L"Сборка конфигурации бота...\r\n",
  L"\r\nСБОРКА УСПЕШНО ЗАВЕРШЕНА!",  
  L"Не удалось найти \"StaticConfig\".",\
  L"Максимальная длина значения для \"StaticConfig/botnet\" не может быть более %u символов.",
  L"Не удалось найти \"StaticConfig/url_config\".",
  L"Максимальная длина значения для \"StaticConfig/url_config\" не может быть более %u символов.",
  L"Не удалось найти \"StaticConfig/encryption_key\".",
  L"Исходный PE-файл поврежден.",
  L"Не удалось найти \"DynamicConfig\".",
  L"Не удалось найти \"DynamicConfig/url_loader\".",
  L"Не удалось найти \"DynamicConfig/url_server\".",
  L"Обнаружена ошибка в \"DynamicConfig/AdvancedConfigs\".",
#if(BO_WININET > 0)
  L"Обнаружена ошибка в \"DynamicConfig/WebFilters\".",
  L"Обнаружена ошибка в \"DynamicConfig/WebDataFilters\".",
#endif
  L"Обнаружена ошибка в \"DynamicConfig/WebFakes\".",
  L"Обнаружена ошибка в \"DynamicConfig/file_webinjects\".",
  L"Сборка HTTP-инжектов...",
  L"Не удалось открыть файл с HTTP-инжектами.",
  L"Обнаружен HTTP-инжект с ошибкой.",
#endif

  L"Язык интерфейса:",
  L"Применить",
  L"Для применения языка необходимо перезапустить приложение.",
  L"Не удалось сохранить все настройки.",
};

//Список доступных языков.
static const Languages::LANGINFO languages[] =
{
  {L"English",           languageEn, sizeof(languageEn) / sizeof(LPWSTR), 1033},
  {L"Русский (Russian)", languageRu, sizeof(languageRu) / sizeof(LPWSTR), 1049}
};

//Данные текущего языка.
static const Languages::LANGINFO *languageCur;

void Languages::init(void)
{
  //Язык по умолчанию.
  languageCur = &languages[0];
  
  //Получаем язык из системы.
  WORD langId = CWA(kernel32, GetUserDefaultUILanguage)();
  for(BYTE i = 0; i < sizeof(languages) / sizeof(Languages::LANGINFO); i++)if(langId == languages[i].id){languageCur = &languages[i]; break;}
  
  //Получем язык из конфига.
  langId = CWA(kernel32, GetPrivateProfileIntW)(L"settings", L"language", langId, settingsFile);
  for(BYTE i = 0; i < sizeof(languages) / sizeof(Languages::LANGINFO); i++)if(langId == languages[i].id){languageCur = &languages[i]; break;}
}

void Languages::uninit(void)
{

}

LPWSTR Languages::get(DWORD id)
{
  return (id < languageCur->stringsCount) ? languageCur->strings[id] : NULL;
}

bool Languages::setDefaultLangId(WORD langId)
{
  WCHAR str[10];
  Str::_FromInt32W(langId, str, 10, false);
  return CWA(kernel32, WritePrivateProfileStringW)(L"settings", L"language", str, settingsFile) == FALSE ? false : true;
}

const Languages::LANGINFO *Languages::getLangInfo(WORD index)
{
  if(index < sizeof(languages) / sizeof(Languages::LANGINFO))return &languages[index];
  return NULL;
}

const Languages::LANGINFO *Languages::getCurLangInfo(void)
{
  return languageCur;
}
