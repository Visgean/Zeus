/*
  Работа с памятью Windows 32/64 на основе Heap.

  Примечание: Данные функции в первую очередь рассчитаны на стабильность, поэтому имеют следующие
              свойства:
                1. Выделяемый блок памяти обязательно заполняется нулями.
*/

#pragma once

#if !defined MEM_PERSONAL_HEAP
#  error MEM_PERSONAL_HEAP not defined!
#endif

#if !defined MEM_ALLOC_SAFE_BYTES
#  error MEM_ALLOC_SAFE_BYTES not defined!
#endif

//Структура для указания параметров блока памяти
typedef struct
{
  void *data;
  SIZE_T size;
}MEMDATA;

//Оффсет в структуре.
#define OFFSETOF(v, m)  ((DWORD_PTR)(&((v *)(NULL))->m))

//Выравнивание.
#define ALIGN_DOWN(x, align)  ((x) & ~(align - 1))
#define ALIGN_UP(x, align) (((x) & (align - 1)) ? ALIGN_DOWN(x, align) + align : (x))

//Размер страницы виртуальной памяти.
#define VM_PAGE_SIZE 4096

#define VM_STEP_MASK (~0xFFFF)
#define VM_STEP      0x10000

namespace Mem
{
  /*
    Инициализация, создание кучи.

    IN initialSize - начальный размер кучи.
  */
#if(MEM_PERSONAL_HEAP == 1)
  void init(SIZE_T initialSize);
#else
  void init(void);
#endif

  /*
    Деинициализация.
  */
  void uninit(void);
  
  /*
    Изменения размера блока памяти.

    IN OUT old - блок памяти для изменения, или NULL для создания нового (равносильно alloc).
    IN size    - новый размер блока памяти.

    Return     - true - в случаи успешного выделения памяти,
                 false - в случаи ошибки, при этом старый блок памяти не освобождается.
  */
  bool reallocEx(void *old, SIZE_T size);

  /*
    Изменения размера блока памяти.

    IN old  - блок памяти для изменения, или NULL для создания нового (равносильно Alloc).
    IN size - новый размер блока памяти.

    Return  - адрес нового блока памяти, или NULL в случаи ошибки.
  */
  void *realloc(void *old, SIZE_T size);

  /*
    Выделение блока памяти.

    IN size - размер блока памяти.

    Return  - адрес нового блока памяти, или NULL в случаи ошибки.
  */
  void *alloc(SIZE_T size);

  /*
    Выделить память, не обнуля содержимое.

    IN size - размер блока памяти.

    Return  - адрес нового блока памяти, или NULL в случаи ошибки.
  */
  void *quickAlloc(SIZE_T size);

  /*
    Освобождение блока памяти.

    IN mem - блок памяти для освобождения.
  */
  void free(void *mem);

  /*
    Освобождение блока памяти с предварительным занулением.

    IN mem  - блок памяти для освобождения, не может быть NULL.
    IN size - количество байт для обнуления.
  */
  void zeroAndFree(void *mem, SIZE_T size);

  /*
    Освобождение памяти по адресам перечисленных в массиве. и освобождение самого массива mem

    IN mem   - массив для освобождения.
    IN count - количество элементов в массиве pMem
  */
  void freeArrayOfPointers(void *mem, SIZE_T count);

  /*
    Копирование блока памяти.

    OUT dest  - адрес памяти для скопированных байтов.
    IN source - адрес памяти с исходными байтами.
    IN size   - количество байт для копирования.
  */
  void ASM_INTERNAL_DEF _copy(void *dest, const void *source, SIZE_T size);

  /*
    Копирование блока памяти начиная с конца блока памяти.

    OUT dest  - адрес памяти для скопированных байтов.
    IN source - адрес памяти с исходными байтами.
    IN size   - количество байт для копирования.
  */
  void _copyFromEnd(void *dest, const void *source, SIZE_T size);

  /*
    Копирование блока памяти, обертка для Copy.

    OUT dest  - адрес памяти для скопированных байт.
    IN source - адрес памяти с исходными байтами.
    IN size   - количество байт для копирования.

    Return    - возвращает dest + size.
  */
  void *_copy2(void *dest, const void *source, SIZE_T size);

  /*
    Копирование блока памяти с автоматическим выделением памяти для копии.

    IN source - адрес памяти с исходными байтами.
    IN size   - количество байт для копирования.

    Return    - адрес памяти с скопированными байтами, или NULL.
  */
  void *copyEx(const void *source, SIZE_T size);

  /*
    Сравнение блоков памяти.

    IN mem1  - адрес первого блока памяти.
    IN mem2  - адрес второго блока памяти.
    IN size - количество байт для сравнения.

    Return    -  <0 - mem1 меньше mem2
                 =0 - mem1 равен mem2
                 >0 - mem1 больше mem2
  */
  int _compare(const void *mem1, const void *mem2, SIZE_T size);

  /*
    Обнуление блока памяти.

    IN mem  - адрес блока памяти для обнуления.
    IN size - количество байт для обнуления.
  */
  void _zero(void *mem, SIZE_T size);

  /*
    Заполнение блока памяти определенным символом.

    IN mem  - адрес блока памяти для заполнения.
    IN size - количество байт для заполнения.
  */
  void ASM_INTERNAL_DEF _set(void *mem, char c, SIZE_T size);

  /*
    Ищет символ в блоке памяти с лева на права.

    IN mem  - блок памяти, в котором производится поиск.
    IN c    - символ для поиска.
    IN size - количество байт в  блоке mem.

    Return  - в случаи успеха указатель на символ,
              в случаи ошибки NULL.
  */
  void *_getL(void *mem, char c, SIZE_T size);

  /*
    Ищет символ в блоке памяти с права на лева.

    IN mem  - блок памяти, в котором производится поиск.
    IN c    - символ для поиска.
    IN size - количество байт в блоке mem.

    Return  - в случаи успеха указатель на символ,
              в случаи ошибки NULL.
  */
  void *_getR(void *mem, char c, SIZE_T size);

  /*
    Подменяет один байть на другой в блоке памяти.
  
    IN mem     - блок памяти, в котором производится поиск.
    IN size    - количество байт в блоке pmem.
    IN oldChar - байт для поиска.
    IN newChar - байт для замены.
  */
  void _replace(void *mem, SIZE_T size, char oldChar, char newChar);

  /*
    Поиск блока данных в памяти.

    IN mem      - указатель на память.
    IN memSize  - размер mem.
    IN data     - блок для поиска.
    IN dataSize - размер data.

    Return       - указатель на первый встретевшийся блок data, или NULL если не найдено.
  */
  void *_findData(const void *mem, SIZE_T memSize, void *data, SIZE_T dataSize);

  /*
    Обмен данными двух адресов.

    IN mem1 - первый адрес.
    IN mem2 - второй адрес.
    IN size - количество байт для обмена.
  */
  void _swap(void *mem1, void *mem2, SIZE_T size);

  /*
    Подмена DWORD значения.

    IN originalValue - старое значение.
    IN newValue      - новое значение.
    IN OUT mem       - память для обработки.
    IN memSize       - размер pMem.

    Return           - кол. измененных занчений.
  */
  SIZE_T _replaceDword(DWORD originalValue, DWORD newValue, void *mem, SIZE_T memSize);

  /*
    IN originalValue - старое значение.
    IN newValue      - новое значение.
    IN OUT mem       - память для обработки.
    IN memSize       - размер pMem.

    Return           - кол. измененных занчений.
  */
  SIZE_T _replaceQword(DWORD64 originalValue, DWORD64 newValue, void *mem, SIZE_T memSize);
};
