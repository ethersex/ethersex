changequote({{,}})dnl
ifdef({{conf_LCD}}, {{}}, {{m4exit(1)}})dnl
ifdef({{conf_LCD_INLINE}}, {{}}, {{m4exit(1)}})dnl
<html>
<head>
<title>Ethersex - LCD</title>
<link rel="stylesheet" href="Sty.c" type="text/css"/>
<script src="scr.js" type="text/javascript"></script>
<script type="text/javascript">
function ecmd(cmd) {
	ArrAjax.ecmd(cmd);
}
</script>
</head>
<body>
<h1>LCD</h1>
<form action="?" name="form">
<input type="text" name="msg"> <input type="button" value="write" onclick="ecmd('lcd+write+'+window.document.form.msg.value.replace(/ /g,'+'))"> <input type="button" value="clear" onclick="ecmd('lcd+clear')"><br>
<input type="button" value="test" onclick="ecmd('lcd+write+hello+world')">
</form>
<br>
<br>
<a href="idx.ht"> Back </a>
<div id="logconsole"></div>
</body>
</html>
