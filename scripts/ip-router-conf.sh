num=`echo $ENC28J60_SUPPORT $RFM12_SUPPORT $ZBUS_SUPPORT $USB_NET_SUPPORT | sed -e 's/[^y]//g' | wc -L`

if [ $num -gt 1 ]; then
  #comment "Using IP Router ($num users)"
  define_bool ROUTER_SUPPORT y
else
  define_bool ROUTER_SUPPORT n
fi
