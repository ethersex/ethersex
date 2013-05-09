var isIE/*@cc_on = true@*/;

function $( id )
{
  if (document.getElementById)
    var returnVar = document.getElementById(id);
  else if (document.all)
    var returnVar = document.all[id];
  else if (document.layers)
    var returnVar = document.layers[id];
  return returnVar;
}

var AjaxQueue = new Array();
var AjaxQueueLock = new Object();
AjaxQueueLock.locked = false;
var ArrAjax = new Object();

ArrAjax.aufruf = function (address, handler, method, data) 
{

  if (AjaxQueueLock.locked) {
//    if (AjaxQueue.length > 20) return;
    var request = new Object;
    request.address = address;
    request.handler = handler;
    request.method = method;
    request.data = data;
    AjaxQueue.push(request);
    return;
  } else {
    AjaxQueueLock.locked = true;
  }

  var xmlHttp = null;
  if (!method) method = "GET";
  // Mozilla, Opera, Safari sowie Internet Explorer 7
  if (typeof XMLHttpRequest != 'undefined') {
    xmlHttp = new XMLHttpRequest();
  }
  if (!xmlHttp) {
    // Internet Explorer 6 und älter
    try {
      xmlHttp  = new ActiveXObject("Msxml2.XMLHTTP");
    } catch(e) {
      try {
        xmlHttp  = new
          ActiveXObject("Microsoft.XMLHTTP");
      } catch(e) {
        xmlHttp  = null;
      }   
    }   
  }
  if (!xmlHttp) {
    alert('No Ajax support possible');
    throw Exception("No ajax support");
    return false;
  }
  xmlHttp.open(method, address, true);
  xmlHttp.onreadystatechange = function() {
    if (xmlHttp.readyState == 4) {
      try {
        if(handler) { handler(xmlHttp, data); }
      } catch(e) {}
      if (AjaxQueue.length > 0) {
        var next = AjaxQueue.shift();
        AjaxQueueLock.locked = false;
        ArrAjax.aufruf(next.address, next.handler, next.method, next.data);
      } else {
        AjaxQueueLock.locked = false;
      }
    }
  }
  xmlHttp.send(null);
  return xmlHttp;
}


//logging
function log_get_lines() {
  return $('logconsole').getElementsByTagName('div').length;
}

function log_clean(bottom) {
  var loglines = 25;
  if ( log_get_lines() > loglines ) {
    var logconsole = $('logconsole');
    var nodes = logconsole.getElementsByTagName('div');
    while (nodes.length > loglines) {
      if ( bottom) 
        logconsole.removeChild(nodes[0]);
      else
        logconsole.removeChild(nodes[loglines]);
    }
  }
}

function logger(code, text, bottom) {
  var logconsole = $('logconsole');
  logconsole.style.visibility = "visible"; 


  var div = document.createElement("div");
  
  var jetzt = new Date();
  var Std = jetzt.getHours();
  var Min = jetzt.getMinutes();
  var StdAusgabe = ((Std < 10) ? "0" + Std : Std);
  var MinAusgabe = ((Min < 10) ? "0" + Min : Min);
  var text = StdAusgabe + ":" + MinAusgabe + " " + text;
  
  var neuText = document.createElement("div");
  if (code == '0') {
    neuText.setAttribute(isIE ? "className" : "class", "lognotice");
    neuText.innerHTML = text;
  } else if (code == 'notice') {
    neuText.setAttribute(isIE ? "className" : "class", "logerror");
    neuText.innerHTML = text;
  } else {
    neuText.setAttribute(isIE ? "className" : "class", "logerror");
    neuText.innerHTML = 'Fehler: ' + text;
  }
  if (bottom)
    logconsole.appendChild(neuText);
  else
    logconsole.insertBefore(neuText, logconsole.firstChild);
  log_clean(bottom);
}

function ecmd_get_io(type, num, handler) {
  var url = '/ecmd?io get ' + type + ' ' + num;
  var request = new Object();
  request.type = type;
  request.num = num;
  ArrAjax.aufruf(url, handler, 'GET', request);
}

function byte2hex(data) {
  var hex = data.toString(16);
  if (hex.length == 1)
    return '0'+hex;
  else
    return hex;
}

function ecmd_set_io(type, portnum, num, value) {
        var i = 0;
        if (value) i = 1;
        var url = '/ecmd?io set ' + type + ' ' + portnum + " "  +  byte2hex(i << num) 
                + ' ' +byte2hex(1<<num);
        ArrAjax.aufruf(url);
}

function ecmd_error(responseText) {
        return (responseText.indexOf("parse error") != -1);

}
function ecmd_parse_io(str) {
        if (ecmd_error(str)) return undefined;
        return parseInt(str.substr(str.indexOf("0x")+2,2), 16);

}

