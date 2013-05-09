ifdef(`need_spi', `dnl
  /* port the enc28j60 is attached to */
  pin(SPI_CS_NET, SPI_CS_HARDWARE)
')dnl

ifdef(`conf_STATUSLED_POWER', `dnl
pin(STATUSLED_POWER, PA3, OUTPUT)
')dnl

ifdef(`conf_STATUSLED_BOOTED', `dnl
pin(STATUSLED_BOOTED, PA3, OUTPUT)
')dnl

ifdef(`conf_STATUSLED_NETLINK', `dnl
pin(STATUSLED_NETLINK, PA2, OUTPUT)
')dnl

ifdef(`conf_RFM12', `dnl
  /* port the rfm12 module CS is attached to */
  pin(SPI_CS_RFM12_0, SPI_CS_HARDWARE)

  /* port the LEDS for rfm12 txrx attached to */
  pin(STATUSLED_RFM12_TX, PD6, OUTPUT)
  pin(STATUSLED_RFM12_RX, PD7, OUTPUT)

  RFM12_USE_INT(0)
')

ifdef(`conf_ZBUS', `dnl
  /* port config for zbus */
  pin(ZBUS_RXTX_PIN, PD2)
  pin(STATUSLED_ZBUS_TX, PD6, OUTPUT)
  pin(STATUSLED_ZBUS_RX, PD7, OUTPUT)
')

ifdef(`conf_PS2', `dnl
  /* port the ps2 device is attached to */
  pin(PS2_CLOCK, PD3)
  pin(PS2_DATA, PD4)

  PS2_USE_INT(1)
')

ifdef(`conf_ST7626', `dnl
  pin(ST7626_CS, PC3, OUTPUT)
  pin(ST7626_RD, PC4, OUTPUT)
  pin(ST7626_WR, PC1, OUTPUT)
  pin(ST7626_A0, PC0, OUTPUT)
  pin(ST7626_RESET, PC2, OUTPUT)
  ST7626_DATA_PORT(A)
')
