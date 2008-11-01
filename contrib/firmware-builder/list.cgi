#!/home/stettberger/bin/haserl -s /bin/bash
<?
echo "Content-Type: text/html"
echo

get_config() {
  sed -n 's/^'"$1"' *\(.*\)$/\1/p' $2
}

cat header.html
echo "<h3>Automated firmware-builder for <a href='http://www.ethersex.de'>ethersex</a></h3>"

echo "<div style='margin-left: 6%'><ul>"
for config in */description; do
  echo "<li>"
  profile=${config%/description}
  echo "<a href='input.cgi?profile=$profile'>$profile</a>"
  echo -
  cat $config
  echo "</li>"
done

echo "</ul></div>"

cat footer.html
?>
