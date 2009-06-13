ifdef(`conf_ONEWIRE', `', `m4exit(1)')dnl
ifdef(`conf_ONEWIRE_INLINE', `', `m4exit(1)')dnl
<html>
<head>
<title>Ethersex 1-Wire Status</title>
<link rel="stylesheet" href="Sty.c" type="text/css"/>
<script src="scr.js" type="text/javascript"></script>
<script type="text/javascript">
var sensors;

function ecmd_1w_list_req() {
	ArrAjax.ecmd('1w list', ecmd_1w_list_req_handler);
}

function ecmd_1w_list_req_handler(request) {
	if (ecmd_error(request))
		return;
	sensors = request.responseText.split("\n");
	var ow_table = returnObjById('ow_table');

	for (var i = 0; i < sensors.length; i++) {
		if (sensors[i] == "OK")
			break;
		ow_table.insertRow(i+1).innerHTML = "<td>" + sensors[i] + "</td><td id='ow" + i +"'>No data</td>";
		ecmd_1w_convert_req(i);
		setInterval('ecmd_1w_convert_req('+ i +')', 5000);
	}
}

function ecmd_1w_convert_req(data) {
	ArrAjax.ecmd('1w convert ' + sensors[data], ecmd_1w_convert_req_handler, 'GET', data);
}

function ecmd_1w_convert_req_handler(request, data) {
	ArrAjax.ecmd('1w get ' + sensors[data], ecmd_1w_get_req_handler, 'GET', data);
}

function ecmd_1w_get_req_handler(request, data) {
	var cell = returnObjById("ow" + data);
	cell.innerHTML = request.responseText + '&deg;C';
}

window.onload = function() {
	ecmd_1w_list_req();
}
</script>
</head><body>
<h1>Ethersex 1-Wire Status</h1>
<table id='ow_table' border=1 cellspacing=0>
<tr><td>Address</td><td>Data</td></tr>
</table>
<div id="logconsole"></div>
</body>
</html>
