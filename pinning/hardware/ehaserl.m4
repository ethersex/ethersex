dnl
dnl ehaserl.m4
dnl
dnl Pin Configuration for EH2010 eHaserl by lilafisch & schneider
dnl

dnl This must also be D+ of the USB signal
USB_USE_INT(0) 

dnl This pin must have the 1k5 Pullup and must be on the same port as the Interupt pin
pin(USB_DMINUS, PD5)
pin(USB_DPLUS, PD2)

dnl Dual Leds
dnl pin(LED_RED1, PD6, OUTPUT)
dnl pin(LED_RED2, PB1, OUTPUT)
dnl pin(LED_GREEN1, PB2, OUTPUT)
dnl pin(LED_GREEN2, PB3, OUTPUT)

dnl alternatively if you'd like to use stella/moodlight:
dnl STELLA_PORT1_RANGE(PB1,PB3)
dnl STELLA_PORT2_RANGE(PD6,PD6)

pin(LIGHT, PC1)

dnl HD44780 compatible LCD display
pin(HD44780_RS, PD4)
pin(HD44780_EN1, PC0)
pin(HD44780_D4, PC2)
pin(HD44780_D5, PC3)
pin(HD44780_D6, PC4)
pin(HD44780_D7, PC5)

dnl Enable display
dnl (just don't set it, if you connect your ehaserl to USB,
dnl otherwise the display'll just show black boxes)
pin(LCDON, PD3, OUTPUT)

RFM12_NO_INT
pin(SPI_CS_RFM12_0, PD1, OUTPUT)
pin(SOFT_SPI_MOSI, HD44780_D6)
pin(SOFT_SPI_MISO, HD44780_D5)
pin(SOFT_SPI_SCK, HD44780_D4)

pin(STATUSLED_RFM12_TX, LED_RED1)
pin(STATUSLED_RFM12_RX, LED_GREEN2)
