/* port the enc28j60 is attached to */
pin(SPI_CS_NET, PB0, OUTPUT)

/* pin(SPI_CS_DF, PB4) */

ifdef(`conf_STATUSLED_POWER', `dnl
pin(STATUSLED_POWER, PA3, OUTPUT)
')dnl

ifdef(`conf_STATUSLED_BOOTED', `dnl
pin(STATUSLED_BOOTED, PA3, OUTPUT)
')dnl

ifdef(`conf_STATUSLED_NETLINK', `dnl
pin(STATUSLED_NETLINK, PA2, OUTPUT)
')dnl

ifdef(`conf_STATUSLED_TX', `dnl
pin(STATUSLED_TX, PE0, OUTPUT)
')dnl

ifdef(`conf_STATUSLED_RX', `dnl
pin(STATUSLED_RX, PE1, OUTPUT)
')dnl
