dnl
dnl ecmd_defs.m4
dnl
dnl  You can declare your ecmd commands here.  The code will be expanded
dnl  by the make system (which in turn uses GNU m4) to build the
dnl  ecmd_defs.c file.
dnl
dnl  Lines starting with `dnl' are comments.
dnl

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

dnl EOR
