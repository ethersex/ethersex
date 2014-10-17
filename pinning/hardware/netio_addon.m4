ifdef(`conf_ONEWIRE', `dnl
  /* onewire port range */
  ONEWIRE_PORT_RANGE(PC2, PC2)
')dnl


/* port the enc28j60 is attached to */
pin(SPI_CS_NET, SPI_CS_HARDWARE)

/* port the sd-reader CS is attached to */
ifdef(`conf_SD_READER', `
  pin(SPI_CS_SD_READER, PB3, OUTPUT)
')

ifdef(`conf_RFM12', `dnl
  /* port the rfm12 module CS is attached to */
  pin(SPI_CS_RFM12_0, PD5, OUTPUT)
  RFM12_USE_INT(1)
  RFM12_ASK_SENSE_USE_INT(1)
  pin(STATUSLED_RFM12_TX, PD4, OUTPUT)
  pin(STATUSLED_RFM12_RX, PD6, OUTPUT)

')

ifdef(`conf_IRMP', `dnl
dnl pin(IRMP_RX, PD3)
#undef IRMP_USE_TIMER2
#define IRMP_RX_LOW_ACTIVE
#undef IRMP_RX_LED_LOW_ACTIVE
dnl pin(STATUSLED_IRMP_RX, PC3, OUTPUT)
pin(IRMP_TX, PD7) dnl OC2/OC2A
#undef IRMP_TX_LED_LOW_ACTIVE
dnl pin(STATUSLED_IRMP_TX, PC2, OUTPUT)
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

ifdef(`conf_STATUSLED_HB_ACT', `dnl
pin(STATUSLED_HB_ACT,PD6, OUTPUT)
')dnl
ifdef(`conf_DCF77', `dnl
  DCF77_USE_INT(1, PD3)
  pin(DCF1_PON, PA1, OUTPUT)
')dnl

ifelse(value_HD44780_CONNECTION,`HD44780_I2CSUPPORT',`dnl
 dnl HD44780_PCF8574x_MAPPING(ADR,RS,RW,EN,DB4,DB5,DB6,DB7,BL)
 HD44780_PCF8574x_MAPPING(0x20, 4, 5, 6, 0, 1, 2, 3, 7)
')dnl

