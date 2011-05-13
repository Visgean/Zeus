#include <windows.h>
#include <shlwapi.h>
#include <fcntl.h>
#include <fdi.h>
#include <fci.h>

#include "mscab.h"
#include "mem.h"
#include "fs.h"
#include "str.h"

#define MSCAB_DLL "cabinet.dll"
#define HEAP_SIZE (512 * 1024)

#define STR_OUTPUT_FILE      "?O"
#define STR_OUTPUT_FILE_SIZE (sizeof(STR_OUTPUT_FILE) - 1)

#define STR_INPUT_FILE       "?I"
#define STR_INPUT_FILE_SIZE  (sizeof(STR_INPUT_FILE) - 1)

#define STR_TEMP_FILE        "?T"
#define STR_TEMP_FILE_SIZE   (sizeof(STR_TEMP_FILE) - 1)

typedef HFCI (DIAMONDAPI *fFciCreate)      (PERF, PFNFCIFILEPLACED, PFNFCIALLOC, PFNFCIFREE, PFNFCIOPEN, PFNFCIREAD, PFNFCIWRITE, PFNFCICLOSE, PFNFCISEEK, PFNFCIDELETE, PFNFCIGETTEMPFILE, PCCAB, void*);
typedef BOOL (DIAMONDAPI *fFciAddFile)     (HFCI, char *, char *, BOOL, PFNFCIGETNEXTCABINET, PFNFCISTATUS, PFNFCIGETOPENINFO, TCOMP);
typedef BOOL (DIAMONDAPI *fFciFlushFolder) (HFCI,       PFNFCIGETNEXTCABINET, PFNFCISTATUS);
typedef BOOL (DIAMONDAPI *fFciFlushCabinet)(HFCI, BOOL, PFNFCIGETNEXTCABINET, PFNFCISTATUS);
typedef BOOL (DIAMONDAPI *fFciDestroy)     (HFCI);

typedef HFDI (DIAMONDAPI *fFdiCreate)      (PFNALLOC, PFNFREE, PFNOPEN, PFNREAD, PFNWRITE, PFNCLOSE, PFNSEEK, int, PERF);
typedef BOOL (DIAMONDAPI *fFdiCopy)        (HFDI hfdi, char *, char *, int, PFNFDINOTIFY, PFNFDIDECRYPT, void*);
typedef BOOL (DIAMONDAPI *fFdiIsCabinet)   (HFDI, INT_PTR, PFDICABINETINFO);
typedef BOOL (DIAMONDAPI *fFdiDestroy)     (HFDI);

//_open
typedef struct
{
  LPWSTR fileName;
  HANDLE handle;
  int oflags;
}CFDATA;

#if(XLIB_MSCAB_FCI > 0)
typedef struct
{
  HFCI hFCI;
  ERF error;
  CCAB cab;
  
  bool bCurrentFileNameAsAnsi;    //Сохранять имя файла в ANSI кодировки.
  WCHAR strCurrentFile[MAX_PATH]; //Текущий файл для сжатия.
  WCHAR strOutputFile[MAX_PATH];  //Текущий файл архива.
  WCHAR strTempPath[MAX_PATH];    //Временная папка.
}FCIDATA;
#endif

#if(XLIB_MSCAB_FDI > 0)
typedef struct
{
  HFDI hFDI;
  ERF error;
  MsCab::FDICOPYFILECALLBACK copyFile;
  void *callbackData;
}FDIDATA;
#endif

static DWORD ref_count;
static HANDLE hMsCabHeap; //Данная гениальная библиотека с 93-го года не научилась убирать за собой свой
                     //мусор поэтому я создаю для нее отдельную кучу.
static HMODULE hDll;

#if(XLIB_MSCAB_FCI > 0)
static fFciCreate       mf_FciCreate;
static fFciAddFile      mf_FciAddFile;
//static fFciFlushFolder  mf_FciFlushFolder;
static fFciFlushCabinet mf_FciFlushCabinet;
static fFciDestroy      mf_FciDestroy;
#endif

#if(XLIB_MSCAB_FDI > 0)
static fFdiCreate       mf_FdiCreate;
static fFdiCopy         mf_FdiCopy;
//static fFdiIsCabinet    mf_FdiIsCabinet;
static fFdiDestroy      mf_FdiDestroy;
#endif

static void *DIAMONDAPI __FxIAlloc(ULONG size)
{
  return CWA(kernel32, HeapAlloc)(hMsCabHeap, HEAP_ZERO_MEMORY, size);
}

static void DIAMONDAPI __FxIFree(void *p)
{
  if(p != NULL)CWA(kernel32, HeapFree)(hMsCabHeap, 0, p);
}

static INT_PTR __FxIOpen(LPWSTR pszFile, int oflag, int pmode)
{
  DWORD dwDesiredAccess = 0;
  DWORD dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
  DWORD dwCreationDisposition = 0;
  DWORD dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL;

  switch(oflag & (_O_RDONLY | _O_WRONLY | _O_RDWR))
  {
    case _O_RDONLY: dwDesiredAccess = GENERIC_READ; break;
    case _O_WRONLY: dwDesiredAccess = GENERIC_WRITE; break;
    case _O_RDWR:   dwDesiredAccess = GENERIC_READ | GENERIC_WRITE; break;
    default: return -1;
  }

  switch(oflag & (_O_CREAT | _O_EXCL | _O_TRUNC))
  {
    case 0:
    case _O_EXCL:                       dwCreationDisposition = OPEN_EXISTING; break;
    case _O_CREAT:                      dwCreationDisposition = OPEN_ALWAYS; break;
    case _O_CREAT | _O_EXCL:
    case _O_CREAT | _O_TRUNC | _O_EXCL: dwCreationDisposition = CREATE_NEW; break;
    case _O_TRUNC:
    case _O_TRUNC | _O_EXCL:            dwCreationDisposition = TRUNCATE_EXISTING; break;
    case _O_CREAT | _O_TRUNC:           dwCreationDisposition = CREATE_ALWAYS; break;
    default: return -1;
  }

  //if(oflag & _O_CREAT && !(pmode & _S_IWRITE))dwFlagsAndAttributes |= FILE_ATTRIBUTE_READONLY;

  /*if(oflag & _O_TEMPORARY)
  {
  dwFlagsAndAttributes |= FILE_FLAG_DELETE_ON_CLOSE;
  dwDesiredAccess      |= DELETE;
  dwShareMode          |= FILE_SHARE_DELETE;
  }*/

  //if(oflag & _O_SHORT_LIVED)dwFlagsAndAttributes |= FILE_ATTRIBUTE_TEMPORARY;

  //if(oflag & _O_SEQUENTIAL)dwFlagsAndAttributes |= FILE_FLAG_SEQUENTIAL_SCAN;
  //else if(oflag & _O_RANDOM)dwFlagsAndAttributes |= FILE_FLAG_RANDOM_ACCESS;

  HANDLE h = CWA(kernel32, CreateFileW)(pszFile, dwDesiredAccess, dwShareMode, NULL, dwCreationDisposition, dwFlagsAndAttributes, NULL);
  if(h != INVALID_HANDLE_VALUE)
  {
    CFDATA *p = (CFDATA *)__FxIAlloc(sizeof(CFDATA));
    if(p == NULL)CWA(kernel32, CloseHandle)(h);
    else
    {
      p->fileName = Str::_CopyExW(pszFile, -1); //Это параметр не критичен, не проверяем его.
      p->handle = h;
      p->oflags = oflag;
      return (INT_PTR)p;
    }
  }

  return (INT_PTR)-1;
}

static UINT __FxIRead(INT_PTR hf, void *memory, UINT cb)
{
  DWORD r; 
  if(hf != -1 && hf != 0)
  {
    if(memory != NULL && CWA(kernel32, ReadFile)(((CFDATA *)hf)->handle, memory, cb, &r, NULL))return r;
  }
  return (UINT)-1;
}

static UINT __FxIWrite(INT_PTR hf, void *memory, UINT cb)
{
  if(hf != -1 && hf != 0)
  {
    DWORD w;
    if(((CFDATA *)hf)->oflags & _O_APPEND)Fs::_setFilePointer(((CFDATA *)hf)->handle, 0, FILE_END);
    if(CWA(kernel32, WriteFile)(((CFDATA *)hf)->handle, memory, cb, &w, NULL))return w;
  }
  return (UINT)-1;
}

static long __FxISeek(INT_PTR hf, long dist, int seektype)
{
  if(hf != -1 && hf != 0)
  {
    DWORD dwMoveMethod;
    switch(seektype)
    {
    case 0/*SEEK_SET*/: dwMoveMethod = FILE_BEGIN; break;
    case 1/*SEEK_CUR*/: dwMoveMethod = FILE_CURRENT; break;
    case 2/*SEEK_END*/: dwMoveMethod = FILE_END; break;
    default: return -1;
    }
    if(Fs::_setFilePointer(((CFDATA *)hf)->handle, (DWORD64)dist, dwMoveMethod))return (long)Fs::_getFilePointer(((CFDATA *)hf)->handle);
  }
  return -1;
}

static int __FxIClose(INT_PTR hf)
{
  if(hf != -1 && hf != 0)
  {
    CWA(kernel32, CloseHandle)(((CFDATA *)hf)->handle);
    Mem::free(((CFDATA *)hf)->fileName);
    __FxIFree((void *)hf);
  }
  return 0;
}

#if(XLIB_MSCAB_FCI > 0)
static bool PathToNormal(LPSTR pszPath, LPWSTR pBuffer, void *pv)
{
  bool r = true;
  if(Mem::_compare(pszPath, STR_OUTPUT_FILE, STR_OUTPUT_FILE_SIZE) == 0)
  {
    Str::_CopyW(pBuffer, ((FCIDATA *)pv)->strOutputFile, -1);
  }
  else if(Mem::_compare(pszPath, STR_INPUT_FILE, STR_INPUT_FILE_SIZE) == 0)
  {
    Str::_CopyW(pBuffer, ((FCIDATA *)pv)->strCurrentFile, -1);
  }
  else if(Mem::_compare(pszPath, STR_TEMP_FILE, STR_TEMP_FILE_SIZE) == 0)
  {
    //Имена временных файлов всегда состоят из US символов, поэтому работа с UTF8 не требуется.
    WCHAR path[MAX_PATH];
    Str::_ansiToUnicode(pszPath + STR_TEMP_FILE_SIZE + 1, -1, path, MAX_PATH);
    if(!Fs::_pathCombine(pBuffer, ((FCIDATA *)pv)->strTempPath, path))r = false;
  }
  else r = false; //Другие пути не поддерживаются.

  return r;
}

static INT_PTR DIAMONDAPI __FCIOpen(LPSTR pszFile, int oflag, int pmode, int *err, void *pv)
{
  WCHAR file[MAX_PATH];
  if(PathToNormal(pszFile, file, pv))return  __FxIOpen(file, oflag, pmode);
  return (INT_PTR)-1;
}

static UINT DIAMONDAPI __FCIRead(INT_PTR hf, void *memory, UINT cb, int *err, void *pv)
{
  return __FxIRead(hf, memory, cb);
}

static UINT DIAMONDAPI __FCIWrite(INT_PTR hf, void *memory, UINT cb, int *err, void *pv)
{
  return __FxIWrite(hf, memory, cb);
}

static int DIAMONDAPI __FCIClose(INT_PTR hf, int *err, void *pv)
{
  return __FxIClose(hf);
}

static long DIAMONDAPI __FCISeek(INT_PTR hf, long dist, int seektype, int *err, void *pv)
{
  return __FxISeek(hf, dist, seektype);
}

static int DIAMONDAPI __FCIDelete(LPSTR pszFile, int *err, void *pv)
{
  WCHAR file[MAX_PATH];
  return (PathToNormal(pszFile, file, pv) && Fs::_removeFile(file)) ? 0 : -1;
}

static BOOL DIAMONDAPI __FCITempFile(char *pszTempName, int cbTempName, void *pv)
{
  BOOL r = FALSE;
  WCHAR file[MAX_PATH];
  FCIDATA *p = (FCIDATA *)pv;

  if(CWA(kernel32, GetTempFileNameW)(p->strTempPath, L"cab", 0, file) > 0 && Fs::_removeFile(file))
  {
    //Имена временных файлов всегда состоят из US символов, поэтому работа с UTF8 не требуется.
    Str::_unicodeToAnsi(CWA(shlwapi, PathFindFileNameW)(file), -1, pszTempName + STR_TEMP_FILE_SIZE + 1, cbTempName - (STR_TEMP_FILE_SIZE + 1));
    Mem::_copy(pszTempName, STR_TEMP_FILE, STR_TEMP_FILE_SIZE);
    pszTempName[STR_TEMP_FILE_SIZE] = '\\';
    r = TRUE;
  }
  return r;
}

static BOOL DIAMONDAPI __FCIGetNextCab(PCCAB pccab, ULONG cbPrevCab, void* pv)
{
  return TRUE;
}

static long DIAMONDAPI __FCIProgress(UINT typeStatus, ULONG cb1, ULONG cb2, void *pv)
{
  return 0;
}

static int DIAMONDAPI __FCIFileDest(PCCAB pccab, char *pszFile, long cbFile, BOOL fContinuation, void *pv)
{ 
  return 0;
}

static INT_PTR DIAMONDAPI __FCIOpenInfo(char *pszName, USHORT *pdate, USHORT *ptime, USHORT *pattribs, int *err, void *pv)
{ 
  INT_PTR r = -1;
  CFDATA *pd = (CFDATA *)__FCIOpen(pszName, _O_RDONLY, 0, err, pv);
  if(pd && (INT_PTR)pd != (INT_PTR)-1)  
  {
    BY_HANDLE_FILE_INFORMATION inf;
    if(CWA(kernel32, GetFileInformationByHandle)(pd->handle, &inf))
    {
      FCIDATA *p = (FCIDATA *)pv;
      FILETIME ft;
      CWA(kernel32, FileTimeToLocalFileTime)(&inf.ftLastWriteTime, &ft);
      CWA(kernel32, FileTimeToDosDateTime)(&ft, pdate, ptime);
      
      //Атрибуты FILE_ATTRIBUTE_* совпадают с _A_*.
      *pattribs = (USHORT)(inf.dwFileAttributes & (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_ARCHIVE));
      if(p->bCurrentFileNameAsAnsi == false)*pattribs |=_A_NAME_IS_UTF;
      r = (INT_PTR)pd;
    }
    else __FCIClose((INT_PTR)pd, err, pv);
  }
  return r;
}
#endif

#if(XLIB_MSCAB_FDI > 0)
static INT_PTR DIAMONDAPI __FDIOpen(LPSTR pszFile, int oflag, int pmode)
{
  LPWSTR file = Str::_utf8ToUnicode(pszFile, -1);
  if(file != NULL)
  {
    INT_PTR r = __FxIOpen(file, oflag, pmode);
    Mem::free(file);
    return r;
  }
  return (INT_PTR)-1;
}

static UINT DIAMONDAPI __FDIRead(INT_PTR hf, void *pv, UINT cb)
{
  return __FxIRead(hf, pv, cb);
}

static UINT DIAMONDAPI __FDIWrite(INT_PTR hf, void *pv, UINT cb)
{
  return __FxIWrite(hf, pv, cb);
}

static int DIAMONDAPI __FDIClose(INT_PTR hf)
{
  return __FxIClose(hf);
}

static long DIAMONDAPI __FDISeek(INT_PTR hf, long dist, int seektype)
{
  return __FxISeek(hf, dist, seektype);
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

static bool AddRef(void)
{
  if(ref_count == 0)
  {
    if((hDll = CWA(kernel32, LoadLibraryA)(MSCAB_DLL)) != NULL)
    {
#     if(XLIB_MSCAB_FCI > 0)
      mf_FciCreate       = (fFciCreate)      CWA(kernel32, GetProcAddress)(hDll, "FCICreate");
      mf_FciAddFile      = (fFciAddFile)     CWA(kernel32, GetProcAddress)(hDll, "FCIAddFile");
      //mf_FciFlushFolder  = (fFciFlushFolder) CWA(kernel32, GetProcAddress)(hDll, "FCIFlushFolder");
      mf_FciFlushCabinet = (fFciFlushCabinet)CWA(kernel32, GetProcAddress)(hDll, "FCIFlushCabinet");
      mf_FciDestroy      = (fFciDestroy)     CWA(kernel32, GetProcAddress)(hDll, "FCIDestroy");
#     endif      
#     if(XLIB_MSCAB_FDI > 0)
      mf_FdiCreate    = (fFdiCreate)   CWA(kernel32, GetProcAddress)(hDll, "FDICreate");;
      mf_FdiCopy      = (fFdiCopy)     CWA(kernel32, GetProcAddress)(hDll, "FDICopy");;
      //mf_FdiIsCabinet = (fFdiIsCabinet)CWA(kernel32, GetProcAddress)(hDll, "FDIIsCabinet");;
      mf_FdiDestroy   = (fFdiDestroy)  CWA(kernel32, GetProcAddress)(hDll, "FDIDestroy");;
#     endif      
      if(
#       if(XLIB_MSCAB_FCI > 0)
          mf_FciCreate != NULL && mf_FciAddFile != NULL /*&& mf_FciFlushFolder != NULL*/ && mf_FciFlushCabinet != NULL && mf_FciDestroy != NULL
#         if(XLIB_MSCAB_FDI > 0)
          &&
#         endif
#       endif      
#         if(XLIB_MSCAB_FDI > 0)
          mf_FdiCreate != NULL && mf_FdiCopy != NULL /*&& mf_FdiIsCabinet != NULL*/ && mf_FdiDestroy != NULL          
#         endif      
          )
      {
        hMsCabHeap = CWA(kernel32, HeapCreate)(0, HEAP_SIZE, 0);
        if(hMsCabHeap != NULL)goto GOODEND;
      }
      CWA(kernel32, FreeLibrary)(hDll);
    }
    return false;
  }

GOODEND:
  ref_count++;
  return true;
}

static void Release(void)
{
  if(--ref_count == 0)
  {
    CWA(kernel32, FreeLibrary)(hDll);    
    CWA(kernel32, HeapDestroy)(hMsCabHeap);
  }
}

void MsCab::Init(void)
{
  ref_count = 0;
}

void MsCab::Uninit(void)
{

}

#if(XLIB_MSCAB_FCI > 0)

void *MsCab::FCICreate(LPWSTR pstrPath, LPWSTR pstrFile, LPWSTR pstrTempPath)
{
  void *r = NULL;
  if(AddRef())
  {
    FCIDATA *p = (FCIDATA *)Mem::alloc(sizeof(FCIDATA));
    if(p != NULL)
    {
      if(Fs::_pathCombine(p->strOutputFile, pstrPath, pstrFile))
      {
        bool k = false;
        int len;
        
        if(pstrTempPath == NULL)
        {
          k = (CWA(kernel32, GetTempPathW)(MAX_PATH - 1, p->strTempPath) > 0);
        }
        else if((len = Str::_LengthW(pstrTempPath)) < MAX_PATH)
        {
          Str::_CopyW(p->strTempPath, pstrTempPath, len);
          k = true;
        }
        
        if(k)
        {
          p->cab.cb             = CB_MAX_DISK;
          p->cab.cbFolderThresh = CB_MAX_DISK;
          p->cab.iCab           = 1;
          p->cab.iDisk          = 1;
          //p->cab.setID          = 0;
          Mem::_copy(p->cab.szCab, MSCAB_DLL, sizeof(MSCAB_DLL)); //Не имеет смысла.
          Mem::_copy(p->cab.szCabPath, STR_OUTPUT_FILE, STR_OUTPUT_FILE_SIZE);

          p->hFCI = mf_FciCreate(&p->error, __FCIFileDest, __FxIAlloc, __FxIFree, __FCIOpen, __FCIRead, __FCIWrite, __FCIClose, __FCISeek, __FCIDelete, __FCITempFile, &p->cab, p);
          if(p->hFCI != NULL)r = p;
        }
      }
      if(r == NULL)Mem::free(p);
    }
    if(r == NULL)Release();
  }
  
  return r;
}

bool MsCab::FCIAddFile(void *pFCI, LPWSTR pstrSourceFile, LPWSTR pstrDestFile)
{
  bool r = false;
  Str::UTF8STRING dest;
  int lDest, lSrc;
  
  if((lDest = Str::_LengthW(pstrDestFile)) < CB_MAX_CAB_PATH - 1 &&
     (lSrc = Str::_LengthW(pstrSourceFile)) < MAX_PATH - 1 &&
     Str::_utf8FromUnicode(pstrDestFile, lDest, &dest))
  {
    FCIDATA *p = (FCIDATA *)pFCI;    
    char *file;
    
    if((p->bCurrentFileNameAsAnsi = (dest.size >= CB_MAX_CAB_PATH - 1)))file = Str::_unicodeToAnsiEx(pstrDestFile, lDest);
    else file = (char *)dest.data;
    
    if(file != NULL)
    {
      Str::_CopyW(p->strCurrentFile, pstrSourceFile, lSrc);
      r = mf_FciAddFile(p->hFCI, STR_INPUT_FILE, file, FALSE, __FCIGetNextCab, __FCIProgress, __FCIOpenInfo, tcompTYPE_MSZIP) ? true : false;
    }
    
    if(p->bCurrentFileNameAsAnsi)Mem::free(file);
    Str::_utf8Free(&dest);
  }
  
  return r;
}

bool MsCab::FCIFlush(void *pFCI)
{
  return mf_FciFlushCabinet(((FCIDATA *)pFCI)->hFCI, FALSE, __FCIGetNextCab, __FCIProgress) ? true : false;
}

void MsCab::FCIClose(void *pFCI)
{
  FCIDATA *p = (FCIDATA *)pFCI;
  mf_FciDestroy(p->hFCI);
  Mem::free(p);
  Release();
}

typedef struct
{
  void *fci;
  DWORD filesCount;
  DWORD cabPathOffset;
}CFFSTRUCT;

static bool createFromFolderProc(const LPWSTR path, const WIN32_FIND_DATAW *fileInfo, void *data)
{
  WCHAR filePath[MAX_PATH];
  if(Fs::_pathCombine(filePath, path, (LPWSTR)fileInfo->cFileName))
  {
    CFFSTRUCT *cs = (CFFSTRUCT *)data;
    if(MsCab::FCIAddFile(cs->fci, filePath, filePath + cs->cabPathOffset))cs->filesCount++;
  }
  return true;
}

bool MsCab::createFromFolder(LPWSTR outputFile, LPWSTR sourceFolder, LPWSTR tempFolder, LPWSTR *fileMask, DWORD fileMaskCount, DWORD flags)
{
  CFFSTRUCT cs;
  bool r = false;
  cs.fci = FCICreate(outputFile, NULL, tempFolder);
  if(cs.fci != NULL)
  {
    cs.filesCount    = 0;
    cs.cabPathOffset = Str::_LengthW(sourceFolder);
    
    //Если путь не кончается на слеш.
    if(cs.cabPathOffset > 0 && sourceFolder[cs.cabPathOffset - 1] != '\\')cs.cabPathOffset++;

    Fs::_findFiles(sourceFolder, fileMask, fileMaskCount, (flags & MsCab::CFF_RECURSE ? Fs::FFFLAG_RECURSIVE : 0) | Fs::FFFLAG_SEARCH_FILES, createFromFolderProc, &cs, NULL, 0, 0);
    
    if(cs.filesCount > 0 && FCIFlush(cs.fci))r = true;
    FCIClose(cs.fci);
  }
  return r;
}
#endif

#if(XLIB_MSCAB_FDI > 0)
void *MsCab::FDICreate(FDICOPYFILECALLBACK copyCallback, void *data)
{
  void *r = NULL;
  if(AddRef())
  {
    FDIDATA *p = (FDIDATA *)Mem::alloc(sizeof(FDIDATA));
    if(p != NULL)
    {
      if((p->hFDI = mf_FdiCreate(__FxIAlloc, __FxIFree, __FDIOpen, __FDIRead, __FDIWrite, __FDIClose, __FDISeek, cpu80386, &p->error)) != NULL)
      {
        r               = p;
        p->copyFile     = copyCallback;
        p->callbackData = data;
      }
      else Mem::free(p);
    }
    if(r == NULL)Release();
  }

  return r;
}

static INT_PTR DIAMONDAPI fdiNotify(FDINOTIFICATIONTYPE fdint, PFDINOTIFICATION pfdin)
{
  FDIDATA *p = (FDIDATA *)pfdin->pv;

  switch(fdint)
  {
    case fdintCOPY_FILE:
    { 
      LPWSTR sourceFile;
      WCHAR destFile[MAX_PATH];
      INT_PTR r = -1;

      if(pfdin->attribs & _A_NAME_IS_UTF)sourceFile = Str::_utf8ToUnicode(pfdin->psz1, -1);
      else sourceFile = Str::_ansiToUnicodeEx(pfdin->psz1, -1);
      
      if(sourceFile != NULL)
      {
        if(p->copyFile == NULL)r = Fs::_pathCombine(destFile, (LPWSTR)p->callbackData, sourceFile) == false ? -1 : 1;
        else r = p->copyFile(sourceFile, destFile, pfdin->cb, p->callbackData);
        Mem::free(sourceFile);

        if(r > 0)
        {
          {
            WCHAR destPath[MAX_PATH];
            Str::_CopyW(destPath, destFile, -1);
            if(CWA(shlwapi, PathRemoveFileSpecW)(destPath) != FALSE)Fs::_createDirectoryTree(destPath, NULL);
          }
          
          if((r = __FxIOpen(destFile, _O_WRONLY | _O_CREAT | _O_TRUNC, /*_S_IWRITE*/0)) < 1)r = -1;
        }
      }

      return r;
    }

    case fdintCLOSE_FILE_INFO:
    {
      if(pfdin->hf == 0 || pfdin->hf == -1)return FALSE;

      CFDATA *hf = (CFDATA *)pfdin->hf;
      FILETIME ftime;
      if(CWA(kernel32, DosDateTimeToFileTime)(pfdin->date, pfdin->time, &ftime))
      {
        FILETIME  local_ftime;
        if(CWA(kernel32, LocalFileTimeToFileTime)(&ftime, &local_ftime))CWA(kernel32, SetFileTime)(hf->handle, &local_ftime, NULL, &local_ftime);
      }

      if(hf->fileName != NULL)
      {
        CWA(kernel32, CloseHandle)(hf->handle);
        hf->handle = INVALID_HANDLE_VALUE;
        CWA(kernel32, SetFileAttributesW)(hf->fileName, (pfdin->attribs & (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_ARCHIVE)));
      }

      __FxIClose(pfdin->hf); 
      return TRUE;
    }
  }
  return 0;
}

bool MsCab::FDICopy(void *pFDI, LPWSTR pstrPath, LPWSTR pstrFile)
{
  WCHAR filePath[MAX_PATH];
  WCHAR fileName[MAX_PATH];

  Str::_CopyW(filePath, pstrPath, -1);
  if(pstrFile == NULL)
  {
    pstrFile = CWA(shlwapi, PathFindFileNameW)(pstrPath);
    if(pstrFile == pstrPath)return false;
    CWA(shlwapi, PathRemoveFileSpecW)(filePath);
  }
  Str::_CopyW(fileName, pstrFile, -1);
  CWA(shlwapi, PathAddBackslashW)(filePath);
  
  bool r = false;
  Str::UTF8STRING u8Path, u8File;
  if(Str::_utf8FromUnicode(filePath, -1, &u8Path))
  {
    if(Str::_utf8FromUnicode(fileName, -1, &u8File))
    {
      r = mf_FdiCopy(((FDIDATA *)pFDI)->hFDI, (char *)u8File.data, (char *)u8Path.data, 0, fdiNotify, NULL, pFDI) != FALSE ? true : false;    
      Str::_utf8Free(&u8File);
    }
    Str::_utf8Free(&u8Path);
  }
  
  return r;
}

void MsCab::FDIClose(void *pFDI)
{
  FDIDATA *p = (FDIDATA *)pFDI;
  mf_FdiDestroy(p->hFDI);
  Mem::free(p);
  Release();
}

bool MsCab::extractFiles(LPWSTR pstrPath, LPWSTR pstrFile, FDICOPYFILECALLBACK copyCallback, void *data)
{
  bool r = false;
  void *p;

  if((p = FDICreate(copyCallback, data)) != NULL)
  {
    r = FDICopy(p, pstrPath, pstrFile);
    FDIClose(p);
  }
  
  return r;
}
#endif
