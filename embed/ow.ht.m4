changequote({{,}})dnl
ifdef({{conf_ONEWIRE}}, {{}}, {{m4exit(1)}})dnl
ifdef({{conf_ONEWIRE_INLINE}}, {{}}, {{m4exit(1)}})dnl
<html>
<head>
<title>Ethersex 1-Wire Status</title>
<link rel="stylesheet" href="Sty.c" type="text/css"/>
<script src="scr.js" type="text/javascript"></script>
<script type="text/javascript">
var sensors;

function ecmd_1w_list_req() {
	ArrAjax.ecmd('1w list t', ecmd_1w_list_req_handler);
}

function ecmd_1w_list_req_handler(request) {
	if (ecmd_error(request))
		return;
	sensors = request.responseText.split("\n");
	var ow_table = $('ow_table');

	ow_table.innerHTML = "<tr>dnl
<td>Address</td>dnl
ifdef({{conf_ONEWIRE_NAMING}}, {{<td>Name</td>}})dnl
<td>Data</td>dnl
</tr>";
	for (var i = 0; i < sensors.length; i++) {
		if (sensors[i] == "OK")
			break;
		var j=0;
		var colums = sensors[i].split("\t");
		var addr = colums[j++];
ifdef({{conf_ONEWIRE_NAMING}}, {{dnl
		var name = colums[j++];
}})dnl
ifdef({{conf_ONEWIRE_ECMD_LIST_VALUES}}, {{dnl
		var temp = parseFloat(colums[j++])
}})dnl
		ow_table.insertRow(i+1).innerHTML = "dnl
<td><code><b>" + addr + "</b></code></td>dnl
ifdef({{conf_ONEWIRE_NAMING}}, {{<td><code><b>" + name + "</b></code></td>}})dnl
ifdef({{conf_ONEWIRE_ECMD_LIST_VALUES}}, {{<td>" + temp + " &deg;C</td>}}, {{<td id='ow" + i +"'>No data</td>}})dnl
";
	}
ifdef({{conf_ONEWIRE_ECMD_LIST_VALUES}}, {{}}, {{dnl
	ecmd_1w_convert_req();
	setInterval('ecmd_1w_convert_req()', 10000);
}})dnl
}

ifdef({{conf_ONEWIRE_ECMD_LIST_VALUES}}, {{}}, {{dnl
function ecmd_1w_convert_req() {
	ArrAjax.ecmd('1w convert');
	for (var i = 0; i < sensors.length; i++) {
		if (sensors[i] == "OK")
			break;
		ArrAjax.ecmd('1w get ' + sensors[i].split("\t")[0], ecmd_1w_get_req_handler, 'GET', i);
	}
}

function ecmd_1w_get_req_handler(request, data) {
	var cell = $("ow" + data);
	cell.innerHTML = request.responseText + '&deg;C';
}

}})dnl
window.onload = function() {
	ecmd_1w_list_req();
ifdef({{conf_ONEWIRE_ECMD_LIST_VALUES}}, {{dnl
	setInterval('ecmd_1w_list_req()', 10000);
}})dnl
}
</script>
</head><body>
<h1>1-Wire Status</h1>
<table id='ow_table' border=1 cellspacing=0>
</table>
<br>
<a href="idx.ht"> Back </a>
<div id="logconsole"></div>
</body>
</html>
