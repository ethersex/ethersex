changequote({{,}})dnl
ifdef({{conf_GAME_INPUT}}, {{}}, {{m4exit(1)}})dnl
ifdef({{conf_GAME_INPUT_INLINE}}, {{}}, {{m4exit(1)}})dnl
<html>
<head>
<title>Ethersex - Game Input</title>
<link rel="stylesheet" href="Sty.c" type="text/css"/>
<script src="scr.js" type="text/javascript"></script>
<script type="text/javascript">
function push(b) {
	ArrAjax.ecmd('push+' + b);
}
</script>
<style>
td {
 height:50px;
 width:50px;
 text-align:center;
 padding:5px;
 font-size:10px;
}
.b {
  background-color:lime;
}
</style>
</head>
<body>
<h1>Game Input</h1>
<table class="c">
<tr>
<td></td>
<td class="b" onclick="push(0)">UP</td>
<td></td>
</tr>
<tr>
<td class="b" onclick="push(2)">LEFT</td>
<td></td>
<td class="b" onclick="push(3)">RIGHT</td>
</tr>
<tr>
<td></td>
<td class="b" onclick="push(1)">DOWN</td>
<td></td>
</tr>
</table>
<br>
<table class="f">
<tr>
<td class="b" onclick="push(4)">FIRE1</td>
<td class="b" onclick="push(5)">FIRE2</td>
<td class="b" onclick="push(6)">FIRE3</td>
<td class="b" onclick="push(7)">FIRE4</td>
</tr>
</table>

<br>
<a href="idx.ht"> Back </a>
<div id="logconsole"></div>
</body>
</html>
