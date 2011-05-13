<?php if(!defined('__CP__'))die();

define('INPUT_WIDTH', '300px');
$errors   = array();

///////////////////////////////////////////////////////////////////////////////////////////////////
// Обработка данных.
///////////////////////////////////////////////////////////////////////////////////////////////////

$is_post = strcmp($_SERVER['REQUEST_METHOD'], 'POST') === 0 ? true : false;

if(!isset($_POST['reports_path']))$reports_path = $config['reports_path'];
else
{
  if(($l = strlen($_POST['reports_path'])) < 1 || $l > 200)$errors[] = LNG_SYS_E1;
  $reports_path = $_POST['reports_path'];
}
$reports_path = trim(str_replace('\\', '/', trim($reports_path)), '/');

$reports_to_db = (isset($_POST['reports_to_db']) && $_POST['reports_to_db'] == 1) ? 1 : ($is_post ? 0 : $config['reports_to_db']);
$reports_to_fs = (isset($_POST['reports_to_fs']) && $_POST['reports_to_fs'] == 1) ? 1 : ($is_post ? 0 : $config['reports_to_fs']);

if(isset($_POST['botnet_timeout']) && is_numeric($_POST['botnet_timeout']))$botnet_timeout = (int)intval($_POST['botnet_timeout']);
else $botnet_timeout = (int)($config['botnet_timeout'] / 60);
if($botnet_timeout < 1 || $botnet_timeout > 9999)$errors[] = LNG_SYS_E2;

if(!isset($_POST['botnet_cryptkey']))$botnet_cryptkey = $config['botnet_cryptkey'];
else
{
  if(($l = strlen($_POST['botnet_cryptkey'])) < 1 || $l > 255)$errors[] = LNG_SYS_E3;
  $botnet_cryptkey = $_POST['botnet_cryptkey'];
}

//Сохранение параметров.
if($is_post && count($errors) == 0)
{
  $updateList['reports_path']    = $reports_path;
  $updateList['reports_to_db']   = $reports_to_db ? 1 : 0;
  $updateList['reports_to_fs']   = $reports_to_fs ? 1 : 0;
  $updateList['botnet_timeout']  = $botnet_timeout * 60;
  $updateList['botnet_cryptkey'] = $botnet_cryptkey;
  
  if(!updateConfig($updateList))$errors[] = LNG_SYS_E4;
  else
  {
    createDir($reports_path);
    header('Location: '.QUERY_STRING.'&u=1');
    die();
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Вывод.
///////////////////////////////////////////////////////////////////////////////////////////////////

ThemeBegin(LNG_SYS, 0, 0, 0);

//Вывод ошибок.
if(count($errors) > 0)
{
  echo THEME_STRING_FORM_ERROR_1_BEGIN;
  foreach($errors as $r)echo $r.THEME_STRING_NEWLINE;
  echo THEME_STRING_FORM_ERROR_1_END;
}
//Вывод сообщений.
else if(isset($_GET['u']))
{
  echo THEME_STRING_FORM_SUCCESS_1_BEGIN.LNG_SYS_UPDATED.THEME_STRING_NEWLINE.THEME_STRING_FORM_SUCCESS_1_END;
}

//Вывод формы.
echo
str_replace(array('{NAME}', '{URL}', '{JS_EVENTS}'), array('options', QUERY_STRING_HTML, ''), THEME_FORMPOST_BEGIN),
str_replace('{WIDTH}', 'auto', THEME_DIALOG_BEGIN).
  THEME_DIALOG_ROW_BEGIN.
    str_replace('{COLUMNS_COUNT}', 1, THEME_DIALOG_GROUP_BEGIN).  
      str_replace(array('{COLUMNS_COUNT}', '{TEXT}'), array(2, LNG_SYS_REPORTS), THEME_DIALOG_TITLE).
      THEME_DIALOG_ROW_BEGIN.
        str_replace('{TEXT}', LNG_SYS_REPORTS_PATH, THEME_DIALOG_ITEM_TEXT).
        str_replace(array('{NAME}', '{VALUE}', '{MAX}', '{WIDTH}'), array('reports_path', htmlEntitiesEx($reports_path), 200, INPUT_WIDTH), THEME_DIALOG_ITEM_INPUT_TEXT).
      THEME_DIALOG_ROW_END.
      THEME_DIALOG_ROW_BEGIN.
        str_replace('{TEXT}', THEME_STRING_SPACE, THEME_DIALOG_ITEM_TEXT).
        str_replace(array('{COLUMNS_COUNT}', '{NAME}', '{VALUE}', '{JS_EVENTS}', '{TEXT}'), array(1, 'reports_to_db', 1, '', LNG_SYS_REPORTS_TODB), $reports_to_db ? THEME_DIALOG_ITEM_INPUT_CHECKBOX_ON_2 : THEME_DIALOG_ITEM_INPUT_CHECKBOX_2).
      THEME_DIALOG_ROW_END.
      THEME_DIALOG_ROW_BEGIN.
        str_replace('{TEXT}', THEME_STRING_SPACE, THEME_DIALOG_ITEM_TEXT).
        str_replace(array('{COLUMNS_COUNT}', '{NAME}', '{VALUE}', '{JS_EVENTS}', '{TEXT}'), array(1, 'reports_to_fs', 1, '', LNG_SYS_REPORTS_TOFS), $reports_to_fs ? THEME_DIALOG_ITEM_INPUT_CHECKBOX_ON_2 : THEME_DIALOG_ITEM_INPUT_CHECKBOX_2).
      THEME_DIALOG_ROW_END.
      str_replace(array('{COLUMNS_COUNT}', '{TEXT}'), array(2, LNG_SYS_BOTNET), THEME_DIALOG_TITLE).
      THEME_DIALOG_ROW_BEGIN.
        str_replace('{TEXT}', LNG_SYS_BOTNET_TIMEOUT, THEME_DIALOG_ITEM_TEXT).
        str_replace(array('{NAME}', '{VALUE}', '{MAX}', '{WIDTH}'), array('botnet_timeout', htmlEntitiesEx($botnet_timeout), 4, INPUT_WIDTH), THEME_DIALOG_ITEM_INPUT_TEXT).
      THEME_DIALOG_ROW_END.
      THEME_DIALOG_ROW_BEGIN.
        str_replace('{TEXT}', LNG_SYS_BOTNET_CRYPTKEY, THEME_DIALOG_ITEM_TEXT).
        str_replace(array('{NAME}', '{VALUE}', '{MAX}', '{WIDTH}'), array('botnet_cryptkey', htmlEntitiesEx($botnet_cryptkey), 255, INPUT_WIDTH), THEME_DIALOG_ITEM_INPUT_TEXT).
      THEME_DIALOG_ROW_END.
    THEME_DIALOG_GROUP_END.
  THEME_DIALOG_ROW_END.
  str_replace('{COLUMNS_COUNT}', 2, THEME_DIALOG_ACTIONLIST_BEGIN).
    str_replace(array('{TEXT}', '{JS_EVENTS}'), array(LNG_SYS_SAVE, ''), THEME_DIALOG_ITEM_ACTION_SUBMIT).
  THEME_DIALOG_ACTIONLIST_END.
THEME_DIALOG_END.
THEME_FORMPOST_END;

ThemeEnd();
?>