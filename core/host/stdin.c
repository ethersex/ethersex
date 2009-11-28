/* 
 * Copyright(C) 2009 Stefan Siegl <stesie@brokenpipe.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>

#include "protocols/ecmd/parser.h"
#include "protocols/ecmd/ecmd-base.h"
#include "core/host/stdin.h"

void
terminal_canonical_off(void)
{
  struct termios ttystate;

  tcgetattr(STDIN_FILENO, &ttystate);

  /* turn off canonical mode */
  ttystate.c_lflag &= ~ICANON;
  /* minimum of number input read. */
  ttystate.c_cc[VMIN] = 1;

  tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
}

void
terminal_canonical_on(void)
{
  struct termios ttystate;

  tcgetattr(STDIN_FILENO, &ttystate);

  /* turn on canonical mode */
  ttystate.c_lflag |= ICANON;

  tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
}


void
stdin_read(void)
{
#define BUFLEN 60
  static char buf[BUFLEN];
  static int ptr;

  char ch = fgetc(stdin);

  if (ch != 10)
    {
      buf[ptr ++] = ch;
      return;
    }

  buf[ptr] = 0;
  int l;

  char output[BUFLEN];
  do {
    l = ecmd_parse_command(buf, output, BUFLEN);
    if (is_ECMD_FINAL(l) || is_ECMD_AGAIN(l)) {
      output[is_ECMD_AGAIN(l) ? ECMD_AGAIN(l) : l] = 0;
      printf_P(PSTR("%s\n"), output);
    }
  } while (is_ECMD_AGAIN(l));

  ptr = 0;
}



/*
  -- Ethersex META --
  header(core/host/stdin.h)
  init(terminal_canonical_off)
  exit(terminal_canonical_on)
*/
