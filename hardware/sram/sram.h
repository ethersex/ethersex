#ifndef _SRAM_H
#define _SRAM_H

#define SRAM_START_ADDRESS (uint8_t*)0x1100
#define SRAM_END_ADDRESS (uint8_t*)0xFFFF

/* debugging support */
#if 1
# define SRAM_DEBUG(a...) debug_printf("sram: " a)
#else
# define SRAM_DEBUG(a...) do { } while(0)
#endif

void sram_init(void);

#endif
