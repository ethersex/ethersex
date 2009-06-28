
body {
	background-color: #333;
	color: #eee;
	font-family: sans-serif;
}

a:hover{
	color: #fff;
}

a:visited, a:active{
	color: #ddd;
}

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

.iotable {
	text-align: center;
	padding: 15px;
	empty-cells: show;
}

.iotable td { border-bottom: 1px dashed #FF9999 }
.iotable th { border-bottom: 2px dashed #9999FF; padding: 3px;}

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
