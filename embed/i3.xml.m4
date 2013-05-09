changequote({{,}})dnl
ifdef({{conf_UPNP_INLINE}}, {{}}, {{m4exit(1)}})dnl
<?xml version="1.0"?>
<scpd xmlns="urn:schemas-upnp-org:service-1-0">
<specVersion>
	<major>1</major>
	<minor>0</minor>
</specVersion>
<actionList>
	<action>
		<name>MagicOn</name>
	</action>
</actionList>
<serviceStateTable>
<stateVariable sendEvents="no">
		<name>OSMajorVersion</name>
		<dataType>i4</dataType>
	</stateVariable>
<stateVariable sendEvents="no">
		<name>OSMinorVersion</name>
		<dataType>i4</dataType>
	</stateVariable>
<stateVariable sendEvents="no">
		<name>OSBuildNumber</name>
		<dataType>i4</dataType>
	</stateVariable>
<stateVariable sendEvents="no">
		<name>OSMachineName</name>
		<dataType>string</dataType>
	</stateVariable>
</serviceStateTable>
</scpd>
