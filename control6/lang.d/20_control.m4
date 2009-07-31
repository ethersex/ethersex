################################
# Control
################################
define(`CONTROL_START', `divert(action_table_divert)struct action action_threads[] = {
divert(ecmd_variable_divert)dnl
#ifdef C6_ECMD_USED

dnl Some hackery to support non-typed ECMD_GLOBAL defaulting to uint8_t.
#define C6_TYPE_		0
#define d_ d_uint8_t

struct c6_option_t {
  PGM_P name;
  struct c6_vario_type value;
};

struct c6_option_t c6_ecmd_vars[] = {
  /* hier alle variablen definieren */
divert(init_divert)void control6_init(void) {
divert(normal_start_divert)void control6_run(void) { dnl
divert(normal_divert)')
define(`CONTROL_END', `divert(control_end_divert)
}
/*
  -- Ethersex META --
  header(control6/control6.h)
  init(control6_init)
  timer(1, control6_run())
*/
divert(ecmd_variable_divert)dnl
};
`uint8_t control6_set(const char *varname, struct c6_vario_type value) {
  uint8_t i;
  for(i = 0;i < ' ecmd_global_count `;i ++) {
    if (strcmp_P(varname, c6_ecmd_vars[i].name) == 0) {
      c6_ecmd_vars[i].value = value;
      return 1;
    }
  }
  return 0;
}
uint8_t control6_get(const char *varname, struct c6_vario_type *value) {
  uint8_t i;
  for(i = 0;i < ' ecmd_global_count `;i ++) {
    if (strcmp_P(varname, c6_ecmd_vars[i].name) == 0) {
      *value = c6_ecmd_vars[i].value;
      return 1;
    }
  }
  return 0;
}

#endif  /* C6_ECMD_USED */'
divert(timer_divert)
};

divert(pin_table_divert)
};

divert(init_divert)dnl
}

divert(action_table_divert)
};

divert(control_end_divert)
')

