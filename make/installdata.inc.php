<?php
///////////////////////////////////////////////////////////////////////////////////////////////////
// Генерация структуры INSTALLDATA.
///////////////////////////////////////////////////////////////////////////////////////////////////  

/*  
  Генерация структуры BASECONFIG.

  Return - содержимое h-файла.
*/
function generateInstallData()
{
  $vars = array
  (
    "DWORD xorKey;      //Ключ шифрования.",
    "DWORD installSize; //Размер CoreInstall::_install().",
    "DWORD updateSize;  //Размер CoreInstall::_update()."
  );

  //Перемешиваем значения.
  shuffle($vars);

  //Добовляем перед каждым значением структуры, произволный буффер.
  $outputStruct = implode("\r\n  ", $vars);

  //Выводим.
  return "typedef struct\r\n{\r\n  {$outputStruct}\r\n}INSTALLDATA;\r\n";
}
?>