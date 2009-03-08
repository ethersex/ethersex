changequote({{,}})dnl
ifdef({{conf_CONFIG_INLINE}}, {{}}, {{m4exit(1)}})dnl
<html><head>
<title>Ethersex - Setup</title>
  <script src="scr.js" type="text/javascript"></script>
  <link href="Sty.c"  media="screen" rel="Stylesheet" type="text/css" />
  <script type="text/javascript"><!--
    function fillFields()
    {
	  getCmd('show+version', writeVal, returnObjById('version'));
	  getCmd('show+mac', writeVal, returnObjById('mac'));
	  getCmd('show+ip', writeVal, returnObjById('ip'));
ifdef({{conf_IPV6}}, {{}}, {{dnl
	  getCmd('show+netmask', writeVal, returnObjById('netmask'));
}})dnl
	  getCmd('show+gw', writeVal, returnObjById('gateway'));
ifdef({{conf_DNS}}, {{dnl 
          getCmd('show+dns', writeVal, returnObjById('dns'));
}})dnl
    }

    function getCmd(cmd, handler, id)
    {
      var url = '/ecmd?' + cmd;
      var request = new Object();
      request.id = id;
      ArrAjax.aufruf(url, handler, 'GET', request);
    }
	
    function setCmd(cmd, value)
    {
       var url = '/ecmd?'+ cmd + ' ' + value;
       ArrAjax.aufruf(url);
    }
    
    function writeVal(request, data)
    {
      data.id.value = request.responseText;
    }
	
    function changeState(request, data)
    {
      if(request.responseText == "OK\n")
      {
        data.id.style.backgroundColor = "green";
      }
      else
      {
        data.id.style.backgroundColor = "red";
      }
    }
	
    function updateValues()
    {
      returnObjById('valdiv').style.visibility = "hidden";
      returnObjById('waitdiv').style.visibility = "visible";
      setCmd('reset', '');
    }
    //--!>
  </script>
</head><body onLoad='fillFields()'>
<h1>Ethersex Setup</h1>
<div id="valdiv">
<center><table> 
    <tr>
        <td>Version</td>
        <td><input type="text" id="version" disabled> </td>
	</tr>
	<tr>
        <td>Mac</td>
        <td><input type="text" id="mac" onChange='getCmd("mac " + this.value, changeState, this);'> </td>
	</tr>
	<tr>
        <td>IP</td>
        <td><input ifdef({{conf_IPV6}}, {{size="40"}}) type="text" id="ip" onChange='getCmd("ip " + this.value, changeState, this);' ifdef({{conf_IPV6}}, {{disabled}})> </td>
	</tr>
ifdef({{conf_IPV6}}, {{}}, {{dnl
	<tr>
        <td>Netmask</td>
        <td><input type="text" id="netmask" onChange='getCmd("netmask " + this.value, changeState, this);'></td>
	</tr>
}})dnl
	<tr>
	<td>Gateway</td>
	<td><input ifdef({{conf_IPV6}}, {{size="40"}}) type="text" id="gateway" onChange='getCmd("gw " + this.value, changeState, this);' ifdef({{conf_IPV6}}, {{disabled}})></td>
	</tr>
ifdef({{conf_DNS}},{{dnl
        <tr>
	<td>DNS Server</td>
        <td><input type="text" id="dns" onChange='getCmd("dns " + this.value, changeState, this);'></td>
        </tr>
}})dnl
	<tr>
	<td></td>
	<td><input type="button" value="RESTART" onClick='updateValues();'></td>
	</tr>
</table></center>

ifdef({{conf_VFS_CONFIG_INLINE}}, {{
<a href="/conf">Get current .config</a><br><br>
}})
<a href="idx.ht"> Back </a>
</div>
<div id="waitdiv" style="visibility:hidden">
<center> Please wait while Server restarts</center>
</div>
<div id="logconsole"></div>
</body>
