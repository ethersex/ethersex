changequote({{,}})dnl
ifdef({{conf_I2C_MASTER}}, {{}}, {{m4exit(1)}})dnl
ifdef({{conf_I2C_INLINE}}, {{}}, {{m4exit(1)}})dnl
<html>
<head>
<title>Ethersex - I2C Master</title>
<link rel="stylesheet" href="Sty.c" type="text/css"/>
<script src="scr.js" type="text/javascript"></script>
<script type="text/javascript">
function ecmd_set(cmd, ic, value) {
	ArrAjax.ecmd(cmd + '+' + ic + '+' + value);
}

</script>
</head>
<body>
<h1>I<sup>2</sup>C</h1>
ifdef({{conf_I2C_DETECT}}, {{
<a href="/ecmd?i2c+detect">detect ICs</a><br>
<br>
}})
ifdef({{conf_I2C_LM75}}, {{
Temperatur<br>
<a href="/ecmd?lm75+0">get Sensor 0</a><br>
<a href="/ecmd?lm75+1">get Sensor 1</a><br>
<br>
}})
ifdef({{conf_I2C_TMP175}}, {{
Temperatur<br>
<a href="/ecmd?tmp175+0">get Sensor 0</a><br>
<a href="/ecmd?tmp175+1">get Sensor 1</a><br>
<br>
}})
ifdef({{conf_I2C_PCA9531}}, {{
PCA9531 8-bit LED dimmer 0<br>
<a href="javascript:ecmd_set('pca9531',0,'0+0+0+0+0+0')">set all I/O to OFF<a/><br>
<a href="javascript:ecmd_set('pca9531',0,'0+0+0+0+55+55')">set all I/O to ON<a/><br>
Blink-LED at PWM0 and Servo at PWM1<br>
<a href="javascript:ecmd_set('pca9531',0,'10+80+2+da+ae+ee')">50hz 2 ms -16<a/><br>
<a href="javascript:ecmd_set('pca9531',0,'10+80+2+e0+ae+ee')">50hz 2 ms -10<a/><br>
<a href="javascript:ecmd_set('pca9531',0,'10+80+2+e5+ae+ee')">50hz 2 ms -5<a/><br>
<a href="javascript:ecmd_set('pca9531',0,'10+80+2+e8+ae+ee')">50hz 2 ms -2<a/><br>
<a href="javascript:ecmd_set('pca9531',0,'10+80+2+e9+ae+ee')">50hz 2 ms -1<a/><br>
<a href="javascript:ecmd_set('pca9531',0,'20+80+2+ea+ae+ee')">50hz 2 ms<a/><br>
<a href="javascript:ecmd_set('pca9531',0,'10+80+2+eb+ae+ee')">50hz 2 ms +1<a/><br>
<a href="javascript:ecmd_set('pca9531',0,'10+80+2+ec+ae+ee')">50hz 2 ms +2<a/><br>
<a href="javascript:ecmd_set('pca9531',0,'10+80+2+ef+ae+ee')">50hz 2 ms +5<a/><br>
<a href="javascript:ecmd_set('pca9531',0,'10+80+2+f5+ae+ee')">50hz 2 ms +10<a/><br>
<a href="javascript:ecmd_set('pca9531',0,'10+80+2+fa+ae+ee')">50hz 2 ms +16<a/><br>
<br>
}})
ifdef({{conf_I2C_PCF8574X}}, {{
PCF8574<br>
<a href="javascript:ecmd_set('pcf8574x+set',0,'0+0')">I/O to OFF<a/><br>
<a href="javascript:ecmd_set('pcf8574x+set',0,'0+ff')">I/O to ON<a/><br>
<br>
}})
<br>
<a href="idx.ht"> Back </a>
<div id="logconsole"></div>
</body>
</html>
