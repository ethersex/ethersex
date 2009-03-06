
CONFIG="$(grep -e "^#define .*_SUPPORT" autoconf.h | \
    sed -e "s/^#define /-Dconf_/" -e "s/_SUPPORT.*//")"
CONFIG="$CONFIG $(grep -e "^#define CONF_.*" autoconf.h |  \
    sed -e "s/^#define CONF_/-Dvalue_/" -re "s/( )/=/" -e "s/[ \"]//g")"

cat >/dev/null <<EOF
<html><head>
<title>Ethersex - Setup</title>
  <script src="scr.js" type="text/javascript"></script>
  <link href="Sty.c"  media="screen" rel="Stylesheet" type="text/css" />
  <script type="text/javascript"><!--
    //--!>
  </script>
</head><body>
<h1>Ethersex Named Pin</h1>
EOF



cat "$1" | sed 's/#.*$//; /^\s*$/d
                  s/\([^ ]\)\s\+\([^ ]\)/\1,\2/g
                  s/^\s*\([^\s]\+\)/np_pin(\1/
                  s/\(.\+\)/\1)/'

cat /dev/null << EOF
<div id="logconsole"></div>
</body>
EOF

