#include <windows.h>

#include "defines.h"
#include "windowstation.h"
#include "cryptedstrings.h"

#include "..\common\mem.h"

#if(BO_VNC > 0)
void WindowStation::init(void)
{

}

void WindowStation::uninit(void)
{

}

LPWSTR WindowStation::_getObjectName(HANDLE handle)
{
  DWORD size = 0;
  WCHAR tempChar;
  LPWSTR string = NULL;

  if(handle != NULL && CWA(user32, GetUserObjectInformationW)(handle, UOI_NAME, &tempChar, 0, &size) == FALSE && size > 0)
  {
    if((string = (LPWSTR)Mem::alloc(size + sizeof(WCHAR))))
    {
      DWORD size2;
      if(CWA(user32, GetUserObjectInformationW)(handle, UOI_NAME, string, size, &size2) == FALSE)
      {
        Mem::free(string);
        string = NULL;
      }
    }
  }

  return string;
}

bool WindowStation::_isEqualObjects(HANDLE handle1, HANDLE handle2)
{
  bool ok = false;
  
  LPWSTR string1 = _getObjectName(handle1);
  if(string1 != NULL)
  {
    LPWSTR string2 = _getObjectName(handle2);  
    if(string2 != NULL)
    {
      if(CWA(kernel32, lstrcmpiW)(string1, string2) == 0)ok = true;
      Mem::free(string2);
    }
    Mem::free(string1);
  }

  return ok;
}

HWINSTA WindowStation::_openWindowStation(const LPWSTR name, ACCESS_MASK desiredAccess)
{
  LPWSTR realName = name;
  WCHAR defaultName[CryptedStrings::len_windowstation_winstation_0];
  if(realName == NULL)
  {
    realName = defaultName;
    CryptedStrings::_getW(CryptedStrings::id_windowstation_winstation_0, defaultName);
  }

  HWINSTA handle = CWA(user32, OpenWindowStationW)(realName, FALSE, desiredAccess);
  if(handle == NULL)handle = CWA(user32, CreateWindowStationW)(realName, 0, desiredAccess, NULL);
  return handle;
}

bool WindowStation::_setProcessWindowStation(HWINSTA handle)
{
  return (_isEqualObjects(CWA(user32, GetProcessWindowStation)(), handle) || CWA(user32, SetProcessWindowStation)(handle) != FALSE);
}

HDESK WindowStation::_openDesktop(const LPWSTR name, ACCESS_MASK desiredAccess)
{
  LPWSTR realName = name;
  WCHAR defaultName[CryptedStrings::len_windowstation_desktop_default];
  if(realName == NULL)
  {
    realName = defaultName;
    CryptedStrings::_getW(CryptedStrings::id_windowstation_desktop_default, defaultName);
  }

  HDESK handle = CWA(user32, OpenDesktopW)(realName, 0, FALSE, desiredAccess);
  if(handle == NULL)handle = CWA(user32, CreateDesktopW)(realName, NULL, NULL, 0, desiredAccess, NULL);

  return handle;
}

bool WindowStation::_setThreadDesktop(HDESK handle)
{
  HDESK currentHandle = CWA(user32, GetThreadDesktop)(CWA(kernel32, GetCurrentThreadId()));
  return (_isEqualObjects(currentHandle, handle) || CWA(user32, SetThreadDesktop)(handle) != FALSE);
}

bool WindowStation::_setThreadDesktopEx(const LPWSTR stationName, const LPWSTR desktopName)
{
  bool ok = false;
  HWINSTA stationHandle = _openWindowStation(stationName, GENERIC_ALL);
  if(stationHandle != NULL)
  {
    HWINSTA currentStationHandle = CWA(user32, GetProcessWindowStation)();
    if(_setProcessWindowStation(stationHandle))
    {
      HDESK desktopHandle = _openDesktop(desktopName, GENERIC_ALL);
      if(desktopHandle != NULL)
      {
        ok = _setThreadDesktop(desktopHandle);
        CWA(user32, CloseDesktop)(desktopHandle);
      }
      if(ok == false && currentStationHandle != NULL)_setProcessWindowStation(currentStationHandle);
    }
    CWA(user32, CloseWindowStation)(stationHandle);
  }

  return ok;
}
#endif
