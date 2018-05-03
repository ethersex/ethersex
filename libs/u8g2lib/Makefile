TOPDIR ?= ../..
include $(TOPDIR)/.config

ifeq ($(U8G2LIB_SUPPORT),y)
$(U8G2LIB_SUPPORT)_SRC += \
	libs/u8g2lib/u8log.c \
	libs/u8g2lib/u8log_u8g2.c \
	libs/u8g2lib/u8log_u8x8.c \
	libs/u8g2lib/u8g2_bitmap.c \
	libs/u8g2lib/u8g2_box.c \
	libs/u8g2lib/u8g2_buffer.c \
	libs/u8g2lib/u8g2_circle.c \
	libs/u8g2lib/u8g2_cleardisplay.c \
	libs/u8g2lib/u8g2_d_memory.c \
	libs/u8g2lib/u8g2_d_setup.c \
	libs/u8g2lib/u8g2_font.c \
	libs/u8g2lib/u8g2_fonts.c \
	libs/u8g2lib/u8g2_hvline.c \
	libs/u8g2lib/u8g2_input_value.c \
	libs/u8g2lib/u8g2_intersection.c \
	libs/u8g2lib/u8g2_kerning.c \
	libs/u8g2lib/u8g2_line.c \
	libs/u8g2lib/u8g2_ll_hvline.c \
	libs/u8g2lib/u8g2_message.c \
	libs/u8g2lib/u8g2_polygon.c \
	libs/u8g2lib/u8g2_selection_list.c \
	libs/u8g2lib/u8g2_setup.c \
	libs/u8g2lib/u8x8_8x8.c \
	libs/u8g2lib/u8x8_byte.c \
	libs/u8g2lib/u8x8_cad.c \
	libs/u8g2lib/u8x8_debounce.c \
	libs/u8g2lib/u8x8_display.c \
	libs/u8g2lib/u8x8_fonts.c \
	libs/u8g2lib/u8x8_gpio.c \
	libs/u8g2lib/u8x8_input_value.c \
	libs/u8g2lib/u8x8_message.c \
	libs/u8g2lib/u8x8_selection_list.c \
	libs/u8g2lib/u8x8_setup.c \
	libs/u8g2lib/u8x8_string.c \
	libs/u8g2lib/u8x8_u16toa.c \
	libs/u8g2lib/u8x8_u8toa.c

$(GLCD_HX1230_96x68_SPI_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_pcf8814_hx1230.c

$(GLCD_IL3820_296x128_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_il3820_296x128.c

$(GLCD_IST3020_ERC19264_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_ist3020.c

$(GLCD_LD7032_60x32_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_ld7032_60x32.c

$(GLCD_LS013B7DH03_128x128_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_ls013b7dh03.c

$(GLCD_MAX7219_32x8_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_max7219.c

$(GLCD_NT7534_TG12864R_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_st7565.c

$(GLCD_PCD8544_84x48_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_pcd8544_84x48.c

$(GLCD_PCF8812_96x65_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_pcf8812.c

$(GLCD_SH1106_64x32_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_sh1106_64x32_noname.c

$(GLCD_SH1106_72x40_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_sh1106_72x40_noname.c

$(GLCD_SH1106_128x64_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_ssd1306_128x64_noname.c

$(GLCD_SH1107_128x128_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_ssd1307.c

$(GLCD_SH1122_256x64_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_sh1122.c

$(GLCD_SSD1305_128x32_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_ssd1305.c

$(GLCD_SSD106_48x64_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_ssd1306_48x64.c

$(GLCD_SSD1306_64x32_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_ssd1306_64x32.c

$(GLCD_SSD1306_64x48_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_ssd1306_64x48.c

$(GLCD_SSD1306_96x16_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_ssd1306_96x16.c

$(GLCD_SSD1306_128x64_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_ssd1306_128x64_noname.c

$(GLCD_SSD1309_128x64_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_ssd1309.c

$(GLCD_SSD1322_256x64_NHD_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_ssd1322.c

$(GLCD_SSD1325_128x64_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_ssd1325.c

$(GLCD_SSD1326_256x32_ER_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_ssd1326.c

$(GLCD_SSD1327_128x128_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_ssd1327.c

$(GLCD_SSD1327_96x96_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_ssd1327.c

$(GLCD_SSD1329_128x96_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_ssd1329.c

$(GLCD_SSD1606_172x72_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_ssd1606_172x72.c

$(GLCD_SSD1607_200x200_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_ssd1607_200x200.c

$(GLCD_ST75256_JLX172104_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_st75256.c

$(GLCD_ST75256_JLX256128_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_st75256.c

$(GLCD_ST75256_JLX25664_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_st75256.c

$(GLCD_ST7565_128x64_ZOLEN_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_st7567.c

$(GLCD_ST7565_64128N_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_st7567.c

$(GLCD_ST7565_EA_DOGM128_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_st7567.c

$(GLCD_ST7565_EA_DOGM132_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_st7567.c

$(GLCD_ST7565_ERC12864_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_st7567.c

$(GLCD_ST7565_LM6059_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_st7567.c

$(GLCD_ST7565_NHD_C12832_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_st7567.c

$(GLCD_ST7565_NHD_C12864_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_st7567.c

$(GLCD_ST7567_132x64_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_st7567.c

$(GLCD_ST7567_JLX12864_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_st7567.c

$(GLCD_ST7588_JLX12864_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_st7588.c

$(GLCD_UC1601_128x32_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_uc1601.c

$(GLCD_UC1604_JLX19264_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_uc1604.c

$(GLCD_UC1608_240x128_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_uc1608.c

$(GLCD_UC1608_ERC240120_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_uc1608.c

$(GLCD_UC1608_ERC24064_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_uc1608.c

$(GLCD_UC1610_EA_DOGXL160_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_uc1610.c

$(GLCD_UC1611_EA_DOGM240_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_uc1611.c

$(GLCD_UC1611_EA_DOGXL240_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_uc1611.c

$(GLCD_UC1611_EW50850_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_uc1611.c

$(GLCD_UC1638_160x128_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_uc1638.c

$(GLCD_UC1701_EA_DOGS102_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_uc1701_dogs102.c

$(GLCD_UC1701_MINI12864_SUPPORT)_SRC += \
	libs/u8g2lib/u8x8_d_uc1617.c
endif

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk

# cpp u8g2.h|sed '/^extern const uint8_t u8g2_font_/!d;s/^.*u8g2_font_\([^\[]*\)\[\] .*$/\1/'|while read f;do printf "#define GLCD_FONT_%-23s u8g2_font_$f\n" ${f^^}; done|sort

