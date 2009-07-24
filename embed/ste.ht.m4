ifdef(`conf_STELLA', `', `m4exit(1)')dnl
ifdef(`conf_STELLA_INLINE', `', `m4exit(1)')dnl
<html><head>
<title>StellaLight Control</title>
<link rel="StyleSheet"  href="Sty.c" type="text/css" />

<script src="scr.js" type="text/javascript"></script>
<script type="text/javascript">

var initial_fetch = false;

function stella_inittable(request) {
	ow_table = $('channels_table');
	ow_caption = $('caption');
	if (ecmd_error(request)) {
		ow_caption.innerHTML = "Fetching...Error!";
		return;
	}

	initial_fetch = true;
	channels = request.responseText.split("\n");

	for (var i = 0; i < channels.length-1; i++) {
		txt = "<td>Channel "+i+"</td>";
		for (var j = 0;j <= 10; j++) {
			txt += "<td><a href='javascript:stella("+i+","+Math.round(j*25.5)+");'>"+j*10+"%</a></td>";
		}
		txt += "<td>Current: <span id='current"+i+"'></span>%</td>";
		ow_table.insertRow(i+1).innerHTML = txt;
	}

	ow_caption.innerHTML = (channels.length-1) + " Channels loaded.";
	stella_update();
}

function stella_updatetable(request) {
	ow_caption = $('caption');
	upind = $('upind');
	if (ecmd_error(request)) {
		return;
	}

	channels = request.responseText.split("\n");
	
	for (var i = 0; i < channels.length-1; i++) {
		$('current'+i).innerHTML = Math.round(parseInt(channels[i])*100/255);
	}
	upind.innerHTML = "";
}

function stella_update() {
	upind = $('upind');
	if (!initial_fetch) {
		upind.innerHTML = "F";
		ArrAjax.ecmd('channel', stella_inittable);
	} else {
		upind.innerHTML = "U";
		ArrAjax.ecmd('channel', stella_updatetable);
	}
}

function stella(channel, val) {
	ArrAjax.ecmd('channel+' + channel + '+' + val);
	$('current'+channel).innerHTML = Math.round(parseInt(val)*100/255) + "U";
}

window.onload = function() {
	setInterval('stella_update();', 5000);
	stella_update();
}
</script>
</head>
<body>
	<h3>Stella Channels</h3>
	<div><span id="caption">Loading...</span> <span id="upind"></span></div>
	<table id="channels_table"><tr><td></td></tr></table>
</body>
</html>
