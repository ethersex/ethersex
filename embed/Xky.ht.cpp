#include "autoconf.h"
#include "pinning.c"
#ifndef KTY_INLINE_SUPPORT
#error Do nt inline this file without KTY_INLINE_SUPPORT
#endif
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html PUBLIC
    "-//W3C//DTD XHTML 1.1 plus MathML 2.0 plus SVG 1.1//EN"
    "http://www.w3.org/2002/04/xhtml-math-svg/xhtml-math-svg.dtd">
<html xmlns="http://www.w3.org/1999/xhtml"
      xmlns:svg="http://www.w3.org/2000/svg">
  <head>
    <title>Ethersex KTY Temperature Sheet</title>
<!--[if IE]>
    <object id="AdobeSVG" classid="clsid:78156a80-c6a1-4bbf-8e6a-3cd390eeb4e2"></object>
    <?import namespace="svg" implementation="#AdobeSVG"?>
<![endif]-->
    <link rel="stylesheet" href="Sty.c" type="text/css"/>
    <script src="scr.js" type="text/javascript"></script>
    <script type="text/javascript"><![CDATA[
var num = 8;
var min = -10, max = 50;
var g;

function kty_trigger_get() {
	setInterval("ArrAjax.ecmd('kty get', kty_get_handler)", 5000);
}

function kty_get_handler(request, data) {
	var sensors = request.responseText.split(" ");
	for (var i = sensors.length-1; i >= 0; i--) {
		if (isNaN(parseFloat(sensors[i])))
			sensors.splice(i, 1);
	}
	num = (sensors.length < num) ? sensors.length : num;
	for (var i = 0; i < num; i++) {
		var dat = $('kty_data' + i);
		dat.innerHTML = sensors[i] + '°C';
		dat.style.color = g[i].color;
		g[i].append(parseInt(sensors[i]));
	}
}

window.onload = function() {
	g = initDiagram(num, "g#", "axis", "text", min, max);
	var kty_table = $('kty_table');
	for (var i = 0; i < num; i++)
		kty_table.insertRow(i+1).innerHTML = '<td id="kty_data' + i +'">No data</td>';
	kty_trigger_get();
}
]]></script>
  </head>
  <body>
    <h1>SVG-powered KTY Status</h1>

    <table>
    <tr><td valign="top">
      <table id='kty_table' border="1" cellspacing="0">
        <tr><td>Data</td></tr>
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
    <div id="logconsole"></div>
  </body>
</html>
