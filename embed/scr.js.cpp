function $(id) {
	if (document.getElementById)
		return document.getElementById(id);
#if defined(VFS_INLINE_OBSOLETE_BROWSER_SUPPORT)
	if (document.all)
		return document.all[id];
	if (document.layers)
		return document.layers[id];
#endif
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
	}
	AjaxQueueLock.locked = true;

	var xmlHttp = null;
	if (!method)
		 method = 'GET';
	// Mozilla, Opera, Safari sowie Internet Explorer 7
	if (typeof XMLHttpRequest != 'undefined')
		xmlHttp = new XMLHttpRequest();
#if defined(VFS_INLINE_OBSOLETE_BROWSER_SUPPORT)
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
#endif

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

	xmlHttp.send(address);
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
	text = Stdtext + ':' + Mintext + ' ' + text;

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

#if defined(VFS_INLINE_INLINESVG_SUPPORT)
// inline SVG support
var vboxh = 300;
var vboxw = 400;
var colors = [ "#FF5C33", "#D6FF33", "#33D6FF", "#FF33D6", "#FFC233", "#0093B8", "#70FF33", "#B82500" ];


function initDiagram(graphs, gid, id, tid, min, max) {
	var g = new Array();
	graphCreateAxis(id, tid, min, max);
	for (var i = 0; i < graphs; i++) {
		g[i] = new Graph(gid.replace(/#/, i), 40, min, max,
				 (i < colors.length) ? colors[i] : undefined);
	}
	return g;
}

function Graph(id, num, min, max, color) {
	this.obj = $(id);
	this.xmult = vboxw / num;
	this.min = min;
	this.max = max;
	this.last_y = -1;
	this.elements = 0;
	this.color = (color) ? color : undefined;
	this.append = function(val) { return graphAppend(this, val); }

	if (color)
		this.obj.setAttribute("stroke", color);

	return this;
}

function addLine(obj, x1, y1, x2, y2) {
	var ne = document.createElementNS("http://www.w3.org/2000/svg", "line");
	ne.setAttribute("x1", x1);
	ne.setAttribute("x2", x2);
	ne.setAttribute("y1", y1);
	ne.setAttribute("y2", y2);

	obj.appendChild(ne);
	return ne;
}

function addText(obj, x, y, text) {
	var ne = document.createElementNS("http://www.w3.org/2000/svg", "text");
	ne.setAttribute("x", x);
	ne.setAttribute("y", y);

	var t = document.createTextNode(text);
	ne.appendChild(t);
	obj.appendChild(ne);
	return ne;
}

function graphAppend(g, val) {
	val = vboxh - ((val - g.min) * vboxh / (g.max - g.min));

	if (g.last_y != -1) {
		var w = (g.elements + 1) * g.xmult;
		addLine(g.obj, g.elements * g.xmult, g.last_y, w, val);
		g.elements++;

		if (w > vboxw) {
			g.obj.removeChild(g.obj.getElementsByTagName("line")[0]);
			g.obj.setAttribute("transform", "translate(-" + (w - vboxw) + ",0)");
		}
	}
	g.last_y = val;
	return g;
}

function graphCreateAxis(id, tid, min, max) {
	var obj = $(id);
	var tobj = $(tid);

	var x;
	for (x = 0; x <= vboxw; x += vboxw / 4)
		addLine(obj, x, 0, x, vboxh);

	var scale = 10;
	for ( ; (max - min) / scale > 10; scale *= 2);

	var i = min;
	if (i % scale)
		 i += scale - (min % scale);
  	for ( ; i <= max; i += scale) {
		var y = vboxh - ((i - min) * vboxh / (max - min));
		addLine(obj, 0, y, vboxw, y);
		addText(tobj, 0, y, i);
	}

	addLine(obj, 0, 0, vboxw, 0);
	addLine(obj, 0, vboxh, vboxw, vboxh);
}
#endif
