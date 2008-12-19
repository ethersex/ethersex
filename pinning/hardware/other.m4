ifdef(`need_spi', `dnl
  /* port the enc28j60 is attached to */
  pin(SPI_CS_NET, SPI_CS_HARDWARE)
')dnl

ifdef(`conf_RFM12', `define(`GENERIC_LED_PINS', `1')')dnl
ifdef(`conf_ZBUS', `define(`GENERIC_LED_PINS', `1')')dnl
ifdef(`GENERIC_LED_PINS', `
  pin(GENERIC_TX_PIN, PD6)
  pin(GENERIC_RX_PIN, PD7)
')dnl

ifdef(`conf_RFM12', `dnl
  /* port the rfm12 module CS is attached to */
  pin(SPI_CS_RFM12, SPI_CS_HARDWARE)

  /* port the LEDS for rfm12 txrx attached to */
  pin(RFM12_TX_PIN, GENERIC_TX_PIN)
  pin(RFM12_RX_PIN, GENERIC_RX_PIN)

  RFM12_USE_INT(0)
')

ifdef(`conf_ZBUS', `dnl
  /* port config for zbus */
  pin(ZBUS_RXTX_PIN, PD2)
  pin(ZBUS_TX_PIN, GENERIC_TX_PIN)
  pin(ZBUS_RX_PIN, GENERIC_RX_PIN))
')

ifdef(`conf_STELLA', `dnl
  STELLA_PORT_RANGE(PB0,PB2)
')

ifdef(`conf_ECMD_SERIAL_USART_RS485', `dnl
  pin(ECMD_SERIAL_USART_TX, PD2)
')
