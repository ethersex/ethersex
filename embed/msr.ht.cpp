#include "autoconf.h"
#ifndef MSR1_INLINE_SUPPORT
#error Do not inline this file without MSR1_INLINE_SUPPORT
#endif
<html>
<head>
<title>Ethersex - MSR1 Status</title>
<link rel="stylesheet" href="Sty.c" type="text/css"/>
<STYLE TYPE="text/css">
  .iob { border-right: 1px dashed #FF9999 }
.iotable td { padding-right: 10px;text-align:left; }
</STYLE>
<script src="scr.js" type="text/javascript"></script>
<script type="text/javascript">
function parse_e8(request,data) {
  var h = split_hex(request.responseText);
  _('stunden', (h[1] * 256 + h[2]) + " Std.");
  _('wartung', h[3] == 255 ? "noch mehr als 254 Stunden" : ("noch " + h[3] + " Stunden"));
  _('ruecklauf', schar(h[4]) + " C");
  _('vorlauf', schar(h[5]) + " C");
  _('abgas', (schar(h[6]) + 15) + " C");
  _('einschalt', schar(h[7]) + " C");

 // Betriebszustand
   var b = new Array();
   b[10] = 'St&ouml;rabschaltung';
   b[11] = 'Abschaltung MV1';
   b[11] = 'Abschaltung MV2';
   b[13] = 'Drehzahl<200 n. 25 sek.';
   b[14] = 'Drehzahl>200 n. 25 sek.';
   b[15] = 'Abschaltung>1 Minute';
   b[16] = 'Abschaltung>4 Minuten';
   b[20] = 'Startvorbereitung';
   b[21] = 'Starteinleitung';
   b[22] = 'Start';
   b[23] = '1,5 sek. nach Start';
   b[24] = 'Startende';
   b[30] = '450<Drehz<800';
   b[32] = 'Zuschaltung Generator';
   b[33] = 'Leistungsregelung runter';
   b[34] = 'Leistungsregelung hoch';
   b[35] = 'Betrieb';
   _('betrieb', b[h[8]]);
   _('elek',  (h[9]/34.0).toFixed(3) + " KW");
   
}
function parse_c0(request, data) {
  var h = split_hex(request.responseText);
  _('soll_ruecklauf', h[21] + " C");
  _('soll_vorlauf', h[23] + " C");
  _('aussen', schar(h[25]) + " C");
  _('kuehl_gen', h[28] + " C");
  _('kuehl_mot', h[34] + " C");
  _('fuehl1', h[26] + " C");
  _('fuehl2', h[27] + " C");
  _('dreh', h[36] * 256 + h[37]);
  _('act_elek', (h[46]/34).toFixed(3) + " KW");
  _('starts', h[47] * 256 + h[48]);
  _('kw_elek', h[38] * 256 * 256 * 256 + h[39] * 256 * 256 + h[40] * 256 + h[41]);
  _('kw_therm', h[42] * 256 * 256 * 256 + h[43] * 256 * 256 + h[44] * 256 + h[45]);
}

function generate_row(a, b, c, d) {
  document.write("<tr><td>"+a+"</td><td id='"+b+"' class=\"iob\"></td><td>"+c+"</td><td id='"+d+"' class=\"iob\"></td></tr>");
}
function split_hex(s) {
  var hex = new Array();
  s.replace('\n', '');
  for (var i = 0; i < s.length / 2; i ++)
    hex.push(parseInt(s.substr(i*2, 2), 16));
  return hex;
}
function schar(a) { return a > 128 ? a - 255: a;}

ArrAjax.ecmd("msr1 get", parse_e8);
ArrAjax.ecmd("msr1 get 1", parse_c0);

</script>
</head><body>
<h1>Ethersex MSR1 Status</h1>

<table class="iotable">
<script type="text/javascript">
  generate_row("Betriebsstunden HKA", "stunden", "Wartungsanzeige", "wartung");
  generate_row("Betriebszustand", "betrieb", "elektr. Leistung", "elek");
  generate_row("Vorlauf", "vorlauf", "Vorlauf (soll)", "soll_vorlauf");
  generate_row("R&uuml;cklauf", "ruecklauf", "R&uuml;cklauf (soll)", "soll_ruecklauf");
  generate_row("Abgas", "abgas", "Einschaltsoll", "einschalt");
  generate_row("Au&szlig;entemperatur", "aussen", "K&uuml;hlwasser (Generator)", "kuehl_gen");
  generate_row("F&uuml;hler 1", "fuehl1", "F&uumlhler 2", "fuehl2");
  generate_row("K&uuml;hlwasser (Motor)", "kuehl_mot", "Drehzahl", "dreh");
  generate_row("KWh elektr.", "kw_elek", "KWh therm.", "kw_therm");
  generate_row("aktuelle elek. Leistung", "act_elek", "Starts", "starts");
</script>
</table>
<div id="logconsole"></div>
</body>
</html>
