#!/bin/sh

. scripts/osdefaults.sh

for profile in $(ls pinning/hardware/ | sort | grep '\.m4$'); do 
  echo $profile |  ${SED} 's/_/:\n/; s/.m4//' \
  | ${SED} -e 's/\(^.\)/\U\1/' | tr -d "\n"
echo " $profile" | ${SED} 's/.m4//';
done 
