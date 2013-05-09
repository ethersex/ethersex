divert(-1)
define(`hdr_divert', 0)
define(`hdr_end_divert', 2)
define(`output_divert', 3)
define(`input_divert', 4)
define(`end_divert', 5)
divert(hdr_divert)
<html><head>
<title>Ethersex - Named Pins</title>
<link rel="stylesheet" href="Sty.c" type="text/css"/>
<script src="scr.js" type="text/javascript"></script>
<script type="text/javascript">
function ecmd_send(cmd, handler, obj) {
	var data = new Object();
	data.cmd = cmd;
	data.obj = obj;
	ArrAjax.ecmd(cmd, handler, 'GET', data);
}

function pin_set(pin, state) {
	ecmd_send('pin set ' + pin + ' ' + state);
	ecmd_send('pin get ' + pin, pin_get, pin);
}

function pin_get(request, data) {
	var obj = $(data.obj);
	obj.innerHTML = request.responseText;
	var on = request.responseText.indexOf("on");
	obj.style.backgroundColor = (on != -1) ? "green" : "#444";
}

function np_init() {
divert(hdr_end_divert)
}

divert(output_divert)
</script>
</head>
<body onLoad="javascript:np_init()">
<h1>Named Pins</h1>
<table><tr><td valign="top">
  <h3>Outputs</h3>
  <table>
divert(input_divert)
</table></td><td valign="top">
<table>
<h3>Inputs</h3>
divert(end_divert)
</table>
</td></tr></table>
<a href="idx.ht"> Back </a>
<div id="logconsole"></div>
</body>
</html>
divert(-1)dnl
changequote({{,}})dnl
define({{np_output}}, {{divert(output_divert)dnl
<tr><td>$3</td>
    <td><a class="small_button" 
 	href="javascript:pin_set('$3', 'on');">On</a></td>
    <td><a class="small_button" 
	href="javascript:pin_set('$3', 'off');">Off</a></td>
    <td><span id="$3" class="lamp"></span></td>
</tr>
divert(hdr_divert)
ecmd_send('pin get $3', pin_get, "$3");
setInterval('ecmd_send("pin get $3", pin_get, "$3");', 5000);}})
define({{np_input}}, {{divert(input_divert)dnl
<tr>
    <td>$3</td><td><span id="$3" class="lamp"></span></td>
</tr>
divert(hdr_divert)
	ecmd_send('pin get $3', pin_get, "$3");
	setInterval('ecmd_send("pin get $3", pin_get, "$3");', 5000);
}})
define({{np_pin}}, {{ifelse({{$2}}, {{OUTPUT}}, 
		    {{np_output({{$1}}, {{$3}}, {{$4}})}},
		    {{np_input({{$1}}, {{$3}}, {{$4}})}})}}
)
divert(0)dnl
