/* port the enc28j60 is attached to */
pin(SPI_CS_NET, PB3, OUTPUT)

/* port the sd-reader CS is attached to */
pin(SPI_CS_SD_READER, PB1, OUTPUT)

ifdef(`conf_STATUSLED_POWER', `dnl
pin(STATUSLED_POWER, PA3, OUTPUT)
')dnl

ifdef(`conf_STATUSLED_BOOTED', `dnl
pin(STATUSLED_BOOTED, PA3, OUTPUT)
')dnl

ifdef(`conf_STATUSLED_NETLINK', `dnl
pin(STATUSLED_NETLINK, PA1, OUTPUT)
')dnl

ifdef(`conf_RFM12', `dnl
  /* port the rfm12 module CS is attached to */
  pin(SPI_CS_RFM12_0, PB0,OUTPUT)
  RFM12_USE_INT(1)
  /* nur INT 0 available = PD2 (Pin 10) */
  RFM12_ASK_SENSE_USE_INT(0)

  /* port the LEDS for rfm12 txrx attached to */
  ifdef(`conf_STATUSLED_RFM12_TX', `
    pin(STATUSLED_RFM12_TX, PA0, OUTPUT)
  ')
  ifdef(`conf_STATUSLED_RFM12_RX', `
    pin(STATUSLED_RFM12_RX, PA2, OUTPUT)
  ')
')


