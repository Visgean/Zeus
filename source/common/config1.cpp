#include <windows.h>
#include "config1.h"
#include "fs.h"
#include "mem.h"
#include "str.h"

//Максимальный размер файла конфигурации.
#define MAX_CONFIG_SIZE (50 * 1024 * 1024)

void Config1::Init(void)
{

}

void Config1::Uninit(void)
{

}

bool Config1::_ParseFile(LPWSTR pstrFile, LPDWORD pdwErrorLine, CONFIGFILE *pcf)
{
  Mem::_zero(pcf, sizeof(CONFIGFILE));
  if(pdwErrorLine)*pdwErrorLine = 0;
  
  //Открываем файл.
  LPSTR *pstrStrings;
  DWORD dwStringsCount;
  {
    Fs::MEMFILE mf;
    if(!Fs::_fileToMem(pstrFile, &mf, false))return false;
    if(mf.dwSize <= UTF8_BOM_SIZE || mf.dwSize > MAX_CONFIG_SIZE){Fs::_CloseMemFile(&mf); return false;}
    
    BYTE bDataOffset = 0;
    if(Str::_UTF8IsBOM(mf.pData))bDataOffset = UTF8_BOM_SIZE;
    dwStringsCount = Str::_SplitToStringsA((LPSTR)mf.pData + bDataOffset, mf.dwSize - bDataOffset, &pstrStrings, Str::STS_TRIM);
    Fs::_CloseMemFile(&mf);
    
    if(dwStringsCount == (DWORD)(-1))return false;
  }

  //Обработка строк.
  VAR **pvTree = NULL;
  DWORD dwTree = 0;
  bool rVal = true;
  VAR *pvLast = NULL;

  DWORD iCurLine = 0;
  for(; iCurLine < dwStringsCount; iCurLine++)if(pstrStrings[iCurLine] != NULL && pstrStrings[iCurLine][0] != 0)
  {
    LPSTR pCur = pstrStrings[iCurLine];
    
    //Игнорируем коментарии.
    if(pCur[0] == '#' || pCur[0] == ';' || (pCur[0] == '/' && pCur[1] == '/'))continue;

    //Получаем аргументы.
    LPSTR *pstrArgs;
    DWORD dwArgsCount = Str::_GetArgumentsA(pCur, Str::_LengthA(pCur), &pstrArgs, Str::STA_FORMAT_C);
    if(dwArgsCount == (DWORD)(-1)){rVal = false; goto END;}
    if(dwArgsCount == 0)continue;

    //Управление потомками.
    if(dwArgsCount == 1 && pstrArgs[0][1] == 0)
    {
      register char c = pstrArgs[0][0];
      if(c == '{' || c == '}')
      {
        Mem::freeArrayOfPointers(pstrArgs, dwArgsCount);
        if(c == '{')
        {
          if(pvLast == NULL ||                                      //Нет родителя.
            (dwTree > 0 && pvTree[dwTree - 1] == pvLast) ||         //Поворное указание ковычек. Напимер. ds {{
            !Mem::reallocEx(&pvTree, sizeof(VAR *) * (dwTree + 1))) //Нехватае памяти.
          {
            rVal = false;
            goto END;
          }
          pvTree[dwTree++] = pvLast;
        }
        else if(/*c == '}' && */dwTree-- == 0)
        {
          rVal = false;
          break;
        }
        continue;
      }
    }

    //Конвертируем UTF-8 в unicode.
    for(DWORD a = 0; a < dwArgsCount; a++)
    {
      LPWSTR pt = Str::_utf8ToUnicode((LPBYTE)pstrArgs[a], -1);
      if(pt == NULL)
      {
        Mem::freeArrayOfPointers(pstrArgs, dwArgsCount);
        rVal = false;
        goto END;
      }
      Mem::free(pstrArgs[a]);
      pstrArgs[a] = (LPSTR)pt;
    }
    
    //Создаем переменную.
    if((pvLast = _AddVar(dwTree > 0 ? pvTree[dwTree - 1] : NULL, pcf, (LPWSTR *)pstrArgs, dwArgsCount)) == NULL)
    {
      rVal = false;
      Mem::freeArrayOfPointers(pstrArgs, dwArgsCount);
      goto END;
    }
    pvLast->dwSourceLine = iCurLine + 1;
  }

END:  
  Mem::free(pvTree);
  Mem::freeArrayOfPointers(pstrStrings, dwStringsCount);
  
  if(rVal == false)
  {
    if(pdwErrorLine)*pdwErrorLine = iCurLine + 1;
    _CloseFile(pcf);
  }
  return rVal;
}

Config1::VAR *Config1::_AddVar(VAR *pvParent, CONFIGFILE *pcf, LPWSTR *pstrValues, DWORD dwValuesCount)
{
  if(pstrValues == NULL || dwValuesCount == 0)return NULL;

  LPDWORD pdwVarsCount;
  VAR **ppVars;

  if(pvParent)
  {
    pdwVarsCount = &pvParent->dwChildsCount;
    ppVars       = &pvParent->pvChilds;
  }
  else
  {
    pdwVarsCount = &pcf->dwVarsCount;
    ppVars       = &pcf->pvVars;
  }

  if(!Mem::reallocEx(ppVars, sizeof(VAR) * ((*pdwVarsCount) + 1)))return NULL;
  VAR *pNew = &(*ppVars)[(*pdwVarsCount)++];

  Mem::_zero(pNew, sizeof(VAR));
  pNew->ppstrValues   = pstrValues;
  pNew->dwValuesCount = dwValuesCount;

  return pNew;
}

Config1::VAR *Config1::_GetVar(VAR *pvParent, CONFIGFILE *pcf, LPWSTR *pstrValues, DWORD dwValuesCount, bool bMathCase)
{
  if(pstrValues != NULL && dwValuesCount > 0)
  {
    DWORD dwVarsCount;
    VAR *pVars;

    if(pvParent)
    {
      dwVarsCount = pvParent->dwChildsCount;
      pVars       = pvParent->pvChilds;
    }
    else
    {
      dwVarsCount = pcf->dwVarsCount;
      pVars       = pcf->pvVars;
    }

    for(DWORD i = 0; i < dwVarsCount; i++)if(pVars[i].dwValuesCount >= dwValuesCount)
    {
      DWORD k = 0;
      for(; k < dwValuesCount; k++)
      {
        int r;
        LPWSTR pv1 = pVars[i].ppstrValues[k];
        LPWSTR pv2 = pstrValues[k];

        if(bMathCase)r = Str::_CompareW(pv1, pv2, -1, -1);
        else
        {
          if(pv1 == NULL)pv1 = L"";
          if(pv2 == NULL)pv2 = L"";
          r = CWA(kernel32, lstrcmpiW)(pv1, pv2);
        }

        if(r != 0)break;
      }
      if(k == dwValuesCount)return &pVars[i];
    }
  }

  return NULL;
}

void Config1::_FreeVar(VAR *pvVar)
{
  Mem::freeArrayOfPointers(pvVar->ppstrValues, pvVar->dwValuesCount);
  while(pvVar->dwChildsCount--)_FreeVar(&pvVar->pvChilds[pvVar->dwChildsCount]);
  Mem::free(pvVar->pvChilds);
  Mem::_zero(pvVar, sizeof(VAR));
}

void Config1::_CloseFile(CONFIGFILE *pcf)
{
  while(pcf->dwVarsCount--)_FreeVar(&pcf->pvVars[pcf->dwVarsCount]);
  Mem::free(pcf->pvVars);
  Mem::_zero(pcf, sizeof(CONFIGFILE));
}