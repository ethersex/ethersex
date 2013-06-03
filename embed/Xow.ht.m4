changequote({{,}})dnl
ifdef({{conf_ONEWIRE}}, {{}}, {{m4exit(1)}})dnl
ifdef({{conf_ONEWIRE_INLINE}}, {{}}, {{m4exit(1)}})dnl
undefine({{substr}})dnl
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html PUBLIC
    "-//W3C//DTD XHTML 1.1 plus MathML 2.0 plus SVG 1.1//EN"
    "http://www.w3.org/2002/04/xhtml-math-svg/xhtml-math-svg.dtd">
<html xmlns="http://www.w3.org/1999/xhtml"
      xmlns:svg="http://www.w3.org/2000/svg">
  <head>
    <title>Ethersex SVG-powered 1-Wire Status</title>
<!--[if IE]>
    <object id="AdobeSVG" classid="clsid:78156a80-c6a1-4bbf-8e6a-3cd390eeb4e2"></object>
    <?import namespace="svg" implementation="#AdobeSVG"?>
<![endif]-->
    <link rel="stylesheet" href="Sty.c" type="text/css"/>
    <script src="scr.js" type="text/javascript"></script>
    <script type="text/javascript"><![CDATA[
var num = 8;
var min = -10, max = 60;
var g;
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
ifdef({{conf_ONEWIRE_ECMD_LIST_VALUES}}, {{<td style='color:" + g[i].color + "'>" + temp + " °C</td>}}, {{<td id='ow" + i +"'>No data</td>}})dnl
";
ifdef({{conf_ONEWIRE_ECMD_LIST_VALUES}}, {{dnl
		g[i].append(temp);
}})dnl
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
	cell.innerHTML = request.responseText + '°C';

	if (data < num) {
		cell.style.color = g[data].color;
		g[data].append(parseFloat(request.responseText));
	}
}

}})dnl
window.onload = function() {
	g = initDiagram(num, "g#", "axis", "text", min, max);
	ecmd_1w_list_req();
ifdef({{conf_ONEWIRE_ECMD_LIST_VALUES}}, {{dnl
	setInterval('ecmd_1w_list_req()', 10000);
}})dnl
}
]]></script>
  </head>
  <body>
    <h1>SVG-powered 1-Wire Status</h1>
    <table>
    <tr><td valign="top">
      <table id='ow_table' border="1" cellspacing="0">
      </table>
    </td>
    <td>
      <svg:svg id="chart" width="400px" height="300px" viewBox="0 0 400 300" zoomAndPan="disable">
        <svg:g class="graph" id="g0"></svg:g>
        <svg:g class="graph" id="g1"></svg:g>
        <svg:g class="graph" id="g2"></svg:g>
        <svg:g class="graph" id="g3"></svg:g>
        <svg:g class="graph" id="g4"></svg:g>
        <svg:g class="graph" id="g5"></svg:g>
        <svg:g class="graph" id="g6"></svg:g>
        <svg:g class="graph" id="g7"></svg:g>
        <svg:g id="axis"></svg:g>
        <svg:g id="text"></svg:g>
      </svg:svg>
    </td></tr>
    </table>
    <a href="idx.ht"> Back </a>
    <div id="logconsole"></div>
  </body>
</html>
