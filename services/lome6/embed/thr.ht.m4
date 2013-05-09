<html>
<head>
<title>lome6</title> 
<link rel="stylesheet" href="Stl.c" type="text/css" /> 
<script src="scr.js" type="text/javascript"></script> 
<script type="text/javascript"> 
function initLOME6() {
	ArrAjax.ecmd('lome6 get_t air', getValue, 'get', 'tempair');
	ArrAjax.ecmd('lome6 get_t cpu', getValue, 'get', 'tempcpu');
	ArrAjax.ecmd('lome6 get_t sb', getValue, 'get', 'tempsb');
	ArrAjax.ecmd('lome6 get_t ram', getValue, 'get', 'tempram');
	ArrAjax.ecmd('lome6 get_t psu', getValue, 'get', 'temppsu');
}
function getValue(request, data) {
	$(data).value = request.responseText;
}
</script> </head>
<body onload="initLOME6();"> 
<div id="header">LIGHTS OUT MANAGEMENT ETHER6</div><div id="spaceHeader"></div>
<div id="topMenu">&nbsp;<ul><li><a href="/idx.ht">Machine State</a></li><li><a href="/thr.ht">Thermal State</a></li><li><a href="/cfg.ht">Configuration</a></li></ul></div>
<div id="content">
<h3>Thermal State</h3>
<table>
<tr><td class="tdl">Air Intake:</td><td><input class="t" readonly="readonly" type="text" id="tempair" size="3" />&deg;C<td></td></tr>
<tr><td class="tdl">CPU:</td><td><input class="t" readonly="readonly" type="text" id="tempcpu" size="3" />&deg;C<td></td></tr>
<tr><td class="tdl">Southbridge:</td><td><input class="t" readonly="readonly" type="text" id="tempsb" size="3" />&deg;C<td></td></tr>
<tr><td class="tdl">RAM:</td><td><input class="t" readonly="readonly" type="text" id="tempram" size="3" />&deg;C<td></td></tr>
<tr><td class="tdl">Power Supply:</td><td><input class="t" readonly="readonly" type="text" id="temppsu" size="3" />&deg;C<td></td></tr>
</table>


</div>
</body>
</html>
