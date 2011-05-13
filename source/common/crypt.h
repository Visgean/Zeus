#pragma once
/*
  Криптография.
*/

//Размер MD5 хэша.
#define MD5HASH_SIZE 16

namespace Crypt
{
  //Данные для RC4.
  typedef struct
  {      
    BYTE state[256];       
    BYTE x;        
    BYTE y;
  }RC4KEY;
  
  /*
    Инициализация.
  */
  void init(void);

  /*
    Деинициализация.
  */
  void uninit(void);

  /*
    Создание MD5 хэша данных.

    OUT output   - буффер для хэша, размером MD5HASH_SIZE.
    IN inputData - данные, хэш которых нужно подсчитать.
    IN dataSize  - количество байт в pInputData.

    Return       - true - в случаи успеха,
                   false - в случаи ошибки.
  */
  bool _md5Hash(LPBYTE output, void *inputData, DWORD dataSize);

  /*
    Псевдо-случайный генератор чисел по меоду Mersenne Twister.

    Return - число от 0 до 0x7FFFFFFF.
  */
  DWORD mtRand(void);

  /*
    Расширение MTRand, позволяющее указать промежуток.

    IN minVal - минимальное число.
    IN maxVal - максимальное число.
              
    Return    - число от dwMin до dwMax.
  */
  DWORD mtRandRange(DWORD minVal, DWORD maxVal);

  /*
    Подсчет CRC32.

    IN data - данные, хэш которых нужно подсчитать.
    IN size - количество байт в pMem.

    Return  - хэш.

    Примечание: при первом вызове функция создает таблицу crc32, процесс создания которой не
                защещен от многопоточности.
  */
  DWORD crc32Hash(const void *data, DWORD size);

  /*
    Инициализация RC4 для текушего ключа.

    IN key        - входной бинарный ключ.
    IN binKeySize - размер бинарного ключа в байтах.
    OUT key       - данные для RC4.
  */
  void _rc4Init(const void *binKey, WORD binKeySize, RC4KEY *key);
  
  /*
    Шифрование/декодирование RC4.

    IN OUT buffer - данные для обработки.
    IN size       - размер данных.
    IN key        - стурктура, инициализированная через RC4Init.
  */
  void _rc4(void *buffer, DWORD size, RC4KEY *key);

  /*
    Объединение rc4Init() и rc4() в одну функцию.

    IN binKey     - входной бинарный ключ.
    IN binKeySize - размер бинарного ключа в байтах.
    IN OUT buffer - данные для обработки.
    IN size       - размер данных.
  */
  void _rc4Full(const void *binKey, WORD binKeySize, void *buffer, DWORD size);

  /*
    Визуальное шифрование буфера. Ключем являестя первый байт.

    IN OUT buffer - данные для обработки.
    IN size       - размер данных.
  */
  void _visualEncrypt(void *buffer, DWORD size);

  /*
    Визуальное дешифрование буфера. Ключем являестя первый байт.

    IN OUT buffer - данные для обработки.
    IN size       - размер данных.
  */
  void _visualDecrypt(void *buffer, DWORD size);

  /*
    Кодирование данных в base64.

    IN source     - данные для обработки.
    IN sourceSize - размер данных.
    OUT destSize  - размер полученой base64 строки без нулевого символа. Может быть NULL.

    Return        - строка с нулевым символом на конце, или NULL в случаи ошибки. Строка должна
                    быть уничтожена через Mem.
  */
  LPSTR _base64Encode(LPBYTE source, SIZE_T sourceSize, SIZE_T *destSize);

  /*
    Декодирование данных из base64.
    
    IN source     - строка для обработки.
    IN sourceSize - размер строки.
    OUT destSize  - размер полученой данных без нулевого символа. Может быть NULL.

    Return        - данные с нулевым символом на конце, или NULL в случаи ошибки. Строка должна
                    быть уничтожена через Mem.
  */
  LPBYTE _base64Decode(LPSTR source, SIZE_T sourceSize, SIZE_T *destSize);

  /*
    Генерация данных заданной длины.

    OUT buffer  - буфер для данных.
    IN size     - количество байт для генеарции.
    IN minValue - минимальное значение байта.
    IN maxValue - максимальное значение байта.
    OUT extMode - true - использовать расишреный режим, улучшает алгоритм, но замедляется на
                  ~(dwSize * 20) ms.
                  false - стандартный режим.
  */
  void _generateBinaryData(void *buffer, DWORD size, BYTE minValue, BYTE maxValue, bool extMode);

  /*
    Генерация RC4 ключа.

    OUT key - ключ.
  */
  void _generateRc4Key(Crypt::RC4KEY *key);
};
