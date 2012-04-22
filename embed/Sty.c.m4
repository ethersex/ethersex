#include "autoconf.h"
body {
	background-color: #333;
	color: #eee;
	font-family: sans-serif;
}

a{color: #ccc;}
a:hover{color: #fff;}
a:visited, a:active{color: #ddd;}

#ifdef MOTORCURTAIN_INLINE_SUPPORT
.small_button:hover {
	background-color: #555;
}

.small_button, .lamp {
	background-color: #444;
	padding: 3px;
	color: black;
	text-decoration: none;
	margin-top: 4px;
	text-align: center;
	display: block;
}
#endif

@@HASH@@logconsole {
	padding: 5px 5px 5px 5px;
	margin-top: 10px;
	width: 98%;
	border: medium dotted black;
	visibility: hidden;
}

@@HASH@@logconsole .lognotice {
	color: green;
}

@@HASH@@logconsole .logerror {
	color: red;
	font-weight: bold;
}

#if defined(VFS_IO_INLINE_SUPPORT) || defined (ADC_INLINE_SUPPORT) || defined(MSR1_INLINE_SUPPORT) || defined(TO1_INLINE_SUPPORT)
.iotable {
	text-align: center;
	padding: 15px;
	empty-cells: show;
}

.iotable td { border-bottom: 1px dashed #FF9999 }
.iotable th { border-bottom: 2px dashed #9999FF; padding: 3px;}
#endif

#ifdef ADC_INLINE_SUPPORT
.adc_graph {
	width: 500px;
}

.adc_graph div {
	background-color: #00ff00;
}

.adc_data, .adc_graph {
	border-left: 1px dashed #FF9999;
	padding: 3px;
}
#endif

#ifdef TANKLEVEL_INLINE_SUPPORT
.tank_wrap {
	position: relative;
}

.tank_wrap, .tank_value, .tank_text {
	width: 500px;
	height: 30px;
}

.tank_wrap, .tank_value {
	background: #777777;
}

.tank_value {
	background: #00ff00;
	width: 0px;
}

.tank_text {
	position: absolute;
	top:0;
	left:0;
	padding-top: 5px;
	text-align: center;
	width: 100%;
}
#endif

#if defined(ONEWIRE_INLINE_SUPPORT) || defined(KTY_INLINE_SUPPORT)
svg {
	stroke: #999;
}

svg .graph {
	stroke-width: 2px;
}

svg #axis {
	stroke-dasharray: 2,5;
}

svg #text {
	font-size: 10pt;
}
#endif
