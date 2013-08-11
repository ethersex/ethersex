ifdef(`conf_DMX_STORAGE', `', `m4exit(1)')dnl
ifdef(`conf_DMX_STORAGE_INLINE', `', `m4exit(1)')dnl

<html><head>
<title>DMX Control Panel</title>
<link rel="StyleSheet"  href="Sty.c" type="text/css" />


<script src="scr.js" type="text/javascript"></script>
<script src="dmx.js" type="text/javascript"></script>
ifdef(`conf_VFS_INLINE_HTML5_RANGE_FF', `
<script src="h5r.js" type="text/javascript"></script>',`')dnl
</head>
<body>
  <h3>DMX Channels</h3><span style="margin-bottom:10px;" id="universeSelection"><span style="padding:5px">Select universe</span></span>
  <span style="padding:5px">State</span><span id="stateButton"></span>
  <div style="padding: 5px"><span id="caption">Please activate Javascript.</span><span id="upind"></span></div>
  <div id="channels"></div>
</body>
</html>
