ifdef(`conf_ONEWIRE', `', `m4exit(1)')dnl
ifdef(`conf_ONEWIRE_INLINE', `', `m4exit(1)')dnl
undefine(`substr')dnl
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html PUBLIC
    "-//W3C//DTD XHTML 1.1 plus MathML 2.0 plus SVG 1.1//EN"
    "http://www.w3.org/2002/04/xhtml-math-svg/xhtml-math-svg.dtd">
<html xmlns="http://www.w3.org/1999/xhtml"
      xmlns:svg="http://www.w3.org/2000/svg">
  <head>
    <title>Ethersex SVG-powered 1-Wire Status</title>
    <link rel="stylesheet" href="Sty.c" type="text/css"/>
    <script src="scr.js" type="text/javascript"></script>
    <script src="gph.js" type="text/javascript"></script>
    <script type="text/javascript"><![CDATA[
var num = 8;
var min = -10, max = 50;
var g = new Array(num);
var colors = [ "red", "blue", "green", "lime", "purple", "maroon", "navy", "yellow" ];
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
	cell.innerHTML = request.responseText + '°C';

	if (data < num) {
		cell.style.color = colors[data];
		var value = parseInt(request.responseText);
		graphAppend(g[data], value);
	}
}

window.onload = function() {
	graphCreateAxis("axis", "text", min, max);
	for (var i = 0; i < num; i++)
		g[i] = new Graph("grph" + i, 40, min, max);
	ecmd_1w_list_req();
}
]]></script>
  </head>
  <body>
    <h1>SVG-powered 1-Wire Status</h1>

    <table>
    <tr><td valign="top">
      <table id='ow_table' border="1" cellspacing="0">
        <tr><td>Address</td><td>Data</td></tr>
      </table>
    </td>
    <td>
      <p><!-- Jippie, we like Microsoft Internet Explorer -->
        <object id="AdobeSVG" classid="clsid:78156a80-c6a1-4bbf-8e6a-3cd390eeb4e2"> </object>
      </p>
      <?import namespace="svg" urn="http://www.w3.org/2000/svg" implementation="#AdobeSVG"?>
      <svg:svg width="400px" height="300px" viewBox="0 0 400 300" zoomAndPan="disable">
        <svg:g stroke="red"   style="stroke-width:2px;" id="grph0"></svg:g>
        <svg:g stroke="blue"  style="stroke-width:2px;" id="grph1"></svg:g>
        <svg:g stroke="green" style="stroke-width:2px;" id="grph2"></svg:g>
        <svg:g stroke="lime"  style="stroke-width:2px;" id="grph3"></svg:g>
        <svg:g stroke="purple" style="stroke-width:2px;" id="grph4"></svg:g>
        <svg:g stroke="maroon" style="stroke-width:2px;" id="grph5"></svg:g>
        <svg:g stroke="navy"   style="stroke-width:2px;" id="grph6"></svg:g>
        <svg:g stroke="yellow" style="stroke-width:2px;" id="grph7"></svg:g>
        <svg:g stroke="#999"  style="stroke-dasharray: 2, 5; " id="axis"></svg:g>
        <svg:g stroke="#999" font-size="12" id="text"></svg:g>
      </svg:svg>
    </td></tr>
    </table>
    <div id="logconsole"></div>
  </body>
</html>
