ifdef(`conf_DMX_STORAGE', `', `m4exit(1)')dnl
ifdef(`conf_DMX_STORAGE_INLINE', `', `m4exit(1)')dnl

<html><head>
<title>DMX Control Panel</title>
<link rel="StyleSheet"  href="Sty.c" type="text/css" />


<script src="scr.js" type="text/javascript"></script>
<script type="text/javascript">
var currentUniverse;
var totalUniverses;
function dmx_init_channels(request) {
	space = $('channels');

	if (ecmd_error(request)) {
		caption.innerHTML = "Fetching error!";
		return;
	}

	var channels_source = request.responseText.split("\n");
	var channels = channels_source.slice(0,channels_source.length-1); 
	var divCount = 0;

        caption.innerHTML = (channels.length) + " Channels loaded.";
	
	
	for (var i = 0; i < channels.length; i++) {
		if(!(i%16))
		{
			var newBigDiv = document.createElement("div");
			newBigDiv.style.float = "left";
			newBigDiv.style.margin = "10px";
			newBigDiv.id = "bigDiv" + divCount;
			space.appendChild(newBigDiv);
			divCount++;
		}
		var newSDiv = document.createElement("div");
		newSDiv.innerHTML = '<span style="width:30px;display:inline-block">' +i+ '</span>';
		newSDiv.innerHTML += '<input style="width:510px" id="range'+i+'" type="range" min="0" max="255" value="'+channels[i]+'" onchange="dmx_set('+i+',this.value)"/>';
		newSDiv.innerHTML += '<span style="margin-left:10px" id="current'+i+'">'+channels[i]+'</span>';
		$('bigDiv'+(divCount-1)).appendChild(newSDiv);
	}
}

function dmx_updatetable(request) {
	upind = $('upind');

	if (ecmd_error(request)) return;
	upind.innerHTML = "";

	var channels_source = request.responseText.split("\n");
	var channels = channels_source.slice(0,channels_source.length-1); 
	
	for (var i = 0; i < channels.length; i++) {
		$('current'+i).innerHTML = channels[i];
		$('range'+i).value = channels[i];
	}
}

function dmx_update() {
	upind = $('upind');
	upind.innerHTML = 'Refreshing...';
	ArrAjax.ecmd('dmx universe ' + currentUniverse , dmx_updatetable);
}

function dmx_set(channel, val) {
        ArrAjax.ecmd('dmx set ' +currentUniverse+ ' ' +channel+ ' ' +val);
        $('current'+channel).innerHTML = val;
}
function dmx_set_universe(value) {
	currentUniverse = value;
	for (var i = 0; i < totalUniverses; i++)
	{
		if(i == currentUniverse)
                	$('currentUniverse'+i).style.backgroundColor = "#61D800";
		else
                	$('currentUniverse'+i).style.backgroundColor = "";
	}
	dmx_update();
}
function dmx_init_universes(request) {
	if (ecmd_error(request)) {
		caption.innerHTML = "Fetching error!";
		return;
	}
	var n = request.responseText[0];
	totalUniverses=n;
	for (var i = 0; i < n ; i++)
	{
		var newUDiv = document.createElement("span");
		if(i == currentUniverse)
                	newUDiv.style.backgroundColor = "#61D800";
		newUDiv.id = "currentUniverse"+i;
                newUDiv.style.padding = "5px";
		newUDiv.innerHTML = '<a href="#" onclick="dmx_set_universe('+i+')">'+i+'</a>';
                universeSelection.appendChild(newUDiv);
	}
}
window.onload = function() {
	ow_caption = $('caption');
        ow_caption.innerHTML = "Fetching channels...";
	setInterval('dmx_update();', 5000);
	dmx_set_universe(0);
	ArrAjax.ecmd('dmx universe ' + currentUniverse, dmx_init_channels);
	ArrAjax.ecmd('dmx universes', dmx_init_universes);
}
</script>
</head>
<body>
	<h3>DMX Channels</h3><span style="margin-bottom:10px;" id="universeSelection"><span style="padding:5px">Select universe</span></span>
	<div style="padding: 5px"><span id="caption">Please activate Javascript.</span><span id="upind"></span></div>
	<div id="channels"></div>
</body>
</html>
