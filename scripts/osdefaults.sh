# osdefaults.sh
# source this one to get reasonable defaults for
# several commands by the e6 build system.
#

THIS_OS=$(uname)

case "$THIS_OS" in
	Linux*)
		AWK=gawk
		SED=sed
		M4=m4
		;;
	*BSD*)
		AWK=gawk
		SED=gsed
		M4=gm4
		;;
	Darwin*)
		AWK=gawk
		SED=gsed
		M4=m4
		;;
	CYGWIN*)
		AWK=awk
		SED=sed
		M4=m4
		;;
	*)
		echo "$0: Unknown Platform. Trying to use GNU-userland." >2
		;;
esac

# use default values for GNU-userland if unset
AWK=${AWK:-gawk}
SED=${SED:-gsed}
M4=${M4:-gm4}

# "development helper"
#echo AWK is $AWK, SED is $SED and M4 is $M4.
