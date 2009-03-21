#!/bin/bash
export MACROS="-DGCC" 

SRC="
../../mcuf/mcuf_modul.c \
../../mcuf/borg-16/xoni_study.c \
../../mcuf/borg-16/matrix.c \
../../mcuf/borg-16/snake.c \
../../mcuf/borg-16/programm.c \
../../mcuf/borg-16/gameoflife.c \
modsim.c"

for FILE in $SRC; do
	echo gcc $MACROS $LIBS $FILE
	gcc -c $MACROS $LIBS $FILE && echo "compile of $FILE OK"
	echo
done

gcc *.o -o modsim

