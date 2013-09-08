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

/*В В Checks whether the stream ended.

В В IN OUT subThread - pointer to hegdl flow. If, returned with the handle == NULL, then the flow ended.*/
void closeThreadIfFinsinhed(HANDLE *subThread);

