dnl
dnl user_Fimser.m4
dnl
dnl Pin Configuration for 'user_Fimser'.  Edit it to fit your needs.
dnl

/* port the rfm12 module CS is attached to */
pin(SPI_CS_RFM12, PD5, OUTPUT)
RFM12_USE_INT(1)
