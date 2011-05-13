/*
  Работа с GUI.
*/
#pragma once

namespace Gui
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
    Кээлбэк для _enumWindows().

    IN window - окно.
    IN param  - параметр.
    
    Return    - true  - продолжить поиск,
                false - прервать поиск.
  */
  typedef bool (ENUMWINDOWSPROC)(HWND window, void *param);

  /*
    Перечисление дочерных окон.

    IN owner     - родитель, может быть NULL.
    IN topToDown - true - перечислять с верху вниз, false - снизу вверх.
    IN proc      - кэлбэк.
    IN param     - параметр для кэлбэка.
  */
  void _enumWindows(HWND owner, bool topToDown, ENUMWINDOWSPROC proc, void *param);

  /*
    Данная функция является обреткой для WindowFromPoint(), которая пропускает HTTRANSPARENT.
    Способ реализации доволно наивен, но мне не удалсоь найти более нормального способа.

    IN point    - координаты для поиска окна.
    IN timeout  - таймаут для WM_NCHITTEST.
    OUT hitTest - HitTest окна. Может быть NULL.

    Return      - хэндл окна, или NULL.
  */
  HWND _windowFromPoint(POINT point, DWORD timeout, DWORD *hitTest);

  /*
    Проверяет имеет ли стиль окна бордюры с изменением размера.

    IN style - стиль окна.

    Return   - true - стиль имеед бордюры,
               false - не имеет.
  */
  bool _styleHaveSizeBorders(DWORD style);

  /*
    Загрузка иконки как shared (не трубует удаления).

    IN module - моудль.
    IN id     - ID.
  */
  HICON _loadSharedIcon(HMODULE module, const LPWSTR id);
  
  /*
    Получение текста окна с выделением памяти.

    IN window - окно.
    OUT size  - размер строки без нулевого соимвола. Может быть NULL.

    Return    - текст окна (нужно освободить через NULL), или NULL в случаи ошибки.
  */
  LPWSTR _getWindowText(HWND window, LPDWORD size);

  /*
    Загрузка курсора как shared (не трубует удаления).

    IN module - моудль.
    IN id     - ID.
  */
  HCURSOR _loadSharedCursor(HMODULE module, const LPWSTR id);

  /*
    Создание шрифта FONT_DIALOG.

    IN pointSize - размер шрифта в поинтах, рекомендуется значение 8.

    Return       - хэндл шрифта, или NULL в случаи ошибки.
  */
  HFONT _createDialogFont(BYTE pointSize);

  /*
    Запуск диалога выбора файла.

    IN owner          - хэндл родителя
    IN initialDir     - директория отностиельно который открывается диалог. Может быть NULL.
    IN OUT fileBuffer - на входе - инициализационный путь/файл,
                        на выходе - выбранный файл.

    Return            - true - в случаи успеха,
                        false - в случаи провала.
  */
  bool _browseForFile(HWND owner, LPWSTR initialDir, LPWSTR fileBuffer);

  /*
    Запуск диалога сохранения файла.

    IN owner            - хэндл родителя
    IN initialDir       - директория отностиельно который открывается диалог. Может быть NULL.
    IN OUT fileBuffer   - на входе - инициализационный путь/файл,
                          на выходе - выбранный файл.
    IN defaultExtension - расширение по умолчанию (без точки). Может быть NULL.
    IN filter           - фильтр. См. OPENFILENAME.lpstrFilter.
    IN filterIndex      - выделеный индекс элемента в filter.

    Return              - true - в случаи успеха,
                          false - в случаи провала.
  */
  bool _browseForSaveFile(HWND owner, LPWSTR initialDir, LPWSTR fileBuffer, LPWSTR defaultExtension, LPWSTR filter, DWORD filterIndex);

  /*
    Инициализация общих контролев (надстройка для InitCommonControlsEx()).

    IN classes - ICC_*.

    Return     - true - в случаи успеха,
                 false - в случаи ошибки.
  */
  bool _loadCommonControl(DWORD classes);
};
