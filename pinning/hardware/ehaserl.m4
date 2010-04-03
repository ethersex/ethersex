dnl
dnl user_ehaserl.m4
dnl
dnl Pin Configuration for 'user_ehaserl'.  Edit it to fit your needs.
dnl

dnl This must also be D+ of the USB signal
USB_USE_INT(0) 

dnl This pin must have the 1k5 Pullup and must be on the same port as the Interupt pin
pin(USB_DMINUS, PD5)
pin(USB_DPLUS, PD2)

dnl Dual Leds
dnl pin(LED_RED1, PD6, OUTPUT)
dnl pin(LED_RED2, PB1, OUTPUT)
dnl pin(LED_GREEN1, PB2, OUTPUT)
dnl pin(LED_GREEN2, PB3, OUTPUT)

pin(LIGHT, PC1)


