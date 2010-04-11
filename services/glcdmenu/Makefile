TOPDIR ?= ../..
include $(TOPDIR)/.config

$(GLCD-MENU_SUPPORT)_SRC += services/glcdmenu/glcdmenu.c
$(GLCD-MENU_SUPPORT)_SRC += services/glcdmenu/menu-text.c
$(GLCD-MENU_SUPPORT)_SRC += services/glcdmenu/menu-interpreter.c
$(GLCD-MENU_SUPPORT)_ECMD_SRC += services/glcdmenu/glcdmenu-ecmd.c

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk
