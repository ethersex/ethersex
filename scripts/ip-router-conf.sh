
num=`echo $ENC28J60_SUPPORT $TAP_SUPPORT $RFM12_IP_SUPPORT $ZBUS_SUPPORT $USB_NET_SUPPORT $OPENVPN_SUPPORT | sed -e 's/[^y]//g' | tr -d '\n' | wc -c`

if [ $num -gt 1 ]; then
  #comment "Using IP Router ($num users)"
  define_bool ROUTER_SUPPORT y
else
  define_bool ROUTER_SUPPORT n
fi

if [ $num -gt 0 ]; then
  define_bool UIP_SUPPORT y
else
  define_bool UIP_SUPPORT n
fi

# this is only glue, so that the net directory is compiled also
define_bool NET_SUPPORT $UIP_SUPPORT

