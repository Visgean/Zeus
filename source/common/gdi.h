/*
  Надстрйока над GDI/GDI+
*/
#pragma once

namespace Gdi
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
    Сохранение HBITMAP в файл.

    IN fileName     - файл.
    IN dc           - DC.
    IN bitmapHandle - HBITMAP.

    Return          - true - в случаи успеха,
                      false - в случаи провала.
  */
  bool _saveBitmapToFile(const LPWSTR fileName, HDC dc, HBITMAP bitmapHandle);

  /*
    Обертка для CreateDIBSection, котрая позволяет более просто создать DIBSection.

    IN dc            - DC.
    IN width         - ширина битмапа.
    IN height        - высота битмапа.
    OUT bitmapInfo   - информация о битмапе, должна быть освобождена через Mem. Может быть NULL.
    OUT bitmapBits   - адрес данных битмапа.
    IN fileMap       - объект Map-файл, или NULL.
    IN fileMapOffset - оффсет в fileMap.

    Return           - битмап, или NULL.

    Примечание: Битмап всегда будет в формате BI_RGB в формате 8/16/32.
  */
  HBITMAP _createDibSection(HDC dc, int width, int height, BITMAPINFO **bitmapInfo, void **bitmapBits, HANDLE fileMap, DWORD fileMapOffset);
};
