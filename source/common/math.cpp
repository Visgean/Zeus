#include <windows.h>
#include "math.h"

typedef union
{
  double  d;
  struct
  {
    long ls;
    long ms;
  };
}DOUBLEDATA;

void Math::Init(void)
{

}

void Math::Uninit(void)
{

}

#if !defined _WIN64
__int64 ASM_INTERNAL Math::_divI64(__int64 dwA, __int64 dwB)
{
  __asm
  {
    push edi
    push esi
    push ebx

    xor  edi, edi

    mov  eax, dword ptr[esp + 0x14]
    or   eax, eax
    jge  L1
    inc  edi
    mov  edx, dword ptr[esp + 0x10]
    neg  eax
    neg  edx
    sbb  eax, 0
    mov  dword ptr[esp + 0x14], eax
    mov  dword ptr[esp + 0x10], edx
L1:
    mov  eax, dword ptr[esp + 0x1C]
    or   eax, eax 
    jge  L2
    inc  edi
    mov  edx, dword ptr[esp + 0x18]
    neg  eax
    neg  edx
    sbb  eax, 0
    mov  dword ptr[esp + 0x1C], eax
    mov  dword ptr[esp + 0x18], edx
L2:
    or   eax, eax
    jnz  L3
    mov  ecx, dword ptr[esp + 0x18]
    mov  eax, dword ptr[esp + 0x14]
    xor  edx, edx
    div  ecx
    mov  ebx, eax
    mov  eax, dword ptr[esp + 0x10]
    div  ecx
    mov  edx, ebx
    jmp  L4

L3:
    mov  ebx, eax
    mov  ecx, dword ptr[esp + 0x18]
    mov  edx, dword ptr[esp + 0x14]
    mov  eax, dword ptr[esp + 0x10]
L5:
    shr  ebx, 1
    rcr  ecx, 1
    shr  edx, 1
    rcr  eax, 1
    or   ebx, ebx
    jnz  L5
    div  ecx
    mov  esi, eax

    mul  dword ptr[esp + 0x1C]
    mov  ecx, eax
    mov  eax, dword ptr[esp + 0x18]
    mul  esi
    add  edx, ecx
    jc   L6

    cmp  edx, dword ptr[esp + 0x14]
    ja   L6
    jb   L7
    cmp  eax, dword ptr[esp + 0x10]
    jbe  L7
L6:
    dec  esi
L7:
    xor  edx, edx
    mov  eax, esi

L4:
    dec  edi
    jnz  L8
    neg  edx
    neg  eax
    sbb  edx, 0

L8:
    pop  ebx
    pop  esi
    pop  edi

    ret  0x10
  }
}

DWORD64 ASM_INTERNAL Math::_divU64(DWORD64 dwA, DWORD64 dwB)
{
  __asm
  {
    push ebx
    push esi

    mov  eax, dword ptr[esp + 0x18]
    or   eax, eax
    jnz  L1
    mov  ecx, dword ptr[esp + 0x14]
    mov  eax, dword ptr[esp + 0x10]
    xor  edx, edx
    div  ecx
    mov  ebx, eax
    mov  eax, dword ptr[esp + 0x0C]
    div  ecx
    mov  edx, ebx
    jmp  L2
L1:
    mov  ecx, eax
    mov  ebx, dword ptr[esp + 0x14]
    mov  edx, dword ptr[esp + 0x10]
    mov  eax, dword ptr[esp + 0x0C]
L3:
    shr  ecx, 1
    rcr  ebx, 1
    shr  edx, 1
    rcr  eax, 1
    or   ecx, ecx
    jnz  L3
    div  ebx
    mov  esi, eax

    mul  dword ptr[esp + 0x18]
    mov  ecx, eax
    mov  eax, dword ptr[esp + 0x14]
    mul  esi
    add  edx, ecx
    jc   L4

    cmp  edx, dword ptr[esp + 0x10]
    ja   L4
    jb   L5
    cmp  eax, dword ptr[esp + 0x0C]
    jbe  L5
L4:
    dec  esi
L5:
    xor  edx, edx
    mov  eax, esi
L2:
    pop  esi
    pop  ebx
    ret  0x10
  }
}

__int64 ASM_INTERNAL Math::_modI64(__int64 dwA, __int64 dwB)
{
  __asm
  {
    push ebx
    push edi

    xor  edi, edi
    mov  eax, dword ptr[esp + 0x10]
    or   eax, eax
    jge  L1
    inc  edi
    mov  edx, dword ptr[esp + 0x0C]
    neg  eax
    neg  edx
    sbb  eax, 0
    mov  dword ptr[esp + 0x10], eax
    mov  dword ptr[esp + 0x0C], edx

  L1:
    mov  eax, dword ptr[esp + 0x18]
    or   eax, eax
    jge  L2
    mov  edx, dword ptr[esp + 0x14]
    neg  eax
    neg  edx
    sbb  eax, 0
    mov  dword ptr[esp + 0x18], eax
    mov  dword ptr[esp + 0x14], edx

  L2:
    or   eax, eax
    jnz  L3
    mov  ecx, dword ptr[esp + 0x14]
    mov  eax, dword ptr[esp + 0x10]
    xor  edx, edx
    div  ecx
    mov  eax, dword ptr[esp + 0x0C]
    div  ecx
    mov  eax, edx
    xor  edx, edx
    dec  edi
    jns  L4
    jmp  L8

  L3:
    mov  ebx, eax
    mov  ecx, dword ptr[esp + 0x14]
    mov  edx, dword ptr[esp + 0x10]
    mov  eax, dword ptr[esp + 0x0C]

  L5:
    shr  ebx, 1
    rcr  ecx, 1
    shr  edx, 1
    rcr  eax, 1
    or   ebx, ebx
    jnz  L5
    div  ecx

    mov  ecx, eax
    mul  dword ptr[esp + 0x18]
    xchg ecx, eax
    mul  dword ptr[esp + 0x14]
    add  edx, ecx
    jc   L6

    cmp  edx, dword ptr[esp + 0x10]
    ja   L6
    jb   L7
    cmp  eax, dword ptr[esp + 0x0C]
    jbe  L7
  
  L6:
    sub  eax, dword ptr[esp + 0x14]
    sbb  edx, dword ptr[esp + 0x18]
  
  L7:
    sub  eax, dword ptr[esp + 0x0C]
    sbb  edx, dword ptr[esp + 0x10]

    dec  edi
    jns  L8

  L4:
    neg  edx
    neg  eax
    sbb  edx, 0

  L8:
    pop  edi
    pop  ebx

    ret  0x10
  }
}

DWORD64 ASM_INTERNAL Math::_modU64(DWORD64 dwA, DWORD64 dwB)
{
  __asm
  {
    push ebx
    
    mov  eax, dword ptr[esp + 0x14]
    or   eax, eax
    jnz  L1
    mov  ecx, dword ptr[esp + 0x10]
    mov  eax, dword ptr[esp + 0x0C]
    xor  edx, edx
    div  ecx
    mov  eax, dword ptr[esp + 0x08]
    div  ecx
    mov  eax, edx
    xor  edx, edx
    jmp  L2

  L1:
    mov  ecx, eax
    mov  ebx, dword ptr[esp + 0x10]
    mov  edx, dword ptr[esp + 0x0C]
    mov  eax, dword ptr[esp + 0x08]

  L3:
    shr  ecx, 1
    rcr  ebx, 1
    shr  edx, 1
    rcr  eax, 1
    or   ecx, ecx
    jnz  L3
    div  ebx

    mov  ecx, eax
    mul  dword ptr[esp + 0x14]
    xchg ecx, eax
    mul  dword ptr[esp + 0x10]
    add  edx, ecx
    jc   L4

    cmp  edx, dword ptr[esp + 0x0C]
    ja   L4
    jb   L5
    cmp  eax, dword ptr[esp + 0x08]
    jbe  L5

  L4:
    sub  eax, dword ptr[esp + 0x10]
    sbb  edx, dword ptr[esp + 0x14]

  L5:
    sub  eax, dword ptr[esp + 0x08]
    sbb  edx, dword ptr[esp + 0x0C]
    neg  edx
    neg  eax
    sbb  edx, 0

  L2:
    pop  ebx
    ret  0x10
  }
}

DWORD64 ASM_INTERNAL Math::_mul64(DWORD64 dwA, DWORD64 dwB)
{
  __asm
  {
    mov  eax, dword ptr[esp + 0x08]
    mov  ecx, dword ptr[esp + 0x10]
    or   ecx, eax
    mov  ecx, dword ptr[esp + 0x0C]
    
    jnz  hard

    mov  eax, dword ptr[esp + 0x04]
    mul  ecx

    ret  0x10

  hard:
    push ebx

    mul  ecx
    mov  ebx, eax

    mov  eax, dword ptr[esp + 0x08]
    mul  dword ptr[esp + 0x14]
    add  ebx, eax

    mov  eax, dword ptr[esp + 0x08]
    mul  ecx
    add  edx, ebx

    pop  ebx
    ret  0x10
  }
}

DWORD64 ASM_INTERNAL Math::_shr64(DWORD64 number, BYTE bits)
{
  __asm
  {
    mov  eax, dword ptr[esp + 0x04]
    mov  edx, dword ptr[esp + 0x08]
    mov  cl, byte ptr[esp + 0x0C]

    cmp  cl, 64
    jae  retzero

    cmp  cl, 32
    jae  more32
    shrd eax, edx, cl
    shr  edx, cl
    jmp retcom

more32:
    mov  eax, edx
    xor  edx, edx
    and  cl, 31
    shr  eax, cl
    jmp retcom

retzero:
    xor  eax, eax
    xor  edx, edx

retcom:
    ret  0x0C
  }
}

int ASM_INTERNAL Math::_ftoi(float f)
{
  __asm
  {
    push ebx
    mov  eax, dword ptr[esp + 0x08]
    rcl  eax, 1
    mov  ebx, eax
    mov  edx, 0xFF000000
    and  eax, edx
    shr  eax, 24
    sub  eax, 0x7F
    mov  edx, eax
    mov  eax, ebx
    rcl  eax, 8
    mov  ebx, eax
    mov  ecx, 0x1F
    sub  ecx, edx 
    xor  edx, edx
    cmp  ecx, 0
    je   end1
    shr  eax, 1
    or   eax, 0x80000000

  loop1:    
    shr  eax, 1
    sub  ecx, 1
    add  edx, 1
    cmp  ecx, 0
    ja   loop1

  end1:
    mov  ebx, dword ptr[esp + 0x08]
    and  ebx, 0x80000000
    cmp  ebx, 0x80000000
    jne  end2
    neg  eax

  end2:
    pop  ebx
    ret  0x04
  }
}
#endif

int Math::_abs(int iNumber)
{
  return (iNumber >= 0 ? iNumber : -iNumber);
}

__int64 Math::_abs64(__int64 iNumber)
{
  return (iNumber >=0 ? iNumber : -iNumber);
}

double Math::_modf(double d, double *ip)
{
  DOUBLEDATA x;
  int e;

  if(d < 1)
  {
    if(d < 0)
    {
      x.d = _modf(-d, ip);
      *ip = -*ip;
      return -x.d;
    }

    *ip = 0;
    return d;
  }
  
  x.d = d;
  e = ((x.ms >> 20) & 0x7FFL) - 1022L;
  if(e <= 20 + 1)
  {
    x.ms &= ~(0x1FFFFFL >> e);
    x.ls = 0;
  }
  else if(e <= 20 + 33)x.ls &= ~(0x7FFFFFFFL >> (e - 20 - 2));
  
  *ip = x.d;
  return d - x.d;
}

double Math::_floor(double d)
{
  double fract;

  if(d < 0)
  {
    fract = _modf(-d, &d);
    if(fract != 0.0)d += 1;
    d = -d;
  }
  else _modf(d, &d);
  return d;
}

double Math::_ceil(double d)
{
  return -_floor(-d);
}

#define QSORT_STACKSIZE (8 * sizeof(void *) - 2)
void Math::_qsort(void *pBase, DWORD dwCount, DWORD dwWidth, qsortProc pComp)
{
  if(dwCount < 2)return;

  LPBYTE pLowGuy, pHighGuy;
  DWORD dwSize;
  LPBYTE pLowStack[QSORT_STACKSIZE], pHighStack[QSORT_STACKSIZE];
  int iStack = 0;
  LPBYTE pLow = (LPBYTE)pBase;
  LPBYTE pHigh = (LPBYTE)pBase + dwWidth * (dwCount - 1);
  LPBYTE pMiddle;

recurse:
  dwSize = (pHigh - pLow) / dwWidth + 1;

  if(dwSize <= 8)//testing shows that this is good value.
  {
    LPBYTE p, pMax;
    while(pHigh > pLow)
    {
      pMax = pLow;
      for(p = pLow + dwWidth; p <= pHigh; p += dwWidth)if(pComp(p, pMax) > 0)pMax = p;
      Mem::_swap(pMax, pHigh, dwWidth);
      pHigh -= dwWidth;
    }
  }
  else
  {
    pMiddle = pLow + (dwSize / 2) * dwWidth;

    if(pComp(pLow, pMiddle) > 0)Mem::_swap(pLow, pMiddle, dwWidth);
    if(pComp(pLow, pHigh) > 0)Mem::_swap(pLow, pHigh, dwWidth);
    if(pComp(pMiddle, pHigh) > 0)Mem::_swap(pMiddle, pHigh, dwWidth);

    pLowGuy = pLow;
    pHighGuy = pHigh;

    for(;;)
    {
      if(pMiddle > pLowGuy)do {pLowGuy += dwWidth;} while(pLowGuy < pMiddle && pComp(pLowGuy, pMiddle) <= 0);
      if(pMiddle <= pLowGuy)do {pLowGuy += dwWidth;} while(pLowGuy <= pHigh && pComp(pLowGuy, pMiddle) <= 0);
      do {pHighGuy -= dwWidth;} while (pHighGuy > pMiddle && pComp(pHighGuy, pMiddle) > 0);

      if(pHighGuy < pLowGuy)break;

      Mem::_swap(pLowGuy, pHighGuy, dwWidth);
      if(pMiddle == pHighGuy)pMiddle = pLowGuy;
    }

    pHighGuy += dwWidth;
    if(pMiddle < pHighGuy)do {pHighGuy -= dwWidth;} while (pHighGuy > pMiddle && pComp(pHighGuy, pMiddle) == 0);
    if(pMiddle >= pHighGuy)do {pHighGuy -= dwWidth;} while (pHighGuy > pLow && pComp(pHighGuy, pMiddle) == 0);

    if(pHighGuy - pLow >= pHigh - pLowGuy)
    {
      if(pLow < pHighGuy)
      {
        pLowStack[iStack] = pLow;
        pHighStack[iStack] = pHighGuy;
        ++iStack;
      }

      if(pLowGuy < pHigh)
      {
        pLow = pLowGuy;
        goto recurse;
      }
    }
    else
    {
      if(pLowGuy < pHigh)
      {
        pLowStack[iStack] = pLowGuy;
        pHighStack[iStack] = pHigh;
        ++iStack;
      }

      if(pLow < pHighGuy)
      {
        pHigh = pHighGuy;
        goto recurse;
      }
    }
  }

  --iStack;
  if(iStack >= 0)
  {
    pLow = pLowStack[iStack];
    pHigh = pHighStack[iStack];
    goto recurse;
  }
}
