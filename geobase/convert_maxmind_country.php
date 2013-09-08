<?php
/*
  Конвертация MaxMind Country CSV в формат [LowIPv4 HighIPv4 Country[3]]

  http://geolite.maxmind.com/download/geoip/database/GeoIPCountryCSV.zip
*/

error_reporting(E_ALL);
set_time_limit(0);

define('INDEX_IPV4LOW',  2);
define('INDEX_IPV4HIGH', 3);
define('INDEX_COUNTRY',  4);
define('FILE_SOURCE',    'GeoIPCountryWhois.csv');
define('FILE_DEST',      'country[maxmind].txt');

echo 'Converting MaxMind database from file "'.FILE_SOURCE."\".\n";

if(($csv = @fopen(FILE_SOURCE, 'r')) === false)die('Failed to open "'.FILE_SOURCE.'".');

$new = '';

while(($c = fgetcsv($csv)) !== false)
{
  if(!is_array($c) || count($c) != 6 || !is_numeric($c[INDEX_IPV4LOW]) || !is_numeric($c[INDEX_IPV4HIGH]) || strlen($c[INDEX_COUNTRY]) != 2)die("Bad format of table.");
  $new .= $c[INDEX_IPV4LOW]."\0".$c[INDEX_IPV4HIGH]."\0".$c[INDEX_COUNTRY]."\n";
}
fclose($csv);

if(file_put_contents(FILE_DEST, $new) === false)die('Failed to write file "'.FILE_DEST.'".');
die('Database converted to file "'.FILE_DEST.'".');
?>