<?php
/*
  Этот файл может свободно модифицироваться.
  Файл содержит код для создания архивного файла.
  
  Архиватор ZIP можно загрузить по адресу http://www.info-zip.org/Zip.html.
  
  ВНИМАНИЕ: Файл находиться в UTF-8 кодировке. Файл всегда должен сохраняться без BOM (заголовок UTF-8).
*/

/*  RЎRѕR · RґR RЅReRμ ° F ° SЂS ... ReRІR °.
  
  IN $ archive - string, RїRѕR "RЅS <P № RїSѓS, SЊ RїRѕ RєRѕS, RѕSЂRѕRјSѓ RґRѕR" P ¶ RμRЅ P ± C <, SЊ SЃRѕR · RґR RЅ ° F ° SЂS ... ReRІ (P ± RμR · SЂR ° SЃS € ReSЂRμRЅReSЏ).
  IN $ files - array, SЃRїReSЃRѕRє C "P ° P № R" RѕRІ RґR "SЏ RґRѕR ± P ° RІR" RμRЅReSЏ RІ P ° SЂS ... ReRІ.
  
  Return - mixed, ReRјSЏ P ° SЂS ... ReRІR ° - RІ SЃR "SѓS ‡ P ° Fe SѓSЃRїRμS € RЅRѕRіRѕ SЃRѕR · RґR RЅReSЏ ° F ° SЂS ... ReRІR °, false - RІ SЃR" SѓS ‡ P ° Fe RѕS € Pepsi ± RєRe .*/
function fsarcCreate($archive, $files)
{
  error_reporting(E_ALL);
  if(strcasecmp(substr(php_uname('s'), 0, 7), 'windows') === 0)
  {
    $archive = str_replace('/', '\\', $archive);
    foreach($files as $k => $v)$files[$k] = str_replace('/', '\\', $v);
  }
  
  $archive .= '.zip';
  $cli = 'zip -r -9 -q -S "'.$archive.'" "'.implode('" "', $files).'"';
  exec($cli, $e, $r);
  
  if($r != 0)echo "(error: $r) ".$cli.'<br/>';  
  return $r ? false : $archive;
}
?>