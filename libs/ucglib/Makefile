TOPDIR ?= ../..
include $(TOPDIR)/.config

ifeq ($(GLCD_COLOR_SUPPORT),y)

#CPPFLAGS += -DUCG_MSG_DRAW_L90TC=1
#CFLAGS += -DUCG_MSG_DRAW_L90TC=1
#CPPFLAGS += -DUCG_MSG_DRAW_L90BF=2
#CFLAGS += -DUCG_MSG_DRAW_L90BF=2

$(GLCD_COLOR_SUPPORT)_SRC += \
	libs/ucglib/ucg_box.c \
	libs/ucglib/ucg_ccs.c \
	libs/ucglib/ucg_circle.c \
	libs/ucglib/ucg_clip.c \
	libs/ucglib/ucg_com_msg_api.c \
	libs/ucglib/ucg_dev_default_cb.c \
	libs/ucglib/ucg_dev_msg_api.c \
	libs/ucglib/ucg_font.c \
	libs/ucglib/ucg_init.c \
	libs/ucglib/ucg_line.c \
	libs/ucglib/ucg_pixel.c \
	libs/ucglib/ucg_pixel_font_data.c \
	libs/ucglib/ucg_polygon.c \
	libs/ucglib/ucg_rotate.c \
	libs/ucglib/ucg_scale.c \
	libs/ucglib/ucg_vector_font_data.c
#	libs/ucglib/ucg_bitmap.c

$(GLCD_ST7735_128x160_SUPPORT)_SRC += \
	libs/ucglib/ucg_dev_tft_128x160_st7735.c \
	libs/ucglib/ucg_dev_ic_st7735.c

$(GLCD_ILI9341_240x320_SUPPORT)_SRC += \
	libs/ucglib/ucg_dev_tft_240x320_ili9341.c \
	libs/ucglib/ucg_dev_ic_ili9341.c

$(GLCD_ILI9163_128x128_SUPPORT)_SRC += \
	libs/ucglib/ucg_dev_tft_128x128_ili9163.c \
	libs/ucglib/ucg_dev_ic_ili9163.c

$(GLCD_SSD1351_128x128_SUPPORT)_SRC += \
	libs/ucglib/ucg_dev_oled_128x128_ilsoft.c \
	libs/ucglib/ucg_dev_ic_ssd1351.c

$(GLCD_SSD1351_128x128_FT_SUPPORT)_SRC += \
	libs/ucglib/ucg_dev_oled_128x128_ft.c \
	libs/ucglib/ucg_dev_ic_ssd1351.c

$(GLCD_SSD1331_96x64_UNIVISION_SUPPORT)_SRC += \
	libs/ucglib/ucg_dev_oled_96x64_univision.c \
	libs/ucglib/ucg_dev_ic_ssd1331.c

$(GLCD_SEPS225_128x128_UNIVISION_SUPPORT)_SRC += \
	libs/ucglib/ucg_dev_oled_128x128_univision.c \
	libs/ucglib/ucg_dev_ic_seps225.c

$(GLCD_HX8352C_240x400_SUPPORT)_SRC += \
	libs/ucglib/ucg_dev_tft_240x400_hx8352c.c \
	libs/ucglib/ucg_dev_ic_hx8352c.c

$(GLCD_ILI9486_320x480_SUPPORT)_SRC += \
	libs/ucglib/ucg_dev_tft_320x480_ili9486.c \
	libs/ucglib/ucg_dev_ic_ili9486.c
endif

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk

# cpp ucg.h|sed '/^extern const ucg_fntpgm_uint8_t ucg_font_/!d;s/^.*ucg_font_\([^\[]*\)\[\] .*$/\1/'|while read f;do printf "#define GLCD_FONT_%-23s ucg_font_$f\n" ${f^^}; done|sort

