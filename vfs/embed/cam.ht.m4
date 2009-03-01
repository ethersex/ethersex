<html><head>
changequote({{,}})dnl
ifdef({{conf_CAMERA}}, {{}}, {{m4exit(1)}})dnl
<title>Ethersex - Camera</title>
<link rel="StyleSheet"  href="Sty.c" type="text/css" />
<script src="scr.js" type="text/javascript"></script>
<script type="text/javascript">
function capture() {
   var url = '/ecmd?dc3840+capture';
   ArrAjax.aufruf(url);
}
</script>
</head>
<body>
<h1>Camera</h1>

ifdef({{conf_DC3840}}, {{
<a href="javascript:capture();">capture DC3440</a>
<img src="/dc3840">
}})

<br>
<a href="idx.ht"> Back </a>
<div id="logconsole"></div>
</body>
</html>
