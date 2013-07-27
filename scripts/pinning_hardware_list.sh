#!/bin/sh

. scripts/osdefaults.sh

ls pinning/hardware/ | ${SED} '/\.m4$/!d;s/.m4//;s/\(.*\)/\1 \1/;s/_/:/;s/^./\U&\E/;s/:./\U&\E/'

