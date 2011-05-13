#pragma once
/*
  Математические функции.
*/

namespace Math
{
  /*
    Инициализация.
  */
  void Init(void);

  /*
    Деинициализация.
  */
  void Uninit(void);

#if !defined _WIN64
  /*
    Деление 64-х битного знакового числа (Доступно толко в Win32).

    IN dwA - делимое.
    IN dwB - делитель.

    Return - результат.
  */
  __int64 ASM_INTERNAL_DEF _divI64(__int64 dwA, __int64 dwB);

  /*
    Деление 64-х битного без знакового числа (Доступно толко в Win32).

    IN dwA - делимое.
    IN dwB - делитель.

    Return - результат.
  */
  DWORD64 ASM_INTERNAL_DEF _divU64(DWORD64 dwA, DWORD64 dwB);

  /*
    Получение модуля от деление 64-х битного знакового числа.

    IN dwA - делимое.
    IN dwB - делитель.

    Return - результат.
  */
  __int64 ASM_INTERNAL_DEF _modI64(__int64 dwA, __int64 dwB);

  /*
    Получение модуля от деление 64-х битного без знакового числа.

    IN dwA - делимое.
    IN dwB - делитель.

    Return - результат.
  */
  DWORD64 ASM_INTERNAL_DEF _modU64(DWORD64 dwA, DWORD64 dwB);

  /*
    Умножение 64-х битного числа (Доступно толко в Win32).

    IN dwA - множимое.
    IN dwB - множитель.

    Return - результат.
  */
  DWORD64 ASM_INTERNAL_DEF _mul64(DWORD64 dwA, DWORD64 dwB);

  /*
    Сдвиг вправо 64-х битного числа (Доступно толко в Win32).

    IN number - число.
    IN bits   - битов для свдига.

    Return    - результат.
  */
  DWORD64 ASM_INTERNAL_DEF _shr64(DWORD64 number, BYTE bits);

  /*
    Преобразование float в int32  (Доступно толко в Win32).

    IN f   - float.
    Return - int32.
  */
  int ASM_INTERNAL_DEF _ftoi(float f);
#endif

  /*
    Возвращает абсолютное значение 32-x битного числа.

    IN iNumber - число.

    Return - абсолютное значение.
  */
  int _abs(int iNumber);

  /*
    Возвращает абсолютное значение 64-x битного числа.

    IN iNumber - число.

    Return - абсолютное значение.
  */
  __int64 _abs64(__int64 iNumber);

  /*
    Разделят дробь на целую и дробную часть.

    IN d   - исходное число.
    OUT ip - целая часть.

    Return - дробная часть.
  */
  double _modf(double d, double *ip);

  /*
    Округляет дробь в меньшую сторону.

    IN d - дробь.

    Return - результат.
  */
  double _floor(double d);

  /*
    Округляет дробь в большую сторону.

    IN d - дробь.

    Return - результат.
  */
  double _ceil(double d);

  /*
    Функция сравнения двух элеметов массива.

    IN p1  - первый элемент.
    IN p2  - второй элемент.

    Return -  > 0 - элемент p1 больше p2.
              = 0 - элементы равны.
              < 0 - элемент p1 меньше p2.
  */
  typedef int (*qsortProc)(const void *p1, const void *p2);

  /*
    Алгоритм быстрой сортировки для массива.

    IN pBase   - базовый адрес массива.
    IN dwCount - количетсво элементов в массиве.
    IN dwWidth - ширина одного элемента.
    IN pComp   - функция сравнения.
  */
  void _qsort(void *pBase, DWORD dwCount, DWORD dwWidth, qsortProc pComp);
};
