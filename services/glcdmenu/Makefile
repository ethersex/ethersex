TOPDIR ?= ../..
include $(TOPDIR)/.config

#for menuinterpreter
CFLAGS += -DAVR_BUILD

$(GLCD_MENU_SUPPORT)_SRC += services/glcdmenu/glcdmenu.c
$(GLCD_MENU_SUPPORT)_SRC += services/glcdmenu/glcdmenu-s1d13305.c
$(GLCD_MENU_SUPPORT)_ECMD_SRC += services/glcdmenu/glcdmenu-ecmd.c
$(GLCD_MENU_SUPPORT)_SRC += services/glcdmenu/menu-interpreter/menu-text.c
$(GLCD_MENU_SUPPORT)_SRC += services/glcdmenu/menu-interpreter/menu-interpreter.c

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk
