<?php if(!defined('__CP__'))die();
$os = php_uname('s').' '.php_uname('r').' '.php_uname('v').', '.php_uname('m');
$php = phpversion().', '.php_sapi_name();
$dir = dirname($_SERVER['SCRIPT_FILENAME']);

ThemeBegin(LNG_SYS, 0, 0, 0);
echo
str_replace('{WIDTH}', 'auto', THEME_LIST_BEGIN).

//R'RμSЂSЃReRe RџRћ.
str_replace(array('{COLUMNS_COUNT}', '{TEXT}'), array(2, LNG_SYS_VERSIONS), THEME_LIST_TITLE).
  THEME_LIST_ROW_BEGIN.
    str_replace(array('{WIDTH}', '{TEXT}'), array('auto', 'Operation system:'),                              THEME_LIST_ITEM_LTEXT_U1).
    str_replace(array('{WIDTH}', '{TEXT}'), array('auto', htmlEntitiesEx($os)),                              THEME_LIST_ITEM_LTEXT_U1).
  THEME_LIST_ROW_END.
  THEME_LIST_ROW_BEGIN.
    str_replace(array('{WIDTH}', '{TEXT}'), array('auto', 'Control panel:'),                                 THEME_LIST_ITEM_LTEXT_U2).
    str_replace(array('{WIDTH}', '{TEXT}'), array('auto', htmlEntitiesEx(BO_CLIENT_VERSION)),                THEME_LIST_ITEM_LTEXT_U2).
  THEME_LIST_ROW_END.
  THEME_LIST_ROW_BEGIN.
    str_replace(array('{WIDTH}', '{TEXT}'), array('auto', 'PHP:'),                                           THEME_LIST_ITEM_LTEXT_U1).
    str_replace(array('{WIDTH}', '{TEXT}'), array('auto', htmlEntitiesEx($php)),                             THEME_LIST_ITEM_LTEXT_U1).
  THEME_LIST_ROW_END.
  THEME_LIST_ROW_BEGIN.
    str_replace(array('{WIDTH}', '{TEXT}'), array('auto', 'Zend engine:'),                                   THEME_LIST_ITEM_LTEXT_U2).
    str_replace(array('{WIDTH}', '{TEXT}'), array('auto', htmlEntitiesEx(zend_version())),                   THEME_LIST_ITEM_LTEXT_U2).
  THEME_LIST_ROW_END.
  THEME_LIST_ROW_BEGIN.
    str_replace(array('{WIDTH}', '{TEXT}'), array('auto', 'MySQL server:'),                                  THEME_LIST_ITEM_LTEXT_U1).
    str_replace(array('{WIDTH}', '{TEXT}'), array('auto', htmlEntitiesEx(mysql_get_server_info())),          THEME_LIST_ITEM_LTEXT_U1).
  THEME_LIST_ROW_END.
  THEME_LIST_ROW_BEGIN.
    str_replace(array('{WIDTH}', '{TEXT}'), array('auto', 'MySQL client:'),                                  THEME_LIST_ITEM_LTEXT_U2).
    str_replace(array('{WIDTH}', '{TEXT}'), array('auto', htmlEntitiesEx(mysql_get_client_info())),          THEME_LIST_ITEM_LTEXT_U2).
  THEME_LIST_ROW_END.

//RџSѓS, Fe.
str_replace(array('{COLUMNS_COUNT}', '{TEXT}'), array(2, LNG_SYS_PATHS), THEME_LIST_TITLE).
  THEME_LIST_ROW_BEGIN.
    str_replace(array('{WIDTH}', '{TEXT}'), array('auto', 'Local path:'),                                    THEME_LIST_ITEM_LTEXT_U1).
    str_replace(array('{WIDTH}', '{TEXT}'), array('auto', htmlEntitiesEx($dir)),                             THEME_LIST_ITEM_LTEXT_U1).
  THEME_LIST_ROW_END.
  THEME_LIST_ROW_BEGIN.
    str_replace(array('{WIDTH}', '{TEXT}'), array('auto', 'Reports path:'),                                  THEME_LIST_ITEM_LTEXT_U2).
    str_replace(array('{WIDTH}', '{TEXT}'), array('auto', htmlEntitiesEx($dir.'/'.$config['reports_path'])), THEME_LIST_ITEM_LTEXT_U2).
  THEME_LIST_ROW_END.

//RљR "ReRμRЅS.
str_replace(array('{COLUMNS_COUNT}', '{TEXT}'), array(2, LNG_SYS_CLIENT), THEME_LIST_TITLE).
  THEME_LIST_ROW_BEGIN.
    str_replace(array('{WIDTH}', '{TEXT}'), array('auto', 'User agent:'),                                    THEME_LIST_ITEM_LTEXT_U1).
    str_replace(array('{WIDTH}', '{TEXT}'), array('auto', htmlEntitiesEx($_SERVER['HTTP_USER_AGENT'])),      THEME_LIST_ITEM_LTEXT_U1).
  THEME_LIST_ROW_END.
  THEME_LIST_ROW_BEGIN.
    str_replace(array('{WIDTH}', '{TEXT}'), array('auto', 'IP:'),                                            THEME_LIST_ITEM_LTEXT_U2).
    str_replace(array('{WIDTH}', '{TEXT}'), array('auto', htmlEntitiesEx($_SERVER['REMOTE_ADDR'])),          THEME_LIST_ITEM_LTEXT_U2).
  THEME_LIST_ROW_END.
THEME_LIST_END;
//host

ThemeEnd();
?>