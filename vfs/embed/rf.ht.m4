<html><head>
ifdef(`conf_RFM12_ASK', `', `m4exit(1)')dnl
<title>Ethersex - RFM12 ASK Control</title>
<link rel="StyleSheet"  href="Sty.c" type="text/css" />
<script src="scr.js" type="text/javascript"></script>
<script type="text/javascript">
function ecmd_set_rf1(type, code, delay, repeat) {
   var url = 'http://fifi/ecmd?rfm12+' + type + '+' + code + '+' + delay + '+' + repeat;
   ArrAjax.aufruf(url);
}
function ecmd_set_rf2(type, code, cmd, delay, repeat) {
   var url = '/ecmd?rfm12+' + type + '+' + code + '+' + cmd + '+' + delay + '+' + repeat;
   ArrAjax.aufruf(url);
}
</script>
</head>
<body>
<h1>RFM12 ASK</h1>
<table> 
<tr>
 <th>Switch</th>
 <th>Command</th>
</tr>
<tr><th colspan="2">2272 (internal + external Switch)</th></tr>
<tr><td>A</td><td><a href="javascript:ecmd_set_rf1(2272,'0,5,80',76,10)">OFF OFF</a>, <a href="javascript:ecmd_set_rf1(2272,'0,5,81',76,10)">ON OFF</a>, <a href="javascript:ecmd_set_rf1(2272,'0,5,84',76,10)">OFF ON</a>, <a href="javascript:ecmd_set_rf1(2272,'0,5,85',76,10)">ON ON</a></td></tr>
<tr><td>B</td><td><a href="javascript:ecmd_set_rf1(2272,'0,17,80',76,10)">OFF OFF</a>, <a href="javascript:ecmd_set_rf1(2272,'0,17,81',76,10)">ON OFF</a>, <a href="javascript:ecmd_set_rf1(2272,'0,17,84',76,10)">OFF ON</a>, <a href="javascript:ecmd_set_rf1(2272,'0,17,85',76,10)">ON ON</a></td></tr>
<tr><td>C</td><td><a href="javascript:ecmd_set_rf1(2272,'0,20,80',76,10)">OFF OFF</a>, <a href="javascript:ecmd_set_rf1(2272,'0,20,81',76,10)">ON OFF</a>, <a href="javascript:ecmd_set_rf1(2272,'0,20,84',76,10)">OFF ON</a>, <a href="javascript:ecmd_set_rf1(2272,'0,20,85',76,10)">ON ON</a></td></tr>
<tr><td>D</td><td><a href="javascript:ecmd_set_rf1(2272,'0,21,16',76,10)">OFF OFF</a>, <a href="javascript:ecmd_set_rf1(2272,'0,21,17',76,10)">ON OFF</a>, <a href="javascript:ecmd_set_rf1(2272,'0,21,20',76,10)">OFF ON</a>, <a href="javascript:ecmd_set_rf1(2272,'0,21,21',76,10)">ON ON</a></td></tr>
<tr><td>E</td><td><a href="javascript:ecmd_set_rf1(2272,'0,21,64',76,10)">OFF OFF</a>, <a href="javascript:ecmd_set_rf1(2272,'0,21,65',76,10)">ON OFF</a>, <a href="javascript:ecmd_set_rf1(2272,'0,21,68',76,10)">OFF ON</a>, <a href="javascript:ecmd_set_rf1(2272,'0,21,69',76,10)">ON ON</a></td></tr>

<tr><th colspan="2">tevion</th></tr>
<tr><td>A</td><td>
<a href="javascript:ecmd_set_rf2('tevion','77,42,170','85,85',99,2)">ON</a> <a href="javascript:ecmd_set_rf2('tevion','77,42,170','86,86',99,2)">OFF</a> <a href="javascript:ecmd_set_rf2('tevion','77,42,170','169,154',99,1)">UP</a> <a href="javascript:ecmd_set_rf2('tevion','77,42,170','170,153',99,1)">DOWN</a>
</td></tr>
</table>
<div id="logconsole"></div>
</body>
</html>
