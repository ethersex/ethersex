ifdef(`conf_VFS_IO_INLINE', `', `m4exit(1)')dnl
undefine(`substr')dnl
<html>
<head>
<title>Ethersex - IO Control</title>
<link rel="stylesheet" href="Sty.c" type="text/css"/>
<script src="scr.js" type="text/javascript"></script>
<script type="text/javascript">
function ecmd_get_io(type, num, handler) {
	var data = new Object();
	data.type = type;
	data.num = num;
	ArrAjax.ecmd('io get ' + type + ' ' + num, handler, 'GET', data);
}

function byte2hex(data) {
	var hex = data.toString(16);
	return (hex.length == 1) ? '0'+hex : hex;
}

function ecmd_set_io(type, portnum, num, value) {
	var i = (value) ? 1 : 0;
	var url = 'io set ' + type + ' ' + portnum + " "  +  byte2hex(i << num)
		+ ' ' +byte2hex(1<<num);
	ArrAjax.ecmd(url);
}

function ecmd_parse_io(str) {
	if (ecmd_error(str))
		 return undefined;
	return parseInt(str.substr(str.indexOf("0x")+2,2), 16);
}

function write_port(request, data) {
	var value = ecmd_parse_io(request.responseText);
	for (var i = 0; i < 8; i++) {
		var id = data.type + data.num + i;
		var obj = returnObjById(id);
		obj.checked = (value & (1 << i)) ? true : false;
		obj.checked_ = obj.checked;
	}
}

function mask_port(request, data) {
	var value = ecmd_parse_io(request.responseText);
	for (var i = 0; i < 8; i++) {
		if (value & (1 << i))  {
			var ddr = returnObjById("ddr" + data.num + i);
			ddr.setAttribute("onchange", "javascript:pin_set_back(this)");
			ddr.setAttribute("readonly", true);
			var obj = returnObjById("port" + data.num + i);
			obj.setAttribute("onchange", "javascript:pin_set_back(this)");
			obj.setAttribute("readonly", true);
		}
	}
}

function generate_port(port, change_handler) {
	document.write('<table class="iotable"><tr><th>Nr</th><th>DDR'+ port
		       + "</th><th>PORT" + port + "</th><th>PIN" + port +" </th></tr>");
	for (var i = 0; i < 8; i++) {
		var tr = "<tr><td>" + i + "</td>";
		tr += "<td><input id='ddr" + port + i +"'" +
			"type='checkbox' onchange='javascript:" + change_handler + "'/></td>";
		tr += "<td><input id='port" + port + i + "'" +
			"type='checkbox' onchange='javascript:" + change_handler + "'/></td>";
		tr += "<td><input id='pin" + port + i + "'" +
			"type='checkbox' onchange='javascript:readonly_cb(this)' readonly/></td>";
		tr += "</tr>";
		document.write(tr);
	}
	document.write("</table>");
	ecmd_get_io("pin", port, write_port);
	setInterval('ecmd_get_io("pin", ' + port + ', write_port)', 5000);

	ecmd_get_io("ddr", port, write_port);
	setInterval('ecmd_get_io("ddr", ' + port + ', write_port)', 5000);

	ecmd_get_io("port", port, write_port);
	setInterval('ecmd_get_io("port", ' + port + ', write_port)', 5000);

	ecmd_get_io("mask", port, mask_port);
}

function readonly_cb(obj) {
	obj.checked = false;
}

function pin_set_back(obj) {
	obj.checked = obj.checked_;
}

function port_change(id) {
	var obj = returnObjById(id);
	var type = id.replace(/[0-9]/g, "");
	var portnum = parseInt(id.substr(type.length, 1));
	var num = parseInt(id.substr(type.length + 1, 1));

	ecmd_set_io(type, portnum, num, obj.checked);
}
</script>
</head><body>
<h1>Ethersex IO Control</h1>
<center>
<table>
	<tr>
		<td><script type="text/javascript">generate_port(0, "port_change(this.id)");</script></td>
		<td><script type="text/javascript">generate_port(1, "port_change(this.id)");</script></td>
		<td><script type="text/javascript">generate_port(2, "port_change(this.id)");</script></td>
		<td><script type="text/javascript">generate_port(3, "port_change(this.id)");</script></td>
	</tr>
</table>
</center>

<div id="logconsole"></div>
</body>
</html>
