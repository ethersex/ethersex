TOPDIR ?= ../..
include $(TOPDIR)/.config

#for menuinterpreter
CFLAGS += -DAVR_BUILD

$(GLCD_MENU_SUPPORT)_SRC += services/glcdmenu/glcdmenu.c
$(GLCD_MENU_SUPPORT)_SRC += services/glcdmenu/menu-text.c
$(GLCD_MENU_SUPPORT)_SRC += services/glcdmenu/menu-interpreter.c
$(GLCD_MENU_SUPPORT)_ECMD_SRC += services/glcdmenu/glcdmenu-ecmd.c

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk
