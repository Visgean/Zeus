/*
  Работа с XML данными, обертка MsXml, требуется ComLibrary.

  Примечание: Не производиться проверка аргументов функций на нулевые указатели.
*/
#pragma once

namespace XmlParser
{
  //Данные об ошибки.
  typedef struct
  {
    int line;       //Номер строки.
    int errorCode;  //Код ошибки.
    LPWSTR reason;  //Текст ошибки.
  }XMLERRORDATA;

  /*
    Инициализация.
  */
  void init(void);

  /*
    Деинициализация.
  */
  void uninit(void);

  /*
    Открытие XML файла.

    IN file   - имя файла для открытия.
    OUT error - информация об ошибки, при ее возниконовение. Может быть NULL.
                В случаи возникновения ошибки, эта структура должна быть освобождена через
                freeXmlErrorData.

    Return    - в случаи успеха данные XML файла, которые должны быть закрыты через CloseFile.
                в случаи ошибки NULL.
  */
  IXMLDOMDocument *_openFile(const LPWSTR file, XMLERRORDATA *error);

  /*
    Закрытие XML файла.

    IN doc - указатель на данные файла для закрытия.
  */
  void _closeFile(IXMLDOMDocument *doc);

  /*
    Освобождение структуры XMLERRORDATA.

    IN pError - структура для освобождения.
  */
  void _freeXmlErrorData(XMLERRORDATA *pError);

  /*
    Получение содержимого элемента.

    IN root - элемент отснсителльно которгого происходит поиск node.
    IN node - элемент для поиск в XPath. Не может быть NULL.

    Return  - текст в случаи успеха (нужно освободить через _freeBstr()),
              или NULL в случаи ошибки.
  */
  BSTR _getNodeTextOfElement(IXMLDOMElement *root, const BSTR node);
  
  /*
    Получение содержимого элемента.

    IN root - элемент отснсителльно которгого происходит поиск node.
    IN node - элемент для поиск в XPath. Не может быть NULL.

    Return  - текст в случаи успеха (нужно освободить через _freeBstr()),
              или NULL в случаи ошибки.
  */
  BSTR _getNodeTextOfNode(IXMLDOMNode *root, const BSTR node);

  /*
    Освобождение BSTR через SysFreeString().

    IN OUT string - строка.
  */
  void _freeBstr(BSTR string);
};
