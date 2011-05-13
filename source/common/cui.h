/*
  Вспомогательные функции для консольного интерфейса.
*/
#pragma once

namespace Cui
{
  //Коды выхода для ExitProcess. По умолчанию код выхода равен EXITCODE_BAD_COMMAND_LINE!
  enum
  {
    EXITCODE_SUCCESSED,                  //Ошибок нет.
    EXITCODE_ERROR_BAD_COMMAND_LINE,     //Ошибка в синтаксисе командной строки.
    EXITCODE_ERROR_FILE_NOT_FOUND,       //Файл не найден.
    EXITCODE_ERROR_NOT_ENOUGH_MEMORY,    //Не достаточно памяти.
    EXITCODE_ERROR_CONFIG_FORMAT,        //Ошибка в синтаксисе файла конфигурации.
    EXITCODE_ERROR_INPUT_FILE_CORRUPTED, //Исходный файл поврежден.
    EXITCODE_ERROR_SOCKET,               //Ошибка при работе с сокетами.
    EXITCODE_ERROR_WRITE_OUTPUT_FILE,    //Ошибка при записи конечного файла.
    EXITCODE_ERROR_CREATE_DIR,           //Ошибка при создании директории.
    EXITCODE_ERROR_UNKNOWN               //Неизвестная ошибка.
  };

  /*
    Функция для дальнейшей обработки командной строки, должны представлять из себя
    Cmd[Cmdname]::Call(...). Функция должна самостоятельно выставлять coreData.exitCode, и
    вызывать Core::showHelp() в случаи соответственной ошибки. Нужно учитывать, что синтаксис
    всех опции уже проверен, т.е. например если опция содержит какой-то параметр, то при вызове
    этой функции он проверено существует.

    IN switches      - опции передаваемые из командной строки. При поиски нужной опции внутри
                       функции, поиск должен производиться с учетом регистра.
    IN switchesCount - количество опции.
  */
  typedef void (* COMMANDCALLBACK)(LPWSTR *switches, DWORD switchesCount);

  //Структуры для работы с командами
  typedef struct
  {
    LPWSTR name;         //Имя опции.
    LPWSTR description;  //Описание опции.
  }SWITCH;

  typedef struct
  {
    LPWSTR name;              //Имя команды.
    LPWSTR description;       //Описание команды.
    COMMANDCALLBACK callback; //Функция ассоциированая с командой.
    SWITCH *switches;         //Опции команды.
    BYTE switchesCount;       //Кол. опций.
  }COMMAND;

  /*
    Инициализация.
  */
  void init(void);

  /*
    Деинициализация.
  */
  void uninit(void);

  /*
    Получение текущей команды из командной строки.
    
    IN args          - массив аргументов командной строки.
    IN argsCount     - количество элементов в args.
    IN commands      - массив поддерживаемых команд.
    IN commandsCount - количество элементов в commands.

    Return           - указатель на элемент из массива commands, или NULL если команда не найдена.
  */
  COMMAND *_getCommand(const LPWSTR *args, int argsCount, const COMMAND *commands, BYTE commandsCount);

  /*
    Проверка опций командной строки.

    IN args          - массив аргументов командной строки.
    IN argsCount     - количество элементов в args.
    IN switches      - массив поддерживаемых опций.
    IN switchesCount - количество элементов в switches.

    Return           - 0 - ошибок не найдено,
                       MAKELONG(1, i) - синтаксическая ошибка, где i индекс элемента в args,
                       MAKELONG(2, i) - неизвестная опция, где i индекс элемента в args.
  */
  DWORD _checkSwitches(const LPWSTR *args, int argsCount, const SWITCH *switches, BYTE switchesCount);

  /*
    Получение значения опции.

    IN switches       - массив опций.
    IN switchesCount  - количество элементов в pstrList.
    IN requeredSwitch - опция, которую нужно найти.

    Return            - если значение существует - указатель на значение опции,
                        если значение отсутвует  - (LPWSTR)1,
                        если опция отсутвует     - NULL.

    Внимание: После работы этой функции аргумент может отличаться от оригинального, из-за удаления
              кавычек в случаи их наличия.
  */
  LPWSTR _getSwitchValue(const LPWSTR *switches, DWORD switchesCount, const LPWSTR requeredSwitch);

  /*
    Отображение помоши.

    IN commands      - список команд.
    IN commandsCount - количетсво команд.
    IN switches      - список опции.
    IN switchesCount - количество опций.
    IN mainTitle     - шаблон заголовка, например
                       "usage: %s <command> -<switch 1> -<switch N>\n\n<Commands>\n"
    IN fileName      - имя файла для примера в mainTitle.
    IN switchesTitle - заголовое для опций.
  */
  void _showHelp(const COMMAND *commands, BYTE commandsCount, const LPWSTR mainTitle, const LPWSTR fileName);
};

/*
  Стандартное обрабодчик командной строки.
*/
#define CUI_DEFAULT_COMMANDLINE_HELPER \
{\
  Cui::COMMAND *currentCommand;\
  DWORD switchesResult;\
  if(coreData.commandLine.argsCount < 2)\
  {\
    Core::showLogo();\
    Cui::_showHelp(commands, sizeof(commands) / sizeof(Cui::COMMAND), lng_help_title, CWA(shlwapi, PathFindFileNameW)(coreData.fileName));\
  }\
  else if((currentCommand = Cui::_getCommand(coreData.commandLine.args, coreData.commandLine.argsCount, commands, sizeof(commands) / sizeof(Cui::COMMAND))) == NULL)\
  {\
    Core::showLogo();\
    Console::writeFormatW(lng_error_uknown_command, coreData.commandLine.args[1]);\
  }\
  else if((switchesResult = Cui::_checkSwitches(coreData.commandLine.args, coreData.commandLine.argsCount, currentCommand->switches, currentCommand->switchesCount)) != 0)\
  {\
    Core::showLogo();\
    WORD index = HIWORD(switchesResult);\
    if(LOWORD(switchesResult) == 1)Console::writeFormatW(lng_error_syntax_error, coreData.commandLine.args[index]);\
    else Console::writeFormatW(lng_error_uknown_switch, coreData.commandLine.args[index]);\
  }\
  else\
  {\
    if(Cui::_getSwitchValue(coreData.commandLine.args + 2, coreData.commandLine.argsCount - 2, lng_switch_nologo) != (LPWSTR)1)Core::showLogo();\
    currentCommand->callback(coreData.commandLine.args + 2, coreData.commandLine.argsCount - 2);\
  }\
}
