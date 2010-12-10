/* onewire support */
ifdef(`conf_ONEWIRE', `dnl
pin(ONEWIRE, PC2)
')

/* port the enc28j60 is attached to */
pin(SPI_CS_NET, SPI_CS_HARDWARE)

/* port the sd-reader CS is attached to */
ifdef(`conf_SD_READER', `
  pin(SPI_CS_SD_READER, PB3, OUTPUT)
')

ifdef(`conf_RFM12', `dnl
  /* port the rfm12 module CS is attached to */
  pin(SPI_CS_RFM12, PD5, OUTPUT)
  RFM12_USE_INT(1)
  RFM12_ASK_SENSE_USE_INT(1)
')

ifdef(`conf_IRMP', `dnl
  pin(IRMP_RX, PB0)
  #define IRMP_USE_TIMER0
  #define IRMP_RX_LOW_ACTIVE
  #undef IRMP_RX_LED_LOW_ACTIVE
  pin(IRMP_TX, PD7) dnl OC2
')

ifdef(`conf_STATUSLED_POWER', `dnl
pin(STATUSLED_POWER, PD2, OUTPUT)
')dnl

ifdef(`conf_STATUSLED_BOOTED', `dnl
pin(STATUSLED_BOOTED, PD2, OUTPUT)
')dnl

ifdef(`conf_STATUSLED_NETLINK', `dnl
pin(STATUSLED_NETLINK, PD4, OUTPUT)
')dnl

ifdef(`conf_STATUSLED_TX', `dnl
pin(STATUSLED_TX, PD4, OUTPUT)
')dnl

ifdef(`conf_STATUSLED_RX', `dnl
pin(STATUSLED_RX, PD6, OUTPUT)
')dnl

ifdef(`conf_STATUSLED_HB_ACT', `dnl
pin(STATUSLED_HB_ACT,PD6, OUTPUT)
')dnl

