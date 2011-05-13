#pragma once
/*
  Набор функций для синхронизации между нитями и процессами.
*/

namespace Sync
{
  /*
    Инициализация.
  */
  void init(void);

  /*
    Деинициализация.
  */
  void uninit(void);

  /*
    Ожидание освобождения мютекса и его захват.

    IN mutexAttributes - SECURITY_ATTRIBUTES для мютекса или NULL.
    IN name            - имя мютекса.

    Return             - хэндл мютекса.

    Примечание: Для освобождения мютекса необходимо вызвать _freeMutex().
  */
  HANDLE _waitForMutex(SECURITY_ATTRIBUTES *mutexAttributes, LPWSTR name);

  /*
    Ожидание событие, и обработка всех оконных сообщений потока.

    IN count        - кол. объектов.
    IN handles      - объекты.
    IN waitAll      - ждать всех обектов.
    IN milliseconds - время ожидания в мс, INFINITE для бескоенчного ожидания.

    Return          - согласно WaitForMultipleObjects().
  */
  DWORD _waitForMultipleObjectsAndDispatchMessages(DWORD count, const HANDLE* handles, bool waitAll, DWORD milliseconds);

  /*
    Освобождение мютекса захваченного через WaitForMutex.

    IN mutex - хэндл мютекса.
  */
  void _freeMutex(HANDLE mutex);

  /*
    Создание уникального мютекса.

    IN mutexAttributes - SECURITY_ATTRIBUTES для мютекса или NULL.
    IN name            - имя мютекса.

    Return             - хэндл мютекса, или NULL в случаи ошибки или если мютекс уже существует.
  */
  HANDLE _createUniqueMutex(SECURITY_ATTRIBUTES *mutexAttributes, LPWSTR name);

  /*
    Проверяет, существует ли мютекс.

    IN name - имя мютекса.

    Return  - true - существует,
              false - не сущетвует.
  */
  bool _mutexExists(LPWSTR name);
};