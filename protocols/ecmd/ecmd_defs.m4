dnl
dnl ecmd_defs.m4
dnl
dnl  You can declare your ecmd commands here.  The code will be expanded
dnl  by the make system (which in turn uses GNU m4) to build the
dnl  ecmd_defs.c file.
dnl
dnl  Lines starting with `dnl' are comments.
dnl

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
