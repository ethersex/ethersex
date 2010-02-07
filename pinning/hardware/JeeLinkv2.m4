ifdef(`conf_STATUSLED_TX', `dnl
pin(STATUSLED_TX, PB1, OUTPUT)
')dnl

ifdef(`conf_STATUSLED_RX', `dnl
pin(STATUSLED_RX, PB1, OUTPUT)
')dnl

ifdef(`conf_RFM12', `dnl
    /* port the rfm12 module CS is attached to */
    pin(SPI_CS_RFM12, SPI_CS_HARDWARE, OUTPUT)
    RFM12_USE_INT(0)
')
