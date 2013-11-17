changequote({{,}})dnl
ifdef({{conf_PWM_SERVO}}, {{}}, {{m4exit(1)}})dnl
ifdef({{conf_PWM_SERVO_INLINE}}, {{}}, {{m4exit(1)}})dnl
<html>
<head>
<title>Ethersex - PWM Servo</title>
<link rel="stylesheet" href="Sty.c" type="text/css"/>
<script src="scr.js" type="text/javascript"></script>
<style>
.pos { 
  margin-bottom:5px;
}
.pos a{ 
  padding:0 10px 0 10px;
  background-color:green;
  text-decoration:none
}
.odd a{
  background-color:#006600;
}
</style>
</head>
<body>
<h1>PWM Servo</h1>
<script type="text/javascript">
function pos(s,p) {
	ArrAjax.ecmd('pwm+servo_set+'+s+'+'+p);
}
function inc(s) {
	ArrAjax.ecmd('pwm+servo_inc+'+s);
}
function dec(s) {
	ArrAjax.ecmd('pwm+servo_dec+'+s);
}
 for (var i=0;i<10;i++) {
   document.write(" <div class='pos "+(i%2?"odd":"")+"'>Servo "+i+" <a href='javascript:pos("+i+",205)'>&lt;&lt;</a> <a href='javascript:pos("+i+",210)'>&lt;</a> ");
   document.write("<a href='javascript:dec("+i+")'>-</a> ");
   document.write("<a href='javascript:pos("+i+",220)'>***</a> ");
   document.write("<a href='javascript:inc("+i+")'>+</a> ");
   document.write("<a href='javascript:pos("+i+",230)'>&gt;</a> <a href='javascript:pos("+i+",240)'>&gt;&gt;</a> </div>");
 }
</script>
<br>
<a href="idx.ht"> Back </a>
<div id="logconsole"></div>
</body>
</html>

