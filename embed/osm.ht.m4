ifdef(`conf_NMEA', `', `m4exit(1)')dnl
ifdef(`conf_NMEA_INLINE', `', `m4exit(1)')dnl
undefine(`substr')dnl
<html xmlns="http://www.w3.org/1999/xhtml">
  <head>
    <title>Ethersex - GPS - Openstreetmap</title>
    <link rel="stylesheet" href="../theme/default/style.css" type="text/css" />
    <link rel="stylesheet" href="style.css" type="text/css" />

    <style type="text/css">
        #map {
			float: left;
            width: 100%;
            height: 100%;
        }
    </style>

    <script src="http://www.openlayers.org/api/OpenLayers.js"></script>
    <script src="http://www.openstreetmap.org/openlayers/OpenStreetMap.js"></script>
    <script src="scr.js"></script>
    <script type="text/javascript">
        var map, marker = null;
	var layerMarkers;
	function nmea_handler(request, data) {
		var data = request.responseText.split(" ");
		var lat = data[0].substr(2, 7);
		var lon = data[1].substr(3, 7);
		lat = parseInt(data[0].substr(0,2), 10) + parseFloat(lat) / 60.0;
		lon = parseInt(data[1].substr(0,3), 10) + parseFloat(lon) / 60.0;
		if (data[0].indexOf('S') >= 0) lat = -lat;
		if (data[1].indexOf('W') >= 0) lon = -lon;
                var lonlat = new OpenLayers.MyLonLat(lon, lat);
		if (!marker) {
  	       		map.setCenter(lonlat, 12);
		} else {
			layerMarkers.removeMarker(marker);
		}
		var new_marker = new OpenLayers.Marker(lonlat);
		layerMarkers.addMarker(new_marker);
		marker = new_marker;
	}

        function init(){
                map = new OpenLayers.Map ("map", {
                        controls:[
                                new OpenLayers.Control.Navigation(),
                                new OpenLayers.Control.PanZoomBar(),
                                new OpenLayers.Control.Attribution()],
                        maxExtent: new OpenLayers.Bounds(-20037508.34,-20037508.34,20037508.34,20037508.34),
                        maxResolution: 156543.0399,
                        numZoomLevels: 19,
                        units: 'm',
			projection: new OpenLayers.Projection("EPSG:900913"),
			displayProjection: new OpenLayers.Projection("EPSG:4326") 
                } );

                var layerMapnik = new OpenLayers.Layer.OSM.Mapnik("Mapnik");
                map.addLayer(layerMapnik);

                layerMarkers = new OpenLayers.Layer.Markers("Markers");
                map.addLayer(layerMarkers);

		var layerLine = new OpenLayers.Layer.Vector("Line Layer");
		map.addLayer(layerLine);


		OpenLayers.MyLonLat = function(lon, lat) {
			return new OpenLayers.LonLat(lon,lat).transform(new OpenLayers.Projection("EPSG:4326"), map.getProjectionObject());
		}
		ArrAjax.ecmd("nmea get", nmea_handler);
		setInterval('ArrAjax.ecmd("nmea get", nmea_handler)', 5000);
	}

    </script>
  </head>
  <body onload="init()">
        <div id="map" class="smallmap"></div>
  </body>
</html>
