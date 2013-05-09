
dnl Block pins used by LCD ...
define(`port_mask_A', eval(port_mask_A | 0xF7))dnl
define(`port_mask_C', eval(port_mask_C | 0xFF))dnl
define(`port_mask_D', eval(port_mask_D | 0xFE))dnl
define(`port_mask_G', eval(port_mask_G | 0x07))dnl



dnl Motor
pin(MOTOR_POS, PB7, OUTPUT)
pin(MOTOR_NEG, PB4, OUTPUT)
pin(MOTOR_DIRP, PG3, OUTPUT)
pin(MOTOR_DIRN, PG4, OUTPUT)

dnl Stellrad
pin(STELLRAD_A, PB6)
pin(STELLRAD_B, PB5)

dnl Taster
pin(TASTE_AUTO, PB3)
pin(TASTE_PROG, PB2)
pin(TASTE_GRAD, PB1)
pin(MONTIERT, PB0)

dnl Lichtschranke
pin(LICHTSCHRANKE_ENABLE, PE3, OUTPUT)
pin(LICHTSCHRANKE_SENSE, PE4)

dnl Temperaturfuehler
pin(TEMP_ENABLE, PF3, OUTPUT)
pin(TEMP_SENSE, PF2)


dnl *******************************************************************************
dnl *  LCD Layout:
dnl *
dnl *        B|B|B|B|B|B|B|B|B|B|B |B |B |B |B |B |B |B |B |B |B |B |B |B |    
dnl *        0|1|2|3|4|5|6|7|8|9|10|11|12|13|14|15|16|17|18|19|20|21|22|23|    
dnl *                                                                          
dnl *        ---------------------------BAR24------------------------------    
dnl *                                                                          
dnl *      Auto      3A         2A                  1A         0A       SUN    
dnl *             3F    3B   2F    2B            1F    1B   0F    0B           
dnl *      Manu      3g         2g       Col2       1g         0g       MOON   
dnl *             3E    3C   2E    2C            1E    1C   0E    0C           
dnl *      Prog      3D         2D       Col1       1D         0D       STAR   
dnl *                                                                          
dnl *******************************************************************************

#define LCD_SEG_B0		0	 // 0, 0
#define LCD_SEG_B1		2	 // 0, 2
#define LCD_SEG_B2		3	 // 0, 3
#define LCD_SEG_B3		4	 // 0, 4
#define LCD_SEG_B4		5	 // 0, 5
#define LCD_SEG_B5		6	 // 0, 6
#define LCD_SEG_B6		7	 // 0, 7
#define LCD_SEG_B7		8	 // 1, 0
#define LCD_SEG_B8		9	 // 1, 1
#define LCD_SEG_B9		10	 // 1, 2
#define LCD_SEG_BAR24		11	 // 1, 3
#define LCD_SEG_B14		12	 // 1, 4
#define LCD_SEG_B15		13	 // 1, 5
#define LCD_SEG_B16		14	 // 1, 6
#define LCD_SEG_B17		15	 // 1, 7
#define LCD_SEG_B18		16	 // 2, 0
#define LCD_SEG_B19		17	 // 2, 1
#define LCD_SEG_B20		18	 // 2, 2
#define LCD_SEG_B21		19	 // 2, 3
#define LCD_SEG_B22		20	 // 2, 4
#define LCD_SEG_B23		21	 // 2, 5
//*****************************************************************
#define LCD_SEG_AUTO		40	 // 5, 0
#define LCD_SEG_PROG		42	 // 5, 2
#define LCD_SEG_3F		43	 // 5, 3
#define LCD_SEG_3G		44	 // 5, 4
#define LCD_SEG_3A		45	 // 5, 5
#define LCD_SEG_3B		46	 // 5, 6
#define LCD_SEG_2F		47	 // 5, 7
#define LCD_SEG_2G		48	 // 6, 0
#define LCD_SEG_2A		49	 // 6, 1
#define LCD_SEG_2B		50	 // 6, 2
#define LCD_SEG_COL2		51	 // 6, 3
#define LCD_SEG_1F		52	 // 6, 4
#define LCD_SEG_1G		53	 // 6, 5
#define LCD_SEG_1A		54	 // 6, 6
#define LCD_SEG_1B		55	 // 6, 7
#define LCD_SEG_0F		56	 // 7, 0
#define LCD_SEG_0G		57	 // 7, 1
#define LCD_SEG_0A		58	 // 7, 2
#define LCD_SEG_0B		59	 // 7, 3
#define LCD_SEG_SNOW		60	 // 7, 4
#define LCD_SEG_SUN		61	 // 7, 5
//*****************************************************************
#define LCD_SEG_MANU		82	 // 10, 2
#define LCD_SEG_3E		83	 // 10, 3
#define LCD_SEG_3D		84	 // 10, 4
#define LCD_SEG_B11		85	 // 10, 5
#define LCD_SEG_3C		86	 // 10, 6
#define LCD_SEG_2E		87	 // 10, 7
#define LCD_SEG_2D		88	 // 11, 0
#define LCD_SEG_B10		89	 // 11, 1
#define LCD_SEG_2C		90	 // 11, 2
#define LCD_SEG_COL1		91	 // 11, 3
#define LCD_SEG_1E		92	 // 11, 4
#define LCD_SEG_1D		93	 // 11, 5
#define LCD_SEG_B13		94	 // 11, 6
#define LCD_SEG_1C		95	 // 11, 7
#define LCD_SEG_0E		96	 // 12, 0
#define LCD_SEG_0D		97	 // 12, 1
#define LCD_SEG_B12		98	 // 12, 2
#define LCD_SEG_0C		99	 // 12, 3
#define LCD_SEG_MOON		101	 // 12, 5
//*****************************************************************

