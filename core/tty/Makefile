TOPDIR ?= ../..
include $(TOPDIR)/.config

$(TTY_SUPPORT)_SRC += core/tty/tty.c

$(TTY_LL_VT100_TELNET)_SRC += core/tty/tty-vt100.c core/tty/tty-vt100-telnet.c
$(TTY_LL_VT100_USART)_SRC += core/tty/tty-vt100.c core/tty/tty-vt100-usart.c

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk
