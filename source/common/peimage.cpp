#include <windows.h>

#include "peimage.h"

//Получение NT-заголовков.
#define IMAGE_GET_NT_HEADERS(image) ((void *)((IMAGE_NT_HEADERS32 *)((LPBYTE)(image) + ((IMAGE_DOS_HEADER *)(image))->e_lfanew)))

//Получение типа NT-заголовков IMAGE_NT_OPTIONAL_HDR*.
#define IMAGE_NT_HEADERS_TYPE(image) ((WORD)(((IMAGE_NT_HEADERS32 *)IMAGE_GET_NT_HEADERS(image))->OptionalHeader.Magic))

//Получение типа процессора IMAGE_FILE_MACHINE_*.
#define IMAGE_MACHINE_TYPE(image)    ((WORD)(((IMAGE_NT_HEADERS32 *)IMAGE_GET_NT_HEADERS(image))->FileHeader.Machine))

//Мааксимальное кол. секций.
#define MAX_SECTIONS 96

typedef PIMAGE_NT_HEADERS (WINAPI *CHECKSUMMAPPEDFILE)(PVOID baseAddress, DWORD fileLength, PDWORD headerSum, PDWORD checkSum);

static DWORD createNtCheckSum(LPBYTE image, DWORD size)
{
  DWORD ret = 0;
  HMODULE dll = CWA(kernel32, LoadLibraryA)("imagehlp.dll");
  if(dll != NULL)
  {
    CHECKSUMMAPPEDFILE f = (CHECKSUMMAPPEDFILE)CWA(kernel32, GetProcAddress)(dll, "CheckSumMappedFile");
    if(f)
    {
      DWORD headerSum, checkSum;
      if(f(image, size, &headerSum, &checkSum))ret = checkSum;
    }
    CWA(kernel32, FreeLibrary)(dll);
  }

  return ret;
}

void PeImage::init(void)
{

}

void PeImage::uninit(void)
{

}

bool PeImage::_createEmpty(PEDATA *pedata, WORD machine)
{
  if(0 
#if(PEIMAGE_32 > 0)
    || machine == IMAGE_FILE_MACHINE_I386
#endif
#if(PEIMAGE_64 > 0)
    || machine == IMAGE_FILE_MACHINE_AMD64
#endif
    )
  {
    Mem::_zero(pedata, sizeof(PEDATA));
    pedata->machine = machine;
    return true;
  }

  return false;
}

void *PeImage::_createFromMemory(PEDATA *pedata, void *mem, DWORD memSize, bool isModule)
{
  if(_isPeImage(mem, memSize))
  {
    IMAGE_DOS_HEADER *dosHeader = (IMAGE_DOS_HEADER *)mem;
    IMAGE_NT_HEADERS32 *t = (IMAGE_NT_HEADERS32 *)((LPBYTE)mem + dosHeader->e_lfanew);

    if(_createEmpty(pedata, t->FileHeader.Machine))
    {
      if(_setDosHeader(pedata, dosHeader) && _setNtHeaderFromNtHeader(pedata, t))
      {
        IMAGE_SECTION_HEADER *sections;
        DWORD fileAligment;
        DWORD moduleAligment;

        switch(t->FileHeader.Machine)
        {
#         if(PEIMAGE_32 > 0)
          case IMAGE_FILE_MACHINE_I386:
            sections       = (IMAGE_SECTION_HEADER *)((LPBYTE)(&((IMAGE_NT_HEADERS32 *)t)->OptionalHeader) + ((IMAGE_NT_HEADERS32 *)t)->FileHeader.SizeOfOptionalHeader);
            fileAligment   = ((IMAGE_NT_HEADERS32 *)t)->OptionalHeader.FileAlignment;
            moduleAligment = ((IMAGE_NT_HEADERS32 *)t)->OptionalHeader.SectionAlignment;
            break;
#         endif
#         if(PEIMAGE_64 > 0)
          case IMAGE_FILE_MACHINE_AMD64:
            sections       = (IMAGE_SECTION_HEADER *)((LPBYTE)(&((IMAGE_NT_HEADERS64 *)t)->OptionalHeader) + ((IMAGE_NT_HEADERS64 *)t)->FileHeader.SizeOfOptionalHeader);
            fileAligment   = ((IMAGE_NT_HEADERS64 *)t)->OptionalHeader.FileAlignment;
            moduleAligment = ((IMAGE_NT_HEADERS32 *)t)->OptionalHeader.SectionAlignment;
            break;
#         endif
        }
      
        WORD maxSectionOffset = 0;
        for(WORD i = 0; i < t->FileHeader.NumberOfSections; i++) 
        {
          IMAGE_SECTION_HEADER *currenSections = &sections[i];
          DWORD dataOffset;

          //Проверки модуля.
          if(isModule)
          {
            DWORD normalVirtualSize = max(currenSections->SizeOfRawData, currenSections->Misc.VirtualSize); 
            normalVirtualSize       = ALIGN_UP(normalVirtualSize, moduleAligment);

            if(currenSections->VirtualAddress + normalVirtualSize > memSize)goto BAD_EXIT;
            dataOffset = currenSections->VirtualAddress;
            if(currenSections->VirtualAddress + currenSections->Misc.VirtualSize > sections[maxSectionOffset].VirtualAddress + sections[maxSectionOffset].Misc.VirtualSize)
            {
              maxSectionOffset = i;
            }
          }
          //Проверка файла.
          else
          {
            DWORD normalRawSize = ALIGN_UP(currenSections->SizeOfRawData, fileAligment);

            if(currenSections->PointerToRawData + normalRawSize > memSize)goto BAD_EXIT;
            dataOffset = currenSections->PointerToRawData;
            if(currenSections->PointerToRawData + currenSections->SizeOfRawData > sections[maxSectionOffset].PointerToRawData + sections[maxSectionOffset].SizeOfRawData)
            {
              maxSectionOffset = i;
            }
          }
          
          //Добавление секции.
          LPBYTE curMem = (LPBYTE)mem + dataOffset;
          if(CWA(kernel32, IsBadReadPtr)(curMem, currenSections->SizeOfRawData) != FALSE || 
             !_addSection(pedata, (LPSTR)currenSections->Name, currenSections->Characteristics, curMem, currenSections->PointerToRawData, currenSections->SizeOfRawData, currenSections->VirtualAddress, currenSections->Misc.VirtualSize, 0))
          {
            goto BAD_EXIT;
          }
        }

        //Вычисляем размер.
        {
          void *retVal;
          if(t->FileHeader.NumberOfSections == 0)
          {
            DWORD size = (DWORD)((DWORD_PTR)sections - (DWORD_PTR)mem);
            DWORD align = isModule ? moduleAligment : fileAligment;
            retVal = (LPBYTE)mem + ALIGN_UP(size, align);
          }
          else
          {
            if(isModule)
            {
              DWORD size = max(sections[maxSectionOffset].Misc.VirtualSize, sections[maxSectionOffset].SizeOfRawData);
              retVal = (LPBYTE)mem + sections[maxSectionOffset].VirtualAddress + ALIGN_UP(size, moduleAligment);
            }
            else
            {
              retVal = (LPBYTE)mem + sections[maxSectionOffset].PointerToRawData + ALIGN_UP(sections[maxSectionOffset].SizeOfRawData, fileAligment);
            }
          }

          return retVal;
        }

BAD_EXIT:;
      }
      _freeImage(pedata);
    }
  }

  return NULL;
}

void PeImage::_freeImage(PEDATA *pedata)
{
  Mem::free(pedata->dosHeader);
  Mem::free(pedata->ntHeader.p32);
  Mem::free(pedata->sections);
  Mem::freeArrayOfPointers(pedata->sectionsRawData, pedata->sectionsCount);
  Mem::free(pedata->sectionsFlags);
}

bool PeImage::_setDosHeader(PEDATA *pedata, void *dosHeader)
{
  IMAGE_DOS_HEADER *dh = (IMAGE_DOS_HEADER *)dosHeader;
  if(dh->e_magic == IMAGE_DOS_SIGNATURE && dh->e_lfanew >= sizeof(WORD))
  {
    void *p = Mem::alloc(dh->e_lfanew);
    if(p)
    {
      Mem::_copy(p, dh, dh->e_lfanew);
      Mem::free(pedata->dosHeader);
      pedata->dosHeader = (IMAGE_DOS_HEADER *)p;
      return true;
    }
  }
  return false;
}

bool PeImage::_setNtHeader(PEDATA *pedata, PENTBASEDATA *basedata)
{
  void *p = Mem::alloc(
                       #if(PEIMAGE_32 > 0 && PEIMAGE_64 > 0)
                       pedata->machine == IMAGE_FILE_MACHINE_I386 ? sizeof(IMAGE_NT_HEADERS32) : sizeof(IMAGE_NT_HEADERS64)
                       #elif(PEIMAGE_32 > 0)
                       sizeof(IMAGE_NT_HEADERS32)
                       #elif(PEIMAGE_64 > 0)
                       sizeof(IMAGE_NT_HEADERS64)
                       #endif
                      );
  
  if(p)
  {
    //* - значения которые будут исправлены после сборки файла.
    
    IMAGE_NT_HEADERS32 *ntHeader = (IMAGE_NT_HEADERS32 * )p;

    ntHeader->Signature                       = IMAGE_NT_SIGNATURE;

    ntHeader->FileHeader.Machine              = pedata->machine;
    ntHeader->FileHeader.NumberOfSections     = 0;//*
    ntHeader->FileHeader.TimeDateStamp        = basedata->timeDateStamp;
    ntHeader->FileHeader.PointerToSymbolTable = 0;
    ntHeader->FileHeader.NumberOfSymbols      = 0;
    
    #if(PEIMAGE_32 > 0 && PEIMAGE_64 > 0)
    ntHeader->FileHeader.SizeOfOptionalHeader = pedata->machine == IMAGE_FILE_MACHINE_I386 ? sizeof(IMAGE_OPTIONAL_HEADER32) : sizeof(IMAGE_OPTIONAL_HEADER64);
    #elif(PEIMAGE_32 > 0)
    ntHeader->FileHeader.SizeOfOptionalHeader = sizeof(IMAGE_OPTIONAL_HEADER32);
    #elif(PEIMAGE_64 > 0)
    ntHeader->FileHeader.SizeOfOptionalHeader = sizeof(IMAGE_OPTIONAL_HEADER64);
    #endif
    
    ntHeader->FileHeader.Characteristics      = basedata->characteristics;
    
    if(0){}
#if(PEIMAGE_32 > 0)
    else if(pedata->machine == IMAGE_FILE_MACHINE_I386)
    {
      IMAGE_NT_HEADERS32 *nt32Header = ntHeader;

      nt32Header->OptionalHeader.Magic                       = IMAGE_NT_OPTIONAL_HDR32_MAGIC;
      nt32Header->OptionalHeader.MajorLinkerVersion          = basedata->majorLinkerVersion;
      nt32Header->OptionalHeader.MinorLinkerVersion          = basedata->minorLinkerVersion;
      nt32Header->OptionalHeader.SizeOfCode                  = 0;//*
      nt32Header->OptionalHeader.SizeOfInitializedData       = 0;//*
      nt32Header->OptionalHeader.SizeOfUninitializedData     = 0;//*
      nt32Header->OptionalHeader.AddressOfEntryPoint         = 0;//*
      nt32Header->OptionalHeader.BaseOfCode                  = 0;//*
      nt32Header->OptionalHeader.BaseOfData                  = 0;//*
      nt32Header->OptionalHeader.ImageBase                   = (DWORD)basedata->imageBase;
      nt32Header->OptionalHeader.SectionAlignment            = basedata->sectionAlignment;
      nt32Header->OptionalHeader.FileAlignment               = basedata->fileAlignment;
      nt32Header->OptionalHeader.MajorOperatingSystemVersion = basedata->majorOperatingSystemVersion;
      nt32Header->OptionalHeader.MinorOperatingSystemVersion = basedata->minorOperatingSystemVersion;
      nt32Header->OptionalHeader.MajorImageVersion           = basedata->majorImageVersion;
      nt32Header->OptionalHeader.MinorImageVersion           = basedata->minorImageVersion;
      nt32Header->OptionalHeader.MajorSubsystemVersion       = basedata->majorSubsystemVersion;
      nt32Header->OptionalHeader.MinorSubsystemVersion       = basedata->minorSubsystemVersion;
      nt32Header->OptionalHeader.Win32VersionValue           = 0;
      nt32Header->OptionalHeader.SizeOfImage                 = 0;//*
      nt32Header->OptionalHeader.SizeOfHeaders               = 0;//*
      nt32Header->OptionalHeader.CheckSum                    = 0;//*
      nt32Header->OptionalHeader.Subsystem                   = basedata->subsystem;
      nt32Header->OptionalHeader.DllCharacteristics          = basedata->dllCharacteristics;
      nt32Header->OptionalHeader.SizeOfStackReserve          = (DWORD)basedata->sizeOfStackReserve;
      nt32Header->OptionalHeader.SizeOfStackCommit           = (DWORD)basedata->sizeOfStackCommit;
      nt32Header->OptionalHeader.SizeOfHeapReserve           = (DWORD)basedata->sizeOfHeapReserve;
      nt32Header->OptionalHeader.SizeOfHeapCommit            = (DWORD)basedata->sizeOfHeapCommit;
      nt32Header->OptionalHeader.LoaderFlags                 = 0;
      nt32Header->OptionalHeader.NumberOfRvaAndSizes         = IMAGE_NUMBEROF_DIRECTORY_ENTRIES;
    }
#endif
#if(PEIMAGE_64 > 0)
    else if(pedata->machine == IMAGE_FILE_MACHINE_AMD64)
    {
      IMAGE_NT_HEADERS64 *nt64Header = (IMAGE_NT_HEADERS64 *)ntHeader;

      nt64Header->OptionalHeader.Magic                       = IMAGE_NT_OPTIONAL_HDR64_MAGIC;
      nt64Header->OptionalHeader.MajorLinkerVersion          = basedata->majorLinkerVersion;
      nt64Header->OptionalHeader.MinorLinkerVersion          = basedata->minorLinkerVersion;
      nt64Header->OptionalHeader.SizeOfCode                  = 0;//*
      nt64Header->OptionalHeader.SizeOfInitializedData       = 0;//*
      nt64Header->OptionalHeader.SizeOfUninitializedData     = 0;//*
      nt64Header->OptionalHeader.AddressOfEntryPoint         = 0;//*
      nt64Header->OptionalHeader.BaseOfCode                  = 0;//*
      nt64Header->OptionalHeader.ImageBase                   = basedata->imageBase;
      nt64Header->OptionalHeader.SectionAlignment            = basedata->sectionAlignment;
      nt64Header->OptionalHeader.FileAlignment               = basedata->fileAlignment;
      nt64Header->OptionalHeader.MajorOperatingSystemVersion = basedata->majorOperatingSystemVersion;
      nt64Header->OptionalHeader.MinorOperatingSystemVersion = basedata->minorOperatingSystemVersion;
      nt64Header->OptionalHeader.MajorImageVersion           = basedata->majorImageVersion;
      nt64Header->OptionalHeader.MinorImageVersion           = basedata->minorImageVersion;
      nt64Header->OptionalHeader.MajorSubsystemVersion       = basedata->majorSubsystemVersion;
      nt64Header->OptionalHeader.MinorSubsystemVersion       = basedata->minorSubsystemVersion;
      nt64Header->OptionalHeader.Win32VersionValue           = 0;
      nt64Header->OptionalHeader.SizeOfImage                 = 0;//*
      nt64Header->OptionalHeader.SizeOfHeaders               = 0;//*
      nt64Header->OptionalHeader.CheckSum                    = 0;//*
      nt64Header->OptionalHeader.Subsystem                   = basedata->subsystem;
      nt64Header->OptionalHeader.DllCharacteristics          = basedata->dllCharacteristics;
      nt64Header->OptionalHeader.SizeOfStackReserve          = basedata->sizeOfStackReserve;
      nt64Header->OptionalHeader.SizeOfStackCommit           = basedata->sizeOfStackCommit;
      nt64Header->OptionalHeader.SizeOfHeapReserve           = basedata->sizeOfHeapReserve;
      nt64Header->OptionalHeader.SizeOfHeapCommit            = basedata->sizeOfHeapCommit;
      nt64Header->OptionalHeader.LoaderFlags                 = 0;
      nt64Header->OptionalHeader.NumberOfRvaAndSizes         = IMAGE_NUMBEROF_DIRECTORY_ENTRIES;
    }
#endif

    Mem::free(pedata->ntHeader.p32);
    pedata->ntHeader.p32 = ntHeader; 
    return true; 
  }
  return false;
}

bool PeImage::_setNtHeaderFromNtHeader(PEDATA *pedata, void *ntHeader)
{
#if(PEIMAGE_32 > 0)
  IMAGE_NT_HEADERS32 *nt32Header = (IMAGE_NT_HEADERS32 *)ntHeader;
#endif
#if(PEIMAGE_64 > 0)
  IMAGE_NT_HEADERS64 *nt64Header = (IMAGE_NT_HEADERS64 *)ntHeader;
#endif

  if(0
#if(PEIMAGE_32 > 0)
     || nt32Header->FileHeader.Machine == IMAGE_FILE_MACHINE_I386
#endif
#if(PEIMAGE_64 > 0)
     || nt64Header->FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64
#endif
    )
  {

    #if(PEIMAGE_32 > 0 && PEIMAGE_64 > 0)
    DWORD size = nt32Header->FileHeader.Machine == IMAGE_FILE_MACHINE_I386 ? sizeof(IMAGE_NT_HEADERS32) : sizeof(IMAGE_NT_HEADERS64);
    #elif(PEIMAGE_32 > 0)
    DWORD size = sizeof(IMAGE_NT_HEADERS32);
    #elif(PEIMAGE_64 > 0)
    DWORD size = sizeof(IMAGE_NT_HEADERS64);
    #endif
    
    void *p = Mem::alloc(size);
    if(p)
    {
      #if(PEIMAGE_32 > 0 && PEIMAGE_64 > 0)
      DWORD numberOfRvaAndSizes = nt32Header->FileHeader.Machine == IMAGE_FILE_MACHINE_I386 ? nt32Header->OptionalHeader.NumberOfRvaAndSizes : nt64Header->OptionalHeader.NumberOfRvaAndSizes;
      #elif(PEIMAGE_32 > 0)
      DWORD numberOfRvaAndSizes = nt32Header->OptionalHeader.NumberOfRvaAndSizes;
      #elif(PEIMAGE_64 > 0)
      DWORD numberOfRvaAndSizes = nt64Header->OptionalHeader.NumberOfRvaAndSizes;
      #endif
      
      size -= sizeof(IMAGE_DATA_DIRECTORY) * IMAGE_NUMBEROF_DIRECTORY_ENTRIES;
      size += sizeof(IMAGE_DATA_DIRECTORY) * numberOfRvaAndSizes;
      
      Mem::_copy(p, ntHeader, size);
      Mem::free(pedata->ntHeader.p32);
      pedata->ntHeader.p32 = (IMAGE_NT_HEADERS32 *)p; 

      //Заполняем dataDirectory.
      Mem::_zero(pedata->dataDirectory, sizeof(IMAGE_DATA_DIRECTORY) * IMAGE_NUMBEROF_DIRECTORY_ENTRIES);

      #if(PEIMAGE_32 > 0 && PEIMAGE_64 > 0)
      IMAGE_DATA_DIRECTORY *idd = (nt32Header->FileHeader.Machine == IMAGE_FILE_MACHINE_I386 ? nt32Header->OptionalHeader.DataDirectory : nt64Header->OptionalHeader.DataDirectory);
      #elif(PEIMAGE_32 > 0)
      IMAGE_DATA_DIRECTORY *idd = nt32Header->OptionalHeader.DataDirectory;
      #elif(PEIMAGE_64 > 0)
      IMAGE_DATA_DIRECTORY *idd = nt64Header->OptionalHeader.DataDirectory;
      #endif

      for(BYTE i = 0; i < numberOfRvaAndSizes; i++)_setDataDirectory(pedata, i, idd[i].VirtualAddress, idd[i].Size);
      return true;
    }
  }
  return false;
}

bool PeImage::_addSection(PEDATA *pedata, LPSTR name, DWORD characteristics, void *data, DWORD dataOffset, DWORD dataSize, DWORD virtualAddress, DWORD virtualDataSize, DWORD flags)
{
  DWORD count = pedata->sectionsCount + 1;
  if(pedata->sectionsCount < MAX_SECTIONS &&
     Mem::reallocEx(&pedata->sections,        count * sizeof(IMAGE_SECTION_HEADER)) &&
     Mem::reallocEx(&pedata->sectionsRawData, count * sizeof(LPBYTE)) &&
     Mem::reallocEx(&pedata->sectionsFlags,   count * sizeof(DWORD)))
  {
    LPBYTE raw = NULL;
    DWORD fileAlignment;
    
    if(dataSize > 0 && (raw = (LPBYTE)Mem::alloc(ALIGN_UP(dataSize, MAX_FILE_ALIGMENT))) == NULL)return false;

    IMAGE_SECTION_HEADER *section = &pedata->sections[pedata->sectionsCount];
    for(register BYTE g = 0; name[g] != 0 && g < IMAGE_SIZEOF_SHORT_NAME; g++)section->Name[g] = name[g];
      
    section->Misc.VirtualSize = virtualDataSize;
    section->VirtualAddress   = virtualAddress;
    section->SizeOfRawData    = dataSize;
    section->PointerToRawData = dataOffset;
    section->Characteristics  = characteristics;
      
    if(raw)
    {
      Mem::_copy(raw, data, dataSize);
      pedata->sectionsRawData[pedata->sectionsCount] = raw;  
    }

    pedata->sectionsFlags[pedata->sectionsCount] = flags;
    pedata->sectionsCount++;
    return true;
  }
  return false;
}

int PeImage::_getSectionIndexByName(PEDATA *pedata, LPSTR name)
{
  for(WORD i = 0; i < pedata->sectionsCount; i++)
  {
    BYTE g = 0;
    LPBYTE sectionName = pedata->sections[i].Name;
    for(; g < IMAGE_SIZEOF_SHORT_NAME; g++)
    {
      if(name[g] != sectionName[g])break;
      if(name[g] == 0)return i;
    }
    if(g == IMAGE_SIZEOF_SHORT_NAME && name[IMAGE_SIZEOF_SHORT_NAME + 1] == 0)return i;
  }
  return -1;
}

bool PeImage::_setDataDirectory(PEDATA *pedata, BYTE index, DWORD virtualAddress, DWORD size)
{
  if(index < IMAGE_NUMBEROF_DIRECTORY_ENTRIES)
  {
    pedata->dataDirectory[index].VirtualAddress = virtualAddress;
    pedata->dataDirectory[index].Size           = size;
    return true;
  }
  return false;
}

DWORD PeImage::_buildImage(PEDATA *pedata, DWORD flags, DWORD rvaOfEntryPoint, LPBYTE *output)
{
  if(output)*output = NULL;
  if(pedata->dosHeader == NULL || pedata->ntHeader.p32 == NULL)return 0;
   
  //Стартовые позиции.
  DWORD fileOffset = 0, virtualOffset = 0;
  DWORD fileAlignment, virtualAlignment;
  
  if(0){}
#if(PEIMAGE_32 > 0)
  else if(pedata->machine == IMAGE_FILE_MACHINE_I386)
  {
    fileAlignment    = pedata->ntHeader.p32->OptionalHeader.FileAlignment;
    virtualAlignment = pedata->ntHeader.p32->OptionalHeader.SectionAlignment;
  }
#endif
#if(PEIMAGE_64 > 0)
  else if(pedata->machine == IMAGE_FILE_MACHINE_AMD64)
  {
    fileAlignment    = pedata->ntHeader.p64->OptionalHeader.FileAlignment;
    virtualAlignment = pedata->ntHeader.p64->OptionalHeader.SectionAlignment;
  }
#endif

  //Выделяем память.
  LPBYTE image;
  if(output)
  {
    //Подсчитываем примерный размер файла.
    DWORD imageSize = pedata->dosHeader->e_lfanew + max(sizeof(IMAGE_NT_HEADERS32), sizeof(IMAGE_NT_HEADERS64)) + (sizeof(IMAGE_SECTION_HEADER) * pedata->sectionsCount);
    imageSize = ALIGN_UP(imageSize, fileAlignment);
    for(DWORD i = 0; i < pedata->sectionsCount; i++)imageSize += ALIGN_UP(pedata->sections[i].SizeOfRawData, fileAlignment);
    
    if((image = (LPBYTE)Mem::alloc(imageSize + pedata->minimalRawOffsetOfSection)) == NULL)return 0;
  }
    
  //DOS-заголовок.
  if(output)Mem::_copy(image + fileOffset, pedata->dosHeader, pedata->dosHeader->e_lfanew);
  fileOffset    += pedata->dosHeader->e_lfanew;
  virtualOffset += pedata->dosHeader->e_lfanew;
  
  //NT-заголовок.
  LPBYTE newNtHeader = (output ? (LPBYTE)(image + fileOffset) : (LPBYTE)pedata->ntHeader.p32);
  
  if(0){}
#if(PEIMAGE_32 > 0)
  else if(pedata->machine == IMAGE_FILE_MACHINE_I386)
  {
    if(output)Mem::_copy(newNtHeader, pedata->ntHeader.p32, sizeof(IMAGE_NT_HEADERS32));
    fileOffset    += sizeof(IMAGE_NT_HEADERS32);
    virtualOffset += sizeof(IMAGE_NT_HEADERS32);
  }
#endif
#if(PEIMAGE_64 > 0)
  else if(pedata->machine == IMAGE_FILE_MACHINE_AMD64)
  {
    if(output)Mem::_copy(newNtHeader, pedata->ntHeader.p64, sizeof(IMAGE_NT_HEADERS64));
    fileOffset    += sizeof(IMAGE_NT_HEADERS64);
    virtualOffset += sizeof(IMAGE_NT_HEADERS64);
  }
#endif  
  
  //Счетчики.
  DWORD sizeOfCode        = 0;
  DWORD initializedData   = 0;
  DWORD uninitializedData = 0;
  
  DWORD baseOfCode        = 0;
  DWORD baseOfData        = 0;
  DWORD sizeOfHeaders     = 0;
  
  //Cекций.
  {
    IMAGE_SECTION_HEADER *sections = (IMAGE_SECTION_HEADER *)(image + fileOffset);

    fileOffset += sizeof(IMAGE_SECTION_HEADER) * pedata->sectionsCount;
    fileOffset  = ALIGN_UP(fileOffset, fileAlignment);

    virtualOffset += sizeof(IMAGE_SECTION_HEADER) * pedata->sectionsCount;
    virtualOffset  = ALIGN_UP(virtualOffset, virtualAlignment);
    
    //Выравниваем до минимальной позици секции.
    if(fileOffset < pedata->minimalRawOffsetOfSection)fileOffset = pedata->minimalRawOffsetOfSection;

    sizeOfHeaders = fileOffset;

    for(DWORD i = 0; i < pedata->sectionsCount; i++)
    {
      DWORD realVirtualSize;
      IMAGE_SECTION_HEADER *ps;
      
      if(output)
      {
        ps = &sections[i];
        Mem::_copy(ps, &pedata->sections[i], sizeof(IMAGE_SECTION_HEADER));
      }
      else
      {
        ps = &pedata->sections[i];
      }
      
      if(ps->Misc.VirtualSize == 0)realVirtualSize = ps->SizeOfRawData;
      else realVirtualSize = max(ps->Misc.VirtualSize, ps->SizeOfRawData);
        
      if((flags & BIF_NO_RECALC_RVA) == 0)ps->VirtualAddress = virtualOffset;
      
      if(ps->SizeOfRawData > 0)
      {
        ps->SizeOfRawData    = ALIGN_UP(ps->SizeOfRawData, fileAlignment);
        ps->PointerToRawData = fileOffset;
      }

      if(output)Mem::_copy(image + fileOffset, pedata->sectionsRawData[i], pedata->sections[i].SizeOfRawData);
      
      if(ps->Characteristics & IMAGE_SCN_CNT_CODE)
      {
        if((flags & BIF_NO_RECALC_RVA) == 0 && baseOfCode == 0)baseOfCode = virtualOffset;
        sizeOfCode += ps->SizeOfRawData;//ALIGN_UP(realVirtualSize, fileAlignment);
      }
      if(ps->Characteristics & IMAGE_SCN_CNT_INITIALIZED_DATA)
      {
        if((flags & BIF_NO_RECALC_RVA) == 0 && baseOfData == 0 && !(ps->Characteristics & IMAGE_SCN_CNT_CODE))baseOfData = virtualOffset;
        initializedData += ALIGN_UP(realVirtualSize, fileAlignment);//ps->SizeOfRawData
      }
      if(ps->Characteristics & IMAGE_SCN_CNT_UNINITIALIZED_DATA)
      {
        uninitializedData += ALIGN_UP(realVirtualSize, fileAlignment);//ps->SizeOfRawData
      }
      
      //Подсчитываем новые позиции.
      fileOffset += ps->SizeOfRawData;
      if((flags & BIF_NO_RECALC_RVA) == 0)virtualOffset += ALIGN_UP(realVirtualSize, virtualAlignment);
    }
  }

  //Обновляем NT-заголовок.
  ((IMAGE_NT_HEADERS32 * )newNtHeader)->FileHeader.NumberOfSections = pedata->sectionsCount;
  bool setChecksum = (output && (flags & BIF_CHECKSUM)) ? true : false;

  if(0){}
#if(PEIMAGE_32 > 0)
  else if(pedata->machine == IMAGE_FILE_MACHINE_I386)
  {
    IMAGE_NT_HEADERS32 *nt32Header = (IMAGE_NT_HEADERS32 *)newNtHeader;

    if(output)Mem::_copy(nt32Header->OptionalHeader.DataDirectory, pedata->dataDirectory, sizeof(IMAGE_DATA_DIRECTORY) * IMAGE_NUMBEROF_DIRECTORY_ENTRIES);
    
    nt32Header->OptionalHeader.AddressOfEntryPoint       = rvaOfEntryPoint == 0 ? pedata->ntHeader.p32->OptionalHeader.AddressOfEntryPoint : rvaOfEntryPoint ;
    if((flags & BIF_NO_RECALC_RVA) == 0)
    {
      nt32Header->OptionalHeader.SizeOfCode              = sizeOfCode;
      nt32Header->OptionalHeader.SizeOfInitializedData   = initializedData;
      nt32Header->OptionalHeader.SizeOfUninitializedData = uninitializedData;
      nt32Header->OptionalHeader.BaseOfCode              = baseOfCode;
      nt32Header->OptionalHeader.BaseOfData              = baseOfData;
      nt32Header->OptionalHeader.SizeOfImage             = virtualOffset;
    }
    else
    {
      nt32Header->OptionalHeader.SizeOfCode              = pedata->ntHeader.p32->OptionalHeader.SizeOfCode;
      nt32Header->OptionalHeader.SizeOfInitializedData   = pedata->ntHeader.p32->OptionalHeader.SizeOfInitializedData;
      nt32Header->OptionalHeader.SizeOfUninitializedData = pedata->ntHeader.p32->OptionalHeader.SizeOfUninitializedData;
      nt32Header->OptionalHeader.BaseOfCode              = pedata->ntHeader.p32->OptionalHeader.BaseOfCode;
      nt32Header->OptionalHeader.BaseOfData              = pedata->ntHeader.p32->OptionalHeader.BaseOfData;
      nt32Header->OptionalHeader.SizeOfImage             = pedata->ntHeader.p32->OptionalHeader.SizeOfImage;
    }
    nt32Header->OptionalHeader.SizeOfHeaders             = sizeOfHeaders;
    nt32Header->OptionalHeader.CheckSum                  = setChecksum ? createNtCheckSum(image, fileOffset) : 0;
  }
#endif
#if(PEIMAGE_64 > 0)
  else if(pedata->machine == IMAGE_FILE_MACHINE_AMD64)
  {
    IMAGE_NT_HEADERS64 *nt64Header = (IMAGE_NT_HEADERS64 *)newNtHeader;

    if(output)Mem::_copy(nt64Header->OptionalHeader.DataDirectory, pedata->dataDirectory, sizeof(IMAGE_DATA_DIRECTORY) * IMAGE_NUMBEROF_DIRECTORY_ENTRIES);
    
    nt64Header->OptionalHeader.AddressOfEntryPoint       = rvaOfEntryPoint == 0 ? pedata->ntHeader.p64->OptionalHeader.AddressOfEntryPoint : rvaOfEntryPoint ;
    if((flags & BIF_NO_RECALC_RVA) == 0)
    {
      nt64Header->OptionalHeader.SizeOfCode              = sizeOfCode;
      nt64Header->OptionalHeader.SizeOfInitializedData   = initializedData;
      nt64Header->OptionalHeader.SizeOfUninitializedData = uninitializedData;
      nt64Header->OptionalHeader.BaseOfCode              = baseOfCode;
      //nt64Header->OptionalHeader.BaseOfData              = baseOfData;
      nt64Header->OptionalHeader.SizeOfImage             = virtualOffset;
    }
    else
    {
      nt64Header->OptionalHeader.SizeOfCode              = pedata->ntHeader.p64->OptionalHeader.SizeOfCode;
      nt64Header->OptionalHeader.SizeOfInitializedData   = pedata->ntHeader.p64->OptionalHeader.SizeOfInitializedData;
      nt64Header->OptionalHeader.SizeOfUninitializedData = pedata->ntHeader.p64->OptionalHeader.SizeOfUninitializedData;
      nt64Header->OptionalHeader.BaseOfCode              = pedata->ntHeader.p64->OptionalHeader.BaseOfCode;
      //nt64Header->OptionalHeader.BaseOfData              = pedata->ntHeader.p64->OptionalHeader.BaseOfData;
      nt64Header->OptionalHeader.SizeOfImage             = pedata->ntHeader.p64->OptionalHeader.SizeOfImage;
    }
    nt64Header->OptionalHeader.SizeOfHeaders             = sizeOfHeaders;
    nt64Header->OptionalHeader.CheckSum                  = setChecksum ? createNtCheckSum(image, fileOffset) : 0;
  }
#endif

  if(output)*output = image;
  return fileOffset;
}

DWORD PeImage::_calcNextSectionRva(PEDATA *pedata, DWORD sectionsCount)
{
  if(pedata->dosHeader == NULL || pedata->ntHeader.p32 == NULL || sectionsCount > MAX_SECTIONS)return 0;  
  
#if(PEIMAGE_32 > 0 && PEIMAGE_64 > 0)
  DWORD alignment = pedata->machine == IMAGE_FILE_MACHINE_I386 ? pedata->ntHeader.p32->OptionalHeader.SectionAlignment : pedata->ntHeader.p64->OptionalHeader.SectionAlignment;
#elif(PEIMAGE_32 > 0)
  DWORD alignment = pedata->ntHeader.p32->OptionalHeader.SectionAlignment;
#elif(PEIMAGE_64 > 0)
  DWORD alignment = pedata->ntHeader.p64->OptionalHeader.SectionAlignment;
#endif
  
  DWORD rva = pedata->dosHeader->e_lfanew +
    (
      #if(PEIMAGE_32 > 0 && PEIMAGE_64 > 0)
      pedata->machine == IMAGE_FILE_MACHINE_I386 ? sizeof(IMAGE_NT_HEADERS32) : sizeof(IMAGE_NT_HEADERS64)
      #elif(PEIMAGE_32 > 0)
      sizeof(IMAGE_NT_HEADERS32)
      #elif(PEIMAGE_64 > 0)
      sizeof(IMAGE_NT_HEADERS64)
      #endif
    );

  rva += sectionsCount * sizeof(IMAGE_SECTION_HEADER);
  rva  = ALIGN_UP(rva, alignment);
  
  for(DWORD i = 0; i < pedata->sectionsCount; i++)rva += ALIGN_UP(pedata->sections[i].Misc.VirtualSize, alignment);
  
  return rva;
}

DWORD PeImage::_getCurrentRawSize(PEDATA *pedata)
{
  if(pedata->dosHeader == NULL || pedata->ntHeader.p32 == NULL)return 0;

  DWORD size = pedata->dosHeader->e_lfanew;
  
  #if(PEIMAGE_32 > 0 && PEIMAGE_64 > 0)
  DWORD alignment = (pedata->machine == IMAGE_FILE_MACHINE_I386 ? pedata->ntHeader.p32->OptionalHeader.FileAlignment : pedata->ntHeader.p64->OptionalHeader.FileAlignment);
  #elif(PEIMAGE_32 > 0)
  DWORD alignment = pedata->ntHeader.p32->OptionalHeader.FileAlignment;
  #elif(PEIMAGE_64 > 0)
  DWORD alignment = pedata->ntHeader.p64->OptionalHeader.FileAlignment;
  #endif
  
  if(0){}
# if(PEIMAGE_32 > 0)
  else if(pedata->machine == IMAGE_FILE_MACHINE_I386)size += sizeof(IMAGE_NT_HEADERS32);
# endif
# if(PEIMAGE_64 > 0)
  else if(pedata->machine == IMAGE_FILE_MACHINE_AMD64)size += sizeof(IMAGE_NT_HEADERS64);
# endif  
  
  size += pedata->sectionsCount * sizeof(IMAGE_SECTION_HEADER);
  size = ALIGN_UP(size, alignment);

  //Выравниваем до минимальной позици секции.
  if(size < pedata->minimalRawOffsetOfSection)size = pedata->minimalRawOffsetOfSection;
  
  //Секции.
  for(DWORD i = 0; i < pedata->sectionsCount; i++)size += ALIGN_UP(pedata->sections[i].SizeOfRawData, alignment);

  return size;
}

DWORD PeImage::_getRawSize(const void *image)
{
  LPBYTE p = (LPBYTE)image;
  IMAGE_NT_HEADERS32 *ntHeader = (IMAGE_NT_HEADERS32 *)(p + ((PIMAGE_DOS_HEADER)p)->e_lfanew);

  IMAGE_SECTION_HEADER *sections   = (IMAGE_SECTION_HEADER *)((LPBYTE)(&ntHeader->OptionalHeader) + ntHeader->FileHeader.SizeOfOptionalHeader);
  IMAGE_SECTION_HEADER *maxSection = NULL;
  
  for(WORD i = 0; i < ntHeader->FileHeader.NumberOfSections; i++)
  {
    if(maxSection == NULL || sections[i].PointerToRawData > maxSection->PointerToRawData)maxSection = &sections[i];
  }

  return maxSection == NULL ? 0 : maxSection->PointerToRawData + maxSection->SizeOfRawData;
}

DWORD PeImage::_getCurrentVirtualSize(PEDATA *pedata)
{
  if(pedata->dosHeader == NULL || pedata->ntHeader.p32 == NULL)return 0;

  DWORD size = pedata->dosHeader->e_lfanew;

#if(PEIMAGE_32 > 0 && PEIMAGE_64 > 0)
  DWORD alignment = (pedata->machine == IMAGE_FILE_MACHINE_I386 ? pedata->ntHeader.p32->OptionalHeader.SectionAlignment : pedata->ntHeader.p64->OptionalHeader.SectionAlignment);
  size += (pedata->machine == IMAGE_FILE_MACHINE_I386 ? sizeof(IMAGE_NT_HEADERS32) : sizeof(IMAGE_NT_HEADERS64));
#elif(PEIMAGE_32 > 0)
  DWORD alignment = pedata->ntHeader.p32->OptionalHeader.SectionAlignment;
  size += sizeof(IMAGE_NT_HEADERS32);
#elif(PEIMAGE_64 > 0)
  DWORD alignment = pedata->ntHeader.p64->OptionalHeader.SectionAlignment;
  size += sizeof(IMAGE_NT_HEADERS64);
#endif  

  size += pedata->sectionsCount * sizeof(IMAGE_SECTION_HEADER);
  size = ALIGN_UP(size, alignment);
  
  for(DWORD i = 0; i < pedata->sectionsCount; i++)size += ALIGN_UP(pedata->sections[i].Misc.VirtualSize, alignment);
  return size;
}

void *PeImage::_getSectionOfModule(HMODULE module, WORD index, LPDWORD virtualSize)
{
  void *r = NULL;
  LPBYTE p = (LPBYTE)module;

#if defined _WIN64
  IMAGE_NT_HEADERS64 *ntHeader = (IMAGE_NT_HEADERS64 *)(p + ((PIMAGE_DOS_HEADER)p)->e_lfanew);
#else
  IMAGE_NT_HEADERS32 *ntHeader = (IMAGE_NT_HEADERS32 *)(p + ((PIMAGE_DOS_HEADER)p)->e_lfanew);
#endif

  IMAGE_SECTION_HEADER *sections = (IMAGE_SECTION_HEADER *)((LPBYTE)(&ntHeader->OptionalHeader) + ntHeader->FileHeader.SizeOfOptionalHeader);
  
  if(ntHeader->FileHeader.NumberOfSections > 0 && index < ntHeader->FileHeader.NumberOfSections)
  {
    IMAGE_SECTION_HEADER *cur = &sections[index];
    
    r = (void *)(p + cur->VirtualAddress);
    
    if(CWA(kernel32, IsBadReadPtr(r, cur->Misc.VirtualSize)) != FALSE)
    {
      r = NULL;
    }
    else if(virtualSize)
    {
      *virtualSize = cur->Misc.VirtualSize;
    }
  }
  
  return r;
}

IMAGE_SECTION_HEADER *PeImage::_getSectionByName(const void *image, LPSTR const sectionName)
{
  void *r = NULL;
  IMAGE_SECTION_HEADER *sections;
  WORD headerType = IMAGE_NT_HEADERS_TYPE(image);
  WORD numberOfSections;

  if(0){}
#if(PEIMAGE_32 > 0)
  else if(headerType == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
  {
    IMAGE_NT_HEADERS32 *ntHeader = (IMAGE_NT_HEADERS32 *)IMAGE_GET_NT_HEADERS(image);
    sections         = (IMAGE_SECTION_HEADER *)((LPBYTE)(&ntHeader->OptionalHeader) + ntHeader->FileHeader.SizeOfOptionalHeader);
    numberOfSections = ntHeader->FileHeader.NumberOfSections;
  }
#elif(PEIMAGE_64 > 0)
  else if(headerType == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
  {
    IMAGE_NT_HEADERS64 *ntHeader = (IMAGE_NT_HEADERS64 *)IMAGE_GET_NT_HEADERS(image);
    sections         = (IMAGE_SECTION_HEADER *)((LPBYTE)(&ntHeader->OptionalHeader) + ntHeader->FileHeader.SizeOfOptionalHeader);
    numberOfSections = ntHeader->FileHeader.NumberOfSections;
  }
#endif
  else return NULL;
  
  int nameLen = Str::_LengthA(sectionName);
  for(WORD i = 0; i < numberOfSections; i++)
  {
    IMAGE_SECTION_HEADER *cur = &sections[i];
    if(Str::_CompareA((LPSTR)cur->Name, sectionName, Mem::_getL(cur->Name, 0, IMAGE_SIZEOF_SHORT_NAME) == NULL ? IMAGE_SIZEOF_SHORT_NAME : -1, nameLen) == 0)return cur;
  }
  return NULL;
}

void *PeImage::_copyModuleToProcess(HANDLE process, void *image)
{
#if defined _WIN64
  PIMAGE_NT_HEADERS64 ntHeader = (PIMAGE_NT_HEADERS)((LPBYTE)image + ((PIMAGE_DOS_HEADER)image)->e_lfanew);
#else
  PIMAGE_NT_HEADERS32 ntHeader = (PIMAGE_NT_HEADERS)((LPBYTE)image + ((PIMAGE_DOS_HEADER)image)->e_lfanew);
#endif
  
  DWORD imageSize = ntHeader->OptionalHeader.SizeOfImage;
  bool ok         = false;

  if(CWA(kernel32, IsBadReadPtr)(image, imageSize) != 0)return NULL;
  
  //Выделние памяти для модуля.
  LPBYTE remoteMem = (LPBYTE)CWA(kernel32, VirtualAllocEx)(process, NULL, imageSize, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
  if(remoteMem != NULL)
  {
    //Создаем локальный буфер, в котором будем вносить измненеия.
    LPBYTE buf = (LPBYTE)Mem::copyEx(image, imageSize);
    if(buf != NULL)
    {
      //Изменяем релоки.
      IMAGE_DATA_DIRECTORY *relocsDir = &ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
      
      if(relocsDir->Size > 0 && relocsDir->VirtualAddress > 0)
      {
        DWORD_PTR delta               = (DWORD_PTR)((LPBYTE)remoteMem - ntHeader->OptionalHeader.ImageBase);
        DWORD_PTR oldDelta            = (DWORD_PTR)((LPBYTE)image - ntHeader->OptionalHeader.ImageBase);
        IMAGE_BASE_RELOCATION *relHdr = (IMAGE_BASE_RELOCATION *)(buf + relocsDir->VirtualAddress);
      
        while(relHdr->VirtualAddress != 0)
        {
          if(relHdr->SizeOfBlock >= sizeof(IMAGE_BASE_RELOCATION))//FIXME: Что это?
          {
            DWORD relCount = (relHdr->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
            LPWORD relList = (LPWORD)((LPBYTE)relHdr + sizeof(IMAGE_BASE_RELOCATION));
            
            for(DWORD i = 0; i < relCount; i++)if(relList[i] > 0)
            {
              DWORD_PTR *p = (DWORD_PTR *)(buf + (relHdr->VirtualAddress + (0x0FFF & (relList[i]))));
              *p -= oldDelta;
              *p += delta;
            }
          }
          
          relHdr = (IMAGE_BASE_RELOCATION *)((LPBYTE)relHdr + relHdr->SizeOfBlock);
        }
      
        //Копируем образ в процесс.
        ok = CWA(kernel32, WriteProcessMemory)(process, remoteMem, buf, imageSize, NULL) ? true : false;
      }
      
      Mem::free(buf);
    }
    
    if(!ok)
    {
      CWA(kernel32, VirtualFreeEx)(process, (void *)remoteMem, 0, MEM_RELEASE);
      remoteMem = NULL;
    }
  }

  return remoteMem;
}

typedef HMODULE (WINAPI *liLoadLibraryA)(LPSTR);
typedef void *(WINAPI *liGetProcAddress)(HMODULE, LPSTR);
bool PeImage::_loadImport(void *image, void *loadLibraryA, void *getProcAddress)
{
#if defined _WIN64
  PIMAGE_NT_HEADERS64 ntHeader = (PIMAGE_NT_HEADERS64)((LPBYTE)image + ((PIMAGE_DOS_HEADER)image)->e_lfanew);
#else
  PIMAGE_NT_HEADERS32 ntHeader = (PIMAGE_NT_HEADERS32)((LPBYTE)image + ((PIMAGE_DOS_HEADER)image)->e_lfanew);
#endif
  
  IMAGE_DATA_DIRECTORY *importDir = &ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
  if(loadLibraryA && getProcAddress && importDir->VirtualAddress > 0 && importDir->Size > sizeof(IMAGE_IMPORT_DESCRIPTOR))
  {
    for(IMAGE_IMPORT_DESCRIPTOR *iid = (IMAGE_IMPORT_DESCRIPTOR *)((LPBYTE)image + importDir->VirtualAddress); iid->Characteristics != 0; iid++)
    {
      //Загружаем DLL.
      HMODULE dll = (((liLoadLibraryA)loadLibraryA)((LPSTR)((LPBYTE)image + iid->Name)));
      if(dll == NULL)return false;
    
#     if defined _WIN64
      IMAGE_THUNK_DATA64 *originalThunk = (IMAGE_THUNK_DATA64 *)((LPBYTE)image + iid->OriginalFirstThunk);
      IMAGE_THUNK_DATA64 *thunk         = (IMAGE_THUNK_DATA64 *)((LPBYTE)image + iid->FirstThunk);
#     else
      IMAGE_THUNK_DATA32 *originalThunk = (IMAGE_THUNK_DATA32 *)((LPBYTE)image + iid->OriginalFirstThunk);
      IMAGE_THUNK_DATA32 *thunk         = (IMAGE_THUNK_DATA32 *)((LPBYTE)image + iid->FirstThunk);
#     endif

      for(; originalThunk->u1.Function != 0; originalThunk++, thunk++)
      {
        //Получаем имя функции.
        LPSTR name;
        
        #if defined _WIN64
        if(originalThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG64)name = (LPSTR)IMAGE_ORDINAL64(originalThunk->u1.Ordinal);
        #else
        if(originalThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG32)name = (LPSTR)IMAGE_ORDINAL32(originalThunk->u1.Ordinal);
        #endif
        else
        {
          IMAGE_IMPORT_BY_NAME *iin = (IMAGE_IMPORT_BY_NAME *)((LPBYTE)image + originalThunk->u1.AddressOfData);
          name = (LPSTR)(iin->Name);
        }

        //Получаем адрес.
        DWORD_PTR addr = (DWORD_PTR)(((liGetProcAddress)getProcAddress)(dll, name));
        if(addr == NULL)return false;
        thunk->u1.Function = addr;
      }
    }
  }
  
  return true;
}

bool PeImage::_repalceImportFunction(void *image, const void *oldFunction, const void *newFunction)
{
  bool ret = false;
#if defined _WIN64
  PIMAGE_NT_HEADERS64 ntHeader = (PIMAGE_NT_HEADERS64)((LPBYTE)image + ((PIMAGE_DOS_HEADER)image)->e_lfanew);
#else
  PIMAGE_NT_HEADERS32 ntHeader = (PIMAGE_NT_HEADERS32)((LPBYTE)image + ((PIMAGE_DOS_HEADER)image)->e_lfanew);
#endif

  IMAGE_DATA_DIRECTORY *importDir = &ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];

  if(importDir->VirtualAddress > 0 && importDir->Size > sizeof(IMAGE_IMPORT_DESCRIPTOR))
  {
    for(IMAGE_IMPORT_DESCRIPTOR *iid = (IMAGE_IMPORT_DESCRIPTOR *)((LPBYTE)image + importDir->VirtualAddress); iid->Characteristics != 0; iid++)
    {
#     if defined _WIN64
      IMAGE_THUNK_DATA64 *originalThunk = (IMAGE_THUNK_DATA64 *)((LPBYTE)image + iid->OriginalFirstThunk);
      IMAGE_THUNK_DATA64 *thunk         = (IMAGE_THUNK_DATA64 *)((LPBYTE)image + iid->FirstThunk);
#     else
      IMAGE_THUNK_DATA32 *originalThunk = (IMAGE_THUNK_DATA32 *)((LPBYTE)image + iid->OriginalFirstThunk);
      IMAGE_THUNK_DATA32 *thunk         = (IMAGE_THUNK_DATA32 *)((LPBYTE)image + iid->FirstThunk);
#     endif

      for(; originalThunk->u1.Function != 0; originalThunk++, thunk++)if((DWORD_PTR)oldFunction == thunk->u1.Function)
      {
        thunk->u1.Function = (DWORD_PTR)newFunction;
        ret = true;
      }
    }
  }

  return ret;
}

DWORD PeImage::_rvaToRsa(PEDATA *pedata, DWORD rva, LPWORD sectionIndex)
{
  //Ищим секцию которой принадлежит адрес.
  DWORD ret = 0;
  for(WORD i = 0; i < pedata->ntHeader.p32->FileHeader.NumberOfSections; i++)
  {
    DWORD a = pedata->sections[i].VirtualAddress;
    if(rva >= a && rva < a + pedata->sections[i].Misc.VirtualSize)
    {
      rva -= a; //Адрес относительно секции.
      if(/*pedata->sections[i].PointerToRawData > 0 && */rva < pedata->sections[i].SizeOfRawData)
      {
        ret = rva;
        if(sectionIndex)*sectionIndex = i;
      }
      break;
    }
  }

  return ret;
}

bool PeImage::_isPeImage(void *mem, DWORD memSize)
{
  if(mem == NULL || memSize <= sizeof(IMAGE_NT_HEADERS32) + sizeof(IMAGE_DOS_HEADER))return false;

  LPBYTE offset               = (LPBYTE)mem;
  LPBYTE end                  = offset + memSize;
  IMAGE_DOS_HEADER *dosHeader = (IMAGE_DOS_HEADER *)mem;
# if(PEIMAGE_32 > 0)
  IMAGE_NT_HEADERS32 *nt32Header = NULL;
# endif
# if(PEIMAGE_64 > 0)
  IMAGE_NT_HEADERS64 *nt64Header = NULL;
# endif

  if(dosHeader->e_magic != IMAGE_DOS_SIGNATURE || dosHeader->e_lfanew < sizeof(WORD) || dosHeader->e_lfanew >= memSize - sizeof(IMAGE_NT_HEADERS32))return false;
  offset += dosHeader->e_lfanew;

  //Получаем NT-заголовок.
  //Здесь проверяются отсновные параметры для определения 32/64 и предотврашения "buffer overflow".
  //Делать более жесткую проверку не вижу смысла.
  if(((IMAGE_NT_HEADERS32 *)offset)->Signature != IMAGE_NT_SIGNATURE)return false;

  IMAGE_NT_HEADERS32 *t = (IMAGE_NT_HEADERS32 *)offset;

  //Размер заголовков.
  if(t->FileHeader.SizeOfOptionalHeader >= (DWORD)(end - (offset + sizeof(IMAGE_FILE_HEADER) + sizeof(DWORD))))return false;

  DWORD fileAligment;
  DWORD virtualAligment;
  switch(t->FileHeader.Machine)
  {
#   if(PEIMAGE_32 > 0)
    case IMAGE_FILE_MACHINE_I386:
      nt32Header = (IMAGE_NT_HEADERS32 *)t;
      if(/*dosHeader->e_lfanew >= memSize - sizeof(IMAGE_NT_HEADERS32) || Проверка сушествляется выше.*/nt32Header->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC)return false;
      fileAligment    = nt32Header->OptionalHeader.FileAlignment;
      virtualAligment = nt32Header->OptionalHeader.SectionAlignment;
      break;
#   endif

#   if(PEIMAGE_64 > 0)
    case IMAGE_FILE_MACHINE_AMD64:
      nt64Header = (IMAGE_NT_HEADERS64 *)t;
      if(dosHeader->e_lfanew >= memSize - sizeof(IMAGE_NT_HEADERS64) || nt64Header->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC)return false;
      fileAligment    = nt64Header->OptionalHeader.FileAlignment;
      virtualAligment = nt64Header->OptionalHeader.SectionAlignment;
      break;
#   endif
    
    default:
      return false;
  }

  //Это проверка требуется для защиты от переполенния буфера входе работы с секциями.
  if(fileAligment > MAX_FILE_ALIGMENT || fileAligment < MIN_FILE_ALIGMENT ||
     virtualAligment > MAX_VIRTUAL_ALIGMENT || virtualAligment < MIN_VIRTUAL_ALIGMENT ||
     virtualAligment < fileAligment ||
     fileAligment % 2 != 0 || virtualAligment % 2 != 0)return false;

  return true;
}
