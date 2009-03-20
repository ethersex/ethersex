#!/bin/bash
export MACROS="-DGCC" 

FILES="
../../mcuf/mcuf_modul.c \
../../mcuf/borg-16/xoni_study.c \
../../mcuf/borg-16/matrix.c \
../../mcuf/borg-16/snake.c \
../../mcuf/borg-16/programm.c \
modsim.c"

for FILE in $FILES; do
	echo gcc $MACROS $LIBS $FILE
	gcc -c $MACROS $LIBS $FILE && echo "compile of $FILE OK"
	echo
done

gcc *.o -o modsim

