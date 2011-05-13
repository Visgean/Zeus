#include <windows.h>
#include <msxml2.h>

#include "xmlparser.h"
#include "str.h"
#include "mem.h"
#include "comlibrary.h"

static IXMLDOMDocument *createAndConfigureInterface(void)
{
  IXMLDOMDocument *xmlDoc = (IXMLDOMDocument *)ComLibrary::_createInterface(CLSID_DOMDocument30, IID_IXMLDOMDocument);
  if(xmlDoc != NULL)
  {
    xmlDoc->put_async(VARIANT_FALSE);
    xmlDoc->put_preserveWhiteSpace(VARIANT_FALSE);
    xmlDoc->put_resolveExternals(VARIANT_FALSE);
    xmlDoc->put_validateOnParse(VARIANT_TRUE);
  }
  return xmlDoc;
}

void XmlParser::init(void)
{

}

void XmlParser::uninit(void)
{

}

IXMLDOMDocument *XmlParser::_openFile(const LPWSTR file, XMLERRORDATA *error)
{
  if(error)
  {
    error->errorCode = -1;
    error->line      = -1;
    error->reason    = NULL;
  }

  bool ok = false;
  IXMLDOMDocument *x = createAndConfigureInterface();
  if(x)
  {
    VARIANT_BOOL bl;
    VARIANT str;

    //Выделяем строку.
    CWA(oleaut32, VariantInit)(&str);
    str.vt      = VT_BSTR;
    str.bstrVal = CWA(oleaut32, SysAllocString)(file);

    if(str.bstrVal != NULL)
    {
      //Загружаем файл.
      if(x->load(str, &bl) == S_OK && bl == VARIANT_TRUE)ok = true;
      //Получаем ошибку.
      else if(error)
      {
        IXMLDOMParseError *ierror;
        if(x->get_parseError(&ierror) == S_OK)
        {
          long l;
          BSTR reason = NULL;

          //Заполняем структуру.
          if(ierror->get_errorCode(&l) == S_OK)error->errorCode = l;
          if(ierror->get_line(&l) == S_OK)error->line = l;
          if(ierror->get_reason(&reason) == S_OK)error->reason = reason;

          ierror->Release();
        }
      }
      CWA(oleaut32, VariantClear)(&str);
    }

    //Объект не создан, освобождаем интерфейс.
    if(ok == false)
    {
      x->Release();
      x = NULL;
    }
  }
  return x;
}

void XmlParser::_closeFile(IXMLDOMDocument *doc)
{
  doc->Release();
}

void XmlParser::_freeXmlErrorData(XMLERRORDATA *error)
{
  _freeBstr(error->reason);
}

/*
  Надстройка для getNodeTextOf*().
*/
static BSTR getChildTextAndRelease(IXMLDOMNode *child)
{
  BSTR text;
  if(child->get_text(&text) != S_OK)text = NULL;
  child->Release();
  return text;
}

BSTR XmlParser::_getNodeTextOfElement(IXMLDOMElement *root, const BSTR node)
{
  IXMLDOMNode *child;
  if(root->selectSingleNode(node, &child) == S_OK)return getChildTextAndRelease(child);
  return NULL;
}

BSTR XmlParser::_getNodeTextOfNode(IXMLDOMNode *root, const BSTR node)
{
  IXMLDOMNode *child;
  if(root->selectSingleNode(node, &child) == S_OK)return getChildTextAndRelease(child);
  return NULL;
}

void XmlParser::_freeBstr(BSTR string)
{
  if(string != NULL)CWA(oleaut32, SysFreeString)(string);
}
