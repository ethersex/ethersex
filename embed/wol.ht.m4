ifdef(`conf_WOL_INLINE', `', `m4exit(1)')dnl
undefine(`substr')dnl
<html>
<head>
<title>Ethersex - Wake on LAN</title>
<link rel="stylesheet" href="Sty.c" type="text/css"/>
<script src="scr.js" type="text/javascript"></script>
<script type="text/javascript">
function ecmd_wol(mac) {
	ArrAjax.ecmd('wol ' + mac, wol_handler);
}
function wol_handler(request, data) {
	$("result").innerHTML = request.responseText;
}
</script>
</head><body>
<h1>Wake on LAN</h1>
<center>
<p>Enter MAC address:
<input type="text" id="mac">
<button onclick="ecmd_wol($('mac').value)">Send wakeup signal</button>
</p>
<p id="result"></p>
</center>

<a href="idx.ht"> Back </a>
<div id="logconsole"></div>
</body>
</html>
