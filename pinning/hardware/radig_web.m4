/* port the enc28j60 is attached to */
pin(SPI_CS_NET, PB3, OUTPUT)

/* port the sd-reader CS is attached to */
pin(SPI_CS_SD_READER, PB1, OUTPUT)

ifdef(`conf_RFM12', `dnl
  /* port the rfm12 module CS is attached to */
  pin(SPI_CS_RFM12, PB0,OUTPUT)
  RFM12_USE_INT(1)
  /* nur INT 0 available = PD2 (Pin 10) */
  RFM12_ASK_SENSE_USE_INT(0)

  /* port the LEDS for rfm12 txrx attached to */
  pin(RFM12_TX_PIN, PA0, OUTPUT)
  pin(RFM12_RX_PIN, PA2, OUTPUT)
')


