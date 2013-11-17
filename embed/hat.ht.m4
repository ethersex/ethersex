changequote({{,}})dnl
ifdef({{conf_HAT_INLINE}}, {{}}, {{m4exit(1)}})dnl
ifdef({{conf_DHT}}, {{}}, dnl
{{ifdef({{conf_SHT}}, {{}}, {{m4exit(1)}})}})dnl
<html>
<head>
<title>Ethersex - Humidity &amp; temperature sensors</title>
<link rel="stylesheet" href="Sty.c" type="text/css"/>
<script src="scr.js" type="text/javascript"></script>
<script type="text/javascript">
ifdef({{conf_DHT}},dnl
var dht_temp = 0;
var dht_humid = 0;

function dht_update_temp(request) {
	if (ecmd_error(request))
		return;

	dht_temp = request.responseText;
}

function dht_update_humid(request) {
	if (ecmd_error(request))
		return;

	dht_humid = request.responseText;
}
)dnl

ifdef({{conf_SHT}},dnl
var sht_temp = 0;
var sht_humid = 0;

function sht_update_temp(request) {
	if (ecmd_error(request))
		return;

	sht_temp = request.responseText;
}

function sht_update_humid(request) {
	if (ecmd_error(request))
		return;

	sht_humid = request.responseText;
}
)dnl

function hat_update() {
	var line = 1;
	var hat_table = $('hat_table');
	hat_table.innerHTML = '<tr><td>Sensor</td><td>Temperature</td><td>Humidity</td></tr>';

ifdef({{conf_DHT}},dnl
	ArrAjax.ecmd("dht temp",  dht_update_temp);
	ArrAjax.ecmd("dht humid", dht_update_humid);
	hat_table.insertRow(line++).innerHTML = '<td>DHT</td><td>' + dht_temp + ' &deg;C</td><td>' + dht_humid + ' %</td>';
)dnl

ifdef({{conf_SHT}},dnl
	ArrAjax.ecmd("sht temp",  sht_update_temp);
	ArrAjax.ecmd("sht humid", sht_update_humid);
	hat_table.insertRow(line++).innerHTML = '<td>SHT</td><td>' + sht_temp + ' &deg;C</td><td>' + sht_humid + ' %</td>';
)dnl
}

window.onload = function() {
	setInterval('hat_update()', 10000);
}
</script>
</head><body>
<h1>Humidity &amp; temperature sensors</h1>
<table id='hat_table' border=1 cellspacing=0>
</table>
<br>
<a href="idx.ht"> Back </a>
<div id="logconsole"></div>
</body>
</html>
