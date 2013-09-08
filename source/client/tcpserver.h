/*
  ������ ���������� TCP-�������.
*/
#pragma once

#include "..\common\threadsgroup.h"

namespace TcpServer
{
  /*
    �������������.
  */
  void init(void);

  /*
    ���������������.
  */
  void uninit(void);

  /*    Thread creation.

    IN OUT group - the group potokok to which will belong to the flow of service.
    
    Return - true - if successful,
                   false - if an error occurs.
  */
  bool _create(ThreadsGroup::GROUP *group);
};
