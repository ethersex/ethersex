#!/bin/bash
export MACROS="-DGCC=1 -DMCUF_MODUL_SUPPORT=1 -DMCUF_CHESS_SUPPORT=1 -DMCUF_CLEAN_SUPPORT=1 -DMCUF_SPIRAL_SUPPORT=1 -DMCUF_MODUL_BORG16_MATRIX_SUPPORT"
#export LIBS="-I. -L."

#FILES="../../mcuf/mcuf_modul.c ../../mcuf/borg-16/xoni_study.c ../../mcuf/borg-16/matrix.c modsim.c"
FILES="../../mcuf/mcuf_modul.c ../../mcuf/borg-16/xoni_study.c ../../mcuf/borg-16/matrix.c modsim.c"

for FILE in $FILES; do
	echo gcc $MACROS $LIBS $FILE
	gcc -c $MACROS $LIBS $FILE && echo "compile of $FILE OK"
	echo
done


#OFILES=`echo $FILES | sed -e "s/\\.c/.o/" -e "s/\\.\\.\///"` 
gcc *.o -o modsim

