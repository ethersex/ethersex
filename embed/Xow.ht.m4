ifdef(`conf_ONEWIRE', `', `m4exit(1)')dnl
ifdef(`conf_ONEWIRE_INLINE', `', `m4exit(1)')dnl
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html PUBLIC
    "-//W3C//DTD XHTML 1.1 plus MathML 2.0 plus SVG 1.1//EN"
    "http://www.w3.org/2002/04/xhtml-math-svg/xhtml-math-svg.dtd">
<html xmlns="http://www.w3.org/1999/xhtml"
      xmlns:svg="http://www.w3.org/2000/svg">
  <head>
    <title>Ethersex Ultimate Onewire Temperature Sheet</title>
    <link rel="stylesheet" href="Sty.c" type="text/css"/>
    <script src="scr.js" type="text/javascript"></script>
    <script src="gph.js" type="text/javascript"></script>
  </head>
  <body>
    <h1>Ultimate SVG-powered Onewire Status</h1>

    <p><!-- Jippie, we like Microsoft Internet Explorer -->
      <object id="AdobeSVG" classid="clsid:78156a80-c6a1-4bbf-8e6a-3cd390eeb4e2"> </object>
    </p>

    <?import namespace="svg" urn="http://www.w3.org/2000/svg" implementation="#AdobeSVG"?>
    <svg:svg width="400px" height="300px" viewBox="0 0 400 300"
	     zoomAndPan="disable">
      <svg:g stroke="red"   style="stroke-width:2px;" id="grph0"></svg:g>
      <svg:g stroke="blue"  style="stroke-width:2px;" id="grph1"></svg:g>
      <svg:g stroke="green" style="stroke-width:2px;" id="grph2"></svg:g>
      <svg:g stroke="#999"  style="stroke-dasharray: 2, 5; " id="axis"></svg:g>
      <svg:g stroke="#999" font-size="12" id="text"></svg:g>
    </svg:svg>

    <table id='ow_table' border="1" cellspacing="0">
    <tr><td>Address</td><td>Data</td></tr>
    </table>
    <div id="logconsole"></div>

    <script type="text/javascript"><![CDATA[
var g = new Array();
var min = -10, max = 50;
g[0] = new Graph("grph0", 40, min, max);
g[1] = new Graph("grph1", 40, min, max);
g[2] = new Graph("grph2", 40, min, max);
graphCreateAxis("axis", "text", min, max);

var sensors;

function ecmd_1w_list_req() {
	ArrAjax.ecmd('1w list', ecmd_1w_list_req_handler, 'GET');
}

function ecmd_1w_trigger_converts() {
	for (var i = 0; i < sensors.length; i++) {
		if (sensors[i] == "OK")
			 break;
		ArrAjax.ecmd('1w convert ' + sensors[i], ecmd_1w_convert_req_handler, 'GET', i);
	}
	setTimeout("ecmd_1w_trigger_converts();", 5000);
}

function ecmd_1w_list_req_handler(request) {
	if (ecmd_error(request))
		return;
	sensors = request.responseText.split("\n");
	var ow_table = returnObjById('ow_table');

	for (var i = 0; i < sensors.length; i++) {
		if (sensors[i] == "OK")
			 break;
		ow_table.innerHTML += "<tr><td>" + sensors[i] + "</td><td id='ow" + sensors[i] +"'>No data</td></tr>";
	}
	ecmd_1w_trigger_converts();
}

function ecmd_1w_convert_req_handler(request, data) {
	ArrAjax.ecmd('1w get ' + sensors[data], ecmd_1w_get_req_handler, 'GET', data);
}

function ecmd_1w_get_req_handler(request, data) {
	var cell = returnObjById("ow" + sensors[data]);
	cell.innerHTML = request.responseText;

	if (data <= 2) {
		var str = request.responseText;
		var i = parseInt(str.substr(str.indexOf(" ") + 1));
		graphAppend(g[data], i);
	}
}

window.onload = function() {
	ecmd_1w_list_req();
}
]]></script>
  </body>
</html>
