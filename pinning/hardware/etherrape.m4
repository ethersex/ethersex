/* port the enc28j60 is attached to */
pin(SPI_CS_NET, PB4)

/* port the dataflash CS is attached to */
pin(SPI_CS_DF, PB1)

ifdef(`conf_SD_READER', `dnl
  /* port the sd-reader CS is attached to */
  pin(SPI_CS_SD_READER, PB2)
')dnl

