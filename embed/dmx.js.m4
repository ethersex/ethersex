ifdef(`conf_DMX_STORAGE', `', `m4exit(1)')dnl
ifdef(`conf_DMX_STORAGE_INLINE', `', `m4exit(1)')dnl
var currentUniverse;
var totalUniverses;
function dmx_init_channels(request) {
	if (ecmd_error(request)) {
		document.getElementById("caption").innerHTML = "Fetching error!";
		return;
	}
	var space = document.getElementById("channels");
	var channels_source = request.responseText.split("\n");
	var channels = channels_source.slice(0, channels_source.length - 1);
	var divCount = 0;
	var i = 0;
        document.getElementById("caption").innerHTML = (channels.length) + " Channels loaded.";
	for (i = 0; i < channels.length; i++) {
		if ( (i % 16) === 0 )
		{
			var newBigDiv = document.createElement("div");
			newBigDiv.style.cssFloat = "left";
			newBigDiv.style.margin = "10px";
			newBigDiv.id = "bigDiv" + divCount;
			space.appendChild(newBigDiv);
			divCount++;
		}
		var newSDiv = document.createElement("div");
		newSDiv.innerHTML = '<span style="width:30px;display:inline-block">' +i+ '</span>';
		newSDiv.innerHTML += '<input style="width:510px" id="range'+i+'" type="range" min="0" max="255" value="'+channels[i]+'" onchange="dmx_set('+i+',this.value)"/>';
		newSDiv.innerHTML += '<span style="margin-left:10px" id="current'+i+'">'+channels[i]+'</span>';
		document.getElementById("bigDiv"+(divCount-1)).appendChild(newSDiv);
	}
}

function dmx_updatetable(request) {
	var i = 0;

	if (ecmd_error(request))
	{
		return;
	}
	document.getElementById("upind").innerHTML = "";

	var channels_source = request.responseText.split("\n");
	var channels = channels_source.slice(0,channels_source.length - 1); 
	
	for (i = 0; i < channels.length; i++) {
		document.getElementById("current"+i).innerHTML = channels[i];
		document.getElementById("range"+i).value = channels[i];
	}
}

function dmx_update() {
	document.getElementById("upind").innerHTML = 'Refreshing...';
	ArrAjax.ecmd("dmx universe " + currentUniverse , dmx_updatetable);
}

function dmx_set(channel, val) {
        ArrAjax.ecmd('dmx set ' +currentUniverse+ ' ' +channel+ ' ' + val);
        document.getElementById("current" + channel).innerHTML = val;
}
function dmx_set_universe(value) {
	var i = 0;
	currentUniverse = value;
	for (i = 0; i < totalUniverses; i++)
	{
		if (i === currentUniverse)
		{
			document.getElementById("currentUniverse" + i).style.backgroundColor = "#61D800";
		}
		else
		{
			document.getElementById("currentUniverse" + i).style.backgroundColor = "";
		}
	}
	dmx_update();
}
function dmx_init_universes(request) {
	var i = 0;
	if (ecmd_error(request)) {
        	document.getElementById("caption").caption.innerHTML = "Fetching error!";
		return;
	}
	var n = request.responseText[0];
	totalUniverses=n;
	for (i = 0; i < n ; i++)
	{
		var newUDiv = document.createElement("span");
		if(i === currentUniverse)
		{
			newUDiv.style.backgroundColor = "#61D800";
		}
		newUDiv.id = "currentUniverse"+i;
                newUDiv.style.padding = "5px";
		newUDiv.innerHTML = '<a href="#" onclick="dmx_set_universe('+i+')">'+i+'</a>';
		document.getElementById("universeSelection").appendChild(newUDiv);
	}
}
window.onload = function() {
	ow_caption = document.getElementById("caption").innerHTML = "Fetching channels...";
	setInterval( function () { dmx_update(); } , 5000);
	dmx_set_universe(0);
	ArrAjax.ecmd('dmx universe ' + currentUniverse, dmx_init_channels);
	ArrAjax.ecmd('dmx universes', dmx_init_universes);
}
