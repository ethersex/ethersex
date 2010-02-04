dnl
dnl ecmd_defs.m4
dnl
dnl  You can declare your ecmd commands here.  The code will be expanded
dnl  by the make system (which in turn uses GNU m4) to build the
dnl  ecmd_defs.c file.
dnl
dnl  Lines starting with `dnl' are comments.
dnl

block([[RFM12_ASK]]) dnl ==========================
ecmd_ifdef(RFM12_ASK_SENDER_SUPPORT)
  ecmd_ifdef(RFM12_ASK_TEVION_SUPPORT)
  ecmd_feature(rfm12_ask_tevion_send, "rfm12 tevion", , housecode command delay cnt)
  ecmd_endif()
  ecmd_ifdef(RFM12_ASK_2272_SUPPORT)
    ecmd_feature(rfm12_ask_2272_send, "rfm12 2272", , housecodeCommand delay cnt)
  ecmd_endif()
  ecmd_ifdef(RFM12_ASK_1527_SUPPORT)
    ecmd_feature(rfm12_ask_1527_send, "rfm12 1527", , housecodeCommand delay cnt)
  ecmd_endif()
ecmd_endif()
ecmd_ifdef(RFM12_ASK_EXTERNAL_FILTER_SUPPORT)
  ecmd_feature(rfm12_ask_external_filter, "rfm12 external filter",[1], Enable ext. filter pin if argument is present (disable otherwise))
ecmd_endif()
ecmd_ifdef(RFM12_ASK_SENSING_SUPPORT)
  ecmd_feature(rfm12_ask_sense, "rfm12 ask sense",, Trigger (Tevion) ASK sensing.  Enable ext. filter pin before!)
ecmd_endif()

block([[Sound]]/Melody support) dnl ==========================
ecmd_ifdef(PWM_MELODY_SUPPORT)
  ecmd_feature(pwm_melody_play, "pwm melody", , Play melody)
ecmd_endif

ecmd_ifdef(PWM_WAV_SUPPORT)
  ecmd_feature(pwm_wav_play, "pwm wav", , Play wav)
  ecmd_feature(pwm_wav_stop, "pwm stop", , Stop wav)
ecmd_endif

block([[I2C]] (TWI)) dnl ==========================
ecmd_ifdef(I2C_DETECT_SUPPORT)
  ecmd_feature(i2c_detect, "i2c detect",,list detected I2C Chips)
ecmd_endif
ecmd_ifdef(I2C_GENERIC_SUPPORT)
  ecmd_feature(i2c_read_byte, "i2c rbb",ADDR,read byte from I2C chip)
  ecmd_feature(i2c_read_byte_data, "i2c rbd",CHIPADDR DATAADDR,read byte from address at I2C chip)
  ecmd_feature(i2c_read_word_data, "i2c rwd",CHIPADDR DATAADDR,read word from address at I2C chip)
  ecmd_feature(i2c_write_byte, "i2c wbb",ADDR HEXVALUE,write byte to I2C chip)
  ecmd_feature(i2c_write_byte_data, "i2c wbd",CHIPADDR DATAADDR HEXVALUE,write byte to address on I2C chip)
  ecmd_feature(i2c_write_word_data, "i2c wwd",CHIPADDR DATAADDR HEXVALUE,write word to address on I2C chip)
ecmd_endif
ecmd_ifdef(I2C_LM75_SUPPORT)
  ecmd_feature(i2c_lm75, "lm75",ADDR, Get temperature)
ecmd_endif
ecmd_ifdef(I2C_DS1631_SUPPORT)
  ecmd_feature(i2c_ds1631_read_temp, "ds1631 temp",ADDR, Read last converted temperature)
  ecmd_feature(i2c_ds1631_start, "ds1631 start",ADDR, Initiate temperature conversions)
  ecmd_feature(i2c_ds1631_stop, "ds1631 stop",ADDR, Stop temperature conversions)
ecmd_endif
ecmd_ifdef(I2C_TSL2550_SUPPORT)
  ecmd_feature(i2c_tsl2550_read_lux, "tsl2550 lux",, Read ADC registers and compute light level)
  ecmd_feature(i2c_tsl2550_pwr_up, "tsl2550 up",, Set TSL2550 to power up state)
  ecmd_feature(i2c_tsl2550_pwr_down, "tsl2550 down",, Set TSL2550 to power down state)
  ecmd_feature(i2c_tsl2550_mode, "tsl2550 mode",VALUE, Set TSL2550 to standard or extended mode [std|ext])
ecmd_endif
ecmd_ifdef(I2C_PCA9531_SUPPORT)
  ecmd_feature(i2c_pca9531, "pca9531",ADDR PERIODPWM1 DUTYPWM1 PERIODPWM2 DUTYPWM2 LED0..3 LED4..7, set PWM1 and PWM2 and LED states)
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
    ecmd_feature(fs20_send, "fs20 send",HOUSECODE ADDR CMD, Send FS20 command. See [[FS20]] for details.)
  ecmd_endif()

  ecmd_ifdef(FS20_RECEIVE_SUPPORT)
    ecmd_feature(fs20_receive, "fs20 receive",, Receive FS20 sequence and display it.)
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
