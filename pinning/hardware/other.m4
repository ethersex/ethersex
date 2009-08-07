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

ifdef(`conf_STATUSLED_TX', `dnl
pin(STATUSLED_TX, PD6, OUTPUT)
')dnl

ifdef(`conf_STATUSLED_RX', `dnl
pin(STATUSLED_RX, PD7, OUTPUT)
')dnl

ifdef(`conf_RFM12', `dnl
  /* port the rfm12 module CS is attached to */
  pin(SPI_CS_RFM12, SPI_CS_HARDWARE)

  /* port the LEDS for rfm12 txrx attached to */
  pin(RFM12_TX_PIN, STATUSLED_TX)
  pin(RFM12_RX_PIN, STATUSLED_RX)

  RFM12_USE_INT(0)
')

ifdef(`conf_ZBUS', `dnl
  /* port config for zbus */
  pin(ZBUS_RXTX_PIN, PD2)
  pin(ZBUS_TX_PIN, STATUSLED_TX)
  pin(ZBUS_RX_PIN, STATUSLED_RX)
')

ifdef(`conf_STELLA', `dnl
  STELLA_PORT_RANGE(PB0,PB2)
')

ifdef(`conf_ECMD_SERIAL_USART_RS485', `dnl
  pin(ECMD_SERIAL_USART_TX, PD2)
')

ifdef(`conf_PS2', `dnl
  /* port the ps2 device is attached to */
  pin(PS2_CLOCK, PD3)
  pin(PS2_DATA, PD4)

  PS2_USE_INT(1)
')
