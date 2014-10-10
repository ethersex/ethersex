dnl
dnl pollin_evalboard.m4
dnl
dnl Pin Configuration for 'pollin_evalboard'.  Edit it to fit your needs.
dnl

ifelse(value_HD44780_CONNECTION,`HD44780_DIREKT',`dnl
pin(HD44780_RS, PB2)
pin(HD44780_RW, PB1)
pin(HD44780_D4, PA4)
pin(HD44780_D5, PA5)
pin(HD44780_D6, PA6)
pin(HD44780_D7, PA7)
pin(HD44780_EN1, PB0)
')dnl

ifdef(`conf_RFM12', `dnl
/* port the rfm12 module CS is attached to */
dnl pin(SPI_CS_RFM12_0, SPI_CS_HARDWARE)
pin(SPI_CS_RFM12_0, PC0, OUTPUT)
RFM12_USE_INT(0)
RFM12_ASK_SENSE_USE_INT(1)
dnl pin(STATUSLED_RFM12_TX, PD5, OUTPUT)
dnl pin(STATUSLED_RFM12_RX, PD6, OUTPUT)
')

ifdef(`conf_FS20', `dnl
/* fs20 support */
pin(FS20_SEND, PD3)
ifdef(`conf_FS20_RECEIVE', `dnl
/* DO NOT CHANGE PIN!  USES INTERNAL COMPARATOR! */
pin(FS20_RECV, PB3)
')dnl
')dnl


ifdef(`conf_RC5', `
  pin(RC5_SEND, PD4)
  RC5_USE_INT(0)
')

ifdef(`conf_IRMP', `dnl
pin(IRMP_RX, PD3)
#undef IRMP_USE_TIMER2
#define IRMP_RX_LOW_ACTIVE
#undef IRMP_RX_LED_LOW_ACTIVE 
pin(STATUSLED_IRMP_RX, PC3, OUTPUT)
pin(IRMP_TX, PD7) dnl OC2/OC2A
#undef IRMP_TX_LED_LOW_ACTIVE 
pin(STATUSLED_IRMP_TX, PC2, OUTPUT)
')

dnl pin(KEY1, PD2)
dnl pin(KEY2, PD3)
dnl pin(KEY3, PD4)
dnl pin(LED1, PD5)
dnl pin(LED2, PD6)
dnl pin(SPEAKER, PD7)

/* port the sd-reader CS is attached to */
ifdef(`conf_SD_READER', `
  pin(SPI_CS_SD_READER, PC5, OUTPUT)
')
pin(SOFT_SPI_MOSI, PD2)
pin(SOFT_SPI_MISO, PD3)
pin(SOFT_SPI_SCK,  PD4)
