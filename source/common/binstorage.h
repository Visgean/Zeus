/*
  Работа с бинарным хранилищем.

  Примичание: Совпадение ID опции дает wID1 == wID2 и ITEMF_IS_* == ITEMF_IS_*.
*/
#pragma once

#include "crypt.h"

#define BINSTORAGE_MAX_SIZE (10 * 1024 * 1024) //WININET_MAXBYTES_TO_MEM

namespace BinStorage
{
  //Флаги для ITEM.
  enum
  {
    ITEMF_COMPRESSED        = 0x00000001, //Данные сжаты.
    
    //Данные флаги служат для указания режима перезаписи.
    ITEMF_COMBINE_ADD       = 0x00010000, //Режим добавления, если опция сущетвует, запись не происходит.
    ITEMF_COMBINE_OVERWRITE = 0x00020000, //Режим перезаписи, если опция не сущетвует, создается новая запись.
    ITEMF_COMBINE_REPLACE   = 0x00040000, //Режим замены,если опция не сущетвует, запись не происходит.
    ITEMF_COMBINE_DELETE    = 0x00080000, //Режим удаления предыдушего элемента и текушего.
    ITEMF_COMBINE_MASK      = ITEMF_COMBINE_ADD | ITEMF_COMBINE_OVERWRITE | ITEMF_COMBINE_REPLACE | ITEMF_COMBINE_DELETE,

    //Данные флаги введены для уменьшения риска совпадения dwID.
    ITEMF_IS_OPTION         = 0x10000000, //Элемент являеся опцией.
    ITEMF_IS_SETTING        = 0x20000000, //Элемент является настройкой.
    ITEMF_IS_HTTP_INJECT    = 0x40000000, //Элемент являеся http-инжектом.
    ITEMF_IS_MASK           = ITEMF_IS_OPTION | ITEMF_IS_SETTING | ITEMF_IS_HTTP_INJECT
  };

  #pragma pack(push, 1)
  //Заголовок.
  typedef struct
  {
    BYTE randData[20];                //Произволные данные.
    DWORD size;                       //Полный размер хранилища.
    DWORD flags;                      //Флаги.
    DWORD count;                      //Количетсво опции.
    BYTE md5Hash[16/*MD5HASH_SIZE*/]; //MD5 хэш от p + sizeof(CONFIGHEADER) до p + dwSize.
  }STORAGE;
  
  //Элемент.
  typedef struct
  {
    DWORD id;       //ID опции, обычно CRC32.
    DWORD flags;    //Флаги.
    DWORD size;     //Размер сжатых данных если установлен флаг ITEMF_COMPRESSED. Иначе равно realSize.
    DWORD realSize; //Реальный размер данных.
  }ITEM;
  #pragma pack(pop)

  //Флаги для _openStorageFile().
  enum
  {
    OSF_WRITE_ACCESS = 0x1 //Открыть сущетвующий файл для записи, или создать новый в случаи
                           //отсутвия файла.
  };
  
  //Данные о хранилиши.
  typedef struct
  {
    HANDLE file;     //OUT Хэндл файла.
    DWORD64 curItem; //OUT позиция текущего элемента.
    DWORD xorKey;    //IN XOR маска шифрования.
  }STORAGEARRAY;
  
  //Флаги для _Pack.
  enum
  {
    PACKF_FINAL_MODE = 0x1 //Режим финализации, используется для сборки конечной конфигурации,
                           //приминяет флаги ITEMF_COMBINE_*.
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
    Проверяет хэш хранилища.

    IN binStorage - хранилище

    Return - true - в случаи совпадения хэша,
             false - в случаи ошибки.
  */
  bool _checkHash(STORAGE *binStorage);
  
  /*
    Создание пустого хранилища.

    Return - хранилище (должен быть закрыт через Mem),
             или NULL в случаи ошибки.
  */
  STORAGE *_createEmpty(void);
  
  /*
    Добавление элемента в хранилище.

    IN OUT binStorage - хранилище.
    IN id             - ID новой опции.
    IN flags          - флаги ITEMF_*.
    IN data           - данные опции.
    IN dataSize       - размер данных.

    Return            - true, в случаи успешного добавления,
                        false, в случаи ошибки. 
    
    Примичание: Наличие опции до добавление не проверяется.
  */
  bool _addItem(STORAGE **binStorage, DWORD id, DWORD flags, void *data, DWORD dataSize);
  
  /*
    Добавление Unicode-строки в хранилище как UTF-8 строки.

    IN OUT binStorage - хранилище.
    IN id             - ID новой опции.
    IN flags          - флаги ITEMF_*.
    IN string         - строка для добавления.

    Return            - true, в случаи успешного добавления,
                        false, в случаи ошибки.     
  */
  bool _addItemAsUtf8StringW(STORAGE **binStorage, DWORD id, DWORD flags, LPWSTR string);
  
  /*
    Добавление ANSI-строки в хранилище как UTF-8 строки.

    IN OUT binStorage - хранилище.
    IN id             - ID нового элемента.
    IN flags          - флаги ITEMF_*.
    IN string         - строка для добавления.

    Return            - true, в случаи успешного добавления,
                        false, в случаи ошибки.     
  */
  bool _addItemAsUtf8StringA(STORAGE **binStorage, DWORD id, DWORD flags, LPSTR string);

  /*
    Модификация элемента.

    IN OUT binStorage - хранилище.
    IN item           - элемент для модификации (после модификации заданый адрес может быть не
                        действтелен).
    IN flags          - флаги ITEMF_*.
    IN data           - данные опции.
    IN dataSize       - размер данных.

    Return            - true, в случаи успешного добавления,
                        false, в случаи ошибки. 
  */
  bool _modifyItem(STORAGE **binStorage, ITEM *item, DWORD flags, void *data, DWORD dataSize);
  
  /*
    Модификация элемента.

    IN OUT binStorage - хранилище.
    IN id             - элемент для модификации.
    IN flags          - флаги ITEMF_*.
    IN data           - данные опции.
    IN dataSize       - размер данных.

    Return            - true, в случаи успешного добавления,
                        false, в случаи ошибки. 
  */
  bool _modifyItemById(STORAGE **binStorage, DWORD id, DWORD flags, void *data, DWORD dataSize);
  
  /*
    Получение следующего элемента из хранилищя.

    IN binStorage - хранилище.
    IN curItem    - текущий элемент, или NULL для получения первого элемента.

    Return        - следующий элемент относительно curItem, или NULL если элементов больше не найдено.
  */
  ITEM *_getNextItem(const STORAGE *binStorage, ITEM *curItem);

  /*
    Получение элемента из хранилищя.

    IN binStorage - хранилище.
    IN id         - ID элемента.
    IN isFlag     - один из флагов ITEMF_IS_* для определения типа элемента.

    Return        - найденый элемент, или NULL если элемент не найден.
  */
  ITEM *_getItem(const STORAGE *binStorage, DWORD id, DWORD isFlag);
  
  /*
    Получение содержимого элемента из хранилищя.

    IN pi  - элемента.

    Return - данные элемента(должны быть освобождены через Mem), или NULL если данных нет.

    Примечание: для выделяемых данных в конец всегда добавляется дополнительный нулевой WCHAR-символ,
                что позволяет безопасно работать со строками.
  */
  void *_getItemData(const ITEM *item);

  /*
    Получение содержимого элемента из хранилища. Оболочка для _GetItem и _GetItemData.

    IN binStorage - хранилище.
    IN id         - ID элемента.
    IN isFlag     - один из флагов ITEMF_IS_* для определения типа элемента.
    OUT size      - размер данных. Может быть NULL.

    Return        - данные элемента(должны быть освобождены через Mem), или NULL если данных нет.
  */
  void *_getItemDataEx(const STORAGE *binStorage, DWORD id, DWORD isFlag, LPDWORD size);

  /*
    Получение содержимого элемента из хранилища как DWORD.

    IN binStorage - хранилище.
    IN id         - ID элемента.
    IN isFlag     - один из флагов ITEMF_IS_* для определения типа элемента.
    OUT data      - данные. Может быть NULL, если значение не нужно.

    Return        - true - в случаи успеха,
                    false - в случаи ошибки.
  */
  bool _getItemDataAsDword(const STORAGE *binStorage, DWORD id, DWORD isFlag, LPDWORD data);
  
  /*
    Упаковка хранилища.

    IN OUT binStorage - на входе хранилище для обработки, на выходе обработаное хранилище,
                        которое уже не пригодно для использования. Для повторного использования
                        нужно вызывать _unpack().
    IN flags          - режим финализации, используется для сборки конечного хранилища, приминяет
                        флаги PACKF_*.
    IN rc4Key         - ключ шифрования или NULL если шифрование не нужно. Ключ не изменяется.

    Return            - размер хранилища, или 0 в случаи ошибки.
  */
  DWORD _pack(STORAGE **binStorage, DWORD flags, Crypt::RC4KEY *rc4Key);

DWORD _pack2(STORAGE **binStorage, DWORD flags, Crypt::RC4KEY *rc4Key);

  /*
    Распаковка конфигурации.

    OUT binStorage - хранилище. Должно быть освобождена через Mem. Если binStorage == NULL, то
                     распаковка происходит внутри data.
    IN OUT data    - данные которые являются упакованым хранилищеи. Если binStorage == NULL, то
                     распаковка происходит внутри data.
    IN dataSize    - размер data.
    IN rc4Key      - ключ шифрования или NULL если шифрование не нужно.

    Return         - размер хранилища, или 0 в случаи ошибки, также *binStorage будет равен NULL.
  */
  DWORD _unpack(STORAGE **binStorage, void *data, DWORD dataSize, Crypt::RC4KEY *rc4Key);

  /*
    Распаковка хранилища из файла.

    OUT binStorage - хранилище. Должно быть освобождена через Mem.
    IN fileName    - файл хранилища.
    IN rc4Key      - ключ шифрования или NULL если шифрование не нужно.

    Return          - размер хранилища, или 0 в случаи ошибки, также *binStorage будет равен NULL.
  */
  DWORD _unpackFromFile(STORAGE **binStorage, LPWSTR fileName, Crypt::RC4KEY *rc4Key);
  
  /*
    Объеденяет два хранилища в одно новое. Входные хранилища должны быть корректными.

    IN binStorage1 - первое хранилище.
    IN binStorage2 - второе хранилище.

    Return         - новое хранилище, или NULL в случаи ошибки.
  */
  STORAGE *_combine(STORAGE *binStorage1, STORAGE *binStorage2);

  /*
    Открытие/создание файла массива. При открытии файла, проверяется его корректность,
    и по возможности исправлются ошибки (при этом часть данных может быть утерена).

    IN fileName         - имя файла.
    IN flags            - флаги OSF_.
    IN OUT storageArray - данные об массиве. На входе должно быть заполенено как в описании.

    Return              - true - в случаи успешного открытия файла,
                          false - в случаи ошибки открытия или критической ошибки в файле(
                          storage->file будет равен INVALID_HANDLE_VALUE).

    Примечание: В случаи возврата false: flags & OSF_WRITE_ACCESS и
                GetLastError() != ERROR_ACCESS_DEFINED рекомендуеся удалять файл, в противном случаи
                попробывать исправить ошибки отркыв файл через OSF_WRITE_ACCESS.
  */
  bool _openStorageArray(LPWSTR fileName, DWORD flags, STORAGEARRAY *storageArray);
  
  /*
    Закрывает массив.
    
    IN OUT storageArray - массив.

    Примечание: На выходе storageArray->file будет равен INVALID_HANDLE_VALUE.
  */
  void _closeStorageArray(STORAGEARRAY *storageArray);
  
  /*
    Добавление хранилища в конец массива.

    IN storageArray - массив.
    IN binStorage   - зашифрованное хранилище.
    IN size         - размер хранилища.

    Return          - true - в случаи успеха,
                      false - в случаи ошибки.
  */
  bool _addToStorageArray(STORAGEARRAY *storageArray, STORAGE *binStorage, DWORD size);
  
  /*
    Сбрасывает указатель массива следующиего хранилища на нулевой.
    
    IN storageArray - массив.
  */
  void _resetStorageArrayPointer(STORAGEARRAY *storageArray);
  
  /*
    Получает следующие хранилище в массиве. Функция вызывает _unpack для проверки и
    расшифровывания хранилища.

    IN storageArray - массив.
    OUT binStorage  - хранилище. Должно быть освобождена через Mem.
    OUT size        - размер хранилищя.
    IN rc4Key       - ключ шифрования или NULL.

    Return          - true - в случаи успеха, если *size - то достигнут конец файла,
                      false - в случаи ошибки.
  */
  bool _getNextFromStorageArray(STORAGEARRAY *storageArray, STORAGE **binStorage, LPDWORD size, Crypt::RC4KEY *rc4Key);
  
  /*
    Удаляет текущиее хранилище из массива.
    
    IN storageArray - массив.
    
    Return          - true - в случаи успеха,
                      false - в случаи ошибки.
  */
  bool _removeCurrentFromStorageArray(STORAGEARRAY *storageArray);
};
