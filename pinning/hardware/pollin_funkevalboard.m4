
ifdef(`conf_STATUSLED_TX', `dnl
pin(STATUSLED_TX, PA1, OUTPUT)
')dnl

ifdef(`conf_STATUSLED_RX', `dnl
pin(STATUSLED_RX, PA2, OUTPUT)
')dnl

ifdef(`conf_ONEWIRE', `dnl
  /* onewire port range */
  ONEWIRE_PORT_RANGE(PA0, PA0)
')dnl

ifdef(`conf_RFM12', `dnl
/* port the rfm12 module CS is attached to */
pin(SPI_CS_RFM12, SPI_CS_HARDWARE, OUTPUT)
RFM12_USE_INT(0)
RFM12_ASK_SENSE_USE_INT(0)
pin(STATUSLED_RFM12_TX, PD5, OUTPUT) 
pin(STATUSLED_RFM12_RX, PD6, OUTPUT) 
')
