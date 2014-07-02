changequote({{,}})dnl
ifdef({{conf_RADIO_INLINE}}, {{}}, {{m4exit(1)}})dnl
<html>
<head>
<title>Ethersex - Radio Control</title>
<link rel="stylesheet" href="Sty.c" type="text/css"/>
<script src="scr.js" type="text/javascript"></script>
<script type="text/javascript">

ifdef({{conf_PROTO_2272}}, {{
function ecmd_set_rf1(type, code, delay, repeat) {
	var url = 'ask+' + type + '+' + code + '+' + delay + '+' + repeat;
	ArrAjax.ecmd(url);
}
}})
ifdef({{conf_PROTO_TEVION}}, {{
function ecmd_set_rf2(type, code, cmd, delay, repeat) {
	var url = 'ask+' + type + '+' + code + '+' + cmd + '+' + delay + '+' + repeat;
	ArrAjax.ecmd(url);
}
}})
ifdef({{conf_PROTO_1527}}, {{
function ecmd_set_rf3(type, code, delay, repeat) {
	var url = 'ask+' + type + '+' + code + '+' + delay + '+' + repeat;
	ArrAjax.ecmd(url);
}
}})
ifdef({{conf_PROTO_INTERTECHNO}}, {{
function ecmd_set_rf4(command) {
   var family = document.getElementsByName("Family")[0].value;
   var group = document.getElementsByName("Group")[0].value;
   var device = document.getElementsByName("Device")[0].value;
   var url = 'ask+intertechno+' + family + '+' + group + '+' + device + '+' + command;
	ArrAjax.ecmd(url);
}
}})
ifdef({{conf_FS20_SEND}}, {{
function ecmd_set_rfs20(acode, bcode, wert) {
	var url = 'fs20+' + 'send+' + '+' + acode + '+' + bcode + '+' + wert;
	ArrAjax.ecmd(url);
}
}})

</script>
</head>
<body>

ifdef({{conf_FS20_SEND}},{{
<h1>FS20</h1>
<h2>ELV-FS20-Steckdosen</h2>
<table width="50%" border="1" style="text-align: center">
<tr><th>FBD</th><th>CODE</th><th>EIN</th><th>AUS</th></tr>


<tr><td>1</td><td>2221</td><td><a href="javascript:ecmd_set_rfs20('0x3836','0x54','0x10')">ON</a></td><td><a href="javascript:ecmd_set_rfs20('0x3836','0x54','0x00')">OFF</a></td></tr>
<tr><td>2</td><td>2222</td><td><a href="javascript:ecmd_set_rfs20('0x3836','0x55','0x10')">ON</a></td><td><a href="javascript:ecmd_set_rfs20('0x3836','0x55','0x00')">OFF</a></td></tr>
<tr><td>3</td><td>2223</td><td><a href="javascript:ecmd_set_rfs20('0x3836','0x56','0x10')">ON</a></td><td><a href="javascript:ecmd_set_rfs20('0x3836','0x56','0x00')">OFF</a></td></tr>
<tr><td>4</td><td>2211</td><td><a href="javascript:ecmd_set_rfs20('0x3836','0x50','0x10')">ON</a></td><td><a href="javascript:ecmd_set_rfs20('0x3836','0x50','0x00')">OFF</a></td></tr>
<tr><td>5</td><td>2212</td><td><a href="javascript:ecmd_set_rfs20('0x3836','0x51','0x10')">ON</a></td><td><a href="javascript:ecmd_set_rfs20('0x3836','0x51','0x00')">OFF</a></td></tr>
<tr><td>6</td><td>2213</td><td><a href="javascript:ecmd_set_rfs20('0x3836','0x52','0x10')">ON</a></td><td><a href="javascript:ecmd_set_rfs20('0x3836','0x52','0x00')">OFF</a></td></tr>
<tr><td>1-ALL</td><td>2244</td><td><a href="javascript:ecmd_set_rfs20('0x3836','0x5F','0x10')">ON</a></td><td><a href="javascript:ecmd_set_rfs20('0x3836','0x5F','0x00')">OFF</a></td></tr>
<tr><td>2-ALL</td><td>4422</td><td><a href="javascript:ecmd_set_rfs20('0x3836','0xF5','0x10')">ON</a></td><td><a href="javascript:ecmd_set_rfs20('0x3836','0xF5','0x00')">OFF</a></td></tr>
<tr><td>4-ALL</td><td>4444</td><td><a href="javascript:ecmd_set_rfs20('0x3836','0xFF','0x10')">ON</a></td><td><a href="javascript:ecmd_set_rfs20('0x3836','0xFF','0x00')">OFF</a></td></tr>
</table>}})


<h1>Radio outlets (433MHz)</h1>
ifdef({{conf_PROTO_2272}}, {{
<h2>2272 (internal + external)</h2>
<table width="50%" border="1" style="text-align: center">
<tr><th>Switch</th><th colspan="4">Command</th></tr>
<tr><td>A</td><td><a href="javascript:ecmd_set_rf1(2272,'0,5,80',76,10)">OFF OFF</a></td><td><a href="javascript:ecmd_set_rf1(2272,'0,5,81',76,10)">ON OFF</a></td><td><a href="javascript:ecmd_set_rf1(2272,'0,5,84',76,10)">OFF ON</a></td><td><a href="javascript:ecmd_set_rf1(2272,'0,5,85',76,10)">ON ON</a></td></tr>
<tr><td>B</td><td><a href="javascript:ecmd_set_rf1(2272,'0,17,80',76,10)">OFF OFF</a></td><td><a href="javascript:ecmd_set_rf1(2272,'0,17,81',76,10)">ON OFF</a></td><td><a href="javascript:ecmd_set_rf1(2272,'0,17,84',76,10)">OFF ON</a></td><td><a href="javascript:ecmd_set_rf1(2272,'0,17,85',76,10)">ON ON</a></td></tr>
<tr><td>C</td><td><a href="javascript:ecmd_set_rf1(2272,'0,20,80',76,10)">OFF OFF</a></td><td><a href="javascript:ecmd_set_rf1(2272,'0,20,81',76,10)">ON OFF</a></td><td><a href="javascript:ecmd_set_rf1(2272,'0,20,84',76,10)">OFF ON</a></td><td><a href="javascript:ecmd_set_rf1(2272,'0,20,85',76,10)">ON ON</a></td></tr>
<tr><td>D</td><td><a href="javascript:ecmd_set_rf1(2272,'0,21,16',76,10)">OFF OFF</a></td><td><a href="javascript:ecmd_set_rf1(2272,'0,21,17',76,10)">ON OFF</a></td><td><a href="javascript:ecmd_set_rf1(2272,'0,21,20',76,10)">OFF ON</a></td><td><a href="javascript:ecmd_set_rf1(2272,'0,21,21',76,10)">ON ON</a></td></tr>
<tr><td>E</td><td><a href="javascript:ecmd_set_rf1(2272,'0,21,64',76,10)">OFF OFF</a></td><td><a href="javascript:ecmd_set_rf1(2272,'0,21,65',76,10)">ON OFF</a></td><td><a href="javascript:ecmd_set_rf1(2272,'0,21,68',76,10)">OFF ON</a></td><td><a href="javascript:ecmd_set_rf1(2272,'0,21,69',76,10)">ON ON</a></td></tr>
</table>
}})
ifdef({{conf_PROTO_TEVION}}, {{
<h2>Tevion</h2>
<table width="50%" border="1" style="text-align: center">
<tr><th>Switch</th><th colspan="3">Command</th></tr>
<tr><td>1</td><td><a href="javascript:ecmd_set_rf2('tevion','77,42,170','85,85',99,2)">OFF</a> <a href="javascript:ecmd_set_rf2('tevion','77,42,170','86,86',99,2)">ON</a></td><td><a href="javascript:ecmd_set_rf2('tevion','77,42,170','85,153',99,1)">UP</a> <a href="javascript:ecmd_set_rf2('tevion','77,42,170','86,154',99,1)">DOWN</a></td><td><a href="javascript:ecmd_set_rf2('tevion','77,42,170','86,86',99,100)">LEARN</a></td></tr>
<tr><td>2</td><td><a href="javascript:ecmd_set_rf2('tevion','77,42,170','149,89',99,2)">OFF</a> <a href="javascript:ecmd_set_rf2('tevion','77,42,170','150,90',99,2)">ON</a></td><td><a href="javascript:ecmd_set_rf2('tevion','77,42,170','149,149',99,1)">UP</a> <a href="javascript:ecmd_set_rf2('tevion','77,42,170','150,150',99,1)">DOWN</a></td><td><a href="javascript:ecmd_set_rf2('tevion','77,42,170','150,90',99,100)">LEARN</a></td></tr>
<tr><td>3</td><td><a href="javascript:ecmd_set_rf2('tevion','77,42,170','165,90',99,2)">OFF</a> <a href="javascript:ecmd_set_rf2('tevion','77,42,170','166,89',99,2)">ON</a></td><td><a href="javascript:ecmd_set_rf2('tevion','77,42,170','165,150',99,1)">UP</a> <a href="javascript:ecmd_set_rf2('tevion','77,42,170','166,149',99,1)">DOWN</a></td><td><a href="javascript:ecmd_set_rf2('tevion','77,42,170','166,89',99,100)">LEARN</a></td></tr>
<tr><td>4</td><td><a href="javascript:ecmd_set_rf2('tevion','77,42,170','101,86',99,2)">OFF</a> <a href="javascript:ecmd_set_rf2('tevion','77,42,170','102,85',99,2)">ON</a></td><td><a href="javascript:ecmd_set_rf2('tevion','77,42,170','101,154',99,1)">UP</a> <a href="javascript:ecmd_set_rf2('tevion','77,42,170','102,153',99,1)">DOWN</a></td><td><a href="javascript:ecmd_set_rf2('tevion','77,42,170','102,85',99,100)">LEARN</a></td></tr>
<tr><td>all</td><td><a href="javascript:ecmd_set_rf2('tevion','77,42,170','169,86',99,2)">OFF</a> <a href="javascript:ecmd_set_rf2('tevion','77,42,170','170,85',99,2)">ON</a></td><td><a href="javascript:ecmd_set_rf2('tevion','77,42,170','169,154',99,1)">UP</a> <a href="javascript:ecmd_set_rf2('tevion','77,42,170','170,153',99,1)">DOWN</a></td><td></td></tr>
</table>
}})
ifdef({{conf_PROTO_1527}}, {{
<h2>1527</h2>
<table width="50%" border="1" style="text-align: center">
<tr><th>Switch</th><th colspan="4">Command</th></tr>
<tr><td>1</td><td><a href="javascript:ecmd_set_rf3(1527,'37,181,87',76,10)">OFF</a></td><td><a href="javascript:ecmd_set_rf3(1527,'37,181,95',76,10)">ON</a></td><td><a href="javascript:ecmd_set_rf3(1527,'37,181,95',76,45)">UP</a></td><td><a href="javascript:ecmd_set_rf3(1527,'37,181,87',76,45)">DOWN</a></td><td><a href="javascript:ecmd_set_rf3(1527,'37,181,95',76,100)">LEARN</a></td></tr>
<tr><td>2</td><td><a href="javascript:ecmd_set_rf3(1527,'37,181,83',76,10)">OFF</a></td><td><a href="javascript:ecmd_set_rf3(1527,'37,181,91',76,10)">ON</a></td><td><a href="javascript:ecmd_set_rf3(1527,'37,181,91',76,45)">UP</a></td><td><a href="javascript:ecmd_set_rf3(1527,'37,181,83',76,45)">DOWN</a></td><td><a href="javascript:ecmd_set_rf3(1527,'37,181,91',76,100)">LEARN</a></td></tr>
<tr><td>3</td><td><a href="javascript:ecmd_set_rf3(1527,'37,181,85',76,10)">OFF</a></td><td><a href="javascript:ecmd_set_rf3(1527,'37,181,93',76,10)">ON</a></td><td><a href="javascript:ecmd_set_rf3(1527,'37,181,93',76,45)">UP</a></td><td><a href="javascript:ecmd_set_rf3(1527,'37,181,85',76,45)">DOWN</a></td><td><a href="javascript:ecmd_set_rf3(1527,'37,181,93',76,100)">LEARN</a></td></tr>
<tr><td>4</td><td><a href="javascript:ecmd_set_rf3(1527,'37,181,86',76,10)">OFF</a></td><td><a href="javascript:ecmd_set_rf3(1527,'37,181,94',76,10)">ON</a></td><td><a href="javascript:ecmd_set_rf3(1527,'37,181,94',76,45)">UP</a></td><td><a href="javascript:ecmd_set_rf3(1527,'37,181,86',76,45)">DOWN</a></td><td><a href="javascript:ecmd_set_rf3(1527,'37,181,94',76,100)">LEARN</a></td></tr>
<tr><td>all</td><td><a href="javascript:ecmd_set_rf3(1527,'37,181,88',76,10)">OFF</a></td><td><a href="javascript:ecmd_set_rf3(1527,'37,181,80',76,10)">ON</a></td><td><a href="javascript:ecmd_set_rf3(1527,'37,181,80',76,45)">UP</a></td><td><a href="javascript:ecmd_set_rf3(1527,'37,181,88',76,45)">DOWN</a></td></tr>
</table>
}})
ifdef({{conf_PROTO_INTERTECHNO}}, {{
<h2>Intertechno ITS-150</h2>
<table>
<tr><td>Familie</td><td>
<select name="Family" size="1">
<option value=1>A</option>
<option value=2>B</option>
<option selected value=3>C</option>
<option value=4>D</option>
<option value=5>E</option>
<option value=6>F</option>
<option value=7>G</option>
<option value=8>H</option>
<option value=9>I</option>
<option value=10>J</option>
<option value=11>K</option>
<option value=12>L</option>
<option value=13>M</option>
<option value=14>N</option>
<option value=15>O</option>
<option value=16>P</option>
</select>
</td></tr><tr><td>Gruppe</td><td>
<select name="Group" size="1">
<option>1</option>
<option>2</option>
<option selected>3</option>
<option>4</option>
</select>
</td></tr><tr><td>Geraet</td><td>
<select name="Device" size="1">
<option selected>1</option>
<option>2</option>
<option>3</option>
<option>4</option>
</select>
</td></tr><tr><td>Aktion</td><td>
<input type="button" name="Ein" value="Ein" onclick="ecmd_set_rf4(1)">
</td><td>
<input type="button" name="Aus" value="Aus" onclick="ecmd_set_rf4(0)">
</td></tr>
</table>
}})
<br>
<a href="idx.ht"> Back </a>
<div id="logconsole"></div>
</body>
</html>
