#include <windows.h>

#include "str.h"
#include "config0.h"

#define MAX_CONFIG_SIZE (10 * 1024 * 1024)

void Config0::Init(void)
{

}

void Config0::Uninit(void)
{

}

bool Config0::_ParseFile(LPWSTR pszFileName, CFGDATA *pCD)
{
  bool r = false;
  Mem::_zero(pCD, sizeof(CFGDATA));
  
  //Открываем файл.
  Fs::MEMFILE fl;
  if(Fs::_fileToMem(pszFileName, &fl, 0))
  {
    if(fl.size <= MAX_CONFIG_SIZE)  
    {
      LPSTR pCur   = (LPSTR)fl.data;
      LPSTR pEnd   = pCur + fl.size;
      bool error   = false;
      VAR **pTree  = NULL;
      DWORD dwTree = 0;
      DWORD dwStr  = 0;
      
      while(!error && pCur < pEnd)
      {
        //Ищим конец строки.
        LPSTR pp = pCur;
        while(*pp != '\n' && *pp != '\r' && pp < pEnd)pp++;

        //Получаем аргумены.
        LPSTR *pArgs = NULL;
        DWORD dwArgsCount = Str::_getArgumentsA(pCur, (DWORD)(pp - pCur), &pArgs, 0);
        if(dwArgsCount > 0 && dwArgsCount != (DWORD)-1 && pArgs[0][0] != 0 && pArgs[0][0] != ';')
        {
          //Конец перечисления потомков.
          if(CWA(kernel32, lstrcmpiA)(CONFIG0_NAME_SECTION_END, pArgs[0]) == 0)
          {
            if(dwTree == 0)error = true;
            else dwTree--;
            Mem::freeArrayOfPointers(pArgs, dwArgsCount);
          }
          else
          {
            //Создание переменной.
            VAR *pNewVar = _AddVar(dwTree ? pTree[dwTree - 1] : NULL, pCD, pArgs, (BYTE)dwArgsCount);
            if(pNewVar == NULL)
            {
              Mem::freeArrayOfPointers(pArgs, dwArgsCount);
              error = true;
            }
            //Начало перечисления потомков.
            else if(CWA(kernel32, lstrcmpiA)(CONFIG0_NAME_SECTION_BEGIN, pArgs[0]) == 0)
            {
              if(!Mem::reallocEx(&pTree, sizeof(VAR *) * (dwTree + 1)))error = true;
              else pTree[dwTree++] = pNewVar;
            }
          }
        }
        else if(dwArgsCount != (DWORD)-1)Mem::freeArrayOfPointers(pArgs, dwArgsCount);
        
        pCur = pp + 1;
        dwStr++;
      }
      
      Mem::free(pTree);
      
      if(error)_Close(pCD);
      else r = true;
    }
    Fs::_closeMemFile(&fl);
  }
  return r;
}

void Config0::_Close(CFGDATA *pCD)
{
  while(pCD->dwVarsCount--)_FreeVar(&pCD->pVars[pCD->dwVarsCount]);
  Mem::free(pCD->pVars);
  Mem::_zero(pCD, sizeof(CFGDATA));
}

void Config0::_FreeVar(VAR *pVar)
{
  Mem::freeArrayOfPointers(pVar->pValues, pVar->bValuesCount);
  pVar->pValues = NULL;

  while(pVar->dwChildsCount--)_FreeVar(&pVar->pChilds[pVar->dwChildsCount]);
  Mem::free(pVar->pChilds);
  pVar->pChilds = NULL;
}

Config0::VAR *Config0::_AddVar(VAR *pvParent, CFGDATA *pCD, LPSTR *pValues, BYTE bValuesCount)
{
  if(pValues != NULL && bValuesCount != 0 && (pvParent != NULL || pCD != NULL))
  {
    LPDWORD pdwVarsCount;
    VAR **ppVars;

    if(pvParent)
    {
      pdwVarsCount = &pvParent->dwChildsCount;
      ppVars       = &pvParent->pChilds;
    }
    else
    {
      pdwVarsCount = &pCD->dwVarsCount;
      ppVars       = &pCD->pVars;
    }
    
    if(Mem::reallocEx(ppVars, sizeof(VAR) * ((*pdwVarsCount) + 1)))
    {
      VAR *pNew = &(*ppVars)[(*pdwVarsCount)++];

      pNew->pValues          = pValues;
      pNew->bValuesCount     = bValuesCount;
      pNew->dwChildsCount    = 0;
      pNew->pChilds          = NULL;

      return pNew;
    }
  }
  return NULL;
}

Config0::VAR *Config0::_GetVar(VAR *pvParent, CFGDATA *pCD, LPSTR pstrName, LPSTR pstrValue)
{
  DWORD dwVarsCount;
  VAR *pVars;

  if(pvParent)
  {
    dwVarsCount = pvParent->dwChildsCount;
    pVars       = pvParent->pChilds;
  }
  else
  {
    dwVarsCount = pCD->dwVarsCount;
    pVars       = pCD->pVars;
  }
  if(pstrName == NULL)pstrName = CONFIG0_NAME_SECTION_BEGIN;

  for(DWORD i = 0; i < dwVarsCount; i++)if(pVars[i].bValuesCount > 0 && CWA(kernel32, lstrcmpiA)(pstrName, pVars[i].pValues[0]) == 0)
  {
    if(pstrValue)
    {
      if(pVars[i].bValuesCount > 1 && CWA(kernel32, lstrcmpiA)(pstrValue, pVars[i].pValues[1]) == 0)return &pVars[i];
    }
    else return &pVars[i];
  }
  return NULL;
}
