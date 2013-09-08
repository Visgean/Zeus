<?php if(!defined('__CP__'))die();

define('INPUT_WIDTH', '180px');
define('DEFAULT_SS_TYPE', 'jpeg');

$screenshotTypes = array('jpeg' => 'jpeg', 'gif' => 'gif', 'png' => 'png');
$llist           = array('en' => 'English', 'ru' => 'Russian (Русский)');
$errors          = array();

///////////////////////////////////////////////////////////////////////////////////////////////////
// Обработка данных.
///////////////////////////////////////////////////////////////////////////////////////////////////
$q = '';

//Обработка основных данных.
$language = strtolower(isset($_POST['language'], $llist[$_POST['language']]) ? $_POST['language'] : $userData['language']);
if(!isset($llist[$language]))$language = DEFAULT_LANGUAGE;
$q .= "language='".addslashes($language)."'";

$screenshotFormat = strtolower(isset($_POST['screenshotFormat'], $screenshotTypes[$_POST['screenshotFormat']]) ? $_POST['screenshotFormat'] : $userData['ss_format']);
if(!isset($screenshotTypes[$screenshotFormat]))$screenshotFormat = DEFAULT_SS_TYPE;
$q .= ", ss_format='".addslashes($screenshotFormat)."'";

$screenshotQuality = isset($_POST['screenshotQuality']) ? intval($_POST['screenshotQuality']) : $userData['ss_quality'];
if($screenshotQuality <= 0 || $screenshotQuality > 100)$screenshotQuality = 30;
$q .= ", ss_quality='".addslashes($screenshotQuality)."'";

//Обработка пароля.
if(isset($_POST['passold'], $_POST['pass1'], $_POST['pass2']) && strlen($_POST['passold']) > 0 && ($l = strlen($_POST['pass1'])) > 0)
{
  if(strcasecmp(md5($_POST['passold']), $userData['pass']) !== 0)$errors[] = LNG_SYS_PASSWORD_E1;
  else if(strcmp($_POST['pass1'], $_POST['pass2']) !== 0)$errors[] = LNG_SYS_PASSWORD_E2;
  else if($l < 6 || $l > 64)$errors[] = sprintf(LNG_SYS_PASSWORD_E3, 6, 64);
  else $q .= ", pass='".addslashes(md5($_POST['pass1']))."'";
}

//Сохранение параметров.
if(strcmp($_SERVER['REQUEST_METHOD'], 'POST') === 0 && count($errors) == 0)
{
  if(!mysqlQueryEx('cp_users', "UPDATE cp_users SET {$q} WHERE id='{$userData['id']}' LIMIT 1"))$errors[] = mysqlErrorEx();
  else
  {
    header('Location: '.QUERY_STRING.'&u=1');
    die();
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Вывод.
///////////////////////////////////////////////////////////////////////////////////////////////////

//Заполнение списков.
$languages = '';
foreach($llist as $k => $v)
{
  $languages .= str_replace(array('{VALUE}', '{TEXT}'), array($k, htmlEntitiesEx($v)), strcasecmp($language, $k) === 0 ? THEME_DIALOG_ITEM_LISTBOX_ITEM_CUR : THEME_DIALOG_ITEM_LISTBOX_ITEM);
}

$screenshotFormats = '';
foreach($screenshotTypes as $k => $v)
{
  $screenshotFormats .= str_replace(array('{VALUE}', '{TEXT}'), array($k, htmlEntitiesEx($v)), strcasecmp($screenshotFormat, $k) === 0 ? THEME_DIALOG_ITEM_LISTBOX_ITEM_CUR : THEME_DIALOG_ITEM_LISTBOX_ITEM);
}

$screenshotQualities = '';
for($i = 5; $i <= 100; $i += 5)
{
  $screenshotQualities .= str_replace(array('{VALUE}', '{TEXT}'), array($i, $i.'%'), ($screenshotQuality >= $i &&  $screenshotQuality <= $i) ? THEME_DIALOG_ITEM_LISTBOX_ITEM_CUR : THEME_DIALOG_ITEM_LISTBOX_ITEM);
}

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
      str_replace(array('{COLUMNS_COUNT}', '{TEXT}'), array(2, LNG_SYS_MAIN), THEME_DIALOG_TITLE).
      THEME_DIALOG_ROW_BEGIN.
        str_replace('{TEXT}', LNG_SYS_LANGUAGE, THEME_DIALOG_ITEM_TEXT).
        str_replace('{COLUMNS_COUNT}', 1, THEME_DIALOG_ITEM_CHILD_BEGIN).
          str_replace(array('{NAME}', '{WIDTH}'), array('language', INPUT_WIDTH), THEME_DIALOG_ITEM_LISTBOX_BEGIN).
            $languages.
          THEME_DIALOG_ITEM_LISTBOX_END.
        THEME_DIALOG_ITEM_CHILD_END.     
      THEME_DIALOG_ROW_END.
      str_replace(array('{COLUMNS_COUNT}', '{TEXT}'), array(2, LNG_SYS_SS), THEME_DIALOG_TITLE).
      THEME_DIALOG_ROW_BEGIN.
        str_replace('{TEXT}', LNG_SYS_SS_FORMAT, THEME_DIALOG_ITEM_TEXT).
        str_replace('{COLUMNS_COUNT}', 1, THEME_DIALOG_ITEM_CHILD_BEGIN).
          str_replace(array('{NAME}', '{WIDTH}'), array('screenshotFormat', INPUT_WIDTH), THEME_DIALOG_ITEM_LISTBOX_BEGIN).
            $screenshotFormats.
          THEME_DIALOG_ITEM_LISTBOX_END.
        THEME_DIALOG_ITEM_CHILD_END.     
      THEME_DIALOG_ROW_END.
      THEME_DIALOG_ROW_BEGIN.
        str_replace('{TEXT}', LNG_SYS_SS_QUALITY, THEME_DIALOG_ITEM_TEXT).
        str_replace('{COLUMNS_COUNT}', 1, THEME_DIALOG_ITEM_CHILD_BEGIN).
          str_replace(array('{NAME}', '{WIDTH}'), array('screenshotQuality', INPUT_WIDTH), THEME_DIALOG_ITEM_LISTBOX_BEGIN).
            $screenshotQualities.
          THEME_DIALOG_ITEM_LISTBOX_END.
        THEME_DIALOG_ITEM_CHILD_END.     
      THEME_DIALOG_ROW_END.
    THEME_DIALOG_GROUP_END.
    str_replace('{COLUMNS_COUNT}', 1, THEME_DIALOG_GROUP_BEGIN).
      str_replace(array('{COLUMNS_COUNT}', '{TEXT}'), array(2, LNG_SYS_PASSWORD), THEME_DIALOG_TITLE).
      THEME_DIALOG_ROW_BEGIN.
        str_replace('{TEXT}', LNG_SYS_PASSWORD_OLD, THEME_DIALOG_ITEM_TEXT).
        str_replace(array('{NAME}', '{VALUE}', '{MAX}', '{WIDTH}'), array('passold', '', 100, INPUT_WIDTH), THEME_DIALOG_ITEM_INPUT_PASSWORD).
      THEME_DIALOG_ROW_END.
      THEME_DIALOG_ROW_BEGIN.
        str_replace('{TEXT}', LNG_SYS_PASSWORD_NEW1, THEME_DIALOG_ITEM_TEXT).
        str_replace(array('{NAME}', '{VALUE}', '{MAX}', '{WIDTH}'), array('pass1', '', 64, INPUT_WIDTH), THEME_DIALOG_ITEM_INPUT_PASSWORD).
      THEME_DIALOG_ROW_END.
      THEME_DIALOG_ROW_BEGIN.
        str_replace('{TEXT}', LNG_SYS_PASSWORD_NEW2, THEME_DIALOG_ITEM_TEXT).
        str_replace(array('{NAME}', '{VALUE}', '{MAX}', '{WIDTH}'), array('pass2', '', 64, INPUT_WIDTH), THEME_DIALOG_ITEM_INPUT_PASSWORD).
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