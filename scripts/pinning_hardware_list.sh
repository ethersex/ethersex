#!/bin/sh

SED=sed
[ `uname` = "FreeBSD" ] && SED=gsed

for profile in $(ls pinning/hardware/ | sort | grep '\.m4$'); do 
  echo $profile |  ${SED} 's/_/:\n/; s/.m4//' \
  | ${SED} -e 's/\(^.\)/\U\1/' | tr -d "\n"
  echo " $profile" | sed 's/.m4//';
done 
