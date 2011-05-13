#include <windows.h>

#include "registry.h"

void Registry::init(void)
{

}

void Registry::uninit(void)
{

}

DWORD Registry::_getValueAsString(HKEY key, const LPWSTR subKey, const LPWSTR value, LPWSTR buffer, DWORD bufferSize)
{
  DWORD type;
  DWORD size = bufferSize * sizeof(WCHAR);
  
  if((size = _getValueAsBinary(key, subKey, value, &type, (LPBYTE)buffer, size)) != (DWORD)-1 &&
     (size % sizeof(WCHAR)) == 0 &&
     (type == REG_SZ || type == REG_EXPAND_SZ))
  {
    //If the data has the REG_SZ, REG_MULTI_SZ or REG_EXPAND_SZ type, the string may not have been
    //stored with the proper terminating null characters.
    
    if(size == 0)*buffer = 0;
    else
    {
      DWORD i = (size / sizeof(WCHAR)) - 1; //Получаем индекс последнего символа.
      
      //Последний сивмол \0, значит размер равен позиции этого символа.
      if(buffer[i] == 0)size = i; 
      else if(bufferSize > ++i)
      {
        buffer[i] = 0;
        size      = i; 
      }
      else goto BAD_END;
    }

    if(size > 2/*мин. разме меременной 3 символа*/ && type == REG_EXPAND_SZ)
    {
      LPWSTR tmpBuf = Str::_CopyExW(buffer, size);
      if(tmpBuf == NULL || CWA(kernel32, ExpandEnvironmentStringsW)(tmpBuf, buffer, bufferSize) == 0)size = (DWORD)-1;
      Mem::free(tmpBuf);
    }

    return size;
  }

BAD_END:
  return (DWORD)-1;
}

bool Registry::_setValueAsString(HKEY key, const LPWSTR subKey, const LPWSTR value, const LPWSTR buffer, DWORD bufferSize)
{
  return _setValueAsBinary(key, subKey, value, REG_SZ, (LPBYTE)buffer, bufferSize * sizeof(WCHAR) + sizeof(WCHAR));
}

DWORD Registry::_getValueAsDword(HKEY key, const LPWSTR subKey, LPWSTR value)
{
  DWORD retVal = 0;
  DWORD type;
  
  if(_getValueAsBinary(key, subKey, value, &type, (LPBYTE)&retVal, sizeof(DWORD)) != sizeof(DWORD) || type != REG_DWORD)retVal = 0;
  return retVal;
}

bool Registry::_setValueAsDword(HKEY key, const LPWSTR subKey, const LPWSTR value, DWORD data)
{
  return _setValueAsBinary(key, subKey, value, REG_DWORD, (LPBYTE)&data, sizeof(DWORD));
}

DWORD Registry::_getValueAsBinary(HKEY key, const LPWSTR subKey, const LPWSTR value, LPDWORD type, void *buffer, DWORD bufferSize)
{
  DWORD retVal = (DWORD)-1;
  if(CWA(advapi32, RegOpenKeyExW)(key, subKey, 0, KEY_QUERY_VALUE, &key) == ERROR_SUCCESS)
  {
    if(CWA(advapi32, RegQueryValueExW)(key, value, NULL, type, (LPBYTE)buffer, &bufferSize) == ERROR_SUCCESS)retVal = bufferSize;
    CWA(advapi32, RegCloseKey)(key);
  }
  return retVal;
}

bool Registry::_setValueAsBinary(HKEY key, const LPWSTR subKey, const LPWSTR value, DWORD type, const void *buffer, DWORD bufferSize)
{
  bool retVal = false;
  if(CWA(advapi32, RegCreateKeyExW)(key, subKey, 0, NULL, 0, KEY_SET_VALUE, NULL, &key, NULL) == ERROR_SUCCESS)
  {
    if(CWA(advapi32, RegSetValueExW)(key, value, 0, type, (LPBYTE)buffer, bufferSize) == ERROR_SUCCESS)retVal = true;
    CWA(advapi32, RegCloseKey)(key);
  }
  return retVal;
}

DWORD Registry::_getValueAsBinaryEx(HKEY key, const LPWSTR subKey, const LPWSTR value, LPDWORD type, void **buffer)
{
  DWORD retVal = (DWORD)-1;
  *buffer      = NULL;

  if(CWA(advapi32, RegOpenKeyExW)(key, subKey, NULL, KEY_QUERY_VALUE, &key) == ERROR_SUCCESS)
  {
    DWORD bufferSize = 0;
    if(CWA(advapi32, RegQueryValueExW)(key, value, NULL, type, NULL, &bufferSize) == ERROR_SUCCESS)
    {
      if(bufferSize == 0)retVal = 0;
      else
      {
        LPBYTE p = (LPBYTE)Mem::alloc(bufferSize + sizeof(WCHAR) * 2/*\0\0 для REG_*SZ*/);
        if(p != NULL)
        {
          if(CWA(advapi32, RegQueryValueExW)(key, value, NULL, type, p, &bufferSize) == ERROR_SUCCESS)
          {
            *buffer = p;
            retVal  = bufferSize;
          }
          else Mem::free(p);
        }
      }
    }
    CWA(advapi32, RegCloseKey)(key);
  }
  return retVal;
}

DWORD Registry::_getsCrc32OfValue(HKEY key, const LPWSTR subKey, const LPWSTR value)
{
  BYTE *data;
  DWORD dataSize;
  DWORD crc32 = 0;
  
  if((dataSize = _getValueAsBinaryEx(key, subKey, value, NULL, (void **)&data)) != (DWORD)-1 && dataSize > 0)
  {
    crc32 = Crypt::crc32Hash(data, dataSize);
    Mem::free(data);
  }

  return crc32;
}

bool Registry::_deleteKey(HKEY key, const LPWSTR subKey)
{
  return (CWA(shlwapi, SHDeleteKeyW)(key, subKey) == ERROR_SUCCESS);
}

bool Registry::_deleteValue(HKEY key, const LPWSTR subKey, const LPWSTR value)
{
  return (CWA(shlwapi, SHDeleteValueW)(key, subKey, value) == ERROR_SUCCESS);
}

bool Registry::_valueExists(HKEY key, const LPWSTR subKey, const LPWSTR value)
{
  bool retVal = false;
  if(CWA(advapi32, RegOpenKeyExW)(key, subKey, NULL, KEY_QUERY_VALUE, &key) == ERROR_SUCCESS)
  {
    retVal = (CWA(advapi32, RegQueryValueExW)(key, value, NULL, NULL, NULL, NULL) == ERROR_SUCCESS);
    CWA(advapi32, RegCloseKey)(key);
  }
  return retVal;
}
