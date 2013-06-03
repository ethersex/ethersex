/* port the enc28j60 is attached to */
pin(SPI_CS_NET, SPI_CS_HARDWARE)

/* port the dataflash CS is attached to */
pin(SPI_CS_DF, PB1, OUTPUT)

ifdef(`conf_ONEWIRE', `dnl
  /* onewire port range */
  ONEWIRE_PORT_RANGE(PD6, PD6)
')dnl

ifdef(`conf_SD_READER', `dnl
  /* port the sd-reader CS is attached to */
  pin(SPI_CS_SD_READER, PB2, OUTPUT)
  dnl  pin(SD_READER_POWERON, PB3, OUTPUT)
')dnl

ifdef(`conf_FS20', `dnl
  /* fs20 support */
  pin(FS20_SEND, PB2)
  ifdef(`conf_FS20_RECEIVE', `dnl
  /* DO NOT CHANGE PIN!  USES INTERNAL COMPARATOR! */
  pin(FS20_RECV, PB3)
  ')dnl
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
  pin(SPI_CS_RFM12_0, PB0, OUTPUT)

dnl   /* port the LEDS for rfm12 txrx attached to */
dnl   pin(STATUSLED_RFM12_TX, PD4)
dnl   pin(STATUSLED_RFM12_RX, PD5)

  RFM12_USE_INT(2)
dnl RFM12_USE_PCINT(0, PB0)

dnl For RFM12 ASK-Sensing another interrupt line is required
dnl (different from the interrupt line required for IP-over-RFM12)
  RFM12_ASK_SENSE_USE_INT(2)
')

ifdef(`conf_ZBUS', `dnl
  /* port config for zbus */
  pin(ZBUS_RXTX_PIN, PD2)
  dnl  pin(STATUSLED_ZBUS_TX, PD4)
  dnl  pin(STATUSLED_ZBUS_RX, PD5)
')

ifdef(`conf_RC5', `
  pin(RC5_SEND, PD4)
  RC5_USE_INT(0)
')

ifdef(`conf_HD44780', `
    pin(HD44780_RS, PA0)
    pin(HD44780_RW, PA1)
    pin(HD44780_EN1, PA2)
    pin(HD44780_D4, PA3)
    pin(HD44780_D5, PA4)
    pin(HD44780_D6, PA5)
    pin(HD44780_D7, PA6)
')

