dnl
dnl ecmd_defs.m4
dnl
dnl  You can declare your ecmd commands here.  The code will be expanded
dnl  by the make system (which in turn uses GNU m4) to build the
dnl  ecmd_defs.c file.
dnl
dnl  Lines starting with `dnl' are comments.
dnl

block([[I2C]] (TWI)) dnl ==========================
ecmd_ifdef(I2C_DETECT_SUPPORT)
  ecmd_feature(i2c_detect, "i2c detect",,list detected I2C Chips)
ecmd_endif
ecmd_ifdef(I2C_GENERIC_SUPPORT)
  ecmd_feature(i2c_read_byte, "i2c rbb",ADDR,read byte from I2C chip)
  ecmd_feature(i2c_read_byte_data, "i2c rbd",CHIPADDR REGADDR,read byte from register address at I2C chip)
  ecmd_feature(i2c_read_word_data, "i2c rwd",CHIPADDR REGADDR,read word from register address at I2C chip)
  ecmd_feature(i2c_write_byte, "i2c wbb",ADDR HEXVALUE,write byte to I2C chip)
  ecmd_feature(i2c_write_byte_data, "i2c wbd",CHIPADDR REGADDR HEXVALUE,write byte to register address on I2C chip)
  ecmd_feature(i2c_write_word_data, "i2c wwd",CHIPADDR REGADDR HEXVALUE,write word to register address on I2C chip)
ecmd_endif
ecmd_ifdef(I2C_LM75_SUPPORT)
  ecmd_feature(i2c_lm75, "lm75",ADDR, Get temperature)
ecmd_endif
ecmd_ifdef(I2C_DS1631_SUPPORT)
  ecmd_feature(i2c_ds1631_read_temperature, "ds1631 temp",ADDR, Read last converted temperature)
  ecmd_feature(i2c_ds1631_set_power_state, "ds1631 convert",ADDR VALUE, Initiate temperature conversions (0: stop, 1: convert))
ecmd_endif
ecmd_ifdef(I2C_TSL2550_SUPPORT)
  ecmd_feature(i2c_tsl2550_show_lux_level, "tsl2550 lux",, Show light level by reading adc registers and computing level)
  ecmd_feature(i2c_tsl2550_set_power_state, "tsl2550 power", VALUE, Set the TSL2550s power state (0: down, 1:up))
  ecmd_feature(i2c_tsl2550_set_operating_mode, "tsl2550 mode", VALUE, Set the TSL2550s operating mode (0: standard range, 1: extended range))
ecmd_endif
ecmd_ifdef(I2C_TSL2561_SUPPORT)
  ecmd_feature(i2c_tsl2561_getlux, "tsl2561 lux", DEVNUM, Get LUX value)
  ecmd_feature(i2c_tsl2561_getraw, "tsl2561 raw", DEVNUM, Get RAW channel values)
  ecmd_feature(i2c_tsl2561_setmode, "tsl2561 setmode", DEVNUM TIME GAIN PACKAGE, Set device mode)
ecmd_endif
ecmd_ifdef(I2C_PCA9531_SUPPORT)
  ecmd_feature(i2c_pca9531, "pca9531",ADDR PERIODPWM1 DUTYPWM1 PERIODPWM2 DUTYPWM2 LED0..3 LED4..7, set PWM1 and PWM2 and LED states)
ecmd_endif
ecmd_ifdef(I2C_PCA9685_SUPPORT)
  ecmd_feature(i2c_pca9685_set_led, "pca9685s",ADDR, LED, ON, OFF )
  ecmd_feature(i2c_pca9685_set_mode, "pca9685m",ADDR, OUTDRV, IVRT, PRESCALE )
ecmd_endif
ecmd_ifdef(I2C_PCF8574X_SUPPORT)
  ecmd_feature(i2c_pcf8574x_read, "pcf8574x read",ADDR CHIP, Get bits)
  ecmd_feature(i2c_pcf8574x_set, "pcf8574x set",ADDR CHIP HEXVALUE, Set bits)
ecmd_endif
ecmd_ifdef(I2C_MAX7311_SUPPORT)
  ecmd_feature(i2c_max7311_setDDRw, "max7311 setDDRw", ADDR VALUE, Set Direction-Register DDR (VALUE as hex))
  ecmd_feature(i2c_max7311_setOUTw, "max7311 setOUTw", ADDR VALUE, Set Output-Register OUT (VALUE as hex))
  ecmd_feature(i2c_max7311_getDDRw, "max7311 getDDRw", ADDR, Get Direction-Register DDR)
  ecmd_feature(i2c_max7311_getOUTw, "max7311 getOUTw", ADDR, Get Output-Register OUT)
  ecmd_feature(i2c_max7311_getINw, "max7311 getINw", ADDR, Get Input-Register IN)
  ecmd_feature(i2c_max7311_set, "max7311 set", ADDR BIT VALUE, Set Output-BIT to VALUE (bool))
  ecmd_feature(i2c_max7311_pulse, "max7311 pulse", ADDR BIT TIME, Toggle Output-BIT for TIME (in ms))
ecmd_endif
ecmd_ifdef(I2C_BMP085_SUPPORT)
  ecmd_feature(i2c_bmp085_temp, "bmp085 temp",, Get temperature in 0.1°C)
  ecmd_feature(i2c_bmp085_apress, "bmp085 apress",, Get absolute pressure in Pa)
  ecmd_ifdef(I2C_BMP085_BAROCALC_SUPPORT)
    ecmd_feature(i2c_bmp085_height, "bmp085 height", PRESSNN, Get height in cm, pressure at N0 needed)
    ecmd_feature(i2c_bmp085_pressnn, "bmp085 pressnn", HEIGHT, Get pressure at N0, height in cm needed)
  ecmd_endif
ecmd_endif

block([[Blinkenlights_MCUF|MCUF]]) dnl ============================
ecmd_ifdef(MCUF_SUPPORT)
  ecmd_ifdef(MCUF_CLOCK_SUPPORT)
    ecmd_feature(mcuf_show_clock, "mcuf showclock",, Show digital clock)
  ecmd_endif
  ecmd_ifdef(MCUF_SCROLLTEXT_SUPPORT)
   ecmd_feature(mcuf_show_string, "mcuf showstring",MESSAGE, Show scrolling MESSAGE on the display)
  ecmd_endif
  ecmd_ifdef(MCUF_MODUL_SUPPORT)
   ecmd_feature(mcuf_modul_list, "mcuf modul list",, List all modules)
   ecmd_feature(mcuf_modul, "mcuf modul",N, Select module N)
  ecmd_endif
ecmd_endif

block([[FS20]]) dnl ==========================
ecmd_ifdef(FS20_SUPPORT)
  ecmd_ifdef(FS20_SEND_SUPPORT)
    ecmd_feature(fs20_send, "fs20 send",HOUSECODE ADDR CMD [CMD2], Send FS20 command. See [[FS20]] for details.)
  ecmd_endif()
  ecmd_ifdef(FHT_SEND_SUPPORT)
    ecmd_feature(fht_send, "fht send",HOUSECODE ADDR CMD [CMD2], Send FHT command. See [[FS20]] for details.)
  ecmd_endif()

  ecmd_ifdef(FS20_RECEIVE_SUPPORT)
    ecmd_feature(fs20_receive, "fs20 receive",, Receive FS20/FHT sequence and display it.)
  ecmd_endif()

  ecmd_ifdef(FS20_RECEIVE_WS300_SUPPORT)
    ecmd_feature(fs20_ws300, "fs20 ws300",, Receive FS20 sequence from WS300 weather station and decode it.)
  ecmd_endif()
ecmd_endif()

block(Miscelleanous) dnl ============================
dnl   Put stuff that doesn't fit elsewhere here, i.e. this is a good place
dnl   for commands that don't fit in any category and would thus require to add
dnl   a category just for one specific command (which we don't want)

ecmd_ifndef(TEENSY_SUPPORT)
  ecmd_feature(help, "help",, List which commands are available.)

  ecmd_feature(version, "version",,Display the version number.)

dnl  ecmd_ifdef(USART_SUPPORT)
dnl    ecmd_feature(usart_baud, "usart baud", BAUD, Set the USART baudrate to BAUD.)
dnl  ecmd_endif()
ecmd_endif()

ecmd_ifdef(FREE_SUPPORT)
  ecmd_feature(free, "free",, Display free space.)
ecmd_endif()

dnl block(EEPROM) dnl ==========================
ecmd_ifdef(EEPROM_SUPPORT)
  ecmd_feature(eeprom_reinit, "eeprom reinit",, Force reinitialization of the EEPROM config area)
ecmd_endif
