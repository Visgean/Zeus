/*
  Управление группой потоков.

  Примичание: функции не безопаны при вызове из разных потоков.
              в группе не может быть более MAXIMUM_WAIT_OBJECTS потоков.

*/
#pragma once

namespace ThreadsGroup
{
  typedef struct
  {
    BYTE count;                           //Количетсво нитей.
    HANDLE handles[MAXIMUM_WAIT_OBJECTS]; //Хэндлы нитей.
  }GROUP;
  
  /*
    Инициализация.
  */
  void init(void);

  /*
    Деинициализация.
  */
  void uninit(void);

  /*
    Созданме новой группы.
    
    OUT group - группа нитей.
  */
  void _createGroup(GROUP *group);

  /*
    Удаление хэндлов завершенных процессов из группы.

    IN OUT group - группа для обработки.
  */
  void _closeTerminatedHandles(GROUP *group);

  /*
    Получение кол. активных потоков.

    IN group - группа для обработки.

    Return   - кол. активных потоков.
  */
  DWORD _numberOfActiveThreads(GROUP *group);

  /*
    Создание новой нити.
    
    IN group         - группа нитей.
    IN stackSize     - размер стека, 0 - использовать по умолчанию.
    IN startAddress  - точка входа нити, или NULL если требуется только оптимизация группы.
    IN parameter     - параметр точки входа.
    OUT threadId     - ID нити, или NULL если не нужен.
    OUT threadHandle - Хэндл нити, или NULL если не нужен. Не в коем случаи нельзя закрывать этот
                        хэндл из вне.

    Return           - true - в случаи успеха,
                       false - в случаи ошибки.
  */
  bool _createThread(GROUP *group, SIZE_T stackSize, LPTHREAD_START_ROUTINE startAddress, LPVOID parameter, LPDWORD threadId, HANDLE *threadHandle);

  /*
    Ожидание завершение всех нитей.

    IN group   - группа нитей.
    IN timeout - время ожидание в миллесекундах, или INFINITE для бесконечного ожидания.     
    
    Return     - true - в случаи успеха,
                 false - в случаи ошибки или таймаута.

  */
  bool _waitForAllExit(GROUP *group, DWORD timeout);

  /*
    Закрытие хэнделов группы нитей (не завершение нитей).

    IN group - группа нитей.
  */
  void _closeGroup(GROUP *group);
};
