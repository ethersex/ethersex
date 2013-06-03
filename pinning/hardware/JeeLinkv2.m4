pin(STATUSLED_HB_ACT, PB1, OUTPUT)

ifdef(`conf_RFM12', `dnl
    /* port the rfm12 module CS is attached to */
    pin(SPI_CS_RFM12_0, SPI_CS_HARDWARE)
    RFM12_USE_INT(0)
')
