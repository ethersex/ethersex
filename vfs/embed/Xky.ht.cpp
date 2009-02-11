#include "../../autoconf.h"
#include "../../pinning.c"
#ifndef KTY_SUPPORT
#error Don't inline this file without KTY_SUPPORT
#endif
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html PUBLIC
    "-//W3C//DTD XHTML 1.1 plus MathML 2.0 plus SVG 1.1//EN"
    "http://www.w3.org/2002/04/xhtml-math-svg/xhtml-math-svg.dtd">
<html xmlns="http://www.w3.org/1999/xhtml"
      xmlns:svg="http://www.w3.org/2000/svg">
  <head>
    <title>Ethersex KTY Temperature Sheet</title>
    <script src="scr.js" type="text/javascript"></script>
    <script src="gph.js" type="text/javascript"></script>
    <link href="Sty.c" media="screen" rel="Stylesheet" type="text/css" />
  </head>
  <body>
    <h1>SVG-powered KTY Status</h1>

    <p><!-- Jippie, we like Microsoft Internet Explorer -->
      <object id="AdobeSVG" classid="clsid:78156a80-c6a1-4bbf-8e6a-3cd390eeb4e2"> </object>
    </p>

    <?import namespace="svg" urn="http://www.w3.org/2000/svg" implementation="#AdobeSVG"?>
    <svg:svg width="400px" height="300px" viewBox="0 0 400 300"
	     zoomAndPan="disable">
      <svg:g stroke="red"   style="stroke-width:2px;" id="grph0"></svg:g>
      <svg:g stroke="blue"  style="stroke-width:2px;" id="grph1"></svg:g>
      <svg:g stroke="green" style="stroke-width:2px;" id="grph2"></svg:g>
      <svg:g stroke="lime" style="stroke-width:2px;" id="grph3"></svg:g>
      <svg:g stroke="purple" style="stroke-width:2px;" id="grph4"></svg:g>
      <svg:g stroke="maroon" style="stroke-width:2px;" id="grph5"></svg:g>
      <svg:g stroke="navy" style="stroke-width:2px;" id="grph6"></svg:g>
      <svg:g stroke="yellow" style="stroke-width:2px;" id="grph7"></svg:g>
      <svg:g stroke="#999"  style="stroke-dasharray: 2, 5; " id="axis"></svg:g>
      <svg:g stroke="#999" font-size="12" id="text"></svg:g>
    </svg:svg>

    <table id='ow_table' border="1" cellspacing="0">
    <tr><td>Daten</td></tr>
    <tr><td id='daten'>No data</td></tr>
    </table>
    <div id="logconsole"></div>

    <script type="text/javascript"><![CDATA[
var g = new Array();
var min = -10, max = 50;
g[0] = new Graph("grph0", 40, min, max);
g[1] = new Graph("grph1", 40, min, max);
g[2] = new Graph("grph2", 40, min, max);
g[3] = new Graph("grph3", 40, min, max);
g[4] = new Graph("grph4", 40, min, max);
g[5] = new Graph("grph5", 40, min, max);
g[6] = new Graph("grph6", 40, min, max);
g[7] = new Graph("grph7", 40, min, max);
graphCreateAxis("axis", "text", min, max);

var sensors;

function kty_trigger () {
  setTimeout("kty_trigger_get ();", 10000);
}

function kty_trigger_get () {
  ArrAjax.aufruf('/ecmd?kty get', kty_get_handler, 'GET', 0);
  kty_trigger ();
}

function kty_get_handler(request, data) {
  var daten = returnObjById("daten");
  daten.innerHTML = request.responseText;
  var laenge = returnObjById("laenge");

  for (var i = 0; i < 7; i++) {
    sensor = request.responseText.substr(i*6, 6);
    var temperatur = parseInt (sensor);
    graphAppend (g[i], temperatur);
  }
}

window.onload = function() {
  kty_trigger();
}

]]></script>
  </body>
</html>
