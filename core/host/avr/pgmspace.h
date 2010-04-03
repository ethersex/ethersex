#ifndef AVR_PGMSPACE_H
#define AVR_PGMSPACE_H

#include <string.h>

int printf_P (const char *fmt, ...);
int sprintf_P (char *, const char *fmt, ...);
int snprintf_P (char *, int, const char *fmt, ...);

#define sscanf_P(a...)		sscanf(a)

#define memcmp_P(a...)		memcmp(a)
#define memcpy_P(a...)		memcpy(a)

#define strlen_P(a...)		strlen(a)
#define strstr_P(a...)		strstr(a)
#define strcat_P(a...)		strcat(a)
#define strcpy_P(a...)		strcpy(a)
#define strncmp_P(a...)		strncmp(a)
#define strncasecmp_P(a...)	strncasecmp(a)

#define pgm_read_word(a)	(*(a))
#define pgm_read_byte(a)	(*(a))

#define PSTR(a)	(a)
#define PROGMEM

typedef const char * PGM_P;

#endif  /* AVR_PGMSPACE_H */
