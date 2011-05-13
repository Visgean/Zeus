#include <windows.h>
#include <wininet.h>
#include <shlwapi.h>

#include "httptools.h"

void HttpTools::init(void)
{

}

void HttpTools::uninit(void)
{

}

LPSTR HttpTools::_findValueInUrlEncoded(const void *list, DWORD listSize, const LPSTR varable, LPDWORD valSize)
{
  LPSTR p = (LPSTR)list;
  LPSTR curVar;
  DWORD varableSize = Str::_LengthA(varable);

  while((curVar = Str::_findSubStringA(p, varable)) != NULL)
  {
    //Переменная найдена.
    if(curVar[varableSize] == '=' && (curVar == (LPSTR)list || *(curVar - 1) == '&')) 
    {
      LPSTR val    = curVar + varableSize + 1;
      LPSTR valEnd = Str::_findCharA(val, '&');

      *valSize = valEnd == NULL ? Str::_LengthA(val) : (DWORD)(valEnd - val);
      return val;
    }
    p = curVar + varableSize;
  }
  return NULL;
}

bool HttpTools::_parseUrl(const LPSTR url, URLDATA *ud)
{
  URL_COMPONENTSA uc;
  Mem::_zero(&uc, sizeof(URL_COMPONENTSA));

  uc.dwStructSize      = sizeof(URL_COMPONENTSA);
  uc.dwHostNameLength  = 1;
  uc.dwUrlPathLength   = 1;
  uc.dwExtraInfoLength = 1;

  if(CWA(wininet, InternetCrackUrlA)(url, 0, 0, &uc) == FALSE)
  {
    return false;
  }
  
  Mem::_zero(ud, sizeof(URLDATA));

  //Хост.
  if((ud->host = Str::_CopyExA(uc.lpszHostName, uc.dwHostNameLength)) == NULL)
  {
    return false;
  }

  //URI.
  {
    //Пропускаем '/'. Типо совместимость, т.к. в MSDN не написано об обезателном наличии или
    //отсувии слеша.
    while(uc.dwUrlPathLength > 0 && *uc.lpszUrlPath == '/')
    {
      uc.dwUrlPathLength--;
      uc.lpszUrlPath++;
    }
    
    DWORD fullSize = 1 + uc.dwUrlPathLength + uc.dwExtraInfoLength;
    if((ud->uri = (LPSTR)Mem::alloc((fullSize + 1) * sizeof(char))) == NULL)
    {
      Mem::free(ud->host);
      return false;
    }

    Mem::_copy(ud->uri + 1, uc.lpszUrlPath, uc.dwUrlPathLength);
    Mem::_copy(ud->uri + 1 + uc.dwUrlPathLength, uc.lpszExtraInfo, uc.dwExtraInfoLength);

    ud->uri[0]         = '/';
    ud->uri[fullSize]  = 0;
    ud->uriExtraOffset = 1 + uc.dwUrlPathLength;
  }

  //Порт.
  ud->port = uc.nPort;

  //Тип
  switch(uc.nScheme)
  {
    case INTERNET_SCHEME_HTTP:  ud->scheme = UDS_HTTP; break;
    case INTERNET_SCHEME_HTTPS: ud->scheme = UDS_HTTPS; break;
    default:                    ud->scheme = UDS_UNKNOWN;
  }

  return true;
}

void HttpTools::_freeUrlData(URLDATA *ud)
{
  Mem::free(ud->host);
  Mem::free(ud->uri);
}

LPWSTR HttpTools::_getFileNameFromUrl(const LPSTR url)
{
  LPWSTR retVal = NULL;
  URLDATA ud;
  
  if(_parseUrl(url, &ud))
  {
    //Убираем доп данные.
    ud.uri[ud.uriExtraOffset] = 0;
    
    LPSTR fileName = Str::_findRCharA(ud.uri, '/');
    if(fileName != NULL && fileName[1] != 0)
    {
      if((fileName = Str::_CopyExA(fileName, -1)) != NULL)
      {
        if(CWA(shlwapi, UrlUnescapeA)(fileName, NULL, NULL, URL_UNESCAPE_INPLACE) == S_OK)
        {
          retVal = Str::_utf8ToUnicode(fileName, -1);
        }
        Mem::free(fileName);
      }
    }
    _freeUrlData(&ud);
  }
  return retVal;
}

bool HttpTools::_compareTagNameA(const LPSTR tag, DWORD tagSize, const LPSTR string)
{
  LPSTR pos = string;
  if(*pos == '<')pos++;
  if(*pos == '/')pos++;

  if(CWA(shlwapi, StrCmpNIA)(pos, tag, tagSize) == 0)
  {
    BYTE c = *(pos + tagSize);
    if(c == 0 || IS_SPACE_CHAR(c) || c == '>' || c == '/')return true;
  }
  return false;
}

#define TAG_SCRIPT           "script"
#define TAG_SCRIPT_SIZE      (sizeof(TAG_SCRIPT) - 1)
#define SYMB_SPACE           "nbsp;"
#define SYMB_SPACE_SIZE      (sizeof(SYMB_SPACE) - 1)

//FIXME: Улучшить алгоритм.
DWORD HttpTools::_removeTagsA(LPSTR string, DWORD size)
{
  static const LPSTR tags[]       = {"br", "hr", "tr", "td", "div", "h1", "h2", "h3", "h4", "h5", "h6", "li"};
  static const BYTE  tags_size[]  = {2,    2,    2 ,   2   , 3    , 2   , 2   , 2   , 2   , 2   , 2   , 2};
  static const BYTE  tags_chars[] = {'\n', '\n', '\n', ' ' , '\n' , '\n', '\n', '\n', '\n', '\n', '\n', '\n'};

  int level       = 0;
  bool isScript   = false;
  DWORD oldOffset = 0;
  DWORD newOffset = 0;

  while(oldOffset < size)
  {
    if(level && string[oldOffset] == '>')level--;
    else if(string[oldOffset] == '<')
    {
      if(level++ == 0)
      {
        DWORD curSize = size - oldOffset;
        LPSTR curPos  = string + oldOffset + 1;

        //Ищим  /script
        if(isScript)
        {
          //FIXME: Если мы встертим внутри скрипта тег "</script>", это будет ошибка.
          if(curSize > TAG_SCRIPT_SIZE + 1 && *curPos == '/' && _compareTagNameA(TAG_SCRIPT, TAG_SCRIPT_SIZE, curPos + 1))isScript = false;
        }
        else
        {
          if(curSize > TAG_SCRIPT_SIZE && _compareTagNameA(TAG_SCRIPT, TAG_SCRIPT_SIZE, curPos))isScript = true;
          else for(BYTE k = 0; k < sizeof(tags) / sizeof(LPSTR); k++)
          {
            BYTE cts = tags_size[k];
            if(curSize > cts && _compareTagNameA(tags[k], cts, curPos))
            {
              curPos += cts;
              string[newOffset++] = tags_chars[k];
              break;
            }
          }
        }
      }
    }
    else if(!level && !isScript && string[oldOffset] != '\r' && string[oldOffset] != '\n' && string[oldOffset] != '\t')
    {
      if(string[oldOffset] == '&' && (size - oldOffset) > SYMB_SPACE_SIZE &&  CWA(shlwapi, StrCmpNIA)(string + oldOffset + 1, SYMB_SPACE, SYMB_SPACE_SIZE) == 0)
      {
        string[newOffset] = ' ';
        oldOffset += SYMB_SPACE_SIZE;
      }
      else string[newOffset] = string[oldOffset];
      
      //FIXME: Игнорировать пробелы если уже есть пробел.
      newOffset++;
    }
    oldOffset++;
  }

  return size - (oldOffset - newOffset);
}

DWORD HttpTools::_urlEncodeA(LPSTR input, DWORD inputSize, LPSTR output)
{
  DWORD x = 0, y = 0;
  BYTE c;
  for(; x < inputSize; x++, y++)
  {
    c = input[x];
    if(c == ' ')output[y] = '+';
    else if((c < '0' && c != '-' && c != '.') || (c < 'A' && c > '9') || (c > 'Z' && c < 'a' && c != '_') || (c > 'z'))
    {
      output[y++] = '%';
      Str::_byteToCharA(c, output + y);
      y++;
    }
    else output[y] = c;
  }
  output[y] = 0;
  return y;
}

DWORD HttpTools::_urlEncodeW(LPWSTR input, DWORD inputSize, LPWSTR output)
{
  DWORD x = 0, y = 0;
  BYTE c;
  for(; x < inputSize; x++, y++)
  {
    c = input[x];
    if(c == ' ')output[y] = '+';
    else if((c < '0' && c != '-' && c != '.') || (c < 'A' && c > '9') || (c > 'Z' && c < 'a' && c != '_') || (c > 'z'))
    {
      output[y++] = '%';
      Str::_byteToCharW(c, output + y);
      y++;
    }
    else output[y] = c;
  }
  output[y] = 0;
  return y;
}

LPSTR HttpTools::_urlEncodeExA(LPSTR input, DWORD inputSize)
{
  LPSTR newString = (LPSTR)Mem::alloc((inputSize * 3 + 1) * sizeof(char));
  if(newString != NULL)
  {
    _urlEncodeA(input, inputSize, newString);
    return newString;
  }
  return NULL;
}

LPWSTR HttpTools::_urlEncodeExW(LPWSTR input, DWORD inputSize)
{
  LPWSTR newString = (LPWSTR)Mem::alloc((inputSize * 3 + 1) * sizeof(WCHAR));
  if(newString != NULL)
  {
    _urlEncodeW(input, inputSize, newString);
    return newString;
  }
  return NULL;
}

LPSTR HttpTools::_getMimeHeader(const void *mimeData, DWORD mimeSize, const LPSTR header, SIZE_T *size)
{
  SIZE_T headerSize = ((DWORD_PTR)header > GMH_COUNT ? Str::_LengthA(header) : 0);
  LPSTR data        = (LPSTR)mimeData;
  LPSTR dataEnd     = data + mimeSize;

  *size = 0;

  while(data < dataEnd)
  {
    LPSTR cur      = data;
    SIZE_T curSize = Str::_getCurrentStringSizeA(data, dataEnd, &data);

    //Если пустая строка.
    if(curSize == 0)
    {
      if((DWORD_PTR)header == GMH_DATA)
      {
        *size = (SIZE_T)(dataEnd - data);
        return data;
      }
      return NULL; //После пустой строки уже нет заголовков.
    }

    //Эти данные могут быть получены только с первой строки.
    if(cur == (LPSTR)mimeData && ((DWORD_PTR)header == GMH_HTTP_METHOD || (DWORD_PTR)header == GMH_HTTP_URI || (DWORD_PTR)header == GMH_REQUEST_HTTP_VERSION))
    {
      //Ищим все пробелы.
      LPSTR spaces[2];
      DWORD count = 0;

      dataEnd = cur + curSize;
      for(LPSTR p = cur; p < dataEnd; p++)if(IS_SPACE_CHAR(*p))
      {
        if(count < 2)spaces[count] = p;
        count++;
      }

      if(count == 2)
      {
        if((DWORD_PTR)header == GMH_HTTP_METHOD)dataEnd = spaces[0];
        else if((DWORD_PTR)header == GMH_HTTP_URI){cur = spaces[0] + 1; dataEnd = spaces[1];}
        else if((DWORD_PTR)header == GMH_REQUEST_HTTP_VERSION)cur = spaces[1] + 1;

        *size = (SIZE_T)(dataEnd - cur);
        return cur;
      }
      return NULL;
    }
    else if(curSize > headerSize && CWA(shlwapi, StrCmpNIA)(header, cur, headerSize) == 0 && cur[headerSize] == ':')
    {
      dataEnd = cur + curSize;
      cur     = cur + headerSize + 1;

      while(IS_SPACE_CHAR(*cur))cur++;
      *size = (SIZE_T)(dataEnd - cur);
      return cur;
    }
  }
  return NULL;
}

DWORD HttpTools::_removeMimeHeader(void *mimeData, DWORD mimeSize, const LPSTR header)
{
  SIZE_T headerSize = Str::_LengthA(header);
  LPSTR data        = (LPSTR)mimeData;
  LPSTR dataEnd     = data + mimeSize;

  while(data < dataEnd)
  {
    LPSTR cur      = data;
    SIZE_T curSize = Str::_getCurrentStringSizeA(data, dataEnd, &data);

    //После пустой строки уже нет заголовков.
    if(curSize == 0)break;

    if(curSize > headerSize && CWA(shlwapi, StrCmpNIA)(header, cur, headerSize) == 0 && cur[headerSize] == ':')
    {
      curSize = (SIZE_T)(data - cur);
      Mem::_copy(cur, cur + curSize, (DWORD)(dataEnd - data));

      mimeSize -= curSize;
      dataEnd  -= curSize;
      data      = cur;
    }
  }
  return mimeSize;
}

DWORD HttpTools::_modifyMimeHeader(void *mimeData, DWORD mimeSize, const LPSTR header, const LPSTR context)
{
  SIZE_T contextSize = Str::_LengthA(context);
  SIZE_T headerSize  = Str::_LengthA(header);
  LPSTR data         = (LPSTR)mimeData;
  LPSTR dataEnd      = data + mimeSize;

  while(data < dataEnd)
  {
    LPSTR cur      = data;
    SIZE_T curSize = Str::_getCurrentStringSizeA(data, dataEnd, &data);

    //После пустой строки уже нет заголовков.
    if(curSize == 0)break;

    if(curSize > headerSize && CWA(shlwapi, StrCmpNIA)(header, cur, headerSize) == 0 && cur[headerSize] == ':')
    {
      SIZE_T postfixSize = (SIZE_T)(dataEnd - data);
      LPSTR tmpBuf; //FIXME: Это лень мозга, сделать алгоритм.
       
      if(postfixSize > 0 && (tmpBuf = (LPSTR)Mem::copyEx(data, postfixSize)) == NULL)continue;

      curSize = headerSize + 1; //Символ ':'.
      if(contextSize > 0)
      {
        cur[curSize++] = ' ';
        Mem::_copy(cur + curSize, context, contextSize);
        curSize += contextSize;
      }
      
      //Это закон RFC, варинат с только \n не рассматриваем.
      cur[curSize++] = '\r';
      cur[curSize++] = '\n';
      
      if(postfixSize > 0)
      {
        Mem::_copy(cur + curSize, tmpBuf, postfixSize);
        Mem::free(tmpBuf);
      }

      mimeSize = (SIZE_T)(cur - (LPSTR)mimeData) + curSize + postfixSize;
      dataEnd  = (LPSTR)mimeData + mimeSize;
      data      = cur + curSize;
    }
  }
  return mimeSize;
}

/*
  Chunk format (simplified):
 
  <HEX SIZE>[ chunk extension ] CRLF
  <DATA> CRLF

  The chunked encoding modifies the body of a message in order to
  transfer it as a series of chunks, each with its own size indicator,
  followed by an OPTIONAL trailer containing entity-header fields. This
  allows dynamically produced content to be transferred along with the
  information necessary for the recipient to verify that it has
  received the full message.

      Chunked-Body   = *chunk
                       last-chunk
                       trailer
                       CRLF

      chunk          = chunk-size [ chunk-extension ] CRLF
                       chunk-data CRLF
      chunk-size     = 1*HEX
      last-chunk     = 1*("0") [ chunk-extension ] CRLF

      chunk-extension= *( ";" chunk-ext-name [ "=" chunk-ext-val ] )
      chunk-ext-name = token
      chunk-ext-val  = token | quoted-string
      chunk-data     = chunk-size(OCTET)
      trailer        = *(entity-header CRLF)

  The chunk-size field is a string of hex digits indicating the size of
  the chunk. The chunked encoding is ended by any chunk whose size is
  zero, followed by the trailer, which is terminated by an empty line.
*/
void *HttpTools::_readChunkedData(const void *data, DWORD dataSize, void **chunkedData, LPDWORD chunkedDataSize)
{
  LPSTR p   = (LPSTR)data;
  LPSTR end = p + dataSize;

  //Читаем.
  if(Str::_getCurrentStringSizeA(p, end, (LPSTR *)chunkedData) > 0) //FIXME: проверять наличие \r\n
  {    
    *chunkedDataSize = Str::_hexStringToDwordA(p); //Читает, пока не втретиться символ отличный от 0-F.
    
    p = ((LPSTR)*chunkedData) + *chunkedDataSize;
    
    if(p < end)
    {
      if(p[0] == '\n')return (p + 1);
      if(p + 1 < end && p[0] == '\r' && p[1] == '\n')return (p + 2);
    }    

    *chunkedData = NULL;
    return p;
  }
  return NULL;
}

LPSTR HttpTools::_catExtraInfoFromUrlToUrlA(const LPSTR source, const LPSTR dest)
{
  int destSize   = Str::_LengthA(dest);
  int sourceSize = Str::_LengthA(source);

  if(destSize > 0 && sourceSize > 0)
  {
    LPSTR sourceInfo = Str::_findCharA(source, '?');
    if(sourceInfo != NULL)
    {
      int sourceInfoSize = Str::_LengthA(++sourceInfo);

      LPSTR p = (LPSTR)Mem::alloc((destSize + sourceInfoSize + 2/*?(&) + \0*/) * sizeof(char));
      if(p != NULL)
      {
        Mem::_copy(p, dest, destSize);
        p[destSize++] = (Str::_findCharA(dest, '?') == NULL ? '?' : '&');
        Str::_CopyA(p + destSize, sourceInfo, sourceInfoSize);
        return p;
      }
    }
  }
  return Str::_CopyExA(dest, destSize);
}

bool HttpTools::_parseAuthorization(const LPSTR source, DWORD sourceSize, LPWSTR *userName, LPWSTR *password)
{
  bool retVal = false;

  //Basic
  if(sourceSize > 6 && Str::_CompareA("Basic ", source, 6, 6) == 0)
  {
    LPSTR p = (LPSTR)source + 6;
    while(IS_SPACE_CHAR(*p))p++; //На всякий случай. Строка 100% коначается на ноль.

    LPSTR decoded;
    SIZE_T decodedSize;
    if((decoded = (LPSTR)Crypt::_base64Decode(p, (SIZE_T)(source + sourceSize - p), &decodedSize)) != NULL)
    {
      if((p = Str::_findCharA(decoded, ':')) != NULL)
      {
        *userName = Str::_utf8ToUnicode(decoded, p - decoded);
        *password = Str::_utf8ToUnicode(p + 1, -1);
        retVal = true;
      }
      Mem::free(decoded);
    }
  }

  return retVal; 
}
