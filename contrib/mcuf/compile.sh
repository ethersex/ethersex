#!/bin/bash
export MACROS="-DGCC" 

export MCUFDIR="../../mcuf"

export OPTS="-O0 -Wall -W -ggdb -lefence"

SRC="
$MCUFDIR/mcuf_modul.c \
$MCUFDIR/mcuf_modul_examples.c \
$MCUFDIR/borg-16/xoni_study.c \
$MCUFDIR/borg-16/matrix.c \
$MCUFDIR/borg-16/snake.c \
$MCUFDIR/borg-16/programm.c \
$MCUFDIR/borg-16/gameoflife.c \
$MCUFDIR/borg-16/invader_draw.c \
$MCUFDIR/borg-16/invader_init.c \
$MCUFDIR/borg-16/invader_proc.c \
$MCUFDIR/borg-16/invaders2.c \
$MCUFDIR/image/image.c \
modsim.c"

for FILE in $SRC; do
#	echo gcc $MACROS $LIBS $FILE
	gcc -c $MACROS $OPTS $LIBS $FILE || exit $?
done

gcc *.o -o modsim
exit $?

