#pragma once
/*
  Формат файла конфигурации #1, только для Malware, только в UTF8.

  //comment
  ;comment
  #comment
  arg1 arg2 argX
  {
    subarg1 subarg2 subargX
  }
  
  //marker - proizvolny marker to indicate the end of the data.
  //within these blocks are located arbitrary data,
  arg1 arg2
  [marker

  marker]
  
  ...
*/

namespace Config1
{
  //Variable description.
  typedef struct _VAR
  {
    DWORD dwSourceLine;  //Position variable in the source file.
    LPWSTR *ppstrValues; //Arguments variable.
    DWORD dwValuesCount; //Kolichetsvo argumenov.
    
    _VAR *pvChilds;      //An array of child elements.
    DWORD dwChildsCount; //Kolichetsvo children.
  }VAR;

  //File koniguratsii.
  typedef struct
  {
    VAR *pvVars;        //Array permennyh.
    DWORD dwVarsCount;  //Kolichetsvo variables.
  }CONFIGFILE;

  /*
    Инициализация.
  */
  void Init(void);

  /*
    Деинициализация.
  */
  void Uninit(void);

  /*
    Парсинг файла.

    IN pstrFile      - файл для обработки.
    OUT pdwErrorLine - строка на которой произошла ошибка, в случаи фозврата функцией false.
    OUT pcf          - данные обработоного файла.

    Return           - true - в случаи успеха,
                       false - в случаи ошибки.
  */
  bool _ParseFile(LPWSTR pstrFile, LPDWORD pdwErrorLine, CONFIGFILE *pcf);

  /*
    Добавление переменной.

    IN OUT pvParent  - родительская переменная или NULL.
    IN OUT pcf       - файл конфигурации.
    IN pstrValues    - значения переменной, массив должен быть выделен средствами Mem, после
                       успешного выполнения этой функции, массив становится частью этого файла,
                       и не должен изменться или удаляься из вне.
    IN dwValuesCount - количетсво элементов в pstrValues.

    Return           - в случаи успеха указатель на переменную, выделеню в конфигурации,
                       или NULL в случаи ошибки.
  */
  VAR *_AddVar(VAR *pvParent, CONFIGFILE *pcf, LPWSTR *pstrValues, DWORD dwValuesCount);

  /*
    Получение перменной из текущего вложения.
    
    IN pvParent      - родительская переменная или NULL.
    IN pcf           - файл конфигурации.
    IN pstrValues    - значения аргументов(слева на право), которые должна содержать переменная.
    IN dwValuesCount - количетсво элементов в pstrValues.
    IN bMathCase     - true - с учетом регистра, false - без учета регистра.

    Return           - в случаи успеха указатель на переменную, или NULL в случаи ошибки.
  */
  VAR *_GetVar(VAR *pvParent, CONFIGFILE *pcf, LPWSTR *pstrValues, DWORD dwValuesCount, bool bMathCase);
  
  /*
    Освобождение парменной и всех ее потомков.

    IN pVar - переменная для обработки.
  */
  void _FreeVar(VAR *pvVar);

  /*В В В В Closure of the configuration file and release memory.

В В В В IN pcf - file for processing.
В В */
  void _CloseFile(CONFIGFILE *pcf);
};
