/* port the enc28j60 is attached to */
pin(SPI_CS_NET, SPI_CS_HARDWARE)

/* port the dataflash CS is attached to */
pin(SPI_CS_DF, PB1, OUTPUT)

ifdef(`conf_ONEWIRE', `dnl
  /* onewire support */
  pin(ONEWIRE, PD6)
')dnl

ifdef(`conf_SD_READER', `dnl
  /* port the sd-reader CS is attached to */
  pin(SPI_CS_SD_READER, PB2, OUTPUT)
')dnl


dnl ifdef(`conf_RFM12', `define(`GENERIC_LED_PINS', `1')')dnl
dnl ifdef(`conf_ZBUS', `define(`GENERIC_LED_PINS', `1')')dnl
dnl ifdef(`GENERIC_LED_PINS', `
dnl   pin(GENERIC_TX_PIN, PB3)
dnl   pin(GENERIC_RX_PIN, PB1)
dnl ')dnl
	

ifdef(`conf_RFM12', `dnl
dnl
dnl Ancient zerties.org RFM12 pinout:
dnl   chip select: PC3
dnl   tx-led: 	   PD4
dnl   rx-led:	   PD5
dnl   interrupt:   INT0
dnl
dnl Configuration suggest for Etherrape hardware
dnl (all pins available at SPI connector)
dnl 
  /* port the rfm12 module CS is attached to */
  pin(SPI_CS_RFM12, PB0, OUTPUT)

dnl   /* port the LEDS for rfm12 txrx attached to */
dnl   pin(RFM12_TX_PIN, GENERIC_TX_PIN)
dnl   pin(RFM12_RX_PIN, GENERIC_RX_PIN)

  RFM12_USE_INT(2)
')

ifdef(`conf_ZBUS', `dnl
  /* port config for zbus */
  pin(ZBUS_RXTX_PIN, PD2)
dnl  pin(ZBUS_TX_PIN, GENERIC_TX_LED)
dnl  pin(ZBUS_RX_PIN, GENERIC_RX_LED)
')

ifdef(`conf_ECMD_SERIAL_USART_RS485', `dnl
  pin(ECMD_SERIAL_USART_TX, PC2)
')

ifdef(`conf_RC5', `
  pin(RC5_SEND, PD4)
')

