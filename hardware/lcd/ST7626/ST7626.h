#ifndef _ST7626_H
#define _ST7626_H

#include <stdio.h>
#include "config.h"

#ifdef ST7626_SUPPORT

#define ST7626_DATA	PORTA

/* prototypes */
void ST7626_reset(void);
void ST7626_write(uint8_t mode, uint8_t data);
void ST7626_load_eeprom(void);
void ST7626_load_paint(void);
void ST7626_init(void);
void ST7626_clear(uint8_t color);
int ST7626_putc(char d, FILE *stream);
void ST7626_putpixel(uint8_t x, uint8_t y, uint16_t color);

#endif /* ST7626_SUPPORT */

#endif /* _ST7626_H */
