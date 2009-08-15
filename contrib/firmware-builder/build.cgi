#!/home/stettberger/bin/haserl  -s /bin/bash
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
cd $profile

echo "<pre>"
pwd
id
echo ""

run() { echo " --> $*"; $* 2>&1; }
run_detached() { echo " --> [D] $*"; screen -D -m $*; }

fail() { 
  echo "<font color=red>$*</font>"
  lockfile-remove build.lock
  cat ../footer.html
  exit 0
}

# Lock the build directory
lockfile-create build.lock || fail "lockfile-create failed"
lockfile-touch --oneshot build.lock || fail "lockfile-touch failed"

echo "Lock successfully aquired, ready to go."

run git pull

grep -e "^CONF_" .config | tr "=" " " | while read option value; do
  get_cmd='echo $FORM_'"$option"
  value="`eval $get_cmd`"

  if [ -n "$value" ]; then
    run_detached ./scripts/set-conf-string $option "$value"
  fi
done

[ -e autoconf.h ] || fail "Eeeek! There is no autoconf.h file anymore."
run rm -f ethersex.hex
run make
[ -e ethersex.hex ] || fail "Eeeek! Cannot find ethersex.hex, even looking thoroughly ..."

id=$(md5sum .config | cut -c-32)
ROOT=/var/www/firmware-images
rm -f $ROOT/$id.hex && run ln ethersex.hex $ROOT/$id.hex
rm -f $ROOT/$id.txt && run ln .config $ROOT/$id.txt
lockfile-remove build.lock

echo "</pre>"

[ -e ethersex.hex ] && {
  echo "<font size=+2 color=green>Your images have been built successfully!</font>"
  echo "<p><b>Download the image: <a href='/firmware-images/$id.hex'>ethersex.hex</a></b>, and the corresponding <a href='/firmware-images/$id.txt'>.config</a> file."
}

cat ../footer.html

?>
