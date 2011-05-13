#include <windows.h>

#include "disasm.h"

extern "C" int WINAPI LDE(void *, int);

void Disasm::init(void)
{

}

void Disasm::uninit(void)
{

}

DWORD Disasm::_getOpcodeLength(void *pAddress)
{
#if defined _WIN64  
  return (DWORD)LDE(pAddress, 64);
#else
  return (DWORD)LDE(pAddress, 0);
#endif
}
