TOPDIR ?= ..
include $(TOPDIR)/.config

$(MCUF_SUPPORT)_SRC += \
	mcuf/mcuf.c \
	mcuf/mcuf_net.c\
        mcuf/mcuf_text.c\
        mcuf/mcuf_modul.c\
        mcuf/mcuf_modul_examples.c\
	mcuf/ledmatrixint.c\
	mcuf/image/image.c\
	mcuf/borg-16/programm.c\
	mcuf/borg-16/xoni_study.c\
	mcuf/borg-16/snake.c\
	mcuf/borg-16/gameoflife.c\
	mcuf/borg-16/invader_draw.c\
	mcuf/borg-16/invader_init.c\
	mcuf/borg-16/invader_proc.c\
	mcuf/borg-16/invaders2.c\
	mcuf/borg-16/matrix.c

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk

##############################################################################
# special stuff for this subdirectory

mcuf/mcuf_modul.c: mcuf/mcuf_modul_defs.c
mcuf/mcuf_modul_defs.c: mcuf/mcuf_modul_magic.m4 mcuf/mcuf_modul_defs.m4
	$(M4) $^ > $@.tmp
	mv -f $@.tmp $@

# extend normal clean rule
CLEAN_FILES += mcuf/mcuf_modul_defs.c

