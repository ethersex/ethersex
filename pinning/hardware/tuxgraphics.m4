dnl
dnl user_tuxgraphics.m4
dnl
dnl Pin Configuration for 'tuxgraphics'.  Edit it to fit your needs.
dnl

/* port the enc28j60 is attached to */
pin(SPI_CS_NET, SPI_CS_HARDWARE)

ifdef(`conf_STATUSLED_POWER', `dnl
pin(STATUSLED_POWER, PB1, OUTPUT)
')dnl

