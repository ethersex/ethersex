/*
* ECMD-commands for DALI light control
*
* Copyright (c) 2011 by Gerd v. Egidy <gerd@egidy.de>
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

#include <string.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#include "config.h"
#include "core/debug.h"
#include "protocols/ecmd/ecmd-base.h"

#include "protocols/dali/dali.h"

int16_t parse_cmd_dali_raw(char *cmd, char *output, uint16_t len)
{
    uint8_t frame[2];

    while(*cmd == ' ') cmd++;

    if (sscanf_P(cmd, PSTR("%hhx %hhx"), frame, frame+1) != 2)
        return ECMD_ERR_PARSE_ERROR;

    dali_send((uint16_t*)frame);
    
    return ECMD_FINAL_OK;
}

static inline uint8_t parse_dali_target(char **cmd, uint8_t *targetcode)
{
    uint8_t targetno=0;

    // strip whitespace
    while(**cmd == ' ') (*cmd)++;

    if (strncmp_P(*cmd,PSTR("all"),3)==0)
    {
        // broadcast
        *targetcode=0xFE;
    }
    else if (sscanf_P(*cmd, PSTR("g%hhu"), &targetno) == 1)
    {
        // group address, 0-15
        if (targetno > 15)
            return 0;
        
        *targetcode=(targetno << 1) | 0x80;
    }
    else if (sscanf_P(*cmd, PSTR("s%hhu"), &targetno) == 1)
    {
        // short address, 0-63
        if (targetno > 63)
            return 0;
        
        *targetcode=(targetno << 1);
    }
    else
        return 0;

    // read pointer to beginning of next arg
    while(**cmd && **cmd != ' ') (*cmd)++;
    while(**cmd == ' ') (*cmd)++;
    
    return 1;
}

static void repeat_dali_cmd(char *cmd, uint16_t *frame)
{
    // read pointer to beginning of next arg
    while(*cmd && *cmd != ' ') cmd++;
    while(*cmd == ' ') cmd++;

    // does user want to repeat?
    if (*cmd == '!')
    {
        // repeat the last command after 20msec (reaction + response time)
        _delay_ms(20);
        dali_send(frame);
    }
}
    
#ifdef DALI_RECEIVE_SUPPORT
static int16_t read_dali_reply(char *cmd, char *output)
{
    // read pointer to beginning of next arg
    while(*cmd && *cmd != ' ') cmd++;
    while(*cmd == ' ') cmd++;

    // does user want to read?
    if (*cmd == '?')
    {
        uint8_t frame;
        int8_t ret = dali_read(&frame);
        
        if (ret == DALI_READ_OK)
        {
            itoa(frame,output,10);
            return ECMD_FINAL(strlen(output));
        }
        else if (ret == DALI_READ_TIMEOUT)
        {
            strcpy_P(output, PSTR("READ TIMEOUT"));
            return ECMD_FINAL(12);  // length of "READ TIMEOUT"
        }
        else // ret == DALI_READ_ERROR
        {
            strcpy_P(output, PSTR("READ ERROR"));
            return ECMD_FINAL(10);  // length of "READ ERROR"
        }
    }
    else
        return ECMD_FINAL_OK;
}
#endif

enum dali_cmd { CMD, DIM };

static int16_t parse_cmd_dali_dimcmd(enum dali_cmd c, char *cmd, char *output, uint16_t len)
{
    uint8_t frame[2];

    if (!parse_dali_target(&cmd,&frame[0]))
        return ECMD_ERR_PARSE_ERROR;
           
    // commands are marked with lsb=1
    if (c == CMD)
        frame[0] |= 1;
        
    if (sscanf_P(cmd, PSTR("%hhu"), frame+1) != 1)
        return ECMD_ERR_PARSE_ERROR;

    dali_send((uint16_t*)frame);

    if (c == CMD)
    {
        repeat_dali_cmd(cmd,(uint16_t*)frame);
#ifdef DALI_RECEIVE_SUPPORT
        return read_dali_reply(cmd, output);
#endif
    }
    
    return ECMD_FINAL_OK;
}

int16_t parse_cmd_dali_dim(char *cmd, char *output, uint16_t len)
{
    return parse_cmd_dali_dimcmd(DIM,cmd,output,len);
}

int16_t parse_cmd_dali_cmd(char *cmd, char *output, uint16_t len)
{
    return parse_cmd_dali_dimcmd(CMD,cmd,output,len);
}

int16_t parse_cmd_dali_scmd(char *cmd, char *output, uint16_t len)
{
    uint8_t frame[2];
    int scmd_int=0;
    
    while(*cmd == ' ') cmd++;

    if (sscanf_P(cmd, PSTR("%i"), &scmd_int) != 1
        || scmd_int > 287 || scmd_int < 256)
        return ECMD_ERR_PARSE_ERROR;
    
    // special commands have numbers 256-287, but that is just naming
        
    // fit into 8 bit
    uint8_t scmd=(scmd_int-0x100);

    // shift, lsb is always set, msb too
    scmd <<= 1;
    scmd++;
    
    // bits can't be used regularly to allow encoding all messages
    if (scmd & 0x20)
    {
        scmd &= 0x1F;
        scmd |= 0xC0;
    }
    else
        scmd |= 0xA0;

    frame[0]=scmd;

    // read pointer to beginning of next arg
    while(*cmd && *cmd != ' ') cmd++;
    while(*cmd == ' ') cmd++;

    if (sscanf_P(cmd, PSTR("%hhu"), frame+1) != 1)
        return ECMD_ERR_PARSE_ERROR;

    dali_send((uint16_t*)frame);

    repeat_dali_cmd(cmd,(uint16_t*)frame);

#ifdef DALI_RECEIVE_SUPPORT
    return read_dali_reply(cmd, output);
#endif
    
    return ECMD_FINAL_OK;
}

/*
  -- Ethersex META --
  block([[DALI]])
  ecmd_ifdef(DALI_SUPPORT)
    ecmd_ifdef(DALI_RAW_SUPPORT)
        ecmd_feature(dali_raw, "dali raw", [BYTE1] [BYTE2], "send a raw frame (two bytes, given in hex) over the DALI bus")
    ecmd_endif()
    ecmd_feature(dali_dim, "dali dim", [TARGET] [LEVEL], "dim targets (all, g00 to g15, s00 to s63) to given level (0-254)")
    ecmd_feature(dali_cmd, "dali cmd", [TARGET] [COMMAND] [!][?], "send the given command (decimal) to targets (all, g00 to g15, s00 to s63), auto repeat with !, read reply with ?")
    ecmd_feature(dali_scmd, "dali scmd", [SPECIAL COMMAND] [DATA] [!][?], "send special command (256-287) with data, auto repeat with !, read reply with ?")
  ecmd_endif()
*/
