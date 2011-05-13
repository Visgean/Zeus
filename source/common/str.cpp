#include <windows.h>
#include <shlwapi.h>

#include "str.h"
#include "math.h"

//FIXME: Переопределить размеры из int в DWORD/SIZE_T.


//Максимальный размер строки для _sprintfExX
#define MAX_SPRINTF_STRING_SIZE (10 * 1024 * 1024)

static int unicodeToX(DWORD codePage, const LPWSTR source, int sourceSize, LPSTR dest, int destSize)
{
  if(sourceSize == -1)sourceSize = Str::_LengthW(source);
  int size = CWA(kernel32, WideCharToMultiByte)(codePage, 0, source, sourceSize, dest, destSize, NULL, NULL);
  if(destSize > 0)
  {
    if(size >= destSize)size = 0; //Нет места на нулевой символ.
    dest[size] = 0;
  }
  return size;
}

static LPSTR unicodeToXEx(DWORD codePage, const LPWSTR source, int size)
{
  if(size == -1)size = Str::_LengthW(source);
  int destSize = unicodeToX(codePage, source, size, NULL, 0);
  if(destSize > 0)
  {
    destSize += sizeof(BYTE);
    LPSTR dest = (LPSTR)Mem::alloc(destSize * sizeof(BYTE));
    if(dest != NULL)
    {
      unicodeToX(codePage, source, size, dest, destSize);
      return dest;
    }
  }
  return NULL;
}

static int xToUnicode(DWORD codePage, const LPSTR source, int sourceSize, LPWSTR dest, int destSize)
{
  if(sourceSize == -1)sourceSize = Str::_LengthA(source);
  int size = CWA(kernel32, MultiByteToWideChar)(codePage, 0, source, sourceSize, dest, destSize);
  if(destSize > 0)
  {
    if(size >= destSize)size = 0; //Нет места на нулевой символ.
    dest[size] = 0;
  }
  return size;
}

static LPWSTR xToUnicodeEx(DWORD codePage, LPSTR source, int size)
{
  if(size == -1)size = Str::_LengthA(source);
  int destSize = xToUnicode(codePage, source, size, NULL, 0);
  if(destSize > 0)
  {
    destSize += sizeof(BYTE);
    LPWSTR dest = (LPWSTR)Mem::alloc(destSize * sizeof(WCHAR));
    if(dest != NULL)
    {
      xToUnicode(codePage, source, size, dest, destSize);
      return dest;
    }
  }
  return NULL;
}

/*
  Общая функция для форматирования Unicode-строк с выделенем памяти.
  
  IN OUT buffer   - буфер для записи, должен быть либо валидным, либо указывать на NULL. В случаи
                    ошибки, буффер не будет возрашен к первоночальному состоянию.
  IN bufferOffset - текуйщий оффсет в символах в *buffer, с которого нужно начать вносить изменения.
  IN format       - формат строки.
  IN arguments    - аргументы для fromat.
  
  Return          - новый полный размер строки, или (DWORD)-1 в случаи ошибки.
*/
static DWORD vsprintfHelperExW(LPWSTR *buffer, DWORD bufferOffset, const LPWSTR format, va_list arguments)
{
  DWORD bufferSize = Str::_LengthW(format);
  int realSize;

  do
  {
    bufferSize += 512;
    if(bufferSize > MAX_SPRINTF_STRING_SIZE || !Mem::reallocEx(buffer, (bufferOffset + bufferSize) * sizeof(WCHAR) + sizeof(WCHAR)))return (DWORD)-1;
  }
  while((realSize = CWA(shlwapi, wvnsprintfW)((*buffer) + bufferOffset, bufferSize, format, arguments)) < 0 || realSize >= (int)(bufferSize - 1));

  bufferOffset += (DWORD)realSize;
  (*buffer)[bufferOffset] = 0;

  return bufferOffset;
}

/*
  Общая функция для форматирования ANSI-строк с выделенем памяти.
  
  IN OUT buffer   - буфер для записи, должен быть либо валидным, либо указывать на NULL. В случаи
                    ошибки, буффер не будет возрашен к первоночальному состоянию.
  IN bufferOffset - текуйщий оффсет в символах в *buffer, с которого нужно начать вносить изменения.
  IN format       - формат строки.
  IN arguments    - аргументы для fromat.
  
  Return          - новый полный размер строки, или (DWORD)-1 в случаи ошибки.
*/
static DWORD vsprintfHelperExA(LPSTR *buffer, DWORD bufferOffset, const LPSTR format, va_list arguments)
{
  DWORD bufferSize = Str::_LengthA(format);
  int realSize;

  do
  {
    bufferSize += 512;
    if(bufferSize > MAX_SPRINTF_STRING_SIZE || !Mem::reallocEx(buffer, (bufferOffset + bufferSize) * sizeof(char) + sizeof(char)))return (DWORD)-1;
  }
  while((realSize = CWA(shlwapi, wvnsprintfA)((*buffer) + bufferOffset, bufferSize, format, arguments)) < 0 || realSize >= (int)(bufferSize - 1));

  bufferOffset += (DWORD)realSize;
  (*buffer)[bufferOffset] = 0;
  return bufferOffset;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Str::Init(void)
{

}

void Str::Uninit(void)
{

}

bool Str::_IsValidA(LPSTR pstrString, DWORD dwSize)
{
  return (pstrString && dwSize > 0 && pstrString[dwSize - 1] == 0);
}

bool Str::_IsValidW(LPWSTR pstrString, DWORD dwSize)
{
  return (pstrString && dwSize > 0 && pstrString[dwSize - 1] == 0);
}

bool Str::_IsDecimalStringW(LPCWSTR pstrString, int iSize)
{
  if(iSize == -1)iSize = _LengthW(pstrString);
  for(int i = (*pstrString == '-' ? 1 : 0); i < iSize; i++)if(pstrString[i] < '0' || pstrString[i] > '9')return false;
  return true;
}

void Str::_byteToCharA(BYTE bt, LPSTR buf)
{
  buf[0] = (BYTE)(bt >> 4);
  buf[1] = (BYTE)(bt & 0xF);

  buf[0] += (buf[0] > 0x9 ? ('A' - 0xA) : '0');
  buf[1] += (buf[1] > 0x9 ? ('A' - 0xA) : '0');
}

void Str::_byteToCharW(BYTE bt, LPWSTR buf)
{
  buf[0] = (BYTE)(bt >> 4);
  buf[1] = (BYTE)(bt & 0xF);

  buf[0] += (buf[0] > 0x9 ? ('A' - 0xA) : '0');
  buf[1] += (buf[1] > 0x9 ? ('A' - 0xA) : '0');
}

void Str::_CopyW(LPWSTR pstrDest, LPCWSTR pstrSource, int iSize)
{
  if(iSize == -1)iSize = _LengthW(pstrSource);
  Mem::_copy(pstrDest, pstrSource, iSize * sizeof(WCHAR));
  pstrDest[iSize] = 0;
}

LPWSTR Str::_CopyExW(LPWSTR pstrSource, int iSize)
{
  if(pstrSource == NULL)return NULL;
  if(iSize == -1)iSize = _LengthW(pstrSource);
  iSize *= sizeof(WCHAR);
  LPWSTR p = (LPWSTR)Mem::alloc(iSize + sizeof(WCHAR));
  if(p != NULL)
  {
    Mem::_copy(p, pstrSource, iSize);
    //p[iSize] = 0;
  }
  return p;
}

void Str::_catW(LPWSTR buf, LPWSTR source, int size)
{
  _CopyW(buf + _LengthW(buf), source, size);
}

bool Str::_CatExW(LPWSTR *ppstrBuf, LPWSTR pstrSource, int iSize)
{
  if(pstrSource != NULL)
  {
    if(iSize == -1)iSize = _LengthW(pstrSource);
    int iBufSize = _LengthW(*ppstrBuf);

    if(Mem::reallocEx(ppstrBuf, (iBufSize + iSize + 1) * sizeof(WCHAR)))
    {
      Mem::_copy((*ppstrBuf) + iBufSize, pstrSource, iSize * sizeof(WCHAR));
      //(*ppstrBuf)[iSize] = 0;
      return true;
    }
  }
  return false;
}

void Str::_CopyA(LPSTR pstrDest, LPCSTR pstrSource, int iSize)
{
  if(iSize == -1)iSize = _LengthA(pstrSource);
  Mem::_copy(pstrDest, pstrSource, iSize);
  pstrDest[iSize] = 0;
}

LPSTR Str::_CopyExA(LPSTR pstrSource, int iSize)
{
  if(pstrSource == NULL)return NULL;
  if(iSize == -1)iSize = _LengthA(pstrSource);
  LPSTR p = (LPSTR)Mem::alloc(iSize + 1);
  if(p != NULL)
  {
    Mem::_copy(p, pstrSource, iSize);
    //p[iSize] = 0;
  }
  return p;
}

void Str::_catA(LPSTR buf, LPSTR source, int size)
{
  _CopyA(buf + _LengthA(buf), source, size);
}

bool Str::_CatExA(LPSTR *ppstrBuf, LPSTR pstrSource, int iSize)
{
  if(pstrSource != NULL)
  {
    if(iSize == -1)iSize = _LengthA(pstrSource);
    int iBufSize = _LengthA(*ppstrBuf);

    if(Mem::reallocEx(ppstrBuf, iBufSize + iSize + 1))
    {
      Mem::_copy((*ppstrBuf) + iBufSize, pstrSource, iSize);
      //(*ppstrBuf)[iSize] = 0;
      return true;
    }
  }
  return false;
}

void Str::_TrimA(LPSTR pstrStr)
{
  int size = _LengthA(pstrStr);
  if(size > 0)
  {
    int trimmed = 0;
    register int i;
    register char ch;
    LPSTR pc = pstrStr;

    for(i = 0; i < size; i++)
    {
      ch = pc[i];
      if(ch == '\r' || ch == '\n' || ch == ' ' || ch == '\t')trimmed++;
      else break;
    }
    size -= trimmed;
    pc += trimmed;

    for(i = size - 1; i >= 0; i--)
    {
      ch = pc[i];
      if(ch == '\r' || ch == '\n' || ch == ' ' || ch == '\t')size--;
      else break;
    }

    if(pc > pstrStr && size > 0)Mem::_copy(pstrStr, pc, size);
    pstrStr[size] = 0;
  }
}

void Str::_TrimW(LPWSTR pstrStr)
{
  int size = _LengthW(pstrStr);
  if(size > 0)
  {
    int trimmed = 0;
    register int i;
    register wchar_t ch;
    LPWSTR pc = pstrStr;

    for(i = 0; i < size; i++)
    {
      ch = pc[i];
      if(ch == '\r' || ch == '\n' || ch == ' ' || ch == '\t')trimmed++;
      else break;
    }
    size -= trimmed;
    pc += trimmed;

    for(i = size - 1; i >= 0; i--)
    {
      ch = pc[i];
      if(ch == '\r' || ch == '\n' || ch == ' ' || ch == '\t')size--;
      else break;
    }

    if(pc > pstrStr && size > 0)Mem::_copy(pstrStr, pc, size * sizeof(WCHAR));
    pstrStr[size] = 0;
  }
}

#if defined _WIN64
  #define TCharToIntOP1(IntType) (v <<= 4)
#else
  #define TCharToIntOP1(IntType) (v = (sizeof(IntType) == sizeof(int) ? v <<= 4 : (IntType)Math::_mul64(v, 8))) //Выбор будет произведен при компиляции.
#endif

#if defined _WIN64
  #define TCharToIntOP2(IntType) (v *= 10)
#else
  #define TCharToIntOP2(IntType) (v = (sizeof(IntType) == sizeof(int) ? v *= 10 : (IntType)Math::_mul64(v, 10))) //Выбор будет произведен при компиляции.
#endif

#define TCharToInt(CharType, IntType)\
{\
  register unsigned IntType v = 0;\
  bool sign = false;\
  register CharType c;\
\
  if(*pstrStr == '-'){pstrStr++; sign = true;}\
  else if(*pstrStr == '+')pstrStr++;\
\
  if(*pstrStr == '0' && (pstrStr[1] == 'x' || pstrStr[1] == 'X'))\
  {\
    pstrStr += 2;\
\
    for(;;)\
    {\
      c = *pstrStr;\
\
      if(c >= '0' && c <= '9')c -= '0';\
      else if(c >= 'a' && c <= 'f')c = c - 'a' + 0xA;\
      else if(c >= 'A' && c <= 'F')c = c - 'A' + 0xA;\
      else break;\
\
      TCharToIntOP1(IntType);\
      v += c;\
      pstrStr++;\
    }\
  }\
  else\
  {\
    for(;;)\
    {\
      c = *pstrStr;\
      if(c < '0' || c > '9')break;\
      TCharToIntOP2(IntType);\
      v += c - '0';\
      pstrStr++;\
    }\
  }\
  if(pbSign)*pbSign = sign;\
  return sign ? (v * -(1)) : (v);\
}

__int64 Str::_ToInt64A(LPSTR pstrStr, bool *pbSign)
{
  TCharToInt(CHAR, __int64);
}

int Str::_ToInt32A(LPSTR pstrStr, bool *pbSign)
{
  TCharToInt(CHAR, int);
}

__int64 Str::_ToInt64W(LPWSTR pstrStr, bool *pbSign)
{
  TCharToInt(WCHAR, __int64);
}

int Str::_ToInt32W(LPWSTR pstrStr, bool *pbSign)
{
  TCharToInt(WCHAR, int);
}

#if defined _WIN64
  #define IntToTCharOP1(IntType) (dwDigVal = (DWORD)(iNumber % bRadix))
#else
  #define IntToTCharOP1(IntType) (dwDigVal = (sizeof(IntType) == sizeof(int) ? (DWORD)(iNumber % bRadix) : (DWORD)Math::_modU64(iNumber, bRadix))) //Выбор будет произведен при компиляции.
#endif

#if defined _WIN64
  #define IntToTCharOP2(IntType) (iNumber /= bRadix)
#else
  #define IntToTCharOP2(IntType) (iNumber = (sizeof(IntType) == sizeof(int) ? iNumber /= bRadix : (IntType)Math::_divU64(iNumber, bRadix))) //Выбор будет произведен при компиляции.
#endif

#define IntToTChar(CharType, IntType)\
{\
  if(bSigned && iNumber < 0)\
  {\
    iNumber = -iNumber;\
    *pBuf++ = '-';\
  }\
\
  DWORD dwDigVal;\
  CharType *p = pBuf;\
\
  do\
  {\
    IntToTCharOP1(IntType);\
    IntToTCharOP2(IntType);\
    *p++ = (CharType)((dwDigVal > 9) ? dwDigVal - 10 + 'A' : dwDigVal + '0');\
  }\
  while(iNumber > 0);\
\
  DWORD dwSize = (DWORD)(p - pBuf);\
  *p-- = 0;\
  do\
  {\
    CharType temp = *p;\
    *p = *pBuf;\
    *pBuf = temp;\
    --p;\
    ++pBuf;\
  }\
  while(pBuf < p);\
\
  return dwSize + (bSigned ? 1 : 0);\
}

DWORD Str::_FromInt64A(__int64 iNumber, LPSTR pBuf, BYTE bRadix, bool bSigned)
{
  IntToTChar(CHAR, __int64);
}

DWORD Str::_FromInt32A(int iNumber, LPSTR pBuf, BYTE bRadix, bool bSigned)
{
  IntToTChar(CHAR, int);
}

DWORD Str::_FromInt64W(__int64 iNumber, LPWSTR pBuf, BYTE bRadix, bool bSigned)
{
  IntToTChar(WCHAR, __int64);
}

DWORD Str::_FromInt32W(int iNumber, LPWSTR pBuf, BYTE bRadix, bool bSigned)
{
  IntToTChar(WCHAR, int);
}

#define hexStringToDword(charType) \
{\
  DWORD v = 0;\
  if(str[0] == '0' && str[1] == 'x')str += 2;\
  for(;;)\
  {\
    charType c = *str;\
    if(c >= '0' && c <= '9')\
    {\
      v *= 16;\
      v += c - '0';\
    }\
    else if(c >= 'A' && c <= 'F')\
    {\
      v *= 16;\
      v += c - 'A' + 0xA;\
    }\
    else if(c >= 'a' && c <= 'f')\
    {\
      v *= 16;\
      v += c - 'a' + 0xA;\
    }\
    else break;\
    str++;\
  }\
  return v;\
}

DWORD Str::_hexStringToDwordA(LPSTR str)
{
  hexStringToDword(char);
}

DWORD Str::_hexStringToDwordW(LPWSTR str)
{
  hexStringToDword(WCHAR);
}

bool Str::_fromHexW(LPWSTR string, void *buffer)
{
  LPBYTE p = (LPBYTE)buffer;
  register WCHAR c;
  register bool odd_digit = false;

  for(;; string++)
  {
    c = *string;

    if(c >= '0' && c <= '9')c = c - '0';
    else if(c >= 'A' && c <= 'F')c = c - 'A' + 0xA;
    else if(c >= 'a' && c <= 'f')c = c - 'a' + 0xA;
    else if(c == 0 && odd_digit == false)break;
    else return false;

    if(odd_digit)
    {
      *p |= c;
      p++;
      odd_digit = false;
    }
    else
    {
      *p = c << 4;
      odd_digit = true;
    }
  }
  return true;
}

void Str::_toHexW(const void *binary, DWORD binarySize, LPWSTR string)
{
  for(DWORD i = 0; i < binarySize; i++, string += 2)_byteToCharW(((LPBYTE)binary)[i], string);
  *string = 0;
}
                                 
void Str::_toHexA(const void *binary, DWORD binarySize, LPSTR string)
{
  for(DWORD i = 0; i < binarySize; i++, string += 2)_byteToCharA(((LPBYTE)binary)[i], string);
  *string = 0;
}

#define fromCFormat(pointerType, charType, length, hexconvert) \
{\
  register int len = length(str) + 1;\
  for(register int i = 0, j = 0; i < len; i++, j++)\
  {\
    if(str[i] == '\\')\
    {\
      charType c = str[i + 1];\
      BYTE size = 1;\
      switch(c)\
      {\
        case '0':  c = '\0'; break;\
        case 'a':  c = '\a'; break;\
        case 'b':  c = '\b'; break;\
        case 'f':  c = '\f'; break;\
        case 'n':  c = '\n'; break;\
        case 'r':  c = '\r'; break;\
        case 't':  c = '\t'; break;\
        case 'v':  c = '\v'; break;\
        case '\'': c = '\''; break;\
        case '\"': c = '\"'; break;\
        case '\\': c = '\\'; break;\
        case 'x':\
        {\
          pointerType num = str + i + 2;\
          c = num[0];\
          charType c1 = num[1];\
          if(((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')) && ((c1 >= '0' && c1 <= '9') || (c1 >= 'A' && c1 <= 'F') || (c1 >= 'a' && c1 <= 'f')))\
          {\
            c1 = num[2];\
            num[2] = 0;\
            c = (charType)hexconvert(num);\
            num[2] = c1;\
            size = 3;\
            break;\
          }\
        }\
        default: goto NORMAL;\
      }\
\
      i += size;\
      str[j] = c;\
    }\
    else\
    {\
NORMAL:\
      str[j] = str[i];\
    }\
  }\
}

void Str::_fromCFormatA(LPSTR str)
{
  fromCFormat(LPSTR, char, _LengthA, _hexStringToDwordA);
}

void Str::_fromCFormatW(LPWSTR str)
{
  fromCFormat(LPWSTR, WCHAR, _LengthW, _hexStringToDwordW);
}

DWORD Str::_splitToStringsA(LPSTR input, DWORD inputSize, LPSTR **strings, DWORD flags, char separator)
{
  LPSTR end = input + inputSize;
  DWORD count = 0;
  *strings = NULL;

  while(input < end)
  {
    LPSTR cur = input;
    SIZE_T curSize;
    
    if(flags & STS_USE_SEPARATOR)
    {
      LPSTR curEnd = input;
      while(curEnd < end && *curEnd != separator)curEnd++;
      curSize = (SIZE_T)(curEnd - input);
      input   = curEnd + 1;
    }
    else
    {
      curSize = _getCurrentStringSizeA(input, end, &input);
    }
    
    //Добавление строки.
    if(!Mem::reallocEx(strings, (count + 1) * sizeof(LPSTR)) || (curSize > 0 && ((*strings)[count] = _CopyExA(cur, curSize)) == NULL))
    {
      Mem::freeArrayOfPointers(strings, count);
      return (DWORD)-1;
    }

    if(flags & STS_TRIM && (*strings)[count] != NULL)_TrimA((*strings)[count]);

    count++;
  }

  return count;
}

DWORD Str::_splitToStringsW(LPWSTR input, DWORD inputSize, LPWSTR **strings, DWORD flags, WCHAR separator)
{
  LPWSTR end = input + inputSize;
  DWORD count = 0;
  *strings = NULL;

  while(input < end)
  {
    LPWSTR cur      = input;
    SIZE_T curSize;

    if(flags & STS_USE_SEPARATOR)
    {
      LPWSTR curEnd = input;
      while(curEnd < end && *curEnd != separator)curEnd++;
      curSize = (SIZE_T)(curEnd - input);
      input   = curEnd + 1;
    }
    else
    {
      curSize = _getCurrentStringSizeW(input, end, &input);
    }

    //Добавление строки.
    if(!Mem::reallocEx(strings, (count + 1) * sizeof(LPWSTR)) || (curSize > 0 && ((*strings)[count] = _CopyExW(cur, curSize)) == NULL))
    {
      Mem::freeArrayOfPointers(strings, count);
      return (DWORD)-1;
    }

    if(flags & STS_TRIM && (*strings)[count] != NULL)_TrimW((*strings)[count]);

    count++;
  }

  return count;
}

#define getArguments(pointerType, charType, copyEx, emptyString, cConvert) \
{\
  pointerType curPos = input;\
  pointerType end    = curPos + inputSize;\
  DWORD count        = 0;\
  *args              = NULL;\
\
  for(;;)\
  {\
    while(curPos < end && IS_SPACE_CHAR(*curPos))curPos++;\
    if(curPos >= end)break;\
\
    pointerType curEnd = curPos + 1;\
    DWORD curSize;\
\
    if(*curPos == '\'' || *curPos == '\"')\
    {\
      charType quote = *curPos;\
      curPos++;\
\
      while(curEnd < end)\
      {\
        if(*curEnd == quote)\
        {\
          if(!(flags & STA_FORMAT_C))break;\
\
          pointerType p = curEnd;\
          DWORD cc      = 0;\
\
          while(*(--p) == '\\')cc++;\
          if(cc % 2 == 0)break;\
        }\
        curEnd++;\
      }\
\
      if(curEnd == end)curPos--;\
      curSize = curEnd - curPos;\
    }\
    else\
    {\
      while(curEnd < end && !IS_SPACE_CHAR(*curEnd))curEnd++;\
      curSize = curEnd - curPos;\
    }\
\
    if(!Mem::reallocEx(args, (count + 1) * sizeof(pointerType)) || ((*args)[count] = (curSize == 0 ? copyEx(emptyString, 1) : copyEx(curPos, curSize))) == NULL)\
    {\
      Mem::freeArrayOfPointers(*args, count);\
      return (DWORD)-1;\
    }\
\
    if(flags & STA_FORMAT_C && (*args)[count] != NULL)cConvert((*args)[count]);\
\
    count++;\
    curPos = curEnd + 1;\
  }\
\
  return count;\
}

DWORD Str::_getArgumentsA(const LPSTR input, DWORD inputSize, LPSTR **args, DWORD flags)
{
  getArguments(LPSTR, char, _CopyExA, "", _fromCFormatA);
}

DWORD Str::_getArgumentsW(const LPWSTR input, DWORD inputSize, LPWSTR **args, DWORD flags)
{
  getArguments(LPWSTR, WCHAR, _CopyExW, L"", _fromCFormatW);
}

LPWSTR Str::_joinArgumentsW(const LPWSTR *args, DWORD argsCount)
{
  DWORD outputSize = 0;
  LPWSTR output    = NULL;
  
  for(DWORD i = 0; i < argsCount; i++)
  {
    bool quote = false;;

    //Проверяем нужно ли заключать аргумент в кавычки.
    {
      LPWSTR p = args[i];
      while(*p != 0)
      {
        if(IS_SPACE_CHAR(*p))
        {
          quote = true;
          break;
        }
        p++;
      }
    }

    //Выделяем память.
    int len = _LengthW(args[i]);
    if(!Mem::reallocEx(&output, (outputSize + 1/*space*/ + 1/*"*/ + len + 1/*"*/ + 1/*\0*/) * sizeof(WCHAR)))
    {
      Mem::free(output);
      return NULL;
    }

    //Копируем.
    if(i > 0)output[outputSize++] = ' ';
    if(quote)output[outputSize++] = '\"';
    
    Str::_CopyW(output + outputSize, args[i], len);
    outputSize += len;
    
    if(quote)
    {
      output[outputSize++] = '\"';
      output[outputSize]   = 0;
    }
  }
  return output;
}

int Str::_unicodeToAnsi(const LPWSTR source, int sourceSize, LPSTR dest, int destSize)
{
  return unicodeToX(CP_ACP, source, sourceSize, dest, destSize);
}

LPSTR Str::_unicodeToAnsiEx(const LPWSTR source, int size)
{
  return unicodeToXEx(CP_ACP, source, size);
}

int Str::_ansiToUnicode(LPSTR source, int sourceSize, LPWSTR dest, int destSize)
{
  return ::xToUnicode(CP_ACP, source, sourceSize, dest, destSize);
}

LPWSTR Str::_ansiToUnicodeEx(LPSTR source, int size)
{
  return xToUnicodeEx(CP_ACP, source, size);
}

int Str::_oemToUnicode(LPSTR source, int sourceSize, LPWSTR dest, int destSize)
{
  return ::xToUnicode(CP_OEMCP, source, sourceSize, dest, destSize);
}

LPWSTR Str::_oemToUnicodeEx(LPSTR source, int size)
{
  return xToUnicodeEx(CP_OEMCP, source, size);
}

int Str::_LengthA(LPCSTR pstrStr)
{
  int c = 0;
  if(pstrStr != NULL)while(pstrStr[c] != 0)c++;
  return c;
}

int Str::_LengthW(LPCWSTR pstrStr)
{
  int c = 0;
  if(pstrStr != NULL)while(pstrStr[c] != 0)c++;
  return c;
}

int Str::_CompareA(LPCSTR pstrStr1, LPCSTR pstrStr2, int iSize1, int iSize2)
{
  //Проверка указателей
  if(pstrStr1 == NULL && pstrStr2 != NULL)return -1;
  if(pstrStr1 != NULL && pstrStr2 == NULL)return 1;
  if(pstrStr1 == NULL && pstrStr2 == NULL)return 0;

  //Если оба размера не определены.
  if(iSize1 == -1 && iSize2 == -1)
  {
    while((iSize1 = *pstrStr1 - *pstrStr2) == 0 && *pstrStr2 != 0)
    {
      pstrStr1++;
      pstrStr2++;
    }
  }
  else
  {
    if(iSize1 == -1)iSize1 = _LengthA(pstrStr1);
    if(iSize2 == -1)iSize2 = _LengthA(pstrStr2);

    //Если размеры не равны, или хотябы один из них равен 0.
    if(iSize1 != iSize2 || iSize1 == 0 || iSize2 == 0)iSize1 -= iSize2;
    //Если размеры равны.
    else for(int c = 0; c < iSize2; c++)
    {
      if((iSize1 = *pstrStr1 - *pstrStr2) != 0)break;
      pstrStr1++;
      pstrStr2++;
    }
  }

  return (iSize1 == 0 ? 0 : (iSize1 > 0 ? 1 : -1));
}

int Str::_CompareW(LPCWSTR pstrStr1, LPCWSTR pstrStr2, int iSize1, int iSize2)
{
  //Проверка указателей
  if(pstrStr1 == NULL && pstrStr2 != NULL)return -1;
  if(pstrStr1 != NULL && pstrStr2 == NULL)return 1;
  if(pstrStr1 == NULL && pstrStr2 == NULL)return 0;

  //Если оба размера не определены.
  if(iSize1 == -1 && iSize2 == -1)
  {
    while((iSize1 = *pstrStr1 - *pstrStr2) == 0 && *pstrStr2 != 0)
    {
      pstrStr1++;
      pstrStr2++;
    }
  }
  else
  {
    if(iSize1 == -1)iSize1 = _LengthW(pstrStr1);
    if(iSize2 == -1)iSize2 = _LengthW(pstrStr2);

    //Если размеры не равны, или хотябы один из них равен 0.
    if(iSize1 != iSize2 || iSize1 == 0 || iSize2 == 0)iSize1 -= iSize2;
    //Если размеры равны.
    else for(int c = 0; c < iSize2; c++)
    {
      if((iSize1 = *pstrStr1 - *pstrStr2) != 0)break;
      pstrStr1++;
      pstrStr2++;
    }
  }

  return (iSize1 == 0 ? 0 : (iSize1 > 0 ? 1 : -1));
}

LPSTR Str::_findCharA(const LPSTR string, char c)
{
  LPSTR p = string;
  
  while(*p != 0 && *p != c)p++;
  if(*p == c)return (LPSTR)p;
  return NULL;
}

LPSTR Str::_findRCharA(const LPSTR string, char c)
{
  LPSTR p = string;
  LPSTR start = p;
  
  while(*p++ != 0);
  while(--p != start && *p != c);
  if(*p == c)return (LPSTR)p;
  return NULL;
}

LPWSTR Str::_findCharW(const LPWSTR string, WCHAR c)
{
  LPWSTR p = string;
  
  while(*p != 0 && *p != c)p++;
  if(*p == c)return (LPWSTR)p;
  return NULL;
}

LPWSTR Str::_findRCharW(const LPWSTR string, WCHAR c)
{
  LPWSTR p = string;
  LPWSTR start = p;
  
  while(*p++ != 0);
  while(--p != start && *p != c);
  if(*p == c)return (LPWSTR)p;
  return NULL;
}

LPSTR Str::_findSubStringA(const LPSTR string, const LPSTR subString)
{
  if(*subString == 0)return (LPSTR)string;

  LPSTR cp = (LPSTR)string;
  LPSTR s1, s2;

  while(*cp)
  {
    s1 = cp;
    s2 = (LPSTR)subString;

    while(*s1 && *s2 && !(*s1-*s2))s1++, s2++;
    if(*s2 == 0)return cp;
    cp++;
  }

  return NULL;
}

LPWSTR Str::_findSubStringW(const LPWSTR string, const LPWSTR subString)
{
  if(*subString == 0)return (LPWSTR)string;

  LPWSTR cp = (LPWSTR)string;
  LPWSTR s1, s2;

  while(*cp)
  {
    s1 = cp;
    s2 = (LPWSTR)subString;

    while(*s1 && *s2 && !(*s1-*s2))s1++, s2++;
    if(*s2 == 0)return cp;
    cp++;
  }

  return NULL;
}

int Str::_sprintfW(LPWSTR pBuf, int iBufSize, LPCWSTR pstrFormat, ...)
{
  va_list list;
  va_start(list, pstrFormat);
  int iSize = _vsprintfW(pBuf, iBufSize, pstrFormat, list);
  va_end(list);
  return iSize;
}

int Str::_sprintfA(LPSTR pBuf, int iBufSize, LPCSTR pstrFormat, ...)
{
  va_list list;
  va_start(list, pstrFormat);
  int iSize = _vsprintfA(pBuf, iBufSize, pstrFormat, list);
  va_end(list);
  return iSize;
}

int Str::_vsprintfW(LPWSTR pBuf, int iBufSize, LPCWSTR pstrFormat, va_list arglist)
{
  if(iBufSize <= 0)return -1;

  Mem::_zero(pBuf, iBufSize * sizeof(WCHAR));
  int iSize = CWA(shlwapi, wvnsprintfW)(pBuf, iBufSize, pstrFormat, arglist);
  pBuf[iBufSize - 1] = 0;

  if(iSize == -1)
  {
    //Гении-индусы решили подмениь возрашаемое значение в Vista на -1, в случаи если не хватает
    //места на всю строку, однако буфер заполняется. Ума устаналивать LastError не хватило...
    iSize = _LengthW(pBuf);
  }
  else pBuf[iSize] = 0;

  return iSize;
}

int Str::_vsprintfA(LPSTR pBuf, int iBufSize, LPCSTR pstrFormat, va_list arglist)
{
  if(iBufSize <= 0)return -1;

  Mem::_zero(pBuf, iBufSize);
  int iSize = CWA(shlwapi, wvnsprintfA)(pBuf, iBufSize, pstrFormat, arglist);
  pBuf[iBufSize - 1] = 0;

  if(iSize == -1)
  {
    //Гении-индусы решили подмениь возрашаемое значение в Vista на -1, в случаи если не хватает
    //места на всю строку, однако буфер заполняется. Ума устаналивать LastError не хватило...
    iSize = _LengthA(pBuf);
  }
  else pBuf[iSize] = 0;

  return iSize;
}

int Str::_sprintfExW(LPWSTR *ppBuf, LPCWSTR pstrFormat, ...)
{
  va_list list;
  va_start(list, pstrFormat);
  int iSize = _vsprintfExW(ppBuf, pstrFormat, list);
  va_end(list);
  return iSize;
}

DWORD Str::_sprintfCatExW(LPWSTR *buffer, DWORD offset, const LPWSTR format, ...)
{
  va_list list;
  va_start(list, format);
  DWORD size = _vsprintfCatExW(buffer, offset, format, list);
  va_end(list);
  return size;
}

int Str::_sprintfExA(LPSTR *ppBuf, LPCSTR pstrFormat, ...)
{
  va_list list;
  va_start(list, pstrFormat);
  int iSize = _vsprintfExA(ppBuf, pstrFormat, list);
  va_end(list);
  return iSize;
}

DWORD Str::_sprintfCatExA(LPSTR *buffer, DWORD offset, const LPSTR format, ...)
{
  va_list list;
  va_start(list, format);
  DWORD size = _vsprintfCatExA(buffer, offset, format, list);
  va_end(list);
  return size;
}

int Str::_vsprintfExW(LPWSTR *ppBuf, LPCWSTR pstrFormat, va_list arglist)
{
  LPWSTR pBuf  = NULL;
  int realSize = (int)vsprintfHelperExW(&pBuf, 0, (LPWSTR)pstrFormat, arglist);

  if(realSize < 0)Mem::free(pBuf);
  else *ppBuf = pBuf;
  
  return realSize;
}

DWORD Str::_vsprintfCatExW(LPWSTR *buffer, DWORD offset, const LPWSTR format, va_list arguments)
{
  LPWSTR source = *buffer;
  DWORD realSize = vsprintfHelperExW(buffer, offset, format, arguments);
  
  if(realSize == (DWORD)-1)
  {
    if(offset == 0 && source == NULL)
    {
      Mem::free(*buffer);
      *buffer = NULL;
    }
    Mem::reallocEx(buffer, (offset + 1/*safe byte*/) * sizeof(WCHAR));
  }
  return realSize;
}

int Str::_vsprintfExA(LPSTR *ppBuf, LPCSTR pstrFormat, va_list arglist)
{
  LPSTR pBuf   = NULL;
  int realSize = (int)vsprintfHelperExA(&pBuf, 0, (LPSTR)pstrFormat, arglist);

  if(realSize < 0)Mem::free(pBuf);
  else *ppBuf = pBuf;
  
  return realSize;
}

DWORD Str::_vsprintfCatExA(LPSTR *buffer, DWORD offset, const LPSTR format, va_list arguments)
{
  LPSTR source = *buffer;
  DWORD realSize = vsprintfHelperExA(buffer, offset, format, arguments);
  
  if(realSize == (DWORD)-1)
  {
    if(offset == 0 && source == NULL)
    {
      Mem::free(*buffer);
      *buffer = NULL;
    }
    Mem::reallocEx(buffer, (offset + 1/*safe byte*/) * sizeof(char));
  }
  return realSize;
}

bool Str::_utf8IsBom(const void *data)
{
  DWORD bom = UTF8_BOM;
  return (Mem::_compare(data, &bom, UTF8_BOM_SIZE) == 0);
}

void Str::_utf8Free(UTF8STRING *string)
{
  Mem::free(string->data);
  Mem::_zero(string, sizeof(UTF8STRING));
}

bool Str::_utf8FromUnicode(const LPWSTR source, int sourceSize, UTF8STRING *dest)
{
  if(sourceSize == -1)sourceSize = _LengthW(source);
  if(sourceSize > 0)
  {
    dest->data = (void *)unicodeToXEx(CP_UTF8, source, sourceSize);
    if(dest->data != NULL)
    {
      dest->lenght = sourceSize;
      dest->size   = _LengthA((LPSTR)dest->data);
      return true;
    }
  }
  return false;
}

bool Str::_utf8FromAnsi(const LPSTR source, int sourceSize, UTF8STRING *dest)
{
  if(sourceSize == -1)sourceSize = _LengthA(source);

  bool r = false;
  if(sourceSize > 0)
  {
    LPWSTR unicodeString = xToUnicodeEx(CP_ACP, source, sourceSize);
    if(unicodeString != NULL)
    {
      r = Str::_utf8FromUnicode(unicodeString, -1, dest);
      Mem::free(unicodeString);
    }
  }
  return r;
}

LPWSTR Str::_utf8ToUnicode(const void *rawString, const int size)
{
  return xToUnicodeEx(CP_UTF8, (LPSTR)rawString, size);
}

bool Str::_isValidMultiStringA(const LPSTR string, DWORD size)
{
  return (string != NULL && size >= 2 && string[size - 1] == 0 && (string)[size - 2] == 0);
}

bool Str::_isValidMultiStringW(const LPWSTR string, DWORD size)
{
  return (string != NULL && size >= 2 && string[size - 1] == 0 && (string)[size - 2] == 0);
}

LPSTR Str::_multiStringGetIndexA(LPSTR string, DWORD index)
{
  if(index == 0)return string;
  for(DWORD i = 0; ; string++)if(*string == 0)
  {
    LPSTR c = string + 1;
    if(*c == 0)break; //Конец мульти-строки.
    if(++i == index)return c;
  }
  return NULL;
}

LPWSTR Str::_multiStringGetIndexW(LPWSTR string, DWORD index)
{
  if(index == 0)return string;
  for(DWORD i = 0; ; string++)if(*string == 0)
  {
    LPWSTR c = string + 1;
    if(*c == 0)break; //Конец мульти-строки.
    if(++i == index)return c;
  }
  return NULL;
}

DWORD Str::_multiStringGetCountA(const LPSTR string)
{
  LPSTR p = string;
  DWORD count = 0;
  for(DWORD i = 0; ; p++)if(p[0] == 0)
  {
    count++;
    if(p[1] == 0)break;
  }
  return count;
}

DWORD Str::_multiStringGetCountW(const LPWSTR string)
{
  LPWSTR p = string;
  DWORD count = 0;
  for(DWORD i = 0; ; p++)if(p[0] == 0)
  {
    count++;
    if(p[1] == 0)break;
  }
  return count;
}

LPSTR Str::_multiStringCreateA(const LPSTR string, LPDWORD length)
{
  *length = 0;
  LPSTR ret = NULL;
  int len   = _LengthA(string);

  if(len > 0 && (ret = (LPSTR)Mem::alloc((len + 2) * sizeof(char))) != NULL)
  {
    Mem::_copy(ret, string, len * sizeof(char));
    ret[len]     = 0;
    ret[len + 1] = 0;
    *length      = len + 2;
  }
  return ret;
}

LPWSTR Str::_multiStringCreateW(const LPWSTR string, LPDWORD length)
{
  *length = 0;
  LPWSTR ret = NULL;
  int len   = _LengthW(string);

  if(len > 0 && (ret = (LPWSTR)Mem::alloc((len + 2) * sizeof(WCHAR))) != NULL)
  {
    Mem::_copy(ret, string, len * sizeof(WCHAR));
    ret[len]     = 0;
    ret[len + 1] = 0;
    *length      = len + 2;
  }
  return ret;
}

LPWSTR Str::_GetErrorString(DWORD dwErrorCode)
{
  LPWSTR pMsgBuf;
  LPWSTR pRet = NULL;
  DWORD dwSize = CWA(kernel32, FormatMessageW)(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK,
                                               NULL,
                                               dwErrorCode,
                                               MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                               (LPWSTR)&pMsgBuf,
                                               0,
                                               NULL);
  if(dwSize > 0)
  {
    pRet = _CopyExW(pMsgBuf, dwSize);
    CWA(kernel32, LocalFree)(pMsgBuf);
  }

  return pRet;
}

void Str::_addSlashesW(LPWSTR source, LPWSTR dest)
{
  while(*source != 0)
  {
    if(*source == '\"' || *source == '\'' || *source == '\\' || *source == '\0')*dest++ = '\\';
    *dest++ = *source++;
  }
  *dest = 0;
}

LPWSTR Str::_addSlashesExW(const LPWSTR source, SIZE_T size)
{
  if(size == -1)size = _LengthW(source);
  LPWSTR dest = (LPWSTR)Mem::quickAlloc((size * 2 + 1) * sizeof(WCHAR));
  if(dest != NULL)_addSlashesW(source, dest);
  return dest;
}

void Str::_addSlashesA(LPSTR source, LPSTR dest)
{
  while(*source != 0)
  {
    if(*source == '\"' || *source == '\'' || *source == '\\' || *source == '\0')*dest++ = '\\';
    *dest++ = *source++;
  }
  *dest = 0;
}

LPSTR Str::_addSlashesExA(const LPSTR source, SIZE_T size)
{
  if(size == -1)size = _LengthA(source);
  LPSTR dest = (LPSTR)Mem::quickAlloc((size * 2 + 1) * sizeof(char));
  if(dest != NULL)_addSlashesA(source, dest);
  return dest;
}

SIZE_T Str::_getCurrentStringSizeW(const LPWSTR string, const LPWSTR memEnd, LPWSTR *next)
{
  LPWSTR curEnd = string; //Указатель на конец строки (символ \n или конец памяти).
  SIZE_T curSize;         //Размер текушей строки без символов \n и \r

  //Поиск конца текушей строки.
  while(curEnd < memEnd && *curEnd != '\n' && *curEnd != '\r')curEnd++;
  curSize = curEnd - string;

  //Отладка строки.
  if(curEnd + 1 < memEnd && curEnd[0] == '\r' && curEnd[1] == '\n')curEnd++;

  *next = curEnd + 1;
  return curSize;
}

SIZE_T Str::_getCurrentStringSizeA(const LPSTR string, const LPSTR memEnd, LPSTR *next)
{
  LPSTR curEnd = string; //Указатель на конец строки (символ \n или конец памяти).
  SIZE_T curSize;        //Размер текушей строки без символов \n и \r

  //Поиск конца текушей строки.
  while(curEnd < memEnd && *curEnd != '\n' && *curEnd != '\r')curEnd++;
  curSize = curEnd - string;

  //Отладка строки.
  if(curEnd + 1 < memEnd && curEnd[0] == '\r' && curEnd[1] == '\n')curEnd++;

  *next = curEnd + 1;
  return curSize;
}

#define engCharToLower(c) (c >= 'A' && c <= 'Z' ? c - 'A' + 'a' : c)
bool Str::_matchA(MATCHDATAA *md)
{
  if(md->flags & MATCH_SEARCH_SUBSSTRING)
  {
    MATCHDATAA subMd;
    Mem::_copy(&subMd, md, sizeof(MATCHDATAA));

    subMd.flags &= ~MATCH_SEARCH_SUBSSTRING;
    for(DWORD stringOffset = 0; stringOffset < md->stringSize; stringOffset++)
    {
      subMd.string     = md->string + stringOffset;
      subMd.stringSize = md->stringSize - stringOffset;

      if(_matchA(&subMd))
      {
        md->endOfMatch   = stringOffset + subMd.endOfMatch;
        md->beginOfMatch = stringOffset;
        return true;
      }
    }

    md->beginOfMatch = 0;
    md->endOfMatch   = 0;
    return false;
  }
  
  md->beginOfMatch = 0;
  for(DWORD stringOffset = 0, maskOffset = 0;; stringOffset++, maskOffset++)
  {
    //Достигнут конец маски.
    if(maskOffset == md->maskSize)
    {
      md->endOfMatch = stringOffset;
      return (md->flags & MATCH_FULL_EQUAL ? (stringOffset == md->stringSize) : true);
    }

    char maskChar = md->mask[maskOffset];

    //Просто пропускаем символ.
    if(maskChar == md->anyCharSymbol)
    {
      //Достигли конец строки. Провал...
      if(stringOffset == md->stringSize)
      {
        md->endOfMatch = stringOffset; 
        return false;
      }
    }
    else if(maskChar == md->anyCharsSymbol)
    {
      while(++maskOffset < md->maskSize && md->mask[maskOffset] == md->anyCharsSymbol);
      
      //Маска распрострониться до конца строки.
      if(maskOffset == md->maskSize)
      {
        md->endOfMatch = md->stringSize; 
        return true;
      }
      
      //Создаем копию структуры со смешением.
      MATCHDATAA subMd;
      bool r;

      Mem::_copy(&subMd, md, sizeof(MATCHDATAA));
      subMd.mask     += maskOffset;
      subMd.maskSize -= maskOffset;
      md->endOfMatch  = stringOffset;

      for(; stringOffset < md->stringSize; stringOffset++)
      {
        subMd.string     = md->string     + stringOffset;
        subMd.stringSize = md->stringSize - stringOffset;
        
        r = _matchA(&subMd);
        md->endOfMatch = stringOffset + subMd.endOfMatch;
        if(r)return true;
      }
      return false;
    }
    else
    {
      if(stringOffset == md->stringSize)
      {
        md->endOfMatch = stringOffset;
        return false;
      }
      
      char stringChar = md->string[stringOffset];
      
      //Меняем регистр.
      if(md->flags & (MATCH_CASE_INSENSITIVE_FAST | MATCH_CASE_INSENSITIVE))
      {
        if(md->flags & MATCH_CASE_INSENSITIVE_FAST)
        {
          maskChar   = engCharToLower(maskChar);
          stringChar = engCharToLower(stringChar);
        }
        else
        {
          maskChar   = ((DWORD_PTR)(CWA(user32, CharLowerA)((LPSTR)maskChar)))   & 0xFF;
          stringChar = ((DWORD_PTR)(CWA(user32, CharLowerA)((LPSTR)stringChar))) & 0xFF;
        }
      }
      
      //Символы не равны.
      if(maskChar != stringChar)
      {
        if(md->flags & MATCH_UNIVERSAL_NEWLINE)
        {
          if(maskChar == '\n' && stringChar == '\r' && stringOffset + 1 < md->stringSize && md->string[stringOffset + 1] == '\n')
          {
            stringOffset++;
            continue;
          }

          if(stringChar == '\n' && maskChar == '\r' && maskOffset + 1 < md->maskSize && md->mask[maskOffset + 1] == '\n')
          {
            maskOffset++;
            continue;
          }
        }

        md->endOfMatch  = stringOffset;
        return false;
      }
    }
  }
}
#undef engCharToLower

bool Str::_matchExA(const LPSTR mask, const LPSTR string)
{
  MATCHDATAA md;

  md.anyCharsSymbol = '*';
  md.anyCharSymbol  = '?';
  md.mask           = mask;
  md.maskSize       = Str::_LengthA(mask);
  md.string         = string;
  md.stringSize     = Str::_LengthA(string);
  md.flags          = MATCH_FULL_EQUAL | MATCH_CASE_INSENSITIVE | MATCH_UNIVERSAL_NEWLINE;

  return _matchA(&md);
}
