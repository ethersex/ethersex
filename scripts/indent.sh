#!/bin/sh

INDENTOPTS="-nbad -sc -nut -bli0 -blf -cbi0 -cli2 -npcs -nbbo"

if test $# -eq 0; then
  echo "usage: $0 [file] [directory]"
  exit 1
fi

while test $# -ne 0
do
  arg=$1
  shift
  if test -f $arg; then
    indent $INDENTOPTS $arg
  fi
  if test -d $arg; then
    find $arg -type f -name "*.[ch]" -exec indent $INDENTOPTS {} \;
  fi
done
	

