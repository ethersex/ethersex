#include "autoconf.h"
#include "pinning.c"
#ifndef TANKLEVEL_INLINE_SUPPORT
#error Do not inline this file without TANKLEVEL_INLINE_SUPPORT
#endif
<html>
<head>
<title>Ethersex - Tank Level</title>
<link rel="stylesheet" href="Sty.c" type="text/css"/>
<script src="scr.js" type="text/javascript"></script>
<script type="text/javascript">

function ecmd_tank_req() {
	ArrAjax.ecmd('tank get', ecmd_tank_req_handler);
}

function ecmd_tank_req_handler(request, data) {
	if (ecmd_error(request))
		return;
	var split = request.responseText.split("\t");
	var ts = split[0];
	var value = parseInt(split[1]);
	var full = parseInt(split[2]);
	if (isNaN(value) || isNaN(full))
		return;
	var pct = (value > 0 && full > 0) ? value * 100 / full : 0;
	if (pct > 100)
		pct = 100;
	$('tank_ts').innerHTML = "Last update: " + ts;
	$('tank_value').style.width = pct + "%";
	$('tank_text').innerHTML = pct.toFixed(1) + "%";
	$('tank_curr').innerHTML = value + " ltr";
	$('tank_full').innerHTML = full + " ltr";
}

window.onload = function() {
	ecmd_tank_req();
	setInterval('ecmd_tank_req()', 5000);
}
</script>
</head><body>
<h1>Tank Level</h1>
<table border=1 cellspacing=0>
<tr>
<td id="tank_ts" colspan="3"></td>
</tr>
<tr>
<td id="tank_curr"></td>
<td>
<div class="tank_wrap">
<div class="tank_value" id="tank_value">
<div class="tank_text" id="tank_text"></div>
</div>
</div>
</td>
<td id="tank_full"></td>
</tr>
</table>
<br/>
<form>
<input type="button" value="Start measure" onclick="ArrAjax.ecmd('tank start', null);"/>
</form>
<a href="idx.ht"> Back </a>
<div id="logconsole"></div>
</body>
</html>

