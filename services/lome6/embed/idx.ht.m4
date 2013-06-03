<html>
<head>
<title>lome6</title> 
<link rel="stylesheet" href="Stl.c" type="text/css" /> 
<script src="scr.js" type="text/javascript"></script> 
<script type="text/javascript"> 
function initLOME6() {
	ArrAjax.ecmd('whm', getValue, 'get', 'whm');
	ArrAjax.ecmd('lome6 uptime', getValue, 'get', 'uptime');
	ArrAjax.ecmd('lome6 state', getValue, 'get', 'powerstate');
}
function getValue(request, data) {
	$(data).value = request.responseText;
}
</script> </head>
<body onload="initLOME6();"> 
<div id="header">LIGHTS OUT MANAGEMENT ETHER6</div><div id="spaceHeader"></div>
<div id="topMenu">&nbsp;<ul><li><a href="/idx.ht">Machine State</a></li><li><a href="/thr.ht">Thermal State</a></li><li><a href="/cfg.ht">Configuration</a></li></ul></div>
<div id="content">
<h3>Machine State</h3>
<table>
<tr><td class="tdl">Powerstate:</td><td><input class="t" readonly="readonly" type="text" id="powerstate" size="3" /></td></tr>
<tr><td class="tdl">Uptime:</td><td><input class="t" readonly="readonly" type="text" id="uptime" size="3" /></td></tr>
<tr><td colspan="2">&nbsp;</td></tr>

<tr><td>&nbsp;</td><td><input type="button" onclick="if (confirm('Really press power?') == true) ArrAjax.ecmd('lome6 power');" value="Press Power Button" /></td></tr>
<tr><td>&nbsp;</td><td><input type="button" onclick="if (confirm('Really press power (long)?') == true) ArrAjax.ecmd('lome6 power long');" value="Press Power Button (long)" /></td></tr>
<tr><td>&nbsp;</td><td><input type="button" onclick="if (confirm('Really press reset?') == true) ArrAjax.ecmd('lome6 reset');" value="Press Reset Button" /></td></tr>

<tr><td colspan="2">&nbsp;</td></tr>
<tr><td class="tdl">lome6 uptime (hh:mm):</td><td><input class="t" readonly="readonly" type="text" id="whm" size="3" /></td></tr>

</table>


</div>
</body>
</html>
