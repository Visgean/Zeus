/*
  ����� ������� ��� ������ �� �������� � ������� Unix.
*/
#pragma once

namespace Time
{
  /*
    �������������.
  */
  void init(void);

  /*
    ���������������.
  */
  void uninit(void);

  /*
    ��������� �������� �������.

    Return - ������� �����.
  */
  DWORD _getTime(void);

  /*
    ��������� �������� ������� GMT.

    Return - ������� �����.
  */
  DWORD _getLocalTime(void);

  /*
    ��������� �������� GMT.

    Return - ������� GMT � ��������.
  */
  int _getLocalGmt(void);

  /*
    ����������� FILETIME � Unix �����.

    IN ft  - ��������� �� FILETIME.

    Return - Unix �����.
  */
  DWORD _fileTimeToTime(const FILETIME *ft);

  /*
    ����������� Unix ������� � FILETIME.

    IN time - �����.
    OUT ft  - ��������� �� FILETIME.

    Return  - Unix �����.
  */
  void _timeToFileTime(const DWORD time, FILETIME *ft);
  
  /*    Converting SYSTEMTIME to Unix time.

    IN st - a pointer to the SYSTEMTIME.

    Return - Unix time.
  */
  DWORD _systemTimeToTime(const SYSTEMTIME *st);
};
