/* port the enc28j60 is attached to */
pin(SPI_CS_NET, SPI_CS_HARDWARE)

ifdef(`conf_ONEWIRE', `dnl
  /* onewire support */
  pin(ONEWIRE, PD7)
')dnl

ifdef(`conf_STATUSLED_HB_ACT', `dnl
pin(STATUSLED_HB_ACT, PA1, OUTPUT)
')dnl

ifdef(`conf_STATUSLED_NETLINK', `dnl
pin(STATUSLED_NETLINK, PA0, OUTPUT)
')dnl


ifdef(`conf_HD44780', `
pin(HD44780_RS, PC5)
pin(HD44780_RW, PC6)
pin(HD44780_EN1, PC4)
pin(HD44780_D4, PC0)
pin(HD44780_D5, PC1)
pin(HD44780_D6, PC2)
pin(HD44780_D7, PC3)
')


pin(RELAIS_POWER, PD3, OUTPUT)
pin(RELAIS_RESET, PD4, OUTPUT)
pin(POWER_STATE, PD2, INPUT)
