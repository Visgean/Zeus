#include <windows.h>

#include "math.h"
#include "time.h"

void Time::init(void)
{

}

void Time::uninit(void)
{

}

DWORD Time::_getTime(void)
{
  SYSTEMTIME st;
  CWA(kernel32, GetSystemTime)(&st);
  return _systemTimeToTime(&st);
}

DWORD Time::_getLocalTime(void)
{
  return _getTime() + _getLocalGmt();
}

int Time::_getLocalGmt(void)
{
  TIME_ZONE_INFORMATION tzi;
  int d = (int)CWA(kernel32, GetTimeZoneInformation)(&tzi);

  if(d == TIME_ZONE_ID_STANDARD)d = tzi.StandardBias;
  else if(d == TIME_ZONE_ID_DAYLIGHT)d = tzi.DaylightBias;
  else return 0;

  return (tzi.Bias + d) * (-60);
}

DWORD Time::_fileTimeToTime(const FILETIME *ft)
{
  //WARN:look FILETIME for more information.
  register DWORD64 tim = (DWORD64)(*((DWORD64 *)ft) - 116444736000000000i64);
#if defined _WIN64
  tim /= 10000000;
#else
  tim = Math::_divU64(tim, 10000000);
#endif
  
  return (DWORD)tim;
}

void Time::_timeToFileTime(const DWORD time, FILETIME *ft)
{
  register DWORD64 tim = 116444736000000000i64;
#if defined _WIN64
  tim += time * 10000000;
#else
  tim += Math::_mul64(time, 10000000);
#endif

  //WARN:look FILETIME for more information.
  *((DWORD64 *)ft) = tim;
}

DWORD Time::_systemTimeToTime(const SYSTEMTIME *st)
{
  FILETIME ft;
  CWA(kernel32, SystemTimeToFileTime)((SYSTEMTIME *)st, &ft);
  return _fileTimeToTime(&ft);
}
