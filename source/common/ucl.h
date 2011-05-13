#pragma once
/*
  Модифицированый UCL 1.03.
  http://www.oberhumer.com/opensource/ucl/
*/

#if !defined XLIB_UCL_ONLY_MAX_COMPRESSION
  #error XLIB_UCL_ONLY_MAX_COMPRESSION not defined!
#endif

#if !defined XLIB_UCL_ENABLE_NRV2B
  #error XLIB_UCL_ENABLE_NRV2B not defined!
#endif

#if !defined XLIB_UCL_ENABLE_NRV2D
  #error XLIB_UCL_ONLY_MAX_COMPRESSION not defined!
#endif

#if !defined XLIB_UCL_ENABLE_NRV2E
  #error XLIB_UCL_ONLY_MAX_COMPRESSION not defined!
#endif

namespace UCL
{
  //Флаги для Compress.
  enum
  {
    //Тип сжатия.
#if(XLIB_UCL_ENABLE_NRV2B > 0)
    CF_NRV2B     = 0x00000001,
#endif
#if(XLIB_UCL_ENABLE_NRV2D > 0)
    CF_NRV2D     = 0x00000002,
#endif
#if(XLIB_UCL_ENABLE_NRV2E > 0)
    CF_NRV2E     = 0x00000004,
#endif

    //Уровень сжатия.
#if(XLIB_UCL_ONLY_MAX_COMPRESSION > 0)
#else
    CF_LEVEL_1   = 0x00000008,
    CF_LEVEL_2   = 0x00000010,
    CF_LEVEL_3   = 0x00000020,
    CF_LEVEL_4   = 0x00000040,
    CF_LEVEL_5   = 0x00000080,
    CF_LEVEL_6   = 0x00000100,
    CF_LEVEL_7   = 0x00000200,
    CF_LEVEL_8   = 0x00000400,
    CF_LEVEL_9   = 0x00000800,
    CF_LEVEL_MIN = CF_LEVEL_1,
#endif
    CF_LEVEL_10  = 0x00001000,
    CF_LEVEL_MAX = CF_LEVEL_10,
  };
  
  //Коды ошибок.
  enum
  {
    E_SUCCESSED,        //Успешное выполнение.
    E_ERROR,            //Неизвесная ошибка.
    E_INVALID_ARGUMENT, //Неверные аргументы.
    E_OUT_OF_MEMORY,    //Не доcтаточно памяти.
    E_OUT_OF_BUFFER     //Не доcтаточно буфера для сжатых данных.
  };

  //Функция для отображения состояния процесса работы.
  typedef struct
  {
    /*
      IN dwTextSize - FIXME: переменная не известна. text size counter.
      IN dwCodeSize - FIXME: переменная не известна. code size counter.
      IN iStatus - состояние работы.
                   -1 - подготовка.
                    3 - прогресс.
                    4 - успешно завершено.
      IN pData - пользовательские данные.
    */
    void (WINAPI *callback)(DWORD dwTextSize, DWORD dwCodeSize, int iStatus, void *pData);
    void *pData; //Пользовательские данные. передаваемые в callback.
  }PROGRESS_CALLBACK;

  /*
    Инициализация.
  */
  void Init(void);

  /*
    Деинициализация.
  */
  void Uninit(void);

  /*
    Сжатие данных.

    IN pSource           - исходные данные.
    IN dwSourceSize      - размер исходных данных в байтах.
    OUT pBuffer          - буфер для сжатых данных.
    IN OUT pdwBufferSize - на входе - максимальный размер pBuffer, рекомендуется расчитывать по
                           формуле dwSourceSize + dwSourceSize / 8.
                           на выходе - размер сжатых данных.
    IN pCallback         - функция для получения состояния.
    IN dwFlags           - флаги CF_*.

    Return               - код ошибки E_*.
  */
  int _Compress(LPBYTE pSource, DWORD dwSourceSize, LPBYTE pBuffer, LPDWORD pdwBufferSize, PROGRESS_CALLBACK *pCallback, DWORD dwFlags);

  /*
    Распаковка данных.

    IN pSource      - исходные сжатые данные.
    IN dwSourceSize - размер исходных сжатыx данных в байтах.
    IN pDest        - буфер для распакованных данных.
    IN pdwDestSize  - на входе - максимальный размер pBuffer.
                      на выходе - размер данных.

    Return          - true - в случаи успешной распкаковки,
                      false - в случаи оишибка (повреждение данных).

    Примечание: Функиции не помеченые постфиксом Safe, могу вызвать исключение, но при этом имеют
                меньший размер.
  */
  bool _DecompressNRV2BSafe(LPBYTE pSource, DWORD dwSourceSize, LPBYTE pDest, LPDWORD );
  bool _DecompressNRV2B(LPBYTE pSource, DWORD dwSourceSize, LPBYTE pDest, LPDWORD pdwDestSize);
  bool _DecompressNRV2DSafe(LPBYTE pSource, DWORD dwSourceSize, LPBYTE pDest, LPDWORD pdwDestSize);
  bool _DecompressNRV2D(LPBYTE pSource, DWORD dwSourceSize, LPBYTE pDest, LPDWORD pdwDestSize);
  bool _DecompressNRV2ESafe(LPBYTE pSource, DWORD dwSourceSize, LPBYTE pDest, LPDWORD pdwDestSize);
  bool _DecompressNRV2E(LPBYTE pSource, DWORD dwSourceSize, LPBYTE pDest, LPDWORD pdwDestSize);
};
