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
ifdef({{conf_DHT}},{{dnl
ifdef({{conf_DHT_LIST_ECMD}},{{dnl
var sensors;

function dht_update_temp(request, data) {
	if (ecmd_error(request))
		return;

	var cell = $("dht" + data + "temp");
	cell.innerHTML = request.responseText + '&deg;C';
}

function dht_update_humid(request, data) {
	if (ecmd_error(request))
		return;

	var cell = $("dht" + data + "humid");
	cell.innerHTML = request.responseText + '%';
}

function dht_update_req() {
	for (var i = 0; i < dht_sensors.length; i++) {
		if (dht_sensors[i] == "OK")
			break;
		ArrAjax.ecmd("dht temp " + i,  dht_update_temp, 'GET', i);
		ArrAjax.ecmd("dht humid " + i, dht_update_humid, 'GET', i);
	}
}

function dht_list_req_handler(request) {
	if (ecmd_error(request))
		return;

	dht_sensors = request.responseText.split("\n");
	var hat_table = $('hat_table');

ifdef({{conf_SHT}},{{dnl
	var firstrow = 2;
}},{{dnl
	var firstrow = 1;
}})dnl
	for (var i = 0; i < dht_sensors.length; i++) {
		if (dht_sensors[i] == "OK")
			break;

		var j = 0;
		colums = dht_sensors[i].split("\t");
		num  = colums[j++];
		name = colums[j++];

		hat_table.insertRow(i+firstrow).innerHTML = "<td>DHT <b>" + name + "</b></td><td id='dht" + i + "temp'>No data</td><td id='dht" + i + "humid'>No data</td>";
	}

	setInterval('dht_update_req()', 10000);
}

function dht_list_req() {
	ArrAjax.ecmd('dht list', dht_list_req_handler);
}
}},{{dnl
function dht_update_temp(request) {
	if (ecmd_error(request))
		return;

	var cell = $("dhttemp");
	cell.innerHTML = request.responseText + '&deg;C';
}

function dht_update_humid(request) {
	if (ecmd_error(request))
		return;

	var cell = $("dhthumid");
	cell.innerHTML = request.responseText + '%';
}

function dht_update_req() {
	ArrAjax.ecmd("dht temp",  dht_update_temp);
	ArrAjax.ecmd("dht humid", dht_update_humid);
}
}})}})dnl

ifdef({{conf_SHT}},{{dnl
function sht_update_temp(request) {
	if (ecmd_error(request))
		return;

	var cell = $("shttemp");
	cell.innerHTML = request.responseText + '&deg;C';
}

function sht_update_humid(request) {
	if (ecmd_error(request))
		return;

	var cell = $("shthumid");
	cell.innerHTML = request.responseText + '%';
}

function sht_update_req() {
	ArrAjax.ecmd("sht temp",  sht_update_temp);
	ArrAjax.ecmd("sht humid", sht_update_humid);
}
}})dnl

window.onload = function() {
ifdef({{conf_SHT}},{{dnl
	setInterval('sht_update_req()', 10000);
}})dnl
ifdef({{conf_DHT}},{{dnl
ifdef({{conf_DHT_LIST_ECMD}},{{dnl
	dht_list_req();
}},{{dnl
	setInterval('dht_update_req()', 10000);
}})}})dnl
}
</script>
</head><body>
<h1>Humidity &amp; temperature sensors</h1>
<table id='hat_table' border=1 cellspacing=0>
<tr><td>Sensor</td><td>Temperature</td><td>Humidity</td></tr>
ifdef({{conf_SHT}},{{dnl
<tr><td>SHT</td><td id='shttemp'>No data</td><td id='shthumid'>No data</td></tr>
}})dnl
ifdef({{conf_DHT_LIST_ECMD}},{{ }},{{dnl
<tr><td>DHT</td><td id='dhttemp'>No data</td><td id='dhthumid'>No data</td></tr>
}})dnl
</table>
<br>
<a href="idx.ht"> Back </a>
<div id="logconsole"></div>
</body>
</html>
