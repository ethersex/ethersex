ifdef(`conf_ONEWIRE', `', `m4exit(1)')
ifdef(`conf_ONEWIRE_INLINE', `', `m4exit(1)')
<html>
<head>
<title>Ethersex - Onewire Status</title>
<link rel="stylesheet" href="Sty.c" type="text/css"/>
<script src="scr.js" type="text/javascript"></script>
<script type="text/javascript">
function ecmd_1w_list_req() {
	ArrAjax.ecmd('1w list', ecmd_1w_list_req_handler, 'GET');
}

function ecmd_1w_list_req_handler(request) {
	if (ecmd_error(request))
		return;
	var sensors = request.responseText.split("\n");
	var ow_table = returnObjById('ow_table');

	for (var i = 0; i < sensors.length; i++) {
		if (sensors[i] == "OK")
			break;
		ow_table.insertRow(i+1).innerHTML = "<td>" + sensors[i] + "</td><td id='ow" + i +"'>No data</td>";
		ArrAjax.ecmd('1w convert ' + sensors[i], ecmd_1w_convert_req_handler, 'GET', sensors[i]);
	}
}

function ecmd_1w_convert_req_handler(request, data) {
	ArrAjax.ecmd('1w get ' + data, ecmd_1w_get_req_handler, 'GET', data);
}

function ecmd_1w_get_req_handler(request, data) {
	setTimeout("ArrAjax.ecmd('1w convert " + data
		+ "', ecmd_1w_convert_req_handler, 'GET', '" + data +"')",
		5000);
	var cell = returnObjById("ow" + data);
	cell.innerHTML = request.responseText;
}

window.onload = function() {
	ecmd_1w_list_req();
}
</script>
</head><body>
<h1>Ethersex Onewire Status</h1>
<table id='ow_table' border=1 cellspacing=0>
<tr><td>Address</td><td>Data</td></tr>
</table>
<div id="logconsole"></div>
</body>
</html>
