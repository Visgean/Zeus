/*
  Различные инструмененты.
*/
#pragma once

/*
  Вывод сообшения в multilne edit control.

  IN hwnd   - хэндл edit-окна.
  IN format - формат сообшение согласно sprintf.
  IN ...    - аргументы сообщения.
*/
void writeOutput(HWND hwnd, LPWSTR format, ...);

/*
  Вывод ошибки в multilne edit control.

  IN hwnd   - хэндл edit-окна.
  IN format - формат сообшение согласно sprintf.
  IN ...    - аргументы сообщения.
*/
void writeOutputError(HWND hwnd, LPWSTR format, ...);

/*
  Проверяет завершился ли поток.

  IN OUT subThread - указатель на хэгдл потока. Если при возрате хэндл == NULL, то поток завершился.
*/
void closeThreadIfFinsinhed(HANDLE *subThread);

