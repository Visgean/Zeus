#include <windows.h>

#include "gdi.h"

void Gdi::init(void)
{

}

void Gdi::uninit(void)
{

}

bool Gdi::_saveBitmapToFile(const LPWSTR fileName, HDC dc, HBITMAP bitmapHandle)
{
  BITMAPINFO *bitmapInfo;
  {
    BITMAP bitmap; 
    if(CWA(gdi32, GetObject)(bitmapHandle, sizeof(BITMAP), &bitmap) != sizeof(BITMAP))return false;

    //Узнаем кол. бит на цвет.
    WORD colorBits = (WORD)(bitmap.bmPlanes * bitmap.bmBitsPixel); 
    if(colorBits == 1)colorBits = 1; 
    else if (colorBits <= 4)colorBits = 4; 
    else if (colorBits <= 8)colorBits = 8; 
    else if (colorBits <= 16)colorBits = 16; 
    else if (colorBits <= 24)colorBits = 24; 
    else colorBits = 32; 

    //Заполняем BITMAPINFO. 
    if((bitmapInfo = (BITMAPINFO *)Mem::alloc(sizeof(BITMAPINFOHEADER) + (colorBits == 24 ? 0 : sizeof(RGBQUAD) * (1 << colorBits)))) == NULL)return false;
    bitmapInfo->bmiHeader.biSize          = sizeof(BITMAPINFOHEADER); 
    bitmapInfo->bmiHeader.biWidth         = bitmap.bmWidth; 
    bitmapInfo->bmiHeader.biHeight        = bitmap.bmHeight; 
    bitmapInfo->bmiHeader.biPlanes        = bitmap.bmPlanes; 
    bitmapInfo->bmiHeader.biBitCount      = bitmap.bmBitsPixel; 
    bitmapInfo->bmiHeader.biCompression   = BI_RGB; 
    bitmapInfo->bmiHeader.biSizeImage     = ((bitmapInfo->bmiHeader.biWidth * colorBits + 31) & ~31) / 8 * bitmapInfo->bmiHeader.biHeight; 
    bitmapInfo->bmiHeader.biXPelsPerMeter = 0;
    bitmapInfo->bmiHeader.biYPelsPerMeter = 0;
    bitmapInfo->bmiHeader.biClrUsed       = colorBits < 24 ? (1 << colorBits) : 0; 
    bitmapInfo->bmiHeader.biClrImportant  = 0; 
  }

  bool retVal = false;
  LPBYTE bitmapBits = (LPBYTE)Mem::alloc(bitmapInfo->bmiHeader.biSizeImage);
  if(bitmapBits != NULL && CWA(kernel32, GetDIBits)(dc, bitmapHandle, 0, bitmapInfo->bmiHeader.biHeight, bitmapBits, bitmapInfo, DIB_RGB_COLORS) > 0)
  {
    DWORD bitmapInfoSize = sizeof(BITMAPINFOHEADER) + bitmapInfo->bmiHeader.biClrUsed * sizeof(RGBQUAD);

    //Заголовок битмапа.
    BITMAPFILEHEADER fileHeader;
    fileHeader.bfType      = 0x4D42; //BM
    fileHeader.bfOffBits   = sizeof(BITMAPFILEHEADER) + bitmapInfoSize; 
    fileHeader.bfSize      = fileHeader.bfOffBits + bitmapInfo->bmiHeader.biSizeImage; 
    fileHeader.bfReserved1 = 0; 
    fileHeader.bfReserved2 = 0; 

    //Пишим в файл.
    HANDLE fileHandle = CWA(kernel32, CreateFileW)(fileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL); 
    if(fileHandle != INVALID_HANDLE_VALUE)
    {
      DWORD size;
      if(CWA(kernel32, WriteFile)(fileHandle, &fileHeader, sizeof(BITMAPFILEHEADER), &size, NULL) != FALSE &&
         CWA(kernel32, WriteFile)(fileHandle, &bitmapInfo->bmiHeader, bitmapInfoSize, &size, NULL) != FALSE &&
         CWA(kernel32, WriteFile)(fileHandle, bitmapBits, bitmapInfo->bmiHeader.biSizeImage, &size, NULL) != FALSE
        )
      {
        retVal = true;
      }
      CWA(kernel32, CloseHandle)(fileHandle);
      if(retVal == false)Fs::_removeFile(fileName);
    }
  }

  Mem::free(bitmapBits);
  Mem::free(bitmapInfo);

  return retVal;
}

HBITMAP Gdi::_createDibSection(HDC dc, int width, int height, BITMAPINFO **bitmapInfo, void **bitmapBits, HANDLE fileMap, DWORD fileMapOffset)
{
  HBITMAP tempBitmap;
  HBITMAP bitmap = NULL;

  if((tempBitmap = CWA(gdi32, CreateCompatibleBitmap)(dc, width, height)) != NULL)
  {
    BITMAPINFO *bmi = (BITMAPINFO *)Mem::alloc(sizeof(BITMAPINFO) + sizeof(RGBQUAD) * 255);
    if(bmi != NULL)
    {
      bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
      //bmi->bmiHeader.biBitCount = 0;

      if(CWA(gdi32, GetDIBits)(dc, tempBitmap, 0, 1, NULL, bmi, DIB_RGB_COLORS) != 0 && CWA(gdi32, GetDIBits)(dc, tempBitmap, 0, 1, NULL, bmi, DIB_RGB_COLORS) != 0)
      {
        CWA(gdi32, DeleteObject)(tempBitmap);
        tempBitmap = NULL;

        //Top-down сканирование.
        bmi->bmiHeader.biHeight = -Math::_abs(bmi->bmiHeader.biHeight);

        //Конвертируем в форматы, которые мы можем отображать (8/16/32).
        switch(bmi->bmiHeader.biBitCount)
        {
          case 1:
          case 4:
            bmi->bmiHeader.biClrUsed      = 0;
            bmi->bmiHeader.biBitCount     = 8;
            bmi->bmiHeader.biClrImportant = 0;
            break;	

          case 24:
            bmi->bmiHeader.biBitCount = 32;
            break;
        }
        bmi->bmiHeader.biSizeImage   = Math::_abs((bmi->bmiHeader.biWidth * bmi->bmiHeader.biHeight * bmi->bmiHeader.biBitCount) / 8);
        bmi->bmiHeader.biCompression = BI_RGB; //Убираем сжатие.

        if(bitmapInfo)*bitmapInfo = bmi;

        bitmap = CWA(gdi32, CreateDIBSection)(dc, bmi, DIB_RGB_COLORS, bitmapBits, fileMap, fileMapOffset);
      }    

      if(bitmap == NULL || bitmapInfo == NULL)Mem::free(bmi);
    }

    if(tempBitmap != NULL)CWA(gdi32, DeleteObject)(tempBitmap);
  }

  return bitmap;
}
