ifdef(`conf_STELLA', `', `m4exit(1)')dnl
ifdef(`conf_STELLA_INLINE', `', `m4exit(1)')dnl
<html><head>
<title>Ethersex - StellaLight Control</title>
<link rel="StyleSheet"  href="Sty.c" type="text/css" />

<script src="scr.js" type="text/javascript"></script>
<script type="text/javascript">

function stella_per(valuestr) {
	return Math.round(parseInt(valuestr)*100/255);
}

function stella_inittable(request) {
	ow_table = $('channels_table');
	ow_caption = $('caption');

	if (ecmd_error(request)) {
		ow_caption.innerHTML = "Fetching error!";
		return;
	}

	var channels_source = request.responseText.split("\n");
	var channels = channels_source.slice(1,channels_source.length-1); //remove first and last line

        ow_caption.innerHTML = (channels.length) + " Channels loaded.";

	for (var i = 0; i < channels.length; i++) {
		txt = "<td>Channel "+i+"</td>";
		for (var j = 0;j <= 10; j++) {
			txt += "<td><a href='javascript:stella("+i+","+Math.round(j*25.5)+");'>"+j*10+"%</a></td>";
		}
		txt += "<td>Current: <span id='current"+i+"'>"+stella_per(channels[i])+"</span>%</td>";
		ow_table.insertRow(i+1).innerHTML = txt;
	}
}

function stella_updatetable(request) {
	upind = $('upind');

	if (ecmd_error(request)) return;
	upind.innerHTML = "";

	var channels_source = request.responseText.split("\n");
	var channels = channels_source.slice(1,channels_source.length-1); //remove first and last line
	
	for (var i = 0; i < channels.length; i++) {
		$('current'+i).innerHTML = stella_per(channels[i]);
	}
}

function stella_update() {
	upind = $('upind');
	upind.innerHTML = "Updating";
	ArrAjax.ecmd('channel', stella_updatetable);
}

function stella(channel, val) {
	ArrAjax.ecmd('channel+' + channel + '+' + val);
	$('current'+channel).innerHTML = stella_per(val) + "U";
}

window.onload = function() {
	ow_caption = $('caption');
        ow_caption.innerHTML = "Fetching channels...";
	setInterval('stella_update();', 5000);
	ArrAjax.ecmd('channel', stella_inittable);
}
</script>
</head>
<body>
	<h3>Stella Channels</h3>
	<div><span id="caption">Kein Javascript aktiviert!</span> <span id="upind"></span></div>
	<table id="channels_table"><tr><td></td></tr></table>
</body>
</html>
