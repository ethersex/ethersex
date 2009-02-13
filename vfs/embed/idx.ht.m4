<html>
 <head>
  <title>Ethersex Index Page</title>
  <link rel="StyleSheet"  href="Sty.c" type="text/css" />
 </head>
 <body>
  <h1>Welcome to Ethersex!</h1>
  <p>Congratulations!  Your Ethersex http server seems to be working <img src="gr.gif" alt=":-)"></p>
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
  <hr>
  <p>To do some control tasks, see <a href="io.ht">here</a>.</p>
ifdef(`conf_ADC', `<p>To see the adc channels see <a href="adc.ht">here</a>.</p>')
ifdef(`conf_KTY', `<p>To see the KTY temperature see <a href="Xky.ht">here</a>.</p>')
ifdef(`conf_ONEWIRE', `<p>The onewire temperature values are accessible <a href="ow.ht">here</a>,
   or with a SVG-capable browser see the <a href="Xow.ht">graphical variant</a>.</p>')
  <p>Configure ethersex <a href="cfg.ht">here</a></p>
 </body>
</html>
