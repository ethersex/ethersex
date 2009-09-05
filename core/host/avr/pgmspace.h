#ifndef AVR_PGMSPACE_H
#define AVR_PGMSPACE_H

#define sscanf_P(a...)		sscanf(a)

#define printf_P(a...)		printf(a)
#define sprintf_P(a...)		sprintf(a)
#define snprintf_P(a...)	snprintf(a)

#define memcmp_P(a...)		memcmp(a)
#define memcpy_P(a...)		memcpy(a)

#define strlen_P(a...)		strlen(a)
#define strstr_P(a...)		strstr(a)
#define strcpy_P(a...)		strcpy(a)
#define strncmp_P(a...)		strncmp(a)

#define pgm_read_word(a)	(*(a))
#define pgm_read_byte(a)	(*(a))

#define PSTR(a)	(a)
#define PROGMEM

typedef const char * PGM_P;

#endif  /* AVR_PGMSPACE_H */
