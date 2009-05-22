#include "autoconf.h"
#include "pinning.c"
#ifndef ADC_INLINE_SUPPORT
#error Don't inline this file without ADC_INLINE_SUPPORT
#endif
<html>
<head>
<title>Ethersex - ADC Status</title>
<script src="scr.js" type="text/javascript"></script>
<link href="Sty.c" media="screen" rel="Stylesheet" type="text/css"/>
<script type="text/javascript">
function ecmd_adc_req() {
	ArrAjax.aufruf('/ecmd?adc get', ecmd_adc_req_handler, 'GET');
}

function ecmd_adc_req_handler(request, data) {
	if (ecmd_error(request.responseText))
		return;
	var split = request.responseText.split(" ");
	for (var i = 0; i < split.length; i++) {
		var value = parseInt(split[i], 16);
		if (isNaN(value))
			 continue;
		var graph = returnObjById('adc_graph' + i);
		graph.style.width = (value * 100 /1023.0) + "%";
		graph.innerHTML = (value != 0) ? "&nbsp;" : "";
		var data = returnObjById('adc_data' + i);
		data.innerHTML = (value * 100 / 1023).toFixed(2) + "% (" + value + ")";
		data.style.borderLeft =  "1px dashed #FF9999";
		data.style.padding = "3px";
	}
}

window.onload = function() {
	var adc_table = returnObjById("adc_table");
	for (var i = 0; i < ADC_CHANNELS; i++) {
		adc_table.innerHTML += '<tr><td>Kanal ' + i +' </td><td style="width:500px"><div id="adc_graph'+i+'" style="background-color: #00ff00;"></div></td><td id="adc_data'+i+'"></td></tr>';
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
