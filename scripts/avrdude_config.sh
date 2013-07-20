#
# avrdude_config.sh
# config.in helper

. scripts/osdefaults.sh

get_avrdude() {
	AVRDUDE=$( command -v avrdude )
	if [ "x${AVRDUDE}" != "x" ]; then
		HAS_AVRDUDE="y"
	fi	
}

get_avrdude_programmer_types() {
	# gnampf, Menuconfig choice does not like names starting
	# with numbers nor math ops like '-'
	AVRDUDE_PROGRAMMER_TYPES=$( ${AVRDUDE} -c ? 2>&1 |\
		${AWK} '/^.* = .*$/ { gsub(/=.*$/,""); \
		 gsub(/ +/, " "); \
		 sub(/^ /, ""); \
		 sub(/ $/, ""); \
		 prog = $1; \
		 sub(/^[0-9]+.*/, "_&", prog); \
		 num = gsub(/-/, "_", prog); \
		 if (num > 0) {\
		 	prog = "_" prog }; \
		 type[$1] = $1 "#" prog } \
		 END { n = asort(type); \
     		   for (i = 1; i <= n; i++) { \
     		    sub(/#/, " ", type[i]); \
     		    if (i > 1) { \
     		    	printf " " }; \
     		    printf type[i] } }')
}

get_avrdude
get_avrdude_programmer_types

#echo "$AVRDUDE"
#echo "$AVRDUDE_PROGRAMMER_TYPES"
