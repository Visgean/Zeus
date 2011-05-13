/*
  Дизассамблер на движке BeaEngine.
  http://beatrix2004.free.fr/BeaEngine/

  ПРИМИЧАНИЕ: Движок доволно грамозткий, однако это компенсируется его унивирсальностю под x32,
              x64.
*/
#pragma once

namespace Disasm
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
    Получение длины опкода.

    IN pAddress - адрес опкода.

    Return - длина опкода, или ((DWORD)-1) в случаи ошибки.
  */
  DWORD _getOpcodeLength(void *pAddress);
};