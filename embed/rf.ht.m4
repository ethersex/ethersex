<html><head>
changequote({{,}})dnl
ifdef({{conf_RFM12_ASK}}, {{}}, {{m4exit(1)}})dnl
ifdef({{conf_RFM12_INLINE}}, {{}}, {{m4exit(1)}})dnl
<title>Ethersex - RFM12 ASK Control</title>
<link rel="StyleSheet"  href="Sty.c" type="text/css" />
<script src="scr.js" type="text/javascript"></script>
<script type="text/javascript">
ifdef({{conf_RFM12_ASK_2272}}, {{
function ecmd_set_rf1(type, code, delay, repeat) {
   var url = '/ecmd?rfm12+' + type + '+' + code + '+' + delay + '+' + repeat;
   ArrAjax.aufruf(url);
}
}})
ifdef({{conf_RFM12_ASK_TEVION}}, {{
function ecmd_set_rf2(type, code, cmd, delay, repeat) {
   var url = '/ecmd?rfm12+' + type + '+' + code + '+' + cmd + '+' + delay + '+' + repeat;
   ArrAjax.aufruf(url);
}
}})
</script>
</head>
<body>
<h1>RFM12 ASK</h1>
ifdef({{conf_RFM12_ASK_2272}}, {{
<h2>2272 (internal + external)</h2>
<table width="50%"> 
<tr><th>Switch</th><th colspan="4">Command</th></tr>
<tr><td>A</td><td><a href="javascript:ecmd_set_rf1(2272,'0,5,80',76,10)">OFF OFF</a></td><td><a href="javascript:ecmd_set_rf1(2272,'0,5,81',76,10)">ON OFF</a></td><td><a href="javascript:ecmd_set_rf1(2272,'0,5,84',76,10)">OFF ON</a></td><td><a href="javascript:ecmd_set_rf1(2272,'0,5,85',76,10)">ON ON</a></td></tr>
<tr><td>B</td><td><a href="javascript:ecmd_set_rf1(2272,'0,17,80',76,10)">OFF OFF</a></td><td><a href="javascript:ecmd_set_rf1(2272,'0,17,81',76,10)">ON OFF</a></td><td><a href="javascript:ecmd_set_rf1(2272,'0,17,84',76,10)">OFF ON</a></td><td><a href="javascript:ecmd_set_rf1(2272,'0,17,85',76,10)">ON ON</a></td></tr>
<tr><td>C</td><td><a href="javascript:ecmd_set_rf1(2272,'0,20,80',76,10)">OFF OFF</a></td><td><a href="javascript:ecmd_set_rf1(2272,'0,20,81',76,10)">ON OFF</a></td><td><a href="javascript:ecmd_set_rf1(2272,'0,20,84',76,10)">OFF ON</a></td><td><a href="javascript:ecmd_set_rf1(2272,'0,20,85',76,10)">ON ON</a></td></tr>
<tr><td>D</td><td><a href="javascript:ecmd_set_rf1(2272,'0,21,16',76,10)">OFF OFF</a></td><td><a href="javascript:ecmd_set_rf1(2272,'0,21,17',76,10)">ON OFF</a></td><td><a href="javascript:ecmd_set_rf1(2272,'0,21,20',76,10)">OFF ON</a></td><td><a href="javascript:ecmd_set_rf1(2272,'0,21,21',76,10)">ON ON</a></td></tr>
<tr><td>E</td><td><a href="javascript:ecmd_set_rf1(2272,'0,21,64',76,10)">OFF OFF</a></td><td><a href="javascript:ecmd_set_rf1(2272,'0,21,65',76,10)">ON OFF</a></td><td><a href="javascript:ecmd_set_rf1(2272,'0,21,68',76,10)">OFF ON</a></td><td><a href="javascript:ecmd_set_rf1(2272,'0,21,69',76,10)">ON ON</a></td></tr>
</table>
}})
ifdef({{conf_RFM12_ASK_TEVION}}, {{
<h2>Tevion</h2>
<table width="50%"> 
<tr><th>Switch</th><th colspan="3">Command</th></tr>
<tr><td>1</td><td><a href="javascript:ecmd_set_rf2('tevion','77,42,170','85,85',99,2)">OFF</a> <a href="javascript:ecmd_set_rf2('tevion','77,42,170','86,86',99,2)">ON</a></td><td><a href="javascript:ecmd_set_rf2('tevion','77,42,170','85,153',99,1)">UP</a> <a href="javascript:ecmd_set_rf2('tevion','77,42,170','86,154',99,1)">DOWN</a></td><td><a href="javascript:ecmd_set_rf2('tevion','77,42,170','86,86',99,100)">LERN</a></td></tr>
<tr><td>2</td><td><a href="javascript:ecmd_set_rf2('tevion','77,42,170','149,89',99,2)">OFF</a> <a href="javascript:ecmd_set_rf2('tevion','77,42,170','150,90',99,2)">ON</a></td><td><a href="javascript:ecmd_set_rf2('tevion','77,42,170','149,149',99,1)">UP</a> <a href="javascript:ecmd_set_rf2('tevion','77,42,170','150,150',99,1)">DOWN</a></td><td><a href="javascript:ecmd_set_rf2('tevion','77,42,170','150,90',99,100)">LERN</a></td></tr>
<tr><td>3</td><td><a href="javascript:ecmd_set_rf2('tevion','77,42,170','165,90',99,2)">OFF</a> <a href="javascript:ecmd_set_rf2('tevion','77,42,170','166,89',99,2)">ON</a></td><td><a href="javascript:ecmd_set_rf2('tevion','77,42,170','165,150',99,1)">UP</a> <a href="javascript:ecmd_set_rf2('tevion','77,42,170','166,149',99,1)">DOWN</a></td><td><a href="javascript:ecmd_set_rf2('tevion','77,42,170','166,89',99,100)">LERN</a></td></tr>
<tr><td>4</td><td><a href="javascript:ecmd_set_rf2('tevion','77,42,170','101,86',99,2)">OFF</a> <a href="javascript:ecmd_set_rf2('tevion','77,42,170','102,85',99,2)">ON</a></td><td><a href="javascript:ecmd_set_rf2('tevion','77,42,170','101,154',99,1)">UP</a> <a href="javascript:ecmd_set_rf2('tevion','77,42,170','102,153',99,1)">DOWN</a></td><td><a href="javascript:ecmd_set_rf2('tevion','77,42,170','102,85',99,100)">LERN</a></td></tr>
<tr><td>all</td><td><a href="javascript:ecmd_set_rf2('tevion','77,42,170','169,86',99,2)">OFF</a> <a href="javascript:ecmd_set_rf2('tevion','77,42,170','170,85',99,2)">ON</a></td><td><a href="javascript:ecmd_set_rf2('tevion','77,42,170','169,154',99,1)">UP</a> <a href="javascript:ecmd_set_rf2('tevion','77,42,170','170,153',99,1)">DOWN</a></td><td></td></tr>
</table>
}})
<br>
<a href="idx.ht"> Back </a>
<div id="logconsole"></div>
</body>
</html>
