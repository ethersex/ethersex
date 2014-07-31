<html>
 <head>
  <title>Ethersex Index Page</title>
  <link rel="stylesheet" href="Sty.c" type="text/css"/>
 </head>
 <body>
  <h1>Welcome to Ethersex!</h1>
  <p>Congratulations!  Your Ethersex http server 'value_HOSTNAME' seems to be working <img src="gr.gif" alt=":-)"></p>
  <p>You should never forget that ...
  <pre align="center">
      _   _                                            _          
     | | | |                                          | |         
  ___| |_| |__   ___ _ __ ___  _____  __    _ __ _   _| | ___ ____
 / _ \ __| '_ \ / _ \ '__/ __|/ _ \ \/ /   | '__| | | | |/ _ \_  /
|  __/ |_| | | |  __/ |  \__ \  __/>  <    | |  | |_| | |  __// / 
 \___|\__|_| |_|\___|_|  |___/\___/_/\_\   |_|   \__,_|_|\___/___|
</pre>
  <p>For details on what ethersex is, <a href="http://www.ethersex.de/">ethersex.de</a>.</p>
ifdef(`conf_MOTD_INLINE', `<p><a href="mo.ht">MOTD</a>: <b id="motd" style="border:1px dashed white"></b></p>
<script src="scr.js" type="text/javascript"></script>
<script>
function motd_get(request, data) {
	_(data, request.responseText);
}

window.onload = function() {
	ArrAjax.ecmd("motd", motd_get, "GET", "motd");
}
</script>
')
  <hr>
ifdef(`conf_VFS_IO_INLINE', `<p>To do some control tasks, see <a href="io.ht">here</a>.</p>')
ifdef(`conf_ADC_INLINE', `<p>To see the adc channels see <a href="adc.ht">here</a>.</p>')
ifdef(`conf_KTY_INLINE', `<p>To see the KTY temperature see <a href="Xky.ht">here</a>.</p>')
ifdef(`conf_ONEWIRE_INLINE', `<p>The onewire temperature values are accessible <a href="ow.ht">here</a>,
   or with a SVG-capable browser see the <a href="Xow.ht">graphical variant</a>.</p>')
ifdef(`conf_HAT_INLINE', `<p>The humidity and temperature sensors are accessible <a href="hat.ht">here</a>.</p>')
ifdef(`conf_RADIO_INLINE', `<p>To control <a href="rf.ht">RFM12 ASK</a>.</p>')
ifdef(`conf_STELLA_INLINE', `<p>To control <a href="ste.ht">StellaLight</a>.</p>')
ifdef(`conf_MOTORCURTAIN_INLINE', `<p>To control <a href="cur.ht">MotorCurtain</a>.</p>')
ifdef(`conf_I2C_INLINE', `<p>Set <a href="i2c.ht">I<sup>2</sup>C-Bus</a> stuff.</p>')
ifdef(`conf_CAMERA_INLINE', `<p>See the <a href="cam.ht">camera examples</a>.</p>')
ifdef(`conf_NAMED_PIN_INLINE', `<p>Control <a href="np.ht">named outputs</a>.</p>')
ifdef(`conf_CONFIG_INLINE', `<p>Configure ethersex <a href="cfg.ht">here</a>.</p>')
ifdef(`conf_NETSTAT', `<p>Visit <a target="_blank" href="http://www.ethersex.de/~habo/stat/">Ethersex online statistic</a>.</p>')
ifdef(`conf_MCUF_INLINE', `<p>Select MCUF Modul <a href="mm.ht">here</a>.</p>')
ifdef(`conf_PWM_SERVO_INLINE', `<p>Easy set <a href="ps.ht">Servo Position</a>.</p>')
ifdef(`conf_LCD_INLINE', `<p>Handle <a href="lcd.ht">LCD and other displays</a>.</p>')
ifdef(`conf_CW_INLINE', `<p>Send <a href="cw.ht">Morse code</a>.</p>')
ifdef(`conf_DMX_STORAGE_INLINE', `<p>Control <a href="dmx.ht">DMX Channels</a>.</p>')
ifdef(`conf_TANKLEVEL_INLINE', `<p>To query the tank level see <a href="tnk.ht">here</a>.</p>')
ifdef(`conf_WOL_INLINE', `<p>Send Wake on LAN signals <a href="wol.ht">here</a>.</p>')
 </body>
</html>
