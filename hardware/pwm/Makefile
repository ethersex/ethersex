TOPDIR ?= ../..
include $(TOPDIR)/.config

$(PWM_GENERAL_SUPPORT)_SRC += hardware/pwm/pwm.c
$(PWM_FREQ_SUPPORT)_SRC += hardware/pwm/pwm_freq.c
$(PWM_DTMF_SUPPORT)_SRC += hardware/pwm/pwm_dtmf.c

$(PWM_WAV_SUPPORT)_SRC += hardware/pwm/pwm_wav.c

$(PWM_MELODY_SUPPORT)_SRC += hardware/pwm/pwm_melody.c

$(PWM_SERVO_SUPPORT)_SRC += hardware/pwm/pwm_servo.c
$(PWM_SERVO_SUPPORT)_ECMD_SRC += hardware/pwm/pwm_servo_ecmd.c
##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk

PWM_WAVFILE ?= $(TOPDIR)/hardware/pwm/ethersex.wav
$(TOPDIR)/hardware/pwm/ethersex_wav.h: $(TOPDIR)/scripts/soundinclude $(PWM_WAVFILE)
	$^ > $@

$(TOPDIR)/hardware/pwm/pwm_wav.c: $(TOPDIR)/hardware/pwm/ethersex_wav.h

CLEAN_FILES += $(TOPDIR)/hardware/pwm/ethersex_wav.h

