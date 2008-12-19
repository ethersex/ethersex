#!/bin/bash

for profile in $(ls pinning/hardware/ | sort); do 
  echo $profile |  sed 's/_/:\n/; s/.m4//' \
  | sed -e 's/\(^.\)/\U\1/' | tr -d "\n"
  echo " $profile" | sed 's/.m4//';
done 
