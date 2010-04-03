ifdef(`conf_MOTORCURTAIN', `', `m4exit(1)')dnl
ifdef(`conf_MOTORCURTAIN_INLINE', `', `m4exit(1)')dnl
<html><head>
<title>Curtain Control</title>
<link rel="StyleSheet"  href="Sty.c" type="text/css" />
<script src="scr.js" type="text/javascript"></script>
<script type="text/javascript">
function pos(val) {
	ArrAjax.ecmd('curtain+' + val);
	return true;
}
</script>
</head>
<body>
	<h3>Curtain</h3>
	<table id="table"><tr>
	<td>Closed</td>
	<td><a class="small_button" href="#" onclick="pos(0);">0</a></td>
	<td><a class="small_button" href="#" onclick="pos(1);">1</a></td>
	<td><a class="small_button" href="#" onclick="pos(2);">2</a></td>
	<td><a class="small_button" href="#" onclick="pos(3);">3</a></td>
	<td><a class="small_button" href="#" onclick="pos(4);">4</a></td>
	<td><a class="small_button" href="#" onclick="pos(5);">5</a></td>
	<td><a class="small_button" href="#" onclick="pos(6);">6</a></td>
	<td><a class="small_button" href="#" onclick="pos(7);">7</a></td>
	<td>Opened</td>
	</tr></table>
</body>
</html>
