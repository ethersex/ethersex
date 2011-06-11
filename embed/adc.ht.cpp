#include "autoconf.h"
#include "pinning.c"
#ifndef ADC_INLINE_SUPPORT
#error Do not inline this file without ADC_INLINE_SUPPORT
#endif
<html>
<head>
<title>Ethersex - ADC Status</title>
<link rel="stylesheet" href="Sty.c" type="text/css"/>
<script src="scr.js" type="text/javascript"></script>
<script type="text/javascript">
function ecmd_adc_req() {
	ArrAjax.ecmd('adc get', ecmd_adc_req_handler);
}

function ecmd_adc_req_handler(request, data) {
	if (ecmd_error(request))
		return;
	var split = request.responseText.split(" ");
	for (var i = 0; i < split.length; i++) {
		var value = parseInt(split[i], 16);
		if (isNaN(value))
			 continue;
		var graph = $('adc_graph' + i);
		graph.style.width = (value * 100 /1023.0) + "%";
		graph.innerHTML = (value != 0) ? "&nbsp;" : "";
		var data = $('adc_data' + i);
		data.innerHTML = (value * 100 / 1023).toFixed(2) + "% (" + value + ")";
	}
}

window.onload = function() {
	var adc_table = $("adc_table");
	for (var i = 0; i < ADC_CHANNELS; i++) {
		adc_table.insertRow(i).innerHTML = '<td>Kanal ' + i +' </td><td class="adc_graph"><div id="adc_graph'+i+'"></div></td><td class="adc_data" id="adc_data'+i+'"></td>';
	}
	ecmd_adc_req();
	setInterval('ecmd_adc_req()', 5000);
}
</script>
</head><body>
<h1>Ethersex ADC Status</h1>
<table id="adc_table" class="iotable">
</table>
<div id="logconsole"></div>
</body>
</html>
