changequote({{,}})dnl
ifdef({{conf_MCUF_MODUL}}, {{}}, {{m4exit(1)}})dnl
ifdef({{conf_MCUF_INLINE}}, {{}}, {{m4exit(1)}})dnl
<html>
<head>
<title>Ethersex - MCUF Modules</title>
<link rel="stylesheet" href="Sty.c" type="text/css"/>
<script src="scr.js" type="text/javascript"></script>
<script type="text/javascript">
function mcuf(cid) {
	ArrAjax.ecmd('mcuf+modul+' + cid);
}
</script>
</head>
<body>
<h1>MCUF Modules</h1>
<ul>
<li><a href="javascript:mcuf(0)">Game Input</a>
<li><a href="javascript:mcuf(1)">xoni</a>
</ul>
<br>
<a href="idx.ht"> Back </a>
<div id="logconsole"></div>
</body>
</html>
