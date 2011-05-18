#pragma once
/*
  Старый формат файла конигурации, был внесен в xlib по причини более удобной моральной адоптации.

  Примечание: По уничтожению zeus_old, уничтожить этот формат.
*/

#define CONFIG0_MAX_ARGS              200
#define CONFIG0_NAME_SECTION_BEGIN    "entry"
#define CONFIG0_NAME_SECTION_END      "end"
#define CONFIG0_NAME_SECTION_END_SIZE 3

namespace Config0
{
  //Variable description.
  typedef struct _VAR
  {
    LPSTR *pValues;         //An array of values
    BYTE bValuesCount;      //Kolichetsvo values.
    _VAR *pChilds;          //An array of descendants.
    DWORD dwChildsCount;    //Kolichetsvo descendants.
  }VAR;

  //Description of the file.
  typedef struct
  {
    VAR *pVars;        //List of root variables.
    DWORD dwVarsCount; //Kolichetsvo variables.
  }CFGDATA;

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

    IN pszFileName - файл для парсинга.
    OUT pCD        - результат, должен быть освобожден через _Close

    Return         - true - в случаи успеха,
                     false - в случаи ошибки.
  */
  bool _ParseFile(LPWSTR pszFileName, CFGDATA *pCD);

  /*
    Освобождение CFGDATA.

    IN pCD - CFGDATA для освобождения.
  */
  void _Close(CFGDATA *pCD);

  /*
    Освобождение переменной.

    IN pVar - переменная для освобождения.
  */
  void _FreeVar(VAR *pVar);

  /*
    Добавление переменной в список.

    IN pvParent     - родительская переменная, может быть NULL если добавление происходит в корень.
    IN pCD          - CFGDATA, может быть NULL если pvParent != NULL.
    IN pValues      - список аргументов для добавления. Массив не копируеся, просто сохраняется
                      этот указатель.
    IN bValuesCount - количество аргументов.

    Return          - указатель на новую переменную, или NULL в случаи ошибки.
  */
  VAR *_AddVar(VAR *pvParent, CFGDATA *pCD, LPSTR *pValues, BYTE bValuesCount);

  /*В В В В Getting a variable from the list.

В В В В IN pvParent - parent variable may be NULL if the search is at the root.
В В В В IN pCD - CFGDATA, may be NULL if pvParent! = NULL.
В В В В IN pstrName - variable name may be NULL if ishetsya occurrence.
В В В В IN pstrValue - an additional condition if pstrName! = NULL, and the name of the entry if
В В В В В В В В В В В В В В В В В В В В В В pstrName == NULL.
В В В В 
В В В В Return - a pointer to a variable, or NULL if an error occurs.
В В */
  VAR *_GetVar(VAR *pvParent, CFGDATA *pCD, LPSTR pstrName, LPSTR pstrValue);
};
