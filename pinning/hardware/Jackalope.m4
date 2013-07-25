dnl
dnl user_Jackalope.m4
dnl
dnl Pin Configuration for 'user_Jackalope'.  Edit it to fit your needs.
dnl

/* port the rfm12 module CS is attached to */
pin(SPI_CS_RFM12_0, PB1, OUTPUT)

RFM12_USE_PCINT(13,PC5)
