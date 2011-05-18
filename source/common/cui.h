/*
  Вспомогательные функции для консольного интерфейса.
*/
#pragma once

namespace Cui
{
  //Exit codes for ExitProcess. By default, the exit code is EXITCODE_BAD_COMMAND_LINE!
  enum
  {
    EXITCODE_SUCCESSED,                  //No errors.
    EXITCODE_ERROR_BAD_COMMAND_LINE,     //Error in command line syntax.
    EXITCODE_ERROR_FILE_NOT_FOUND,       //File not found.
    EXITCODE_ERROR_NOT_ENOUGH_MEMORY,    //Not enough memory.
    EXITCODE_ERROR_CONFIG_FORMAT,        //Error in the syntax of the configuration file.
    EXITCODE_ERROR_INPUT_FILE_CORRUPTED, //The source file is corrupted.
    EXITCODE_ERROR_SOCKET,               //Error when working with sockets.
    EXITCODE_ERROR_WRITE_OUTPUT_FILE,    //Error while writing destination file.
    EXITCODE_ERROR_CREATE_DIR,           //Failed to create directory.
    EXITCODE_ERROR_UNKNOWN               //Unknown error.
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

  //Structure to work with teams
  typedef struct
  {
    LPWSTR name;         //The name of the option.
    LPWSTR description;  //Description of the option.
  }SWITCH;

  typedef struct
  {
    LPWSTR name;              //The name of the team.
    LPWSTR description;       //Description of the team.
    COMMANDCALLBACK callback; //Function associating with the team.
    SWITCH *switches;         //Command options.
    BYTE switchesCount;       //Count. options.
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

/*В В Standard obrabodchik command line.*/
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
