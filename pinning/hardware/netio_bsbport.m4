/* port the enc28j60 is attached to */
pin(SPI_CS_NET, SPI_CS_HARDWARE)

ifdef(`conf_SD_READER', `dnl
  /* port the sd-reader CS is attached to */
  pin(SPI_CS_SD_READER, PB2, OUTPUT)
')dnl

ifdef(`conf_ONEWIRE', `dnl
  /* onewire port range */
  ONEWIRE_PORT_RANGE(PA4, PA4)
')dnl

ifdef(`conf_STELLA', `dnl
  /* stella port 1 */
  STELLA_PORT1_RANGE(PC0, PC7)
  dnl /* stella port 2*/
  dnl STELLA_PORT2_RANGE(PA0, PA3)
  STELLA_USE_TIMER(2)
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

ifdef(`conf_STATUSLED_HB_ACT', `dnl
  pin(STATUSLED_HB_ACT, PA0, OUTPUT)
')dnl

ifdef(`conf_FS20', `dnl
  /* fs20 support */
  pin(FS20_SEND, PD7)
  ifdef(`conf_FS20_RECEIVE', `dnl
  /* DO NOT CHANGE PIN!  USES INTERNAL COMPARATOR! */
    pin(FS20_RECV, PB3)
  ')dnl
')dnl

ifdef(`conf_RFM12', `dnl
  /* port the rfm12 module CS is attached to */
  pin(SPI_CS_RFM12_0, PD5, OUTPUT)
  RFM12_USE_INT(1)
  RFM12_ASK_SENSE_USE_INT(1)
')

ifdef(`conf_ZBUS', `dnl
  /* port config for zbus */
  pin(ZBUS_RXTX_PIN, PD2)
')

ifdef(`conf_RC5', `
  pin(RC5_SEND, PD4)
')

ifdef(`conf_LTC1257', `
  pin(LTC1257_CLK, PA2, OUTPUT)
  pin(LTC1257_DATA, PA1, OUTPUT)
  pin(LTC1257_LOAD, PA0, OUTPUT)
')

ifdef(`conf_USTREAM', `
  pin(VS1053_CS, PB0, OUTPUT)
  pin(VS1053_DREQ, PB3, INPUT)
')

ifelse(value_HD44780_CONNECTION,`HD44780_DIREKT',`dnl
   pin(HD44780_RS, PC0)
   pin(HD44780_RW, PC1)
   pin(HD44780_EN1, PC2)
   pin(HD44780_D4, PC3)
   pin(HD44780_D5, PC4)
   pin(HD44780_D6, PC5)
   pin(HD44780_D7, PC6)
')
ifdef(`conf_HD44780_BACKLIGHT', `
  pin(HD44780_BL, PB1, OUTPUT)
')
ifdef(`conf_DCF77', `dnl
  DCF77_USE_INT(1, PD3)
  pin(DCF1_PON, PA1, OUTPUT)
')dnl

ifdef(`conf_TANKLEVEL', `
  pin(TANKLEVEL_PUMP, PC3, OUTPUT)
')
ifdef(`conf_TANKLEVEL_LOCK', `
  pin(TANKLEVEL_LOCK, PA2, INPUT)
')
