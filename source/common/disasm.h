/*
  Дизассамблер на движке BeaEngine.
  http://beatrix2004.free.fr/BeaEngine /

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

  /*В В В В Getting the length of the opcode.

В В В В IN pAddress - opcode address.

В В В В Return - the length of the opcode, or ((DWORD) -1) in case of error.
В В */
  DWORD _getOpcodeLength(void *pAddress);
};