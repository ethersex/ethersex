dnl
dnl user_Fimser.m4
dnl
dnl Pin Configuration for 'user_Fimser'.  Edit it to fit your needs.
dnl

/* port the rfm12 module CS is attached to */
pin(SPI_CS_RFM12_0, PD5, OUTPUT)
dnl RFM12_USE_INT(1)
RFM12_USE_PCINT(1, PD3)

pin(SIG_VIBR,     PC0, OUTPUT)
pin(SIG_PA_ON,    PC1, OUTPUT)
pin(SIG_RX,       PC2, OUTPUT)
pin(SPARE1,       PC3, OUTPUT)
pin(NRESET_LCD,   PC4, OUTPUT)
pin(VIN,          PC5, OUTPUT)
pin(VIN2,         PC7, OUTPUT)

pin(SEL_TAST1,    PD0, OUTPUT)
pin(SEL_TAST2,    PD1, OUTPUT)
pin(DC_LCD,       PD2, OUTPUT)
pin(CS_LCD_TAST,  PD4, OUTPUT)
pin(CS_MEM1,      PD6, OUTPUT)
pin(CS_MEM2,      PD7, OUTPUT)

pin(SIGN_LB0,     PB0, OUTPUT)
pin(SPK,          PB1, OUTPUT)
dnl double use
dnl pin(LED,          PB2, OUTPUT)
dnl double use
dnl pin(TAST_IN,      PB4, OUTPUT)
pin(SPARE2,       PB7, OUTPUT)
