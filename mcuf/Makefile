TARGET = libmcuf.a
TOPDIR = ..
SRC = \
        ../pinning.c \
	mcuf.c \
	mcuf_net.c\
        mcuf_text.c\
        mcuf_modul.c\
        mcuf_modul_examples.c\
	ledmatrixint.c\
	image/image.c\
	borg-16/programm.c\
	borg-16/xoni_study.c\
	borg-16/snake.c\
	borg-16/gameoflife.c\
	borg-16/invader_draw.c\
	borg-16/invader_init.c\
	borg-16/invader_proc.c\
	borg-16/invaders2.c\
	borg-16/matrix.c

##############################################################################
# generic fluff
include $(TOPDIR)/defaults.mk
include $(TOPDIR)/rules.mk

##############################################################################
# special stuff for this subdirectory

DOXYGEN := doxygen

doc: $(HEADERS) $(SOURCES) Doxyfile
	$(DOXYGEN) Doxyfile

mcuf_modul.c: mcuf_modul_defs.c
mcuf_modul_defs.c: mcuf_modul_magic.m4 mcuf_modul_defs.m4
	m4 $^ > $@.tmp
	mv -f $@.tmp $@

# extend normal clean rule
clean: clean-common
	$(RM) -f mcuf_modul_defs.c

