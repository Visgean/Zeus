#pragma once
/*
  Инструменты для работы со строками ANSI и Unicode.

  Примечение: Фунции не расчитаны на работу со строками размером более 0x7FFFFFFF симолов.
*/

//Опеределяет пробельный ли это символ.
#define IS_SPACE_CHAR(c) ((c) == 0x20 || ((c) >= 0x9 && (c) <= 0xD))

//UTF8 BOM
#define UTF8_BOM      0xBFBBEF
#define UTF8_BOM_SIZE 3

//UTF16 BOM
#define UTF16_BOM      0xFEFF
#define UTF16_BOM_SIZE 2

namespace Str
{  
  enum
  {
    STS_TRIM          = 0x1, //Автоматически вызываь Trim для каждой строки.
    STS_USE_SEPARATOR = 0x2, //Использовать разделитель вместо стандартного способа \r\n.
  };
  
  enum
  {
    STA_FORMAT_C = 0x1, //Аргументы находяться в формате C-строк.
  };

  //Описание UTF-8 строки.
  typedef struct
  {
    void *data;   //Строка.
    DWORD lenght; //Размер строки в символах, исключая нулевой символ.
    DWORD size;   //Размер строки в байтах, исключая нулевой символ.
  }UTF8STRING;

  typedef struct
  {
    char anyCharSymbol;  //Символ одного любого символа, обычно '?'.
    char anyCharsSymbol; //Символ любого кол. любых символов, обычно '*'.
    LPSTR mask;          //Маска.
    DWORD maskSize;      //Размер маски.
    LPSTR string;        //Строка.
    DWORD stringSize;    //Размер строки.
    DWORD beginOfMatch;  //OUT индекс string, с которого начинается сравнение. Всегда 0, если не
                         //установлен флаг MATCH_SEARCH_SUBSSTRING.
    DWORD endOfMatch;    //OUT индекс string, на котором остановилось сравнение.
    DWORD flags;         //Флаги MATCH_*.
  }MATCHDATAA;

  //Флаги MATCHDATAX.flags.
  enum
  {
    MATCH_FULL_EQUAL            = 0x01, //Маска должна распорстарняться до конца строки.    
    MATCH_UNIVERSAL_NEWLINE     = 0x02, //Не различтаать \r\n и \n.
    MATCH_CASE_INSENSITIVE      = 0x04, //Без учета регистра.
    MATCH_CASE_INSENSITIVE_FAST = 0x08, //Без учета регистра для символов A-Z, a-z.
    MATCH_SEARCH_SUBSSTRING     = 0x10  //Поиск поодстроки в строке. (Аналогично маске "*mask").
  };
  
  /*
    Инициализация.
  */
  void Init(void);

  /*
    Деинициализация.
  */
  void Uninit(void);

  /*
    Проверяет, является ли строка правильной null-terminated строкой.

    IN pstrString - строка для проверки.
    IN dwSize     - размер строки в символах.
    
    Return        - true - в случаи успеха,
                    false - в случаи ошибки.
  */
  bool _IsValidA(LPSTR pstrString, DWORD dwSize);
  
  /*
    Проверяет, является ли строка правильной null-terminated строкой.

    IN pstrString - строка для проверки.
    IN dwSize     - размер строки в символах.
    
    Return        - true - в случаи успеха,
                    false - в случаи ошибки.
  */
  bool _IsValidW(LPWSTR pstrString, DWORD dwSize);

  /*
    Проверяет, является ли строка десятичным числом.

    IN pstrString - строка для проверки.
    IN iSize      - размер строки, или -1.

    Return        - true - строка десятичное число,
                    false - строка не десятичное число.
  */
  bool _IsDecimalStringW(LPCWSTR pstrString, int iSize);

  /*
    Преобразует байт в 16-ти ричное число.

    IN bt   - байт.
    OUT buf - строка.
  */
  void _byteToCharA(BYTE bt, LPSTR buf);

  /*
    Преобразует байт в 16-ти ричное число.

    IN bt   - байт.
    OUT buf - строка.
  */
  void _byteToCharW(BYTE bt, LPWSTR buf);

  /*
    Копирование Unicode строки.
    
    OUT pstrDest  - конечная срока, не может быть NULL.
    IN pstrSource - исходная строка, может быть NULL.
    IN iSize      - количество символов для копирования, или -1 для полного копирование строки.
  */
  void _CopyW(LPWSTR pstrDest, LPCWSTR pstrSource, int iSize);

  /*
    Копирование Unicode строки с выделением памяти.

    IN pstrSource - исходная строка, может быть NULL.
    IN iSize      - количество символов для копирования, или -1 для полного копирование строки.

    Return        - указатель на новую копию строки.
  */
  LPWSTR _CopyExW(LPWSTR pstrSource, int iSize);

  /*
    Добавление Unicode строки в конец другой строки.

    IN buf    - буффер.
    IN source - исходная строка, может быть NULL.
    IN size   - количество символов для копирования, или -1 для полного копирование строки.
  */
  void _catW(LPWSTR buf, LPWSTR source, int size);

  /*
    Добавление Unicode строки в конец другой строки, с выделением памяти.

    IN OUT ppstrBuf - буффер.
    IN pstrSource   - исходная строка, может быть NULL.
    IN iSize        - количество символов для копирования, или -1 для полного копирование строки.
  */
  bool _CatExW(LPWSTR *ppstrBuf, LPWSTR pstrSource, int iSize);

  /*
    Копирование ANSI строки.

    OUT pstrDest  - конечная срока, не может быть NULL.
    IN pstrSource - исходная строка, может быть NULL.
    IN iSize      - количество символов для копирования, или -1 для полного копирование строки.
  */
  void _CopyA(LPSTR pstrDest, LPCSTR pstrSource, int iSize);

  /*
    Копирование ANSI строки с выделением памяти.

    IN pstrSource - исходная строка, может быть NULL.
    IN iSize      - количество символов для копирования, или -1 для полного копирование строки.

    Return        - указатель на новую копию строки.
  */
  LPSTR _CopyExA(LPSTR pstrSource, int iSize);

  /*
    Добавление ANSI строки в конец другой строки.

    IN buf    - буффер.
    IN source - исходная строка, может быть NULL.
    IN size   - количество символов для копирования, или -1 для полного копирование строки.
  */
  void _catA(LPSTR buf, LPSTR source, int size);

  /*
    Добавление ANSI строки в конец другой строки, с выделением памяти.

    IN OUT ppstrBuf - буффер.
    IN pstrSource   - исходная строка, может быть NULL.
    IN iSize        - количество символов для копирования, или -1 для полного копирование строки.
  */
  bool _CatExA(LPSTR *ppstrBuf, LPSTR pstrSource, int iSize);
  
  /*
    Удаление символов \r, \n, \t и пробела с начала и конца ANSI строки.

    IN pstrStr - строка для обработки.
  */
  void _TrimA(LPSTR pstrStr);

  /*
    Удаление символов \r, \n, \t и пробела с начала и конца Unicode строки.

    IN pstrStr - строка для обработки.
  */
  void _TrimW(LPWSTR pstrStr);

  /*
    Преоброзование десятичной/шеснацатиричной ANSI строки в int64. Преоброзование длиться до тех пор пока не
    встретиться символ не равный '0' - 'F'.

    IN pstrStr - строка для обработки.
    OUT pbSign - если не NULL, возращает: true - число было со знаоком '-',
                 false - число было без знака '-'.

    Return     - полученое число.
  */
  __int64 _ToInt64A(LPSTR pstrStr, bool *pbSign);
  
  /*
    Преоброзование десятичной/шеснацатиричной ANSI строки в int32. Преоброзование длиться до тех пор пока не
    встретиться символ не равный '0' - 'F'.

    IN pstrStr - строка для обработки.
    OUT pbSign - если не NULL, возращает: true - число было со знаоком '-',
                 false - число было без знака '-'.

    Return     - полученое число.
  */
  int _ToInt32A(LPSTR pstrStr, bool *pbSign);

  /*
    Преоброзование десятичной/шеснацатиричной Unicode строки в int64. Преоброзование длиться до тех пор пока не
    встретиться символ не равный '0' - 'F'.

    IN pstrStr - строка для обработки.
    OUT pbSign - если не NULL, возращает: true - число было со знаоком '-',
                 false - число было без знака '-'.
    
    Return     - полученое число.
  */
  __int64 _ToInt64W(LPWSTR pstrStr, bool *pbSign);

  /*
    Преоброзование десятичной/шеснацатиричной Unicode строки в int32. Преоброзование длиться до тех пор пока не
    встретиться символ не равный '0' - 'F'.

    IN pstrStr - строка для обработки.
    OUT pbSign - если не NULL, возращает: true - число было со знаоком '-',
                 false - число было без знака '-'.

    Return     - полученое число.
  */
  int _ToInt32W(LPWSTR pstrStr, bool *pbSign);
  
  /*
    Преобразование x64 числа в ANSI строку.

    IN iNumber - исходное число.
    OUT pBuf   - буффер.
    IN bRadix  - система счисления.
    IN bSigned - true - если числознаковое, false - если число беззнаковое.

    Return     - количество скопированных байт в pBuf.
  */
  DWORD _FromInt64A(__int64 iNumber, LPSTR pBuf, BYTE bRadix, bool bSigned);
  
  /*
    Преобразование x32 числа в ANSI строку.

    IN iNumber - исходное число.
    OUT pBuf   - буффер.
    IN bRadix  - система счисления.
    IN bSigned - true - если числознаковое, false - если число беззнаковое.

    Return     - количество скопированных байт в pBuf.
  */
  DWORD _FromInt32A(int iNumber, LPSTR pBuf, BYTE bRadix, bool bSigned);

  /*
    Преобразование x64 числа в Unicode строку.

    IN iNumber - исходное число.
    OUT pBuf   - буффер.
    IN bRadix  - система счисления.
    IN bSigned - true - если числознаковое, false - если число беззнаковое.

    Return     - количество скопированных байт в pBuf
  */
  DWORD _FromInt64W(__int64 iNumber, LPWSTR pBuf, BYTE bRadix, bool bSigned);

  /*
    Преобразование x64 числа в Unicode строку.

    IN iNumber - исходное число.
    OUT pBuf   - буффер.
    IN bRadix  - система счисления.
    IN bSigned - true - если числознаковое, false - если число беззнаковое.

    Return     - количество скопированных байт в pBuf
  */
  DWORD _FromInt32W(int iNumber, LPWSTR pBuf, BYTE bRadix, bool bSigned);

  /*
    Преоброзование десятичной ANSI строки в DWORD. Преоброзование длиться до тех пор пока не
    встретиться символ не равный '0' - '9', 'A' - 'F', 'a' - 'f'. Также если в начале строки
    стоит прейикс 0x, он успешно пропустится.

    IN str - строка для обработки.

    Return - полученое число.
  */
  DWORD _hexStringToDwordA(LPSTR str);

  /*
    Преоброзование десятичной Unicode строки в DWORD. Преоброзование длиться до тех пор пока не
    встретиться символ не равный '0' - '9', 'A' - 'F', 'a' - 'f'. Также если в начале строки
    стоит прейикс 0x, он успешно пропустится.

    IN str - строка для обработки.

    Return - полученое число.
  */
  DWORD _hexStringToDwordW(LPWSTR str);

  /*
    Преобразование HEX-строки в бинарные данные.

    IN string  - строка.
    OUT buffer - буфер. Может быть указателем на string.

    Return     - true - если строка успешно приобразована, или пуста,
                 false - в случаи ошибки.                                                      
  */
  bool _fromHexW(LPWSTR string, void *buffer);

  /*
    Преобразование бинарных данных в HEX-строку.

    IN buffer     - буфер. Может быть указателем на string.
    IN bufferSize - размер буфера.
    OUT string    - строка. Ее размер должн быть не менее (bufferSize * 2 + 1) символов.
  */
  void _toHexW(const void *binary, DWORD binarySize, LPWSTR string);

  /*
    Преобразование бинарных данных в HEX-строку.

    IN buffer     - буфер. Может быть указателем на string.
    IN bufferSize - размер буфера.
    OUT string    - строка. Ее размер должн быть не менее (bufferSize * 2 + 1) символов.

    Return     - true - если строка успешно приобразована, или пуста,
                 false - в случаи ошибки.                                                      
  */
  void _toHexA(const void *binary, DWORD binarySize, LPSTR string);

  /*
    Преобразует строковые спец. символы С в номарльные байты.
    
    IN OUT str - строка для обработки.
  */
  void _fromCFormatA(LPSTR str);

  /*
    Преобразует строковые спец. символы С в номарльные байты.
    
    IN OUT str - строка для обработки.
  */
  void _fromCFormatW(LPWSTR str);

  /*
    Разделение строки по строкам на основе символа \n или \r. Уневераслен для Win, Mac, Unix.

    IN input     - исходная строка.
    IN inputSize - размер исходной строки.
    OUT strings  - массив указателей строк, должен быть освобожден через Mem::freeArrayOfPointers.
                   Строки, которые будут иметь нулевой размер могут быть отмечены как
                   NULL-указателем, так и пустой строкой.
    IN flags     - флаги STS_*.
    IN separator - разделитель строк, актуально тольк при флаге STS_USE_SEPARATOR.
                  
    Return       - количетсво полученых строк, или (DWORD)-1 в случаи возниконвения ошибки.

  */
  DWORD _splitToStringsA(LPSTR input, DWORD inputSize, LPSTR **strings, DWORD flags, char separator);

  /*
    Разделение строки по строкам на основе символа \n или \r. Уневераслен для Win, Mac, Unix.

    IN input     - исходная строка.
    IN inputSize - размер исходной строки.
    OUT strings  - массив указателей строк, должен быть освобожден через Mem::freeArrayOfPointers.
                   Строки, которые будут иметь нулевой размер могут быть отмечены как
                   NULL-указателем, так и пустой строкой.
    IN flags     - флаги STS_*.
    IN separator - разделитель строк, актуально тольк при флаге STS_USE_SEPARATOR.
                  
    Return       - количетсво полученых строк, или (DWORD)-1 в случаи возниконвения ошибки.

  */
  DWORD _splitToStringsW(LPWSTR input, DWORD inputSize, LPWSTR **strings, DWORD flags, WCHAR separator);

  /*
    Разделение строки на агрумента с подедржкой ковычек.

    IN input     - исходная строка.
    IN inputSize - размер исходной строки.
    OUT args     - массив указателей строк, должен быть освобожден через Mem::freeArrayOfPointers.
                   Строки, которые будут иметь нулевой размер будут отмечены как "\0".
    IN flags     - флаги STA_*.

    Return       - количетсво полученых аргументов, или (DWORD)-1 в случаи возниконвения ошибки.
  */
  DWORD _getArgumentsA(const LPSTR input, DWORD inputSize, LPSTR **args, DWORD flags);

  /*
    Разделение строки на агрумента с подедржкой ковычек.

    IN input     - исходная строка.
    IN inputSize - размер исходной строки.
    OUT args     - массив указателей строк, должен быть освобожден через Mem::freeArrayOfPointers.
                   Строки, которые будут иметь нулевой размер будут отмечены как "\0".
    IN flags     - флаги STA_*.

    Return       - количетсво полученых аргументов, или (DWORD)-1 в случаи возниконвения ошибки.
  */
  DWORD _getArgumentsW(const LPWSTR input, DWORD inputSize, LPWSTR **args, DWORD flags);

  /*
    Объединение аргументов в одну строку.

    IN args      - аргументы.
    IN argsCount - кол. акргументов.

    Return       - строка в случаи успеха (нужно освободить через Mem),
                   false - в случаи провала.
  */
  LPWSTR _joinArgumentsW(const LPWSTR *args, DWORD argsCount);
  
  /*
    Создание ANSI строки из Unicode.

    IN source     - исходная Unicode-строка.
    IN sourceSize - размер source в символах, или -1 для автоопределения.
    OUT dest      - новая ANSI-строка, всегда будет кончаться на 0. Может быть NULL при
                    destSize == 0.
    IN destSize   - размер буфера dest в байтах, включая нулевой символ. Если 0, то функуции
                    вернет требуемый размер буфера в байтах, исключая нулевой символ.

    Return        - 0 - ошибка (dest[0] будет равен нулю),
                    или кол. байт скопированных или требуемых в dest, исключая нулевой символ.
  */
  int _unicodeToAnsi(const LPWSTR source, int sourceSize, LPSTR dest, int destSize);

  /*
    Создание ANSI строки из Unicode, с выделением памяти.

    IN source - исходная строка.
    IN size   - размер pstrSource в символах, или -1 для автоопределения.

    Return    - ANSI-строка(всегда завершена на 0), или NULL в случаи ошибки.
  */
  LPSTR _unicodeToAnsiEx(const LPWSTR source, int size);

  /*
    Создание Unicode строки из ANSI.

    IN source     - исходная ANSI-строка.
    IN sourceSize - размер source в символах, или -1 для автоопределения.
    OUT dest      - новая Unicode-строка, всегда будет кончаться на 0. Может быть NULL при
                    destSize == 0.
    IN destSize   - размер буфера dest в символах, включая нулевой символ. Если 0, то функуции
                    вернет требуемый размер буфера в символах, исключая нулевой символ.
    
    Return        - 0 - ошибка (dest[0] будет равен нулю),
                    или кол. символоов скопированных или требуемых в dest, исключая нулевой символ.
  */
  int _ansiToUnicode(LPSTR source, int sourceSize, LPWSTR dest, int destSize);

  /*
    Создание Unicode строки из ANSI, с выделением памяти.

    IN source - исходная ANSI-строка.
    IN size   - размер pstrSource в символах, или -1 для автоопределения.

    Return    - Unicode-строка(всегда завершена на 0), или NULL в случаи ошибки.
  */
  LPWSTR _ansiToUnicodeEx(LPSTR source, int size);

  /*
    Создание Unicode строки из OEM.

    IN source     - исходная OEM-строка.
    IN sourceSize - размер source в символах, или -1 для автоопределения.
    OUT dest      - новая Unicode-строка, всегда будет кончаться на 0. Может быть NULL при
                    destSize == 0.
    IN destSize   - размер буфера dest в символах, включая нулевой символ. Если 0, то функуции
                    вернет требуемый размер буфера в символах, исключая нулевой символ.
    
    Return        - 0 - ошибка (dest[0] будет равен нулю),
                    или кол. символоов скопированных или требуемых в dest, исключая нулевой символ.
  */
  int _oemToUnicode(LPSTR source, int sourceSize, LPWSTR dest, int destSize);

  /*
    Создание Unicode строки из OEM, с выделением памяти.

    IN source - исходная OEM-строка.
    IN size   - размер pstrSource в символах, или -1 для автоопределения.

    Return    - Unicode-строка, или NULL в случаи ошибки.
  */
  LPWSTR _oemToUnicodeEx(LPSTR source, int size);

  /*
    Вычисляет размер Unicode строки.

    IN pstrStr - строка для обработки. Может быть NULL.

    Return     - размер строки.
  */
  int _LengthA(LPCSTR pstrStr);

  /*
    Вычисляет размер ANSI строки.

    IN pstrStr - строка для обработки. Может быть NULL.

    Return     - размер строки.
  */
  int _LengthW(LPCWSTR pstrStr);

  /*
    Сравнивает две ANSI строки.

    IN pstrStr1 - первая строка. Может быть NULL.
    IN pstrStr2 - вторая строка. Может быть NULL.
    IN iSize1   - размер первой строки или -1.
    IN iSize2   - размер второй строки или -1.

    Return      -  0 - строки равны,
                  -1 - первая строка меньше второй строки.
                   1 - первая строка больше второй строки.
  */
  int _CompareA(LPCSTR pstrStr1, LPCSTR pstrStr2, int iSize1, int iSize2);

  /*
    Сравнивает две Unicode строки.

    IN pstrStr1 - первая строка. Может быть NULL.
    IN pstrStr2 - вторая строка. Может быть NULL.
    IN iSize1   - размер первой строки или -1.
    IN iSize2   - размер второй строки или -1.

    Return      -  0 - строки равны,
                  -1 - первая строка меньше второй строки.
                   1 - первая строка больше второй строки.
  */
  int _CompareW(LPCWSTR pstrStr1, LPCWSTR pstrStr2, int iSize1, int iSize2);

  /*
    Поиск символа в ANSI строке.

    IN string - строка, в которой будет производиться поиск.
    IN c      - символ для поиска.

    Return    - указатель на первый найденый символ, или NULL если символ не найден.
  */
  LPSTR _findCharA(const LPSTR string, char c);

  /*
    Поиск символа в ANSI строке с правой стороны.

    IN string - строка, в которой будет производиться поиск.
    IN c      - символ для поиска.

    Return    - указатель на первый найденый символ, или NULL если символ не найден.
  */
  LPSTR _findRCharA(const LPSTR string, char c);

  /*
    Поиск символа в Unicode строке.

    IN string - строка, в которой будет производиться поиск.
    IN c      - символ для поиска.

    Return    - указатель на первый найденый символ, или NULL если символ не найден.
  */
  LPWSTR _findCharW(const LPWSTR string, WCHAR c);
  
  /*
    Поиск символа в Unicode строке с правой стороны.

    IN string - строка, в которой будет производиться поиск.
    IN c      - символ для поиска.

    Return    - указатель на первый найденый символ, или NULL если символ не найден.
  */
  LPWSTR _findRCharW(const LPWSTR string, WCHAR c);

  /*
    Поиск подстроки в ANSI строке.

    IN string    - строка, в которой будет производиться поиск. Не может быть NULL.
    IN subString - подстрока для поиска. Не может быть NULL.

    Return       - указатель на первую найденую строку, или NULL если строка не найдена.
  */
  LPSTR _findSubStringA(const LPSTR string, const LPSTR subString);

  /*
    Поиск подстроки в Unicode строке.
    
    IN string    - строка, в которой будет производиться поиск. Не может быть NULL.
    IN subString - подстрока для поиска. Не может быть NULL.

    Return           - указатель на первую найденую строку, или NULL если строка не найдена.
  */
  LPWSTR _findSubStringW(const LPWSTR string, const LPWSTR subString);
  
  /*
    Расширение для wnsprintfW..

    OUT pBuf      - указатель на буфер конечной строки.
    IN iBufSize   - размер буфера (включая нулевой символ).
    IN pstrFormat - формат строки.
    IN ...        - аргументы строки.

    Return        - размер строки без нулевого символа, или -1 в случаи ошибки.
  */
  int _sprintfW(LPWSTR pBuf, int iBufSize, LPCWSTR pstrFormat, ...);
  
  /*
    Расширение для wnsprintfA..

    OUT pBuf      - указатель на буфер конечной строки.
    IN iBufSize   - размер буфера (включая нулевой символ).
    IN pstrFormat - формат строки.
    IN ...        - аргументы строки.

    Return        - размер строки без нулевого символа, или -1 в случаи ошибки.
  */
  int _sprintfA(LPSTR pBuf, int iBufSize, LPCSTR pstrFormat, ...);

  /*
    Расширение для wvnsprintfW..

    OUT pBuf      - указатель на буфер конечной строки.
    IN iBufSize   - размер буфера (включая нулевой символ).
    IN pstrFormat - формат строки.
    IN arglist    - аргументы сроки.

    Return        - размер строки без нулевого символа, или -1 в случаи ошибки.
  */
  int _vsprintfW(LPWSTR pBuf, int iBufSize, LPCWSTR pstrFormat, va_list arglist);

  /*
    Расширение для wvnsprintfW..

    OUT pBuf      - указатель на буфер конечной строки.
    IN iBufSize   - размер буфера (включая нулевой символ).
    IN pstrFormat - формат строки.
    IN arglist    - аргументы сроки.

    Return        - размер строки без нулевого символа, или -1 в случаи ошибки.
  */
  int _vsprintfA(LPSTR pBuf, int iBufSize, LPCSTR pstrFormat, va_list arglist);

  /*
    Расширение для wsprintfW, которое автоматичепски выделяет память.

    OUT ppBuf     - указатель на буфер конечной строки. Должен быть освобожден через Mem.
    IN pstrFormat - формат строки.
    IN ...        - аргументы строки.

    Return        - размер строки без нулевого символа, или -1 в случаи ошибки.

    Примечание: макимальный размер выходной строки ~MAX_SPRINTF_STRING_SIZE;
  */
  int _sprintfExW(LPWSTR *ppBuf, LPCWSTR pstrFormat, ...);

  /*
    Расширение для _sprintfExW, которое добавляет новые строку в конец сушетвующей.

    IN OUT buffer - буффер. Может указывать на NULL. В случаи ошибки и если не был указан NULL,
                    будет восстановлен до offset символов с окончанием на нулевой символ.
    IN offset     - оффсет в символах в *buffer, с которого нужно начать вносить изменения.
    IN format     - формат строки.
    IN ...        - аргументы для строки.

    Return        - полный размер строки без нулевого символа, или (DWORD)-1 в случаи ошибки.
  */
  DWORD _sprintfCatExW(LPWSTR *buffer, DWORD offset, const LPWSTR format, ...);

  /*
    Расширение для wsprintfA, которое автоматичепски выделяет память.

    OUT ppBuf     - указатель на буфер конечной строки. Должен быть освобожден через Mem.
    IN pstrFormat - формат строки.
    IN ...        - аргументы строки.

    Return        - размер строки без нулевого символа, или -1 в случаи ошибки.

    Примечание: макимальный размер выходной строки ~MAX_SPRINTF_STRING_SIZE;
  */
  int _sprintfExA(LPSTR *ppBuf, LPCSTR pstrFormat, ...);

  /*
    Расширение для wvsprintfW, которое автоматичепски выделяет память.

    OUT ppBuf     - указатель на буфер конечной строки. Должен быть освобожден через Mem.
    IN pstrFormat - формат строки.
    IN arglist    - аргументы сроки.

    Return        - размер строки без нулевого символа, или -1 в случаи ошибки.

    Примечание: макимальный размер выходной строки ~MAX_SPRINTF_STRING_SIZE;
  */
  int _vsprintfExW(LPWSTR *ppBuf, LPCWSTR pstrFormat, va_list arglist);
  
  /*
    Расширение для _sprintfExA, которое добавляет новые строку в конец сушетвующей.

    IN OUT buffer - буффер. Может указывать на NULL. В случаи ошибки и если не был указан NULL,
                    будет восстановлен до offset символов с окончанием на нулевой символ.
    IN offset     - оффсет в символах в *buffer, с которого нужно начать вносить изменения.
    IN format     - формат строки.
    IN ...        - аргументы для строки.

    Return        - полный размер строки без нулевого символа, или (DWORD)-1 в случаи ошибки.
  */
  DWORD _sprintfCatExA(LPSTR *buffer, DWORD offset, const LPSTR format, ...);

  /*
    Расширение для _vsprintfExW, которое добавляет новые строку в конец сушетвующей.

    IN OUT buffer - буффер. Может указывать на NULL. В случаи ошибки и если не был указан NULL,
                    будет восстановлен до offset символов с окончанием на нулевой символ.
    IN offset     - оффсет в символах в *buffer, с которого нужно начать вносить изменения.
    IN format     - формат строки.
    IN arguments  - аргументы для строки.

    Return        - полный размер строки без нулевого символа, или (DWORD)-1 в случаи ошибки.
  */
  DWORD _vsprintfCatExW(LPWSTR *buffer, DWORD offset, const LPWSTR format, va_list arguments);

  /*
    Расширение для wvsprintfA, которое автоматичепски выделяет память.

    OUT ppBuf     - указатель на буфер конечной строки. Должен быть освобожден через Mem.
    IN pstrFormat - формат строки.
    IN arglist    - аргументы сроки.

    Return        - размер строки без нулевого символа, или -1 в случаи ошибки.

    Примечание: макимальный размер выходной строки ~MAX_SPRINTF_STRING_SIZE;
  */
  int _vsprintfExA(LPSTR *ppBuf, LPCSTR pstrFormat, va_list arglist);
  
  /*
    Расширение для _vsprintfExA, которое добавляет новые строку в конец сушетвующей.

    IN OUT buffer - буффер. Может указывать на NULL. В случаи ошибки и если не был указан NULL,
                    будет восстановлен до offset символов с окончанием на нулевой символ.
    IN offset     - оффсет в символах в *buffer, с которого нужно начать вносить изменения.
    IN format     - формат строки.
    IN arguments  - аргументы для строки.

    Return        - полный размер строки без нулевого символа, или (DWORD)-1 в случаи ошибки.
  */
  DWORD _vsprintfCatExA(LPSTR *buffer, DWORD offset, const LPSTR format, va_list arguments);

  /*
    Проверяет являются ли байты UTF-8 BOM.

    IN data - байты для проверки.
  
    Return  - true - в случаи успеха,
              false - в случаи ошибки.
  */
  bool _utf8IsBom(const void *data);

  /*
    Освобождение строки.

    IN string  - строка UTF-8.
  */
  void _utf8Free(UTF8STRING *string);

  /*
    Создание строки UTF-8 из Unicode.

    IN source     - исходная строка.
    IN sourceSize - размер строки pstrSource, или -1 для автоопределения.
    OUT dest      - строка UTF-8.

    Return        - true - в случаи успеха,
                    false - в случаи ошибки.
  */
  bool _utf8FromUnicode(const LPWSTR source, int sourceSize, UTF8STRING *dest);

  /*
    Создание строки UTF-8 из ANSI.

    IN source     - исходная строка.
    IN sourceSize - размер строки pstrSource, или -1 для автоопределения.
    OUT dest      - строка UTF-8.

    Return        - true - в случаи успеха,
                    false - в случаи ошибки.
  */
  bool _utf8FromAnsi(const LPSTR source, int sourceSize, UTF8STRING *dest);

  /*
    Приобразование UTF-8 строки в Unicode.

    IN rawString - UTF-8 строка.
    IN size      - размер строки в байтах, или -1 для автоопределения.

    Return       - Unicode-строка в случаи успеха (должна быть освобождена через Mem),
                   или NULL в случаи ошибки.
  */
  LPWSTR _utf8ToUnicode(const void *rawString, const int size);

  /*
    Проверяет является ли строка, правильной мульти-строкой.

    IN string - мульти-строка для проверки.
    IN size   - размер мульти-строки.
    
    Return    - true - в случаи успеха,
                false - в случаи ошибки.
  */
  bool _isValidMultiStringA(const LPSTR string, DWORD size);

  /*
    Проверяет является ли строка, правильной мульти-строкой.

    IN string - мульти-строка для проверки.
    IN size   - размер мульти-строки.
    
    Return    - true - в случаи успеха,
                false - в случаи ошибки.
  */
  bool _isValidMultiStringW(const LPWSTR string, DWORD size);

  /*
    Получает строку из мульти-строки по индексу.

    IN string - мульти-строка.
    IN index  - индекс.

    Return    - указатель на строку, или NULL в случаи ошибки.
  */
  LPSTR _multiStringGetIndexA(LPSTR string, DWORD index);

  /*
    Получает строку из мульти-строки по индексу.

    IN string - мульти-строка.
    IN index  - индекс.

    Return    - указатель на строку, или NULL в случаи ошибки.
  */
  LPWSTR _multiStringGetIndexW(LPWSTR string, DWORD index);

  /*
    Получение кол. строк в мульти-строке.

    IN string - мульти-строка.

    Return    - кол. строк.
  */
  DWORD _multiStringGetCountA(const LPSTR string);
  
  /*
    Получение кол. строк в мульти-строке.

    IN string - мульти-строка.

    Return    - кол. строк.
  */
  DWORD _multiStringGetCountW(const LPWSTR string);

  /*
    Создает мультистроку с одной строкой из строки.

    IN string  - исходная строка.
    OUT length - полный размер строки в символах, влючая нулевые символы.

    Return     - мульистрока(нужно освободить через Mem), или NULL в вслучаи ошибки.
  */
  LPSTR _multiStringCreateA(const LPSTR string, LPDWORD length);

  /*
    Создает мультистроку с одной строкой из строки.

    IN string  - исходная строка.
    OUT length - полный размер строки в символах, влючая нулевые символы.

    Return     - мульистрока(нужно освободить через Mem), или NULL в вслучаи ошибки.
  */
  LPWSTR _multiStringCreateW(const LPWSTR string, LPDWORD length);

  /*
    Возращает не форматированую строку для системной ошибки.

    IN dwErrorCode - код ошибки.

    Return         - строку ошибки, или NULL в случаи ошибки. Строка должна быть освобождена через
                     Mem.
  */
  LPWSTR _GetErrorString(DWORD dwErrorCode);

  /*
    Добавление экранизация симоволов \, ', ", \0 - слэшем. Аналог addslashes из PHP.

    IN source - исходная строка.
    IN dest   - конечная строка.
  */
  void _addSlashesW(LPWSTR source, LPWSTR dest);

  /*
    Добавление экранизация симоволов с выделением памяти.

    IN source - исходная строка.
    IN size   - размер исходной строки в символах. Может быть -1.

    Return    - NULL - в случаи ошибки, адрес новой строки в случаи успеха.
  */
  LPWSTR _addSlashesExW(const LPWSTR source, SIZE_T size);

  /*
    Добавление экранизация симоволов \, ', ", \0 - слэшем. Аналог addslashes из PHP.

    IN source - исходная строка.
    IN dest   - конечная строка.
  */
  void _addSlashesA(LPSTR source, LPSTR dest);

  /*
    Добавление экранизация симоволов с выделением памяти.

    IN source - исходная строка.
    IN size   - размер исходной строки в символах. Может быть -1.

    Return    - NULL - в случаи ошибки, адрес новой строки в случаи успеха.
  */
  LPSTR _addSlashesExA(const LPSTR source, SIZE_T size);

  /*
    Получение размера текщей строки в списке строк разделнных через \r\n или \n.

    IN string - начало строки. В последующих вызовах должен быть next.
    IN memEnd - конец памяти. Обычно string + size.
    OUT next  - указатель на следующию строку, или конец памяти.

    Return    - размер строки.
  */
  SIZE_T __inline _getCurrentStringSizeW(const LPWSTR string, const LPWSTR memEnd, LPWSTR *next);

  /*
    Получение размера текщей строки в списке строк разделнных через \r\n или \n.

    IN string - начало строки. В последующих вызовах должен быть next.
    IN memEnd - конец памяти. Обычно string + size.
    OUT next  - указатель на следующию строку, или конец памяти.

    Return    - размер строки.
  */
  SIZE_T __inline _getCurrentStringSizeA(const LPSTR string, const LPSTR memEnd, LPSTR *next);

  /*
    Сравнение строки с маской.

    IN md - данные сравненния.

    Return - true  - совпадение найдено,
             false - совпадение не найдено.
  */
  bool _matchA(MATCHDATAA *md);

  /*
    Облегченная настрока над _matchA(), для сравнения с символами '*', '?', а тауже без учета
    регистра.

    IN mask   - маска.
    IN string - строка.

    Return - true  - совпадение найдено,
             false - совпадение не найдено.
  */
  bool _matchExA(const LPSTR mask, const LPSTR string);
};
