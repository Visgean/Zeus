#include <windows.h>

#include "wahook.h"
#include "disasm.h"

#if defined _WIN64
  //FIXME
#else
  #define WRITE_JMP(mem, thisAddress, to) {*((LPBYTE)(mem)) = OPCODE_JMP; *((DWORD *)(((LPBYTE)(mem)) + OPCODE_JMP_SIZE)) = (DWORD)((DWORD_PTR)(to) - (DWORD_PTR)(thisAddress) - INJECT_SIZE);}
  #define OPCODE_JMP      0xE9
  #define OPCODE_JMP_SIZE 1 
  #define JMP_ADDR_SIZE   (sizeof(DWORD) + OPCODE_JMP_SIZE)
  #define INJECT_SIZE     JMP_ADDR_SIZE
  #define OPCODE_MAX_SIZE 15
#endif

/*
  ѕровер€ет, сколько доступно байт дл€ изменени€.

  IN process - процесс.
  IN address - адрес.

  Return     - кол. доступных байт.
*/
static DWORD_PTR checkAvalibleBytes(HANDLE process, void *address)
{
  DWORD_PTR avalibeBytes = 0;
  MEMORY_BASIC_INFORMATION mbi;
  if(CWA(kernel32, VirtualQueryEx)(process, address, &mbi, sizeof(MEMORY_BASIC_INFORMATION)) != 0 && mbi.State == MEM_COMMIT && (mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD)) == 0)
  {
    DWORD_PTR tmp = (DWORD_PTR)((LPBYTE)address - (LPBYTE)mbi.BaseAddress);
    if(mbi.RegionSize > tmp/*парано€*/)avalibeBytes = mbi.RegionSize - tmp;
  }
  return avalibeBytes;
}

void WaHook::init(void)
{

}

void WaHook::uninit(void)
{

}

DWORD WaHook::_hook(HANDLE process, void *functionForHook, void *hookerFunction, void *originalFunction, HOTPATCHCALLBACK hotPatchCallback)
{
  DWORD retVal = 0;

  DWORD oldProtect;
  DWORD_PTR avalibeBytes = checkAvalibleBytes(process, functionForHook);

  //Give all the rights of the affected pages.
  if(avalibeBytes >= OPCODE_MAX_SIZE * 2 && CWA(kernel32, VirtualProtectEx)(process, functionForHook, OPCODE_MAX_SIZE * 2, PAGE_EXECUTE_READWRITE, &oldProtect) != 0)
  {
    //Read the old code.
    BYTE buf[OPCODE_MAX_SIZE * 2 + JMP_ADDR_SIZE];
    Mem::_set(buf, (char)0x90, sizeof(buf));/*парано€*/

    if(CWA(kernel32, ReadProcessMemory)(process, functionForHook, buf, OPCODE_MAX_SIZE * 2, NULL) == 0)goto END;

    //Read opcodes, while their total length does not reach INJECT_SIZE.
    DWORD_PTR opcodeOffset = 0;
    for(;;)
    {
      LPBYTE currentOpcode = bufB + OpcodeOffset;
      DWORD currentOpcodeLen = Disasm::_getOpcodeLength(currentOpcode);

      //Unknown opcode.
      if(currentOpcodeLen == (DWORD)-1)
      {
        #if defined(WDEBUG2)
        WDEBUG2(WDDT_ERROR, "Bad opcode detected at offset %u for function 0x%p", opcodeOffset, functionForHook);
        #endif

        goto END; 
      }

      opcodeOffset += currentOpcodeLen;

      if(opcodeOffset > sizeof(buf) - JMP_ADDR_SIZE)
      {
        #if defined(WDEBUG2)
        WDEBUG2(WDDT_ERROR, "Very long opcode detected at offset %u for function 0x%p", opcodeOffset - currentOpcodeLen, functionForHook);
        #endif
        
        goto END; 
      }
      
      //Otnostielnye call and jmp.
      if((currentOpcode[0] == 0xE9 || currentOpcode[0] == 0xE8) && currentOpcodeLen == 1 + sizeof(DWORD)) //FIXME: not sure for x64.
      {
#       if defined(WDEBUG0)
        WDEBUG1(WDDT_INFO, "Relative JMP/CALL(%02X) detected.", currentOpcode[0]);
#       endif

        DWORD *relAddrSet = (DWORD *)(currentOpcode + 1);
        DWORD_PTR to = (*relAddrSet) + ((DWORD_PTR)functionForHookB + OpcodeOffset);
        *relAddrSet = (DWORD)(to - ((DWORD_PTR)originalFunctionB + OpcodeOffset));
      }
      
      if(opcodeOffset >= INJECT_SIZE)break;
    }

    //Save the original opcodes in originalFunction.
    {
      //Appends buffer, jump to the continuation functionForHook.
      LPBYTE pjmp = bufB + OpcodeOffset;
      WRITE_JMP(pjmp, originalFunction/*B + OpcodeOffset*/, functionForHook/*B + OpcodeOffset*/);
      if(CWA(kernel32, WriteProcessMemory)(process, originalFunction, buf, opcodeOffset + JMP_ADDR_SIZE, NULL) == 0)goto END;
    }

    //Pishim inject into the function.
    {
      WRITE_JMP(buf, functionForHook, hookerFunction);
      hotPatchCallback(functionForHook, originalFunction);
      if(CWA(kernel32, WriteProcessMemory)(process, functionForHook, buf, INJECT_SIZE, NULL) == 0)goto END;
    }

    retVal = opcodeOffset + JMP_ADDR_SIZE; //The size of fragment excision.

END:
    //Vosstanalivaem law.
    CWA(kernel32, VirtualProtectEx)(process, functionForHook, OPCODE_MAX_SIZE * 2, oldProtect, &oldProtect);
  }
  
  return retVal;
}

bool WaHook::_unhook(HANDLE process, void *hookedFunction, void *originalFunction, DWORD size)
{
  bool ret = false;
  DWORD oldProtect;
  DWORD_PTR avalibeBytes = checkAvalibleBytes(process, hookedFunction);

  //Give all the rights of the affected pages.
  if(avalibeBytes >= OPCODE_MAX_SIZE * 2 && CWA(kernel32, VirtualProtectEx)(process, hookedFunction, OPCODE_MAX_SIZE * 2, PAGE_EXECUTE_READWRITE, &oldProtect) != 0)
  {
    if(CWA(kernel32, WriteProcessMemory)(process, hookedFunction, originalFunction, size - JMP_ADDR_SIZE, NULL) != 0)ret = true;
    
    //Vosstanalivaem law.
    CWA(kernel32, VirtualProtectEx)(process, hookedFunction, OPCODE_MAX_SIZE * 2, oldProtect, &oldProtect);
  }

  return ret;
}

bool WaHook::_isHooked(HANDLE process, void *function)
{
  BYTE opcode = 0; 
  if(CWA(kernel32, ReadProcessMemory)(process, function, &opcode, 1, NULL) != FALSE && opcode == OPCODE_JMP)return true;
  return false;
}

void *WaHook::_allocBuffer(HANDLE process, DWORD maxFunctions)
{
  return CWA(kernel32, VirtualAllocEx)(process, NULL, maxFunctions * ((OPCODE_MAX_SIZE * 2) + JMP_ADDR_SIZE), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
}
