changequote({{,}})dnl
ifdef({{conf_UPNP_INLINE}}, {{}}, {{m4exit(1)}})dnl
<?xml version="1.0" encoding="UTF-8"?>
<root xmlns="urn:schemas-upnp-org:device-1-0">

<specVersion>
	<major>1</major>
	<minor>0</minor>
</specVersion>

<device>
	<deviceType>urn:schemas-upnp-org:device:InternetGatewayDevice:1</deviceType>
	<friendlyName>Ethersex Router</friendlyName>
	<manufacturer>Ethersex</manufacturer>
	<manufacturerURL>http://www.ethersex.de/</manufacturerURL>
	<modelDescription>Internet Gateway Device with UPnP support</modelDescription>
	<modelName>value_HOSTNAME</modelName>
	<modelNumber>v2</modelNumber>
	<modelURL>http://www.ethersex.de/</modelURL>
	<serialNumber>J639090617</serialNumber>
	<UDN>uuid:00000000-0000-0001-0000-0001e3d766e5</UDN>
	<UPC/>

	<serviceList>
	<service>
		<serviceType>urn:schemas-microsoft-com:service:OSInfo:1</serviceType>
		<serviceId>urn:microsoft-com:serviceId:OSInfo1</serviceId>
		<SCPDURL>/i3.xml</SCPDURL>
		<controlURL>/upnp/control?OSInfo</controlURL>
		<eventSubURL>/upnp/event?OSInfo</eventSubURL>
	</service>

	</serviceList>

	<deviceList>
	<device>
		<deviceType>urn:schemas-upnp-org:device:WANDevice:1</deviceType>
		<friendlyName>Ethersex Router(WAN)</friendlyName>
		<manufacturer>Ethersex Community</manufacturer>
		<manufacturerURL>http://www.ethersex.de/</manufacturerURL>
		<modelDescription>IPv6 capable AVR Microcontroller firmware</modelDescription>
		<modelName>value_HOSTNAME WAN Interface</modelName>
		<modelNumber>v2</modelNumber>
		<modelURL>http://www.ethersex.de/</modelURL>
		<serialNumber>J639090617</serialNumber>
    	<UDN>uuid:00000000-0000-0001-0001-0001e3d766e5</UDN>
    	<UPC/>
		<serviceList>
		</serviceList>

		<deviceList>
		</deviceList>
	</device>
	</deviceList>

	<presentationURL>/</presentationURL>
</device>
</root>
