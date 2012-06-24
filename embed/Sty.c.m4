body {
	background-color: #333;
	color: #eee;
	font-family: sans-serif;
}

a{color: #ccc;}
a:hover{color: #fff;}
a:visited, a:active{color: #ddd;}

ifdef(`conf_MOTORCURTAIN_INLINE', `dnl
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

')dnl
#logconsole {
	padding: 5px 5px 5px 5px;
	margin-top: 10px;
	width: 98%;
	border: medium dotted black;
	visibility: hidden;
}

#logconsole .lognotice {
	color: green;
}

#logconsole .logerror {
	color: red;
	font-weight: bold;
}

ifdef(`conf_VFS_IO_INLINE', `define(`_inline_iotable', `y')')dnl
ifdef(`conf_ADC_INLINE', `define(`_inline_iotable', `y')')dnl
ifdef(`conf_MSR1_INLINE', `define(`_inline_iotable', `y')')dnl
ifdef(`conf_TO1_INLINE', `define(`_inline_iotable', `y')')dnl
ifdef(`_inline_iotable', `dnl
.iotable {
	text-align: center;
	padding: 15px;
	empty-cells: show;
}

.iotable td { border-bottom: 1px dashed #FF9999 }
.iotable th { border-bottom: 2px dashed #9999FF; padding: 3px;}

')dnl
ifdef(`conf_ADC_INLINE', `dnl
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

')dnl
ifdef(`conf_TANKLEVEL_INLINE', `dnl
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

')dnl
ifdef(`conf_ONEWIRE_INLINE', `define(`_inline_svg', `y')')dnl
ifdef(`conf_KTY_INLINE', `define(`_inline_svg', `y')')dnl
ifdef(`_inline_svg', `dnl
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

')dnl
