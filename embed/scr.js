function $(id) {
	if (document.getElementById)
		return document.getElementById(id);
	if (document.all)
		return document.all[id];
	if (document.layers)
		return document.layers[id];
	return null;
}

function _(id, value) {
	var obj = $(id);
	if (obj) obj.innerHTML = value;
}

var AjaxQueue = new Array();
var AjaxQueueLock = new Object();
AjaxQueueLock.locked = false;
var ArrAjax = new Object();

ArrAjax.aufruf = function (address, handler, method, data) {
	if (AjaxQueueLock.locked) {
		// if (AjaxQueue.length > 20) return;
		var request = new Object();
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
		 method = 'GET';
	// Mozilla, Opera, Safari sowie Internet Explorer 7
	if (typeof XMLHttpRequest != 'undefined')
		xmlHttp = new XMLHttpRequest();
  	if (!xmlHttp) {
		// Internet Explorer 6 und älter
		try {
			xmlHttp = new ActiveXObject('Msxml2.XMLHTTP');
		} catch(e) {
			try {
				xmlHttp = new ActiveXObject('Microsoft.XMLHTTP');
			} catch(e) {
				xmlHttp = null;
			}
		}
	}

	if (!xmlHttp) {
		alert('No Ajax support');
		throw Exception('No Ajax support');
		return;
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
function log_clean(howmany, bottom) {
	var logconsole = $('logconsole');
	var nodes = logconsole.getElementsByTagName('div');
	while (nodes.length > howmany) {
		logconsole.removeChild(nodes[(bottom) ? 0 : howmany]);
	}
}

function logger(code, text, bottom) {
	var logconsole = $('logconsole');
	logconsole.style.visibility = 'visible';

	var jetzt = new Date();
	var Std = jetzt.getHours();
	var Min = jetzt.getMinutes();
	var Stdtext = (Std < 10) ? '0' + Std : Std;
	var Mintext = (Min < 10) ? '0' + Min : Min;
	var text = Stdtext + ':' + Mintext + ' ' + text;

	var neuText = document.createElement('div');
	if (code == '0') {
		neuText.className = 'lognotice';
		neuText.innerHTML = text;
	} else if (code == 'notice') {
		neuText.className = 'logerror';
		neuText.innerHTML = text;
	} else {
		neuText.className = 'logerror';
		neuText.innerHTML = 'Fehler: ' + text;
	}
	if (bottom)
		logconsole.appendChild(neuText);
	else
		logconsole.insertBefore(neuText, logconsole.firstChild);
	log_clean(25, bottom);
}

function ecmd_error(response) {
	if (!response)
		return true;
        return (response.responseText.indexOf('parse error') != -1);
}

