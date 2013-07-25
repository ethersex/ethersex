TOPDIR ?= ..
include $(TOPDIR)/.config

$(MCUF_SUPPORT)_ECMD_SRC += \
	mcuf/mcuf_ecmd.c

$(MCUF_SUPPORT)_SRC += \
	mcuf/mcuf.c \
	mcuf/mcuf_net.c\
        mcuf/mcuf_text.c\
        mcuf/mcuf_modul.c\
        mcuf/mcuf_modul_examples.c\
        mcuf/mcuf_modul_test_game_input.c\
	mcuf/ledmatrixint.c\
	mcuf/image/image.c\
	mcuf/borg-16/xoni_study.c\
	mcuf/borg-16/snake.c

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

