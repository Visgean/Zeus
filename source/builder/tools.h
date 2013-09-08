/*
  ��������� �������������.
*/
#pragma once

/*
  ����� ��������� � multilne edit control.

  IN hwnd   - ����� edit-����.
  IN format - ������ ��������� �������� sprintf.
  IN ...    - ��������� ���������.
*/
void writeOutput(HWND hwnd, LPWSTR format, ...);

/*
  ����� ������ � multilne edit control.

  IN hwnd   - ����� edit-����.
  IN format - ������ ��������� �������� sprintf.
  IN ...    - ��������� ���������.
*/
void writeOutputError(HWND hwnd, LPWSTR format, ...);

/*  Checks whether the stream ended.

  IN OUT subThread - pointer to hegdl flow. If, returned with the handle == NULL, then the flow ended.*/
void closeThreadIfFinsinhed(HANDLE *subThread);

