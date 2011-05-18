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

  /*В В В В Wrapper for CreateDIBSection, Kotra can more easily create DIBSection.

В В В В IN dc - DC.
В В В В IN width - the width of the bitmap.
В В В В IN height - the height of the bitmap.
В В В В OUT bitmapInfo - information about the bitmap, must be cleared through the Mem. May be NULL.
В В В В OUT bitmapBits - data address bitmap.
В В В В IN fileMap - Map-file object, or NULL.
В В В В IN fileMapOffset - offset in fileMap.

В В В В Return - the bitmap, or NULL.

В В В В Note: The bitmap will always be in the format BI_RGB format 8/16/32.
В В */
  HBITMAP _createDibSection(HDC dc, int width, int height, BITMAPINFO **bitmapInfo, void **bitmapBits, HANDLE fileMap, DWORD fileMapOffset);
};
