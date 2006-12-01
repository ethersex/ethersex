#include <avr/io.h>  // RAMEND
#include "mem-check.h"

// Mask to init SRAM and check against
#define MASK 0xaa

// From linker script
extern unsigned char __heap_start;

unsigned short
get_mem_unused (void)
{
   unsigned short unused = 0;
   unsigned char *p = &__heap_start;

   do
   {
      if (*p++ != MASK)
         break;

      unused++;
   } while (p <= (unsigned char*) RAMEND);

      return unused;
}

/* !!! never call this function !!! */
void __attribute__ ((naked, section (".init8")))
__init8_mem (void)
{
   __asm volatile (
      "ldi r30, lo8 (__heap_start)"  "\n\t"
      "ldi r31, hi8 (__heap_start)"  "\n\t"
      "ldi r24, %0"                  "\n\t"
      "ldi r25, hi8 (%1)"            "\n"
      "0:"                           "\n\t"
      "st  Z+,  r24"                 "\n\t"
      "cpi r30, lo8 (%1)"            "\n\t"
      "cpc r31, r25"                 "\n\t"
      "brlo 0b"
         :
         : "i" (MASK), "i" (RAMEND+1)
   );
}
