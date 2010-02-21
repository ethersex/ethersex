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
function dc3840(c,v) {
	ArrAjax.ecmd('dc3840+'+c+'+'+v);
}
function capture() {
	ArrAjax.ecmd('dc3840+sync');
	ArrAjax.ecmd('dc3840+capture');
    window.setTimeout("location.href='/cam.ht'",1500);
}
}})
ifdef({{conf_PWM_SERVO}}, {{
function sinc() {
	ArrAjax.ecmd('pwm+servo_inc 0');
        window.setTimeout("capture()",250);
}
function sdec() {
	ArrAjax.ecmd('pwm+servo_dec 0');
        window.setTimeout("capture()",250);
}
}})
</script>
</head>
<body>
<h1>Camera</h1>
ifdef({{conf_VFS_DC3840}}, {{
<a href="javascript:capture();">capture DC3840</a><br>
<img src="/dc3840"><br>
<a href="?">reload</a><br>
Zoom <a href="javascript:dc3840('zoom',1);">on</a> <a href="javascript:dc3840('zoom',0);">off</a><br>
Bright <a href="javascript:dc3840('light',1);">on</a> <a href="javascript:dc3840('light',0);">off</a><br>
}})
<br>
ifdef({{conf_PWM_SERVO}}, {{
Servo pos: <a href="javascript:sdec();">dec</a> - <a href="javascript:sinc();">inc</a>
<br>
}})
<br>
<a href="idx.ht"> Back </a>
<div id="logconsole"></div>
</body>
</html>
