/*
  Парсер PE файлов. 

  FIXME: Потеряна связь и логика меджду функциями.


*/
#pragma once

#if !defined PEIMAGE_32
#  error PEIMAGE_32 not defined!
#endif

#if !defined PEIMAGE_64
#  error PEIMAGE_64 not defined!
#endif

namespace PeImage
{
  enum
  {
    MAX_FILE_ALIGMENT    = (64 * 1024), //Макс. выравнивание для FileAligment.
    MIN_FILE_ALIGMENT    = 512,         //Мин. выравнивание для FileAligment.
    MAX_VIRTUAL_ALIGMENT = (64 * 1024), //Макс. выравнивание для SectionAligment.
    MIN_VIRTUAL_ALIGMENT = 512,         //Мин. выравнивание для SectionAligment.
  };
  
  //Заголовок для VS_VERSION_INFO. На основе VS_VERSIONINFO.
  typedef struct
  {
    WORD length;
    WORD valueLength;
    WORD type;
    WCHAR key[16];
    VS_FIXEDFILEINFO vsf;
  }VERSIONINFO_HEADER;

  typedef struct
  {
    WORD length; 
    WORD valueLength; 
    WORD type; 
    WCHAR key[1]; 
  }VERSIONINFO_VAR;

  //Стурктура для хранения данных о файле.
  typedef struct
  {
    WORD machine;                     //Тип файла. IMAGE_FILE_MACHINE_I386/IMAGE_FILE_MACHINE_AMD64.
    IMAGE_DOS_HEADER *dosHeader;      //DOS-заголовок.
    union
    {
#     if(PEIMAGE_32 > 0)
      IMAGE_NT_HEADERS32 *p32;         //NT32-заголовок.
#     endif
#     if(PEIMAGE_64 > 0)
      IMAGE_NT_HEADERS64 *p64;         //NT64-заголовок.
#     endif
    }ntHeader;

    IMAGE_DATA_DIRECTORY dataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES]; //..
    
    WORD sectionsCount;              //Количество секций
    IMAGE_SECTION_HEADER *sections;  //Секции.
    LPBYTE *sectionsRawData;         //Содержимое секции.
    DWORD *sectionsFlags;            //Пользовательские флаги секций, самим PeImage не используются.
    DWORD minimalRawOffsetOfSection; //Минимальная RAW позиция первой секции в образе. 0 или
                                     //кратен FileAlignment.
  }PEDATA;

  //Базавые данные для NT-Заголовка. Все переменные соответвуют соответвенным переменным из
  //IMAGE_NT_HEADERS
  typedef struct
  {
    DWORD   timeDateStamp;
    WORD    characteristics;
    BYTE    majorLinkerVersion;
    BYTE    minorLinkerVersion;
    DWORD64 imageBase;
    DWORD   sectionAlignment;
    DWORD   fileAlignment;
    WORD    majorOperatingSystemVersion;
    WORD    minorOperatingSystemVersion;
    WORD    majorImageVersion;
    WORD    minorImageVersion;
    WORD    majorSubsystemVersion;
    WORD    minorSubsystemVersion;
    WORD    subsystem;
    WORD    dllCharacteristics;
    DWORD64 sizeOfStackReserve;
    DWORD64 sizeOfStackCommit;
    DWORD64 sizeOfHeapReserve;
    DWORD64 sizeOfHeapCommit;
  }PENTBASEDATA;
  
  enum
  {
    BIF_CHECKSUM      = 0x1, //Подсчитать ческсумму.
    BIF_NO_RECALC_RVA = 0x2, //Не пересчитывать виртуальные адреса, и все данные связаные с ними.
                             //Использование этого флага может привести к порче образа, если 
                             //добавлены новые секции или данные в сушетвующие секции.
  };

  /*
    Инициализация.
  */
  void init(void);

  /*
    Деинициализация.
  */
  void uninit(void);

  /*
    Создание пустого образа.

    OUT pedata - структура для хранения образа.
    IN machine - тип платформы. IMAGE_FILE_MACHINE_I386 или IMAGE_FILE_MACHINE_AMD64.
    
    Return     - true  - в случаи успеха,
                 false - в случаи ошибки.
  */
  bool _createEmpty(PEDATA *pedata, WORD machine);

  /*
    Создание образа из памяти. Данные проверяются на ошибки.

    OUT pedata  - структура для хранения образа.
    IN mem      - указатель на память.
    IN memSize  - размер памяти, можно указать MAXDWORD, но если образ иммет ошибки, это может
                  привести к исключению.
    IN isModule - содержимое памяти это загруженый модуль.

    Return      - адрес конца образа - в случаи успеха,
                  NULL - в случаи ошибки.
  */
  void *_createFromMemory(PEDATA *pedata, void *mem, DWORD memSize, bool isModule);

  /*
    Освобождение образа.

    IN pedata - образ для освобождения.
  */
  void _freeImage(PEDATA *pedata);

  /*
    Устанавливает DOS-заголовок для образа.

    IN OUT pedata - образ для обработки.
    IN dosHeader  - DOS-заголовок, правильно должны быть указаны параметры e_magic и e_lfanew.

    Return        - true  - в случаи успеха,
                    false - в случаи ошибки.
  */
  bool _setDosHeader(PEDATA *pedata, void *dosHeader);

  /*
    Устанавливает NT-заголовок и задает базовые параметры.

    IN OUT pedata - образ для обработки.
    IN basedata   - базовые данные.

    Return        - true  - в случаи успеха,
                    false - в случаи ошибки.    
  */
  bool _setNtHeader(PEDATA *pedata, PENTBASEDATA *basedata);

  /*
    Устанавливает NT-заголовок полнсотью копирую оригинальный заголовок. Также заполняет
    PEDATA.DataDirectory.

    IN OUT pedata - образ для обработки.
    IN ntHeader   - указатель на IMAGE_NT_HEADERS32 или IMAGE_NT_HEADERS64. Заголовок должен быть
                    корректным.

    Return        - true  - в случаи успеха,
                    false - в случаи ошибки.    
  */
  bool _setNtHeaderFromNtHeader(PEDATA *pedata, void *ntHeader);

  /*
    Добавление секции.

    IN OUT pedata      - образ для обработки.
    IN name            - название секции, не более IMAGE_SIZEOF_SHORT_NAME символов.
    IN characteristics - аналог IMAGE_SECTION_HEADER.Characteristics.
    IN data            - содержимое секции.
    IN dataOffset      - позиция данные в RAW (PointerToRawData). Не имеет смысла для _buildImage().
    IN dataSize        - размер pData. Автоматически выравниется до FileAligment при сборке.
    IN virtualAddress  - RVA секции, на этапе сборки не имеет смысла по скольку будет пересчитано.
    IN virtualDataSize - виртуальный размер секции. Любое число, может быть не выровнено по 
                         SectionAligment. По нему опередляется SizeOfInitializedData и 
                         SizeOfUninitializedData.
    IN flags           - пользовательские флаги секций.
    
    Return             - true  - в случаи успеха,
                         false - в случаи ошибки.
  */
  bool _addSection(PEDATA *pedata, LPSTR name, DWORD characteristics, void *data, DWORD dataOffset, DWORD dataSize, DWORD virtualAddress, DWORD virtualDataSize, DWORD flags);

  /*
    Получение индекса секции по ее имени.

    IN pedata - образ.
    IN name   - имя секции с учетом регистра.

    Return    - -1 - если секция не найдена, или индекс в случаи успеха.
  */
  int _getSectionIndexByName(PEDATA *pedata, LPSTR name);
  
  /*
    Установка данных для DataDirectory.

    IN OUT pedata     - образ для обработки.
    IN index          - одно из значений IMAGE_DIRECTORY_ENTRY_*.
    IN virtualAddress - RVA-адрес.
    IN size           - виртуальный размер.

    Return            - true  - в случаи успеха,
                        false - в случаи ошибки.
  */
  bool _setDataDirectory(PEDATA *pedata, BYTE index, DWORD virtualAddress, DWORD size);

  /*
    Сборка образа. Способ сборки максимально приблизин к способу MS Linker 9.0.

    IN pedata          - образ для обработки.
    IN flags           - флаги BIF_*.
    IN rvaOfEntryPoint - RVA точки входа. Если 0, значение берется из pedata.
    OUT output         - буфер для собраного образа. Если равно NULL, обновляются все внутрении
                         данные pedata (размер, RVA секций и т.д.).

    Return             - 0 - в случаи ошибки, или размер образа в случаи успеха.
  */
  DWORD _buildImage(PEDATA *pedata, DWORD flags, DWORD rvaOfEntryPoint, LPBYTE *output);
  
  /*
    Подсчитывает RVA следующей секции, без учета IMAGE_SECTION_HEADER.PointerToRawData.

    IN pedata        - образ для обработки.
    IN sectionsCount - предположительное количетсво секций в образе, важно ТОЧНО указывать
                         это значение.

    Return           - RVA секции в случаи успеха,
                       0 - в случаи ошибки.
  */
  DWORD _calcNextSectionRva(PEDATA *pedata, DWORD sectionsCount);

  /*
    Подсчет текушего RAW размера образа.
    
    IN pedata - образ для обработки.

    Retunrn   - размер образа.
  */
  DWORD _getCurrentRawSize(PEDATA *pedata);

  /*
    Подсчет RAW размера образа.
    
    IN image - образ для обработки. Корректность образа не проверяется.

    Retunrn  - размер образа.
  */
  DWORD _getRawSize(const void *image);
  
  /*
    Подсчет текушего Virtual размера образа, без учета IMAGE_SECTION_HEADER.VirtualAddress.

    IN pedata - образ для обработки.

    Retunrn   - размер образа.
  */
  DWORD _getCurrentVirtualSize(PEDATA *pedata);

  /*
    Получение секции из загруженого модуля.
  
    IN hModule      - модуль.
    IN wIndex       - индекс секции.
    OUT virtualSize - виртуальный размер секции, может быть NULL.

    Return          - NULL - в случаи ошибки, или адрес секции в случаи успеха.

    Внимание: Корректность и доступность модуля не проверяется.
  */
  void *_getSectionOfModule(HMODULE module, WORD index, LPDWORD virtualSize);

  /*
    Получение заголовка секции по ее имени.

    IN image       - образ (RAW/VA).
    IN sectionName - имя секции для поиска.

    Return         - адрес заголовка секции, или NULL - в случаи ошибки.
  */
  IMAGE_SECTION_HEADER *_getSectionByName(const void *image, const LPSTR sectionName);

  /*
    Копирование образа в другой процесс с приминением релоков. Функция облегчена, и не учитывает
    прав страниц, так же все исходны страницы должны быть доступны для чтенния, иначе функция
    вернет ошибку.

    IN process - процесс, в который будет производиться копирование. Нужны права
                 PROCESS_VM_OPERATION | PROCESS_VM_WRITE
    IN image   - образ для копирования, в текушем процессе.

    Return     - адрес образа в процессе process, или NULL в случаи ошибки.
  */
  void *_copyModuleToProcess(HANDLE process, void *image);

  /*
    Загрузка таблицы импорта.

    IN image          - образ, в котором будут производиться изменения.
    IN loadLibraryA   - указатель на функцию LoadLibraryA.
    IN getProcAddress - указатель на функцию GetProcAddress.

    Return            - true  - в случаи успеха,
                        false - в случаи ошибки.
  */
  bool _loadImport(void *image, void *loadLibraryA, void *getProcAddress);

  /*
    Замена адреса в таблице импорта.

    IN image       - образ, в котором будут производиться изменения.
    IN oldFunction - адрес текущей функции.
    IN newFunction - адрес новой функции.

    Return         - true  - в случаи успеха,
                     false - в случаи ошибки.       
  */
  bool _repalceImportFunction(void *image, const void *oldFunction, const void *newFunction);

  /*
    Ковертация RVA в RSA.

    IN pedata       - образ для обработки.
    IN rva          - RVA.
    IN sectionIndex - индекс секции, которой принадлежат адреса. Или NULL если секция не
                      требуется.

    Return          - адрес относительно секции dwSectionIndex, или 0 в случаи ошибки.
  */
  DWORD _rvaToRsa(PEDATA *pedata, DWORD rva, LPWORD sectionIndex);

  /*
    Проверяет является ли образ корректным.

    IN mem     - образ.
    IN memSize - размер образа.

    Return     - true  - в случаи успеха,
                 false - в случаи ошибки.       
  */
  bool _isPeImage(void *mem, DWORD memSize);
};
