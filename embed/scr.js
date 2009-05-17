var isIE/*@cc_on = true@*/;

function returnObjById(id) {
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

ArrAjax.aufruf = function (address, handler, method, data) {
	if (AjaxQueueLock.locked) {
		// if (AjaxQueue.length > 20) return;
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
	if (!method)
		 method = "GET";
	// Mozilla, Opera, Safari sowie Internet Explorer 7
	if (typeof XMLHttpRequest != 'undefined')
		xmlHttp = new XMLHttpRequest();
  	if (!xmlHttp) {
		// Internet Explorer 6 und älter
		try {
			xmlHttp  = new ActiveXObject("Msxml2.XMLHTTP");
		} catch(e) {
			try {
				xmlHttp  = new ActiveXObject("Microsoft.XMLHTTP");
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
				if (handler)
					 handler(xmlHttp, data);
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

ArrAjax.ecmd = function (address, handler, method, data) {
	return ArrAjax.aufruf('/ecmd?' + address, handler, method, data);
}

//logging
function log_get_lines() {
	return returnObjById('logconsole').getElementsByTagName('div').length;
}

function log_clean(bottom) {
	var loglines = 25;
	if (log_get_lines() > loglines) {
		var logconsole = returnObjById('logconsole');
		var nodes = logconsole.getElementsByTagName('div');
		while (nodes.length > loglines) {
			logconsole.removeChild(nodes[(bottom) ? 0 : loglines]);
		}
	}
}

function logger(code, text, bottom) {
	var logconsole = returnObjById('logconsole');
	logconsole.style.visibility = "visible";

	var jetzt = new Date();
	var Std = jetzt.getHours();
	var Min = jetzt.getMinutes();
	var Stdtext = (Std < 10) ? "0" + Std : Std;
	var Mintext = (Min < 10) ? "0" + Min : Min;
	var text = Stdtext + ":" + Mintext + " " + text;

	var neuText = document.createElement("div");
	var attr = (isIE) ? "className" : "class";
	if (code == '0') {
		neuText.setAttribute(attr, "lognotice");
		neuText.innerHTML = text;
	} else if (code == 'notice') {
		neuText.setAttribute(attr, "logerror");
		neuText.innerHTML = text;
	} else {
		neuText.setAttribute(attr, "logerror");
		neuText.innerHTML = 'Fehler: ' + text;
	}
	if (bottom)
		logconsole.appendChild(neuText);
	else
		logconsole.insertBefore(neuText, logconsole.firstChild);
	log_clean(bottom);
}

function ecmd_error(responseText) {
        return (responseText.indexOf("parse error") != -1);

}

