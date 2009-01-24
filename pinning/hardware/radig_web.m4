/* port the enc28j60 is attached to */
pin(SPI_CS_NET, PB3, OUTPUT)

/* port the sd-reader CS is attached to */
pin(SPI_CS_SD_READER, PB1, OUTPUT)

ifdef(`conf_RFM12', `dnl
  /* port the rfm12 module CS is attached to */
  pin(SPI_CS_RFM12, PB0,OUTPUT)
  RFM12_USE_INT(1)

  /* port the LEDS for rfm12 txrx attached to */
dnl  pin(RFM12_TX_PIN, PA0)
dnl  dnl  pin(RFM12_RX_PIN, PA2)
')
