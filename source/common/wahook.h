/*
  �������� WinAPI.
*/
#pragma once

namespace WaHook
{
  /*
    �������������.
  */
  void init(void);

  /*
    ���������������.
  */
  void uninit(void);

  /*
    ������� ��� _hook().

    IN functionForHook  - ��������� �� �������, ������� ����� �����������.
    IN originalFunction - �����, �� �������� ��� �������� ������ ���������� �������, ���� �����
                          ���������� �������� ��� ������ ������������ ������� ����� ��������� ����.
  */
  typedef void (*HOTPATCHCALLBACK)(const void *functionForHook, const void *originalFunction);
  
  /*
    ��������� ����.

    IN process             - �������, � ������� ���������� �����������.
    IN OUT functionForHook - ��������� �� �������, ������� ����� �����������.
    IN hookerFunction      - ��������� �� ����� �������.
    OUT originalFunction   - �����, �� �������� ����� �������� ������ ���������� �������, ���� �����
                             ���������� �������� ��� ������ ������������ ������� ����� ���������
                             ����.
    IN hotPatchCallback    - �������, ������� ���������� ���������� ����� ������� ���� �
                             ������������ �������.
    
    Return                 - ������ ������ ���������� �� ������ originalFunction, ��� 0 � ������
                             ������.
  */
  DWORD _hook(HANDLE process, void *functionForHook, void *hookerFunction, void *originalFunction, HOTPATCHCALLBACK hotPatchCallback);

  /*
    ������ ����.

    IN process             - �������, � ������� ���������� �����������.
    IN OUT functionForHook - ��������� �� ������������ �������.
    IN originalFunction    - ����� �� �������� �������� ����������� ������.
    IN size                - ������ originalFunction.

    Return                 - true - � ������ ������,
                             false - � ������ ������.
  */
  bool _unhook(HANDLE process, void *hookedFunction, void *originalFunction, DWORD size);

  /*
    ��������� �������� �� ������� ������������, �������� ������ ������.

    IN process  - �������
    IN function - �������.

    Return      - true - �����������,
                  false - �� �����������.
  */
  bool _isHooked(HANDLE process, void *function);
  
  /*    Allocates a region of memory where you can make replacement parts intercepted functions.
    
    IN process - a process in which the modification.
    IN maxFunctions - approximate kolichetsvo features that will be intercepted.
    
    Return - address, or NULL in case of error.
  */
  void *_allocBuffer(HANDLE process, DWORD maxFunctions);
};
