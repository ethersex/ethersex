changequote({{,}})dnl
ifdef({{conf_CAMERA}}, {{}}, {{m4exit(1)}})dnl
ifdef({{conf_CAMERA_INLINE}}, {{}}, {{m4exit(1)}})dnl
ifdef({{conf_VFS_DC3840}}, {{}}, {{m4exit(1)}})dnl
<html>
<head>
<title>Ethersex - Camera</title>
<link rel="stylesheet" href="Sty.c" type="text/css"/>
<script src="scr.js" type="text/javascript"></script>
<script type="text/javascript">
ifdef({{conf_VFS_DC3840}}, {{
function capture() {
	ArrAjax.ecmd('dc3840+capture');
}
}})
</script>
</head>
<body>
<h1>Camera</h1>
ifdef({{conf_VFS_DC3840}}, {{
<a href="javascript:capture();">capture DC3440</a>
<a href="?">reload</a><br>
<img src="/dc3840">
}})
<br>
<a href="idx.ht"> Back </a>
<div id="logconsole"></div>
</body>
</html>
