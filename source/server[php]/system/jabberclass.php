<?php
class Jabber
{
  var $server;
  var $port;
  var $username;
  var $password;
  var $resource;
  var $jid;
  var $streamId;
  var $packetQueue;
  var $connector;
  var $activeSocket;

  function Jabber()
  {
    $this->server = '127.0.0.1';
    $this->port   = 5222;

    $this->username = '';
    $this->password = '';
    $this->resource = NULL;

    $this->packetQueue = array();
  }

  function openSocket($server, $port)
  {
    if($this->activeSocket = fsockopen($server, $port))
    {
      socket_set_blocking($this->activeSocket, 0);
      socket_set_timeout($this->activeSocket, 31536000);
      return true;
    }
    return false;
  }

  function closeSocket()
  {
    return fclose($this->activeSocket);
  }

  function writeToSocket($data)
  {
    return fwrite($this->activeSocket, $data);
  }

  function readFromSocket($chunksize)
  {
    return fread($this->activeSocket, $chunksize);
  }

  function connect()
  {
    if($this->openSocket($this->server, $this->port))
    {
      $this->sendPacket("<?xml version='1.0' encoding='UTF-8' ?".">\n");
      $this->sendPacket("<stream:stream to='{$this->server}' xmlns='jabber:client' xmlns:stream='http://etherx.jabber.org/streams'>\n");
      sleep(2);
      if($this->checkConnected())return true;
    }
    return false;
  }

  function disconnect()
  {
    sleep(1);
    $this->sendPacket("</stream:stream>");
    $this->closeSocket();
  }

  function sendAuth()
  {
    $this->authId   = "auth_".md5(time());
    $this->resource = ($this->resource != NULL) ? $this->resource : ("Class.Jabber.PHP ".md5($this->authId));
    $this->jid      = "{$this->username}@{$this->server}/{$this->resource}";

    $payload = "<username>{$this->username}</username>";
    $packet  = $this->sendIq(NULL, 'get', $this->authId, "jabber:iq:auth", $payload);

    if($this->getInfoFromIqType($packet) == 'result' && $this->getInfoFromIqId($packet) == $this->authId)
    {
      if(function_exists('mhash') && isset($packet['iq']['#']['query'][0]['#']['sequence'][0]["#"]) && isset($packet['iq']['#']['query'][0]['#']['token'][0]["#"]))
      {
        return $this->sendAuth0k($packet['iq']['#']['query'][0]['#']['token'][0]["#"], $packet['iq']['#']['query'][0]['#']['sequence'][0]["#"]);
      }
      else if(function_exists('mhash') && isset($packet['iq']['#']['query'][0]['#']['digest']))
      {
        $payload = "<username>{$this->username}</username><resource>{$this->resource}</resource><digest>".bin2hex(mhash(MHASH_SHA1, $this->streamId.$this->password))."</digest>";
        $packet = $this->sendIq(NULL, 'set', $this->authId, "jabber:iq:auth", $payload);
        if($this->getInfoFromIqType($packet) == 'result' && $this->getInfoFromIqId($packet) == $this->authId)return true;
      }
      else if($packet['iq']['#']['query'][0]['#']['password'])
      {
        $payload = "<username>{$this->username}</username><password>{$this->password}</password><resource>{$this->resource}</resource>";
        $packet = $this->sendIq(NULL, 'set', $this->authId, "jabber:iq:auth", $payload);
        if($this->getInfoFromIqType($packet) == 'result' && $this->getInfoFromIqId($packet) == $this->authId)return true;
      }
    }
    return false;
  }

  function sendPacket($xml)
  {
    return $this->writeToSocket(trim($xml));
  }

  function listen()
  {
    $incoming = '';
    while($line = $this->readFromSocket(4096))$incoming .= $line;
    $incoming = trim($incoming);
    if($incoming != '')
    {
      $temp = $this->splitIncoming($incoming);
      for($a = 0; $a < count($temp); $a++)$this->packetQueue[] = $this->xmlize($temp[$a]);
    }
    return true;
  }

  function sendMessage($to, $type = "normal", $id = NULL, $content = NULL, $payload = NULL)
  {
    if($to && is_array($content))
    {
      if(!$id)$id = $type.'_'.time();

      $content = $this->arrayHtmlSpecialChars($content);
      $xml = "<message to='$to' type='$type' id='$id'>\n";
      if(!empty($content['subject']))$xml .= "<subject>".$content['subject']."</subject>\n";
      if(!empty($content['thread']))$xml .= "<thread>".$content['thread']."</thread>\n";

      $xml .= "<body>{$content['body']}</body>\n{$payload}</message>\n";

      if($this->sendPacket($xml))return true;
    }
    return false;
  }

  function sendPresence($type = NULL, $to = NULL, $status = NULL, $show = NULL, $priority = NULL)
  {
    $xml = "<presence";
    $xml .= ($to) ? " to='$to'" : '';
    $xml .= ($type) ? " type='$type'" : '';
    $xml .= ($status || $show || $priority) ? ">\n" : " />\n";

    $xml .= ($status) ? " <status>$status</status>\n" : '';
    $xml .= ($show) ? " <show>$show</show>\n" : '';
    $xml .= ($priority) ? " <priority>$priority</priority>\n" : '';

    $xml .= ($status || $show || $priority) ? "</presence>\n" : '';

    return $this->sendPacket($xml);
  }

  function getFromQueueById($packetType, $id)
  {
    $foundMessage = false;
    foreach ($this->packetQueue as $key => $value)if($value[$packetType]['@']['id'] == $id)
    {
      $foundMessage = $value;
      unset($this->packetQueue[$key]);
      break;
    }
    return is_array($foundMessage) ? $foundMessage : false;
  }

  function sendIq($to = NULL, $type = 'get', $id = NULL, $xmlns = NULL, $payload = NULL, $from = NULL)
  {
    if(!preg_match("/^(get|set|result|error)$/", $type))unset($type);
    else if($id && $xmlns)
    {
      $xml = "<iq type='{$type}' id='{$id}'";
      if($to)$xml .= " to='{$to}'";
      if($from)$xml .= " from='{$from}'";
      $xml .= "><query xmlns='$xmlns'>$payload</query></iq>";

      $this->sendPacket($xml);
      sleep(1);
      $this->listen();

      return (preg_match("/^(get|set)$/", $type)) ? $this->getFromQueueById("iq", $id) : true;
    }
    return false;
  }

  function sendAuth0k($zerokToken, $zerokSequence)
  {
    $zerokHash = bin2hex(mhash(MHASH_SHA1, $this->password));
    $zerokHash = bin2hex(mhash(MHASH_SHA1, $zerokHash.$zerokToken));

    for($a = 0; $a < $zerokSequence; $a++)$zerokHash = bin2hex(mhash(MHASH_SHA1, $zerokHash));

    $payload = "<username>{$this->username}</username><hash>$zerokHash</hash><resource>{$this->resource}</resource>";
    $packet = $this->sendIq(NULL, 'set', $this->authId, "jabber:iq:auth", $payload);
    if($this->getInfoFromIqType($packet) == 'result' && $this->getInfoFromIqId($packet) == $this->authId)return true;
    return false;
  }

  function listenIncoming()
  {
    $incoming = '';
    while($line = $this->readFromSocket(4096))$incoming .= $line;
    $incoming = trim($incoming);
    return $this->xmlize($incoming);
  }

  function checkConnected()
  {
    $incomingArray = $this->listenIncoming();
    if(is_array($incomingArray))if($incomingArray["stream:stream"]['@']['from'] == $this->server && $incomingArray["stream:stream"]['@']['xmlns'] == "jabber:client" && $incomingArray["stream:stream"]['@']["xmlns:stream"] == "http://etherx.jabber.org/streams")
    {
      $this->streamId = $incomingArray["stream:stream"]['@']['id'];
      return true;
    }
    return false;
  }

  function splitIncoming($incoming)
  {
    $temp = preg_split("/<(message|iq|presence|stream)/", $incoming, -1, PREG_SPLIT_DELIM_CAPTURE);
    $array = array();
    $c     = count($temp);
    for($a = 1; $a < $c; $a = $a + 2)$array[] = "<".$temp[$a].$temp[($a + 1)];
    return $array;
  }

  function arrayHtmlSpecialChars($array)
  {
    if(is_array($array))foreach($array as $k => $v)$v = is_array($v) ? $this->arrayHtmlSpecialChars($v) : htmlspecialchars($v);
    return $array;
  }

  function getInfoFromIqType($packet)
  {
    return is_array($packet) ? $packet['iq']['@']['type'] : false;
  }

  function getInfoFromIqId($packet)
  {
    return is_array($packet) ? $packet['iq']['@']['id'] : false;
  }

  function xmlize($data)
  {
    $vals = $index = $array = array();
    $parser = xml_parser_create();
    xml_parser_set_option($parser, XML_OPTION_CASE_FOLDING, 0);
    xml_parser_set_option($parser, XML_OPTION_SKIP_WHITE, 1);
    xml_parse_into_struct($parser, $data, $vals, $index);
    xml_parser_free($parser);

    $i = 0;
    $tagName = $vals[$i]['tag'];
    $array[$tagName]['@'] = $vals[$i]['attributes'];
    $array[$tagName]['#'] = $this->xmlDepth($vals, $i);

    return $array;
  }

  function xmlDepth($vals, &$i)
  {
    $children = array();
    if(!empty($vals[$i]['value']))array_push($children, trim($vals[$i]['value']));

    while(++$i < count($vals))switch($vals[$i]['type'])
    {
      case 'cdata':
        array_push($children, trim($vals[$i]['value']));
        break;

      case 'complete':
        $tagName = $vals[$i]['tag'];
        $size = empty($children[$tagName]) ? 0 : sizeof($children[$tagName]);
        $children[$tagName][$size]['#'] = empty($vals[$i]['value']) ? '' : trim($vals[$i]['value']);
        if(!empty($vals[$i]['attributes']))$children[$tagName][$size]['@'] = $vals[$i]['attributes'];
        break;

      case 'open':
        $tagName = $vals[$i]['tag'];
        $size = empty($children[$tagName]) ? 0 : sizeof($children[$tagName]);
        if(!empty($vals[$i]['attributes']))
        {
          $children[$tagName][$size]['@'] = $vals[$i]['attributes'];
          $children[$tagName][$size]['#'] = $this->xmlDepth($vals, $i);
        }
        else $children[$tagName][$size]['#'] = $this->xmlDepth($vals, $i);
        break;

      case 'close':
        return $children;
        break;
    }
    return $children;
  }
}
?>