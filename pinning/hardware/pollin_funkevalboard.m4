
ifdef(`conf_STATUSLED_TX', `dnl
pin(STATUSLED_TX, PA1, OUTPUT)
')dnl

ifdef(`conf_STATUSLED_RX', `dnl
pin(STATUSLED_RX, PA2, OUTPUT)
')dnl

ifdef(`conf_ONEWIRE', `dnl
  /* onewire support */
  pin(ONEWIRE, PA0)
')dnl

ifdef(`conf_RFM12', `dnl
/* port the rfm12 module CS is attached to */
pin(SPI_CS_RFM12, SPI_CS_HARDWARE, OUTPUT)
RFM12_USE_INT(0)
RFM12_ASK_SENSE_USE_INT(0)
pin(RFM12_TX_PIN, PD5, OUTPUT) 
pin(RFM12_RX_PIN, PD6, OUTPUT) 
')
