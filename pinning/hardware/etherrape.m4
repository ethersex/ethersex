/* port the enc28j60 is attached to */
pin(SPI_CS_NET, SPI_CS_HARDWARE)

/* port the dataflash CS is attached to */
pin(SPI_CS_DF, PB1, OUTPUT)

ifdef(`conf_SD_READER', `dnl
  /* port the sd-reader CS is attached to */
  pin(SPI_CS_SD_READER, PB2, OUTPUT)
')dnl

