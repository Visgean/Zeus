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
  //Описание переменной.
  typedef struct _VAR
  {
    LPSTR *pValues;         //Массив значений
    BYTE bValuesCount;      //Количетсво значений.
    _VAR *pChilds;          //Массив потомков.
    DWORD dwChildsCount;    //Количетсво потомков.
  }VAR;

  //Описание файла.
  typedef struct
  {
    VAR *pVars;        //Список корневых переменных.
    DWORD dwVarsCount; //Количетсво переменных.
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

  /*
    Получение переменной из списка.

    IN pvParent     - родительская переменная, может быть NULL если поиск происходит в корне.
    IN pCD          - CFGDATA, может быть NULL если pvParent != NULL.
    IN pstrName     - имя переменной, может быть NULL если ишется вхождение.
    IN pstrValue    - дополнительное условие если pstrName != NULL, и имя вхождения если
                      pstrName == NULL.
    
    Return          - указатель на переменную, или NULL в случаи ошибки.
  */
  VAR *_GetVar(VAR *pvParent, CFGDATA *pCD, LPSTR pstrName, LPSTR pstrValue);
};
