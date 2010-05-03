/*
*
* Copyright (c) 2009 by Daniel Walter <fordprfkt@googlemail.com>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 3
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
* For more information on the GPL, please go to:
* http://www.gnu.org/copyleft/gpl.html
*/

#include "config.h"
#include "core/debug.h"
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include "s1d13305.h"
#include "s1d13305_cfg.h"

#ifdef LCD_DRAWBITMAP_USED
    extern uint8_t lcd_getBMPData(uint16_t address_ui16);
/* EXAMPLE:
    uint8_t lcd_getBMPData(uint16_t address_ui16)
    {
        return pgm_read_byte(yourBMP+address_ui16);
    }
*/
#endif

/* local variables */
bool displayOn_gb = 0;  /* 1=display on, 0=display off */

/**
* @brief Initialize the display controller
*
* Resets and initializes the display controller for graphics
* mode or mixed text/graphics mode. In mixed mode,
* layer 1 is the text layer, layer 2 is the graphic layer.
* In graphics mode, layers 1,2,3 are graphics layers.
*/
void lcd_init(void)
{
  uint8_t param_ui8[12];

  /* Set control pins to defined state and reset the controller */
  S1D13305_DATA = 0x00;
  PIN_SET(S1D13305_WR);
  PIN_SET(S1D13305_RD);
  PIN_SET(S1D13305_A0);
  PIN_CLEAR(S1D13305_CS);
  PIN_CLEAR(S1D13305_RES);
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  PIN_SET(S1D13305_RES);
  PIN_SET(S1D13305_CS);
  PIN_CLEAR(S1D13305_CS);
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  PIN_SET(S1D13305_CS);
  PIN_CLEAR(S1D13305_A0);

  wdt_kick();

  /* Initialize controller with display parameters */
  param_ui8[0] = LCD_SYSTEMSET_P1;
  param_ui8[1] = 0x80|(LCD_CHR_SIZE_X - 1); /* WF 0 0 0 0 _FX */
  param_ui8[2] = (LCD_CHR_SIZE_Y - 1);		/* Y size */
  param_ui8[3] = LCD_CR;					/* CR */
  param_ui8[4] = LCD_TCR;					/* TCR */
  param_ui8[5] = LCD_LF;					/* LF */
  param_ui8[6] = LCD_CR+1;                  /* APL */
  param_ui8[7] = 0x00;                      /* APH */
  lcd_writeCmd(CMD_SYSTEM_SET, 8, param_ui8);

  wdt_kick();

  /* Initialize layer start adresses */
  param_ui8[0] = 0x00;                /* SAD1L */
  param_ui8[1] = 0x00;                /* SAD1H */
  param_ui8[2] = CONF_S1D13305_RESY;  /* SL1 */
  param_ui8[3] = LOW_BYTE(LCD_SAD2);  /* SAD2L */
  param_ui8[4] = HIGH_BYTE(LCD_SAD2); /* SAD2H */
  param_ui8[5] = CONF_S1D13305_RESY;  /* SL2 */
  param_ui8[6] = LOW_BYTE(LCD_SAD3);  /* SAD3L */
  param_ui8[7] = HIGH_BYTE(LCD_SAD3); /* SAD3H */
  lcd_writeCmd(CMD_SCROLL, 8, param_ui8);

  /* Set cursor size */
  param_ui8[0] = LCD_CHR_SIZE_X / 2;    /* 0  0 0 0 _CRX */
  param_ui8[1] = 0x80 + LCD_CHR_SIZE_Y; /* CM 0 0 0 _CRY */
  lcd_writeCmd(CMD_CSRFORM, 2, param_ui8);

  /* Set horizontal scrolling to 0 pixels */
  param_ui8[0] = 0x00;
  lcd_writeCmd(CMD_HDOT_SCR, 1, param_ui8);

  /* Set overlay mode and text or graphic mode for the layers */
  lcd_setOverlayMode(LCD_COMBINE_XOR);

  /* Disable display for clearing, switch all layers on */
  lcd_setDisplayOn(false);
  lcd_setLayerModes(LCD_CURSOR_OFF, LCD_LAYER1_ON|LCD_LAYER2_ON|LCD_LAYER3_ON);

  /* Cursor goes to the right */
  lcd_writeCmdByte(CMD_CSRDIR_RI);

  /* Clear display memory */
  lcd_clear(LCD_ALL_LAYERS);
  lcd_setDisplayOn(true);
  lcd_backlightOn(true);

  #if S1D13305_MODE == S1D13305_MIXED
    LCDDEBUG("initialized, mixed mode.\n");
  #else
    LCDDEBUG("initialized, graphics mode.\n");
  #endif
}

/**
* @brief Write text on the text layer
*
* Write a string with the given lenght on the text layer
* in mixed mode on the given position.
*
* @param x_ui8 X Position
* @param y_ui8 Y Position
* @param text_pui8 Pointer to string
* @param len_ui16 Lenght of the string
*/
void
lcd_writeTextPos(uint8_t x_ui8, uint8_t y_ui8, uint8_t* text_pui8, uint16_t len_ui16)
{
  /* Text display only in mixed mode */
  #if S1D13305_MODE == S1D13305_MIXED
    lcd_setCursorPos(LCD_LAYER1, x_ui8, y_ui8);
    lcd_writeText(text_pui8, len_ui16);
  #endif
}

/**
* @brief Write text on the text layer
*
* Write a string with the given lenght on the text layer
* in mixed mode.
*
* @param text_pui8 Pointer to string
* @param len_ui16 Lenght of the string
*/
void
lcd_writeText(uint8_t* text_pui8, uint16_t len_ui16)
{
  /* Text display only in mixed mode */
  #if S1D13305_MODE == S1D13305_MIXED
    uint16_t i;
    lcd_writeCmdByte(CMD_MWRITE);
    for (i = 0; i < len_ui16; i++)
    {
      lcd_waitForCntrlrReady();
      lcd_writeData(text_pui8[i]);
    }
  #endif
}

/**
* @brief Draws a bitmap on the screen
*
* Draw a bitmap. lcd_DrawBitmap calls lcd_GetBitmapData
* to get the bitmap bytes. You must define LCD_DRAWBITMAP_USED
* in your application and implement lcd_GetBitmapData.
*
* @param layer_e Display layer where the bitmap should be drawn
* @param posX_ui16 X pos. where the bitmap should be drawn
* @param posY_ui16 Y pos. where the bitmap should be drawn
* @param byteWidth_ui16 Width of the bitmap in bytes
* @param height_ui16 Height of the bitmap in lines
*/
#ifdef LCD_DRAWBITMAP_USED
void
lcd_drawBitmap(uint8_t layer_ui8, uint16_t posX_ui16, uint16_t posY_ui16, uint16_t byteWidth_ui16, uint16_t height_ui16)
{
  uint16_t i,j;

  /* In mixed mode layer 1 displays text */
  #if S1D13305_MODE == S1D13305_MIXED
    if (LCD_LAYER1 == layer_ui8)
      return;
  #endif

  for (j = 0; j < height_ui16; j++)
  {
    lcd_setCursorPos(layer_ui8, posX_ui16, posY_ui16+j);
    lcd_writeCmdByte(CMD_MWRITE);
    for (i = 0; i < byteWidth_ui16; i++)
    {
      lcd_waitForCntrlrReady();
      lcd_writeData(lcd_getBMPData(i+(j*byteWidth_ui16)));
    }
  }
}
#endif

/**
* @brief Switch the display on or off
*
* Swicthes the display on or off. If display
* is off, the screen goes blank.
*
* @param displayOn_b true = display on, false = off
*/
void
lcd_setDisplayOn(bool displayOn_b)
{
  lcd_writeCmdByte(true == displayOn_b ? CMD_DISP_ON:CMD_DISP_OFF);
  displayOn_gb = displayOn_b;
}

/**
* @brief Controls cursor and layer display
*
* Controls the display of the cursor and the layers.
* Each layer and the cursor can be switched on or off
* and set to fast or slow blinking mode.
*
* @param cursorMode_e Cursor display mode
* @param layerModes_ui8 Layer display mode
*/
void
lcd_setLayerModes(uint8_t cursorMode_ui8, uint8_t layerModes_ui8)
{
  /* Only two layers in mixed mode */
  #if S1D13305_MODE == S1D13305_MIXED
    layerModes_ui8 &= ~LCD_LAYER3_BLINK_FAST;
  #endif

  lcd_writeCmdByte(true == displayOn_gb ? CMD_DISP_ON:CMD_DISP_OFF);
  lcd_writeData(cursorMode_ui8|layerModes_ui8);

  LCDDEBUG("layer mode: %i cursor mode: %i\n", layerModes_ui8, cursorMode_ui8);
}

/**
* @brief Set the layer mixing mode
*
* The overlay mode controls how the display layers are combined.
*
* @param mode_e Overlay mode
*/
void
lcd_setOverlayMode(uint8_t mode_ui8)
{
  /* Overlay mode byte
     0 0 0 OV DM2 DM1 MX1 MX0
     DM1 and DM2 must be set the same value */

  mode_ui8 &= 0x03;
  lcd_writeCmdByte(CMD_OVLAY);

  #if S1D13305_MODE == S1D13305_GRAPHICS
    /* Display 1+2+3 graphic mode, three layers */
    lcd_writeData(mode_ui8 | 0x1C);
  #else
    /* Display 1 text mode, display 2 graphic mode, two layers */
    lcd_writeData(mode_ui8);
  #endif

  LCDDEBUG("overlay mode: %i\n", mode_ui8);
}

/**
* @brief Set the cursor position
*
* Sets the cursor position within the display memory.
* The cursor is set to the given coordinates and the given
* screen layer. The display memory can only accessed per byte,
* so in graphics mode the cursor can only be moven by 8 pixels.
*
* @param layer_e Screen layer to be used
* @param posX_ui16 Horizontel position
* @param posY_ui16 Vertical position
*/
void
lcd_setCursorPos(uint8_t layer_ui8, uint16_t posX_ui16, uint16_t posY_ui16)
{
  uint8_t param_ui8[2];
  uint16_t address_ui16;

  #if S1D13305_MODE == S1D13305_MIXED
  if (LCD_LAYER1 == layer_ui8)
  {
    if (posX_ui16 > (CONF_S1D13305_RESX / LCD_CHR_SIZE_X))
    {
        posX_ui16 = (CONF_S1D13305_RESX / LCD_CHR_SIZE_X);
    }

    if (posY_ui16 > (CONF_S1D13305_RESY / LCD_CHR_SIZE_Y))
    {
        posY_ui16 = (CONF_S1D13305_RESY / LCD_CHR_SIZE_Y);
    }
  }
  #endif

  if (posX_ui16 > CONF_S1D13305_RESX)
  {
    posX_ui16 = CONF_S1D13305_RESX;
  }

  if (posY_ui16 > CONF_S1D13305_RESY)
  {
    posY_ui16 = CONF_S1D13305_RESY;
  }

  switch (layer_ui8)
  {
    case LCD_LAYER1:
      #if S1D13305_MODE == S1D13305_GRAPHICS
        address_ui16 = (posY_ui16*LCD_BYTEWIDTH) + (posX_ui16/8);
      #else
        address_ui16 = (posY_ui16*LCD_BYTEWIDTH) + (posX_ui16);
      #endif
    break;

    case LCD_LAYER2:
      address_ui16 = LCD_SAD2 + (posY_ui16*LCD_BYTEWIDTH) + (posX_ui16 / 8);
    break;

    case LCD_LAYER3:
      #if S1D13305_MODE == S1D13305_GRAPHICS
	address_ui16 = LCD_SAD3 + (posY_ui16*LCD_BYTEWIDTH) + (posX_ui16 / 8);
      #else
        address_ui16 = 0;
      #endif
    break;

    default:
      address_ui16 = 0;
    break;
  }

  param_ui8[0] = LOW_BYTE(address_ui16);
  param_ui8[1] = HIGH_BYTE(address_ui16);
  lcd_writeCmd(CMD_CSRW, 2, param_ui8);
}

/**
* @brief Clear a display layer
*
* Clears the display memory for a layer.
* A graphic layer is filled with 0x00, a text layer
* is filled with spaces.
*
* @param layer_e Layers to be cleared
*/
void
lcd_clear(uint8_t layer_ui8)
{
  uint16_t i;
  uint8_t param_ui8[2];

  if ((LCD_LAYER1 & layer_ui8) == LCD_LAYER1)
  {
    /* Set cursor to layer 1 */
    param_ui8[0] = 0;
    param_ui8[1] = 0;
    lcd_writeCmd(CMD_CSRW, 2, param_ui8);

    wdt_kick();

    /* Clear. Screen 1 must be treated differently *
     * in graphics or text mode                    */
    lcd_writeCmdByte(CMD_MWRITE);
    #if S1D13305_MODE == S1D13305_GRAPHICS
      for (i=0; i<LCD_BYTES_GRAPHIC; i++)
      {
	    lcd_waitForCntrlrReady();
        lcd_writeData(0x00);
      }
    #else
      for (i=0; i<LCD_BYTES_TEXT; i++)
      {
        lcd_writeData(0x20);
      }
    #endif

    LCDDEBUG("layer 1 cleared\n");
  }

  if ((LCD_LAYER2 & layer_ui8) == LCD_LAYER2)
  {
    /* Set cursor to layer 2 */
    param_ui8[0] = LOW_BYTE(LCD_SAD2);
    param_ui8[1] = HIGH_BYTE(LCD_SAD2);
    lcd_writeCmd(CMD_CSRW, 2, param_ui8);

    wdt_kick();

	/* Clear */
	lcd_writeCmdByte(CMD_MWRITE);
    for (i=0; i < LCD_BYTES_GRAPHIC; i++)
    {
	  lcd_waitForCntrlrReady();
      lcd_writeData(0x00);
    }

    LCDDEBUG("layer 2 cleared\n");
  }

  #if S1D13305_MODE == S1D13305_GRAPHICS
    if ((LCD_LAYER3 & layer_ui8) == LCD_LAYER3)
    {
      /* Set cursor to layer 3 */
      param_ui8[0] = LOW_BYTE(LCD_SAD3);
      param_ui8[1] = HIGH_BYTE(LCD_SAD3);
      lcd_writeCmd(CMD_CSRW, 2, param_ui8);

      wdt_kick();

	  /* Clear */
	  lcd_writeCmdByte(CMD_MWRITE);
      for (i=0; i < LCD_BYTES_GRAPHIC; i++)
      {
	    lcd_waitForCntrlrReady();
        lcd_writeData(0x00);
      }

      LCDDEBUG("layer 3 cleared\n");
    }
  #endif
}

/**
* @brief Write a data byte to the display memory
*
* Switches the controller in data mode and writes
* a data byte into display memory at the current
* cursor position. The cursor position is incremented
* after writing.
*
* @param data_ui8 The data byte
*/
void
lcd_writeData(uint8_t data_ui8)
{
  S1D13305_DATA_DDR = 0xFF;
  PIN_CLEAR(S1D13305_A0);	/* data mode */
  S1D13305_DATA = data_ui8;	/* write data byte to data port */
  PIN_CLEAR(S1D13305_CS);
  PIN_CLEAR(S1D13305_WR);
  asm("nop");
  asm("nop");
  PIN_SET(S1D13305_WR);
  PIN_SET(S1D13305_CS);
}

/**
* @brief Read a data byte from the display memory
*
* Reads a data byte from the display memory at the current
* cursor position. The cursor position is incremented after
* reading.
*
* @result Display data byte
*/
uint8_t
lcd_readData()
{
  uint8_t result_ui8;

  /* configure LCD data port as input */
  S1D13305_DATA = 0xFF;
  S1D13305_DATA_DDR = 0x00;

  /* Set controller in read mode */
  PIN_SET(S1D13305_WR);
  PIN_SET(S1D13305_A0);
  PIN_SET(S1D13305_RD);
  PIN_CLEAR(S1D13305_CS);
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  PIN_CLEAR(S1D13305_RD);
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");

  /* read... */
  result_ui8 = S1D13305_DATAIN;

  /* disable controller */
  PIN_SET(S1D13305_RD);
  PIN_SET(S1D13305_CS);

  /* reconfigure LCD data port */
  S1D13305_DATA_DDR = 0xFF;
  S1D13305_DATA = 0x00;

  return result_ui8;
}

/**
* @brief Send a single command byte to the controller
*
* Switches the controller in command mode and sends
* a command byte.
*
* @param cmd_ui8 The controller command byte
*/
void
lcd_writeCmdByte(uint8_t cmd_ui8)
{
  S1D13305_DATA_DDR = 0xFF;
  PIN_SET(S1D13305_A0);     /* command mode */
  S1D13305_DATA = cmd_ui8;  /* write command byte to data port */
  PIN_CLEAR(S1D13305_CS);
  PIN_CLEAR(S1D13305_WR);
  asm("nop");
  asm("nop");
  PIN_SET(S1D13305_WR);
  PIN_SET(S1D13305_CS);
  PIN_SET(S1D13305_A0);
}

/**
* @brief Send a command with parameters to the controller
*
* Sends a command byte and parameters to the
* lcd controller. The parameters are given as
* array of bytes.
*
* @param cmd_ui8 The controller command byte
* @param size_ui8 Size of param_aui8
* @param param_aui8 Array of parameters
*/
void
lcd_writeCmd(uint8_t cmd_ui8, uint8_t size_ui8, uint8_t* param_aui8)
{
  uint8_t i;
  lcd_writeCmdByte(cmd_ui8);
  for (i = 0; i < size_ui8; i++)
  {
    lcd_writeData(param_aui8[i]);
  }
}

/**
* @brief Waits for the controller ready flag
*
* Waits for the lcd controller ready flag.
* Display memory should only be written
* while the ready flag is high to prevent
* flickering on the screen. The function returns
* when the ready flag becomes high or after 255
* wait cycles.
*/
void
lcd_waitForCntrlrReady(void)
{
  uint8_t ctr_ui8 = 0;
  uint8_t result_ui8 = 0xFF;

  /* Configure LCD data port as input */
  S1D13305_DATA = 0xFF;
  S1D13305_DATA_DDR = 0x00;

  /* Set status flag read mode */
  PIN_SET(S1D13305_WR);
  PIN_CLEAR(S1D13305_A0);
  PIN_CLEAR(S1D13305_RD);
  PIN_CLEAR(S1D13305_CS);

  wdt_kick();

  /* Wait 255 cycles for pin 6 to go low (=memory can be written) */
  while ((ctr_ui8++ < 255) && ((result_ui8 & 0x40) == 0x40))
  {
    result_ui8 = S1D13305_DATAIN;
  }

  /* Disable controller */
  PIN_SET(S1D13305_RD);
  PIN_SET(S1D13305_A0);
  PIN_SET(S1D13305_CS);

  /* Reconfigure port  */
  S1D13305_DATA_DDR = 0xFF;
  S1D13305_DATA = 0x00;
}

/**
* @brief Controls sleep mode
*
* Puts the controller into sleep mode or
* wake the controller up.
*
* @param sleepIn_b true = sleep in, false = wake up
*/
void
lcd_setSleepMode(bool sleepIn_b)
{
  if (true == sleepIn_b)
  {
    lcd_writeCmdByte(CMD_SLEEP_IN);
    LCDDEBUG("sleep in\n");
  }
  else
  {
    /* to wake the controller,
       systemSet with P1 must be send,
       and the display must be turned on again. */
    lcd_writeCmdByte(CMD_SYSTEM_SET);
    lcd_writeData(LCD_SYSTEMSET_P1);
    lcd_writeCmdByte(CMD_DISP_ON);
    LCDDEBUG("wake up\n");
  }
}

/**
* @brief Switch the backlight on or off
*
* If backlight support is configured,
* the backlight can be swiched on or off
*
* @param lightOn_b true = backlight on, false = off
*/
void
lcd_backlightOn(bool lightOn_b)
{
  #ifdef LCD_BACKLIGHT_SUPPORT
    if (true == lightOn_b)
      PIN_SET(LCD_BACKLIGHT);
    else
      PIN_CLEAR(LCD_BACKLIGHT);

    LCDDEBUG("backlight %b\n", lightOn_b);
  #endif /* LCD_BACKLIGHT_SUPPORT */
}

/*
  -- Ethersex META --
  header(hardware/lcd/s1d13305/s1d13305.h)
  init(lcd_init)
*/

/* EOF */
