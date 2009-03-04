<html><head>
changequote({{,}})dnl
ifdef({{conf_CAMERA}}, {{}}, {{m4exit(1)}})dnl
<title>Ethersex - Camera</title>
<link rel="StyleSheet"  href="Sty.c" type="text/css" />
<script src="scr.js" type="text/javascript"></script>
<script type="text/javascript">
ifdef({{conf_VFS_DC3840}}, {{
function call(cmd) {
   ArrAjax.aufruf('/ecmd?'+cmd);
}
function capture() {
   ArrAjax.aufruf('/ecmd?dc3840+capture');
   window.setTimeout("cont()", 1500);
}
function cont() {
   location.href="?";
}
}})
</script>
</head>
<body>
<h1>Camera</h1>
ifdef({{conf_VFS_DC3840}}, {{
<a href="javascript:capture();"><img src="/dc3840"></a>
<br>
<br><a href="?">reload</a><br>
<a href="javascript:call('$servo+left');">left<a/>
<a href="javascript:call('$servo+center');">center<a/>
<a href="javascript:call('$servo+right');">right<a/>
}})
<br>
<br>
<br>
<a href="idx.ht"> Back </a>
<div id="logconsole"></div>
</body>
</html>
