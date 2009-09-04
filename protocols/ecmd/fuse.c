
int16_t parse_cmd_fuse(char *cmd, char *output, uint16_t len)
{
    (void) cmd;
#ifdef _SPMCR
    _SPMCR = 1<<BLBSET | 1<<SPMEN;
    uint8_t lo = pgm_read_byte(0);
    _SPMCR = 1<<BLBSET | 1<<SPMEN;
    uint8_t hi = pgm_read_byte(3);

    return ECMD_FINAL(snprintf_P(output, len, PSTR("Fuses: low=%02X high=%02X"), lo, hi));
#else
    return ECMD_FINAL(snprintf_P(output, len, PSTR("Fuses: unsupported")));
#endif
}

/*
  -- Ethersex META --
  ecmd_feature(fuse, "fuse",,Display the fuse settings.)
*/
