#include "autoconf.h"
#ifndef TO1_INLINE_SUPPORT
#error Do not inline this file without TO1_INLINE_SUPPORT
#endif
<html>
<head>
<title>Ethersex - TO-1 Status</title>
<link rel="stylesheet" href="Sty.c" type="text/css"/>
<STYLE TYPE="text/css">
  .iob { border-right: 1px dashed #FF9999 }
.iotable td { padding-right: 10px;text-align:left; }
</STYLE>
<script src="scr.js" type="text/javascript"></script>
<script type="text/javascript">
var kennung = new Array();
kennung[2] = "Anzeigeger&auml;t LX-1";
kennung[3] = "Anzeige und &Uuml;berwachungsger&auml;t TO-1 (mit 2 Relais)";
kennung[4] = "Anzeige und &Uuml;berwachungsger&auml;t LC-1 (mit 1 Relais)";
kennung[5] = "Anzeigeger&auml;t LX-2";
function bcd(h,l) {
	l = l.toString(16);
	if (l.length == 1) l = "0" + l;
	return (h ? h.toString(16): "") + l;
}
function parse_to1(request,data) {
	var lines = request.responseText.split("\n");
	var sensors = new Array();		
	for (var i = 0; i < lines.length; i++) {
		if (lines[i].length != 30) continue;
		var s = split_hex(lines[i]);
		var sensor = new Object();
		sensor.id = s[1] << 8 | s[2];
		sensor.d = s;
		sensors.push(sensor);
	}
	sensors.sort(function (a,b) { return a.id - b.id; });
	var text = "";
	for (var i = 0; i < sensors.length; i++) {
		var s = sensors[i];
		text += "<h3>" + kennung[s.d[0]] + ": " + s.id + "</h3>";
		text += "<table class=iotable>"
		if (s.d[4] & 1) {
			text += "<tr><td class=iob>St&ouml;reingang Alarm</td><td> <font color='red'>gesetzt</font>, Kontakt: ";
			text += (s.d[4] & 2) ? "geschlossen" : "ge&ouml;ffnet";
			text += "</td></tr>";
		}
		if (s.d[0] == 3 || s.d[0] == 4) {
			text += "<tr><td class=iob style='text-align:right'>Relais 1</td><td>"+ ( (s.d[4] & 4) ? "geschlossen" : "ge&ouml;ffnet") + "</td></tr>";
			text += "<tr><td class=iob>Grenzwert Relais 1</td><td>"+ s.d[13] +" %</td></tr>";
		}
		if (s.d[0] == 3) {
			text += "<tr><td class=iob style='text-align:right'>Relais 2</td><td>"+ ( (s.d[4] & 8) ? "geschlossen" : "ge&ouml;ffnet") + "</td></tr>";
			text += "<tr><td class=iob>Grenzwert Relais 1</td><td>"+ s.d[14] +" %</td></tr>";
		}

		if (s.d[4] & (1 << 4)) text += "<tr><td class=iob>Quitierung Piepser</td><td>Ja</td></tr>";

		text += "<tr><td class=iob>Aktueller Bestand</td><td>" + bcd(s.d[5], s.d[6]) + " Liter</td></tr>";
		text += "<tr><td class=iob>Tankvolumen</td><td>" + (s.d[9] << 8 | s.d[10]) + " Liter</td></tr>";
		text += "<tr><td class=iob>Tankh&ouml;he</td><td>" + ((s.d[11] << 8 | s.d[12])/10) + " cm</td></tr>";
		text += "</table>";



	}
	_("to1_status", text);
}

function split_hex(s) {
  var hex = new Array();
  s.replace('\n', '');
  for (var i = 0; i < s.length / 2; i ++)
    hex.push(parseInt(s.substr(i*2, 2), 16));
  return hex;
}

window.onload = function() {
  ArrAjax.ecmd("to1 get", parse_to1);
}


</script>
</head><body>
<h1>TO-1 Status</h1>
<div id="to1_status"></div>

<div id="logconsole"></div>
</body>
</html>
