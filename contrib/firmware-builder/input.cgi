#!/home/stettberger/bin/haserl -s /bin/bash
<?

echo "Content-Type: text/html"
echo

cat header.html

profile="$(echo $FORM_profile | tr -d './#()?*&')"
test x$profile = x && profile=avr-net-io

[ -e $profile/description ] || {
  echo "<font color=red>Error, no such profile: $profile</font>"
  cat footer.html
  exit 0
}

echo "<h1>"; cat $profile/description; echo "</h1>"

[ -e $profile/info ] && { echo "<p>"; cat "$profile/info"; }

echo "<form action='build.cgi' method='get'>" 
echo "<input type=hidden name=profile value=\"$profile\">"
echo "<div style='margin-left: 10%'><table border=1 cellspacing=5>"

grep $(for A in $(cat $profile/options); do echo " -e "^$A=" "; done) $profile/.config | tr "=" " " \
  | while read option value; do

  if [ "$option" = "CONF_ENC_MAC" ]; then
    description="The Ethernet MAC address."
  elif [ "$option" = "CONF_HOSTNAME" ]; then
    description="The hostname of the Ethersex."
  elif [ "$option" = "CONF_ENC_IP" ]; then
    description="The (default) IP address to use."
  elif [ "$option" = "CONF_ENC_IP4_NETMASK" ]; then
    description="The network mask belonging to the IP address, e.g. 255.255.255.0."
  elif [ "$option" = "CONF_ETHERRAPE_GATEWAY" ]; then
    description="The IP address of your default router."
  elif [ "$option" = "CONF_TFTP_IP" ]; then
    description="The IP address of your TFTP server."
  elif [ "$option" = "CONF_TFTP_IMAGE" ]; then
    description="The name of the filename to ask the TFTP server for."
  else
    description=""
  fi

  echo "<tr><td>${option#CONF_}</td>" | tr "[A-Z]" "[a-z]"
  echo "<td><input size=30 style='padding: 2px' type='text' name='$option' value=$value /></td>"
  echo "<td>$description</td></tr>"
done 

echo "</table>"
echo "<p><input type='submit' value='compile ethersex image'>"
echo "</div></form>"

cat footer.html
?>
