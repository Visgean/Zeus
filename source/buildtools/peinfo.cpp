/*
  Получение информации из PE-файла.
*/
#include <windows.h>

#include "defines.h"
#include "core.h"
#include "language.h"

#include "..\common\console.h"
#include "..\common\cui.h"
#include "..\common\fs.h"
#include "..\common\peimage.h"
#include "..\common\str.h"

void commandPeInfo(LPWSTR *switches, DWORD switchesCount)
{
  LPWSTR file = Cui::_getSwitchValue(switches, switchesCount, lng_switch_file);
  if(file == NULL || file == (LPWSTR)1)
  {
    Console::writeStringW(lng_peinfo_no_pefile, sizeof(lng_peinfo_no_pefile) / sizeof(WCHAR) - 1);
    return;
  }

  //Загружаем файл.
  PeImage::PEDATA peData;
  {
    Fs::MEMFILE mf;
    if(!Fs::_fileToMem(file, &mf, 0))
    {
      coreData.exitCode = Cui::EXITCODE_ERROR_FILE_NOT_FOUND;
      Console::writeFormatW(lng_error_fopen, file);
      return;
    }

    void *ok = PeImage::_createFromMemory(&peData, mf.data, mf.size, false);
    Fs::_closeMemFile(&mf);
    
    if(ok == NULL)
    {
      coreData.exitCode = Cui::EXITCODE_ERROR_INPUT_FILE_CORRUPTED;
      Console::writeStringW(lng_peinfo_pefile_corrupted, sizeof(lng_peinfo_pefile_corrupted) / sizeof(WCHAR) - 1);
      return;
    }
  }
  
  DWORD_PTR imageBase = 0;
  if(Cui::_getSwitchValue(switches, switchesCount, lng_switch_as_va) != NULL)
  {
    imageBase = peData.machine == IMAGE_FILE_MACHINE_I386 ? peData.ntHeader.p32->OptionalHeader.ImageBase : peData.ntHeader.p64->OptionalHeader.ImageBase;
  }
  
  //Поиск в DataDirectory.
  LPWSTR s;
  if((s = Cui::_getSwitchValue(switches, switchesCount, lng_switch_datadirectory)) != NULL && s != (LPWSTR)1)
  {
    DWORD index  = Str::_ToInt32W(s, NULL);
    DWORD_PTR ddRva  = 0;
    DWORD_PTR ddSize = 0;

    if(index < IMAGE_NUMBEROF_DIRECTORY_ENTRIES)
    {
      ddRva  = imageBase + peData.dataDirectory[index].VirtualAddress;
      ddSize = peData.dataDirectory[index].Size;
    }

    //Вывод.
    if(Cui::_getSwitchValue(switches, switchesCount, lng_switch_nologo) != NULL)
    {
      Console::writeFormatW(imageBase == 0 ? L"0x%08X 0x%08X" : L"0x%p 0x%08X", ddRva, ddSize);
    }
    else
    {
      Console::writeFormatW(imageBase == 0 ?
                            L"IMAGE_OPTIONAL_HEADER::DataDirectory[%u].VirtualAddress = 0x%08X\n"
                            L"IMAGE_OPTIONAL_HEADER::DataDirectory[%u].Size           = 0x%08X"
                            :                    
                            L"IMAGE_OPTIONAL_HEADER::DataDirectory[%u].VirtualAddress = 0x%p\n"
                            L"IMAGE_OPTIONAL_HEADER::DataDirectory[%u].Size           = 0x%08X",
                            index, ddRva,
                            index, ddSize);
    }
    coreData.exitCode = Cui::EXITCODE_SUCCESSED;
  }
  else if((s = Cui::_getSwitchValue(switches, switchesCount, lng_switch_patch_dw)) != NULL && s != (LPWSTR)1)
  {
    LPWSTR separator = Str::_findCharW(s, ',');
    if(separator == NULL)
    {
      Console::writeStringW(lng_peinfo_invalid_rva, sizeof(lng_peinfo_invalid_rva) / sizeof(WCHAR) - 1);      
    }
    else
    {
      //Получаем RVA и DWORD.
      *separator = 0;
      DWORD_PTR rva  = Str::_ToInt32W(s, NULL) - imageBase;
      DWORD dword = (DWORD)Str::_ToInt32W(separator + 1, NULL);
      
      //Ищим секцию.    
      WORD i = 0;
      for(; i < peData.sectionsCount; i++)
      {   
        IMAGE_SECTION_HEADER *section = &peData.sections[i];
        if(rva >= section->VirtualAddress && section->SizeOfRawData >= sizeof(DWORD) && rva <= section->VirtualAddress + section->SizeOfRawData - sizeof(DWORD))
        {
          //Выситываем файловый offset.
          rva = section->PointerToRawData + (rva - section->VirtualAddress); 
        
          //Вносим изменения.
          HANDLE fileHandle = CWA(kernel32, CreateFileW)(file, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
          DWORD writed;
          if(fileHandle == INVALID_HANDLE_VALUE || !Fs::_setFilePointer(fileHandle, rva, FILE_BEGIN) ||
             CWA(kernel32, WriteFile)(fileHandle, &dword, sizeof(DWORD), &writed, NULL) == FALSE || writed != sizeof(DWORD))
          {
            Console::writeFormatW(lng_error_fwrite, file);
            coreData.exitCode = Cui::EXITCODE_ERROR_WRITE_OUTPUT_FILE;
          }
          else
          {
            Console::writeStringW(lng_peinfo_patched, sizeof(lng_peinfo_patched) / sizeof(WCHAR) - 1);
            coreData.exitCode = Cui::EXITCODE_SUCCESSED;
          }

          if(file != INVALID_HANDLE_VALUE)CWA(kernel32, CloseHandle)(fileHandle);        
          break;
        }
      }

      //Секция не найдена.
      if(i == peData.sectionsCount)
      {
        Console::writeStringW(lng_peinfo_not_patched, sizeof(lng_peinfo_not_patched) / sizeof(WCHAR) - 1);
        coreData.exitCode = Cui::EXITCODE_ERROR_INPUT_FILE_CORRUPTED;
      }
    }
  }

  PeImage::_freeImage(&peData);
}
