/*
  ������ � ��������.
*/
#pragma once

namespace Console
{
  /*
    �������������.

    Return - true - � ������ ������,
             false - � ������ ������.
  */
  bool init(void);

  /*
    ���������������.
  */
  void uninit(void);

  /*
    ������ ������ � �������.

    IN data - ������ ��� ������.
    IN size - ���������� ���� ��� ������.

    Return  - ���������� ���������� ����.
  */
  DWORD writeData(void *data, DWORD size);

  /*
    ������ Unicode ������ � �������.

    IN string - ������ ��� ������.
    IN size   - ���������� �������� ��� ������, ��� (DWORD)-1 ���� ������ ������������ �� \0.

    Return    - ���������� ���������� ��������.
  */
  DWORD writeStringW(LPWSTR string, DWORD size);

  /*    Analogue of wprintf.

    IN format - line-format output.
    IN ... - Arguments for the row.

    Return - number of characters written.
  */
  DWORD writeFormatW(LPWSTR format, ...);
};
