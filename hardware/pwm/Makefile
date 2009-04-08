TOPDIR ?= ../..
include $(TOPDIR)/.config

$(PWM_WAV_SUPPORT)_SRC += hardware/pwm/pwm_wav.c
$(PWM_MELODY_SUPPORT)_SRC += hardware/pwm/pwm_melody.c

##############################################################################
# generic fluff
include $(TOPDIR)/rules.mk

ifneq ($(rootbuild),t)
hardware/pwm/pwm_wav.c: scripts/soundinclude ethersex.wav > ethersex_wav.h
endif

CLEAN_FILES += ethersex_wav.h
