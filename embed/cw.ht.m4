changequote({{,}})dnl
ifdef({{conf_CW}}, {{}}, {{m4exit(1)}})dnl
ifdef({{conf_CW_INLINE}}, {{}}, {{m4exit(1)}})dnl
<html>
<head>
<title>Ethersex - Morse code</title>
<link rel="stylesheet" href="Sty.c" type="text/css"/>
<script src="scr.js" type="text/javascript"></script>
</head>
<script type="text/javascript">
function send(value) {
	ArrAjax.ecmd('cw+send+' + value);
}
</script>
<body>
<h1>Morse code</h1>
<input type="text" name="msg" onChange="send(this.value);this.value='';">
<br>
<br>
<br>
<a href="idx.ht"> Back </a>
<div id="logconsole"></div>
</body>
</html>
