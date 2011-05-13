#pragma once

//На всякий случай, для надежности BO_*.
#include "../common/config.h"

namespace Languages
{
  typedef struct
  {
    const LPWSTR name;     //Имя
    const LPWSTR *strings; //Список строк.
    WORD stringsCount;     //Кол. строк.
    WORD id;               //ID языка.
  }LANGINFO;

  //ID строк.
  enum
  {
    error_not_enough_memory,
    error_fwrite_failed,
    error_output_template,

    main_title,

    tool_title_info,
#if(BO_CLIENT_PLATFORMS > 0)
    tool_title_builder,
#endif
    tool_title_settings,

    tool_info_version_titile,
    tool_info_version,
    tool_info_bot_title,
    tool_info_bot_key_title,
    tool_info_bot_info_title,
    tool_info_bot_refresh,
    tool_info_bot_remove,
    tool_info_bot_removing,
    tool_info_bot_removed,
    tool_info_bot_not_removed,
    tool_info_bot_default_botnet,
    tool_info_bot_status,
    tool_info_bot_status_na,

#if(BO_CLIENT_PLATFORMS > 0)
    tool_builder_source_title,
    tool_builder_source_browse,
    tool_builder_source_edit,
    tool_builder_build_title,
    tool_builder_build_config,
    tool_builder_build_bot,
    tool_builder_source_not_defined,
    tool_builder_source_edit_failed,
    tool_builder_source_fopen_failed,

    builder_bot_proc_begin,
    builder_bot_proc_config,
    builder_bot_proc_creating,
    builder_bot_proc_output_info,
    builder_bot_proc_end,
    
    builder_config_begin,
    builder_config_end,
    builder_staticconfig_not_founded,
    builder_staticconfig_botnet_bad,
    builder_staticconfig_url_config_not_founded,
    builder_staticconfig_url_config_bad,
    builder_staticconfig_encryption_key_not_founded,
    builder_bot_corrupted,
    builder_dynamicconfig_not_founded,
    builder_dynamicconfig_url_loader_not_founded,
    builder_dynamicconfig_url_server_not_founded,
    builder_dynamicconfig_advancedconfigs_error,
#if(BO_WININET > 0)
    builder_dynamicconfig_webfilters_error,
    builder_dynamicconfig_webdatafilters_error,
#endif
    builder_dynamicconfig_webfakes_error,
    builder_dynamicconfig_file_webinjects_error,
    builder_httpinjects_begin,
    builder_httpinjects_fopen_failed,
    builder_httpinjects_bad_format,
#endif

    tool_settings_language_title,
    tool_settings_apply,
    tool_settings_language_saved,
    tool_settings_apply_failed,
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
    Получение строки для id.

    IN id  - ID.

    Return - строка, или NULL.
  */
  LPWSTR get(DWORD id);

  /*
    Сохраняет код языка в качастве языка по умолчанию.

    IN langId - код языка.

    Return    - true - если успешно сохранено,
                false - в случаи ошибки.
  */
  bool setDefaultLangId(WORD langId);

  /*
    Получение информации о языке.

    IN index - индекс языка.

    Return   - информация о языке, или NULL если index привысил кол. языков.
  */
  const LANGINFO *getLangInfo(WORD index);

  /*
    Получение информации о текущем языке.

    Return - язык.
  */
  const Languages::LANGINFO *getCurLangInfo(void);
};
