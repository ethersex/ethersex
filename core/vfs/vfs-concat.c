/*
 * Copyright (c) 2008 by Stefan Siegl <stesie@brokenpipe.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#include <stdint.h>
typedef uint32_t vfs_size_t;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h> // PATH_MAX

#include "vfs_inline.h"

#define MAX_IMAGE_SIZE 131072

static void
usage(int exitval)
{
  fprintf(exitval ? stderr : stdout,
          "Usage: vfs-concat IMAGE BLOCKSZ FILE\n"
          "Concatenate FILE to existing ethersex IMAGE.\n\n");
  exit(exitval);
}

static uint8_t
crc_update(uint8_t crc, uint8_t data)
{
  uint8_t i;
  for (i = 0; i < 8; i++)
  {
    if ((crc ^ data) & 1)
      crc = (crc >> 1) ^ 0x8c;
    else
      crc = crc >> 1;

    data = data >> 1;
  }

  return crc;
}


static uint8_t
crc_calc(uint8_t * data, int len)
{
  uint8_t crc = 0;
  int i;

  for (i = 0; i < len; i++)
    crc = crc_update(crc, data[i]);

  return crc;
}


int
main(int argc, char **argv)
{
  uint8_t buf_image[MAX_IMAGE_SIZE], buf_file[MAX_IMAGE_SIZE];
  int image_len, file_len, pagesz;
  FILE *f;
  union vfs_inline_node_t node = {.s = {.fn = "",.len = 0} };
  char *ptr;
  char filename_gz[PATH_MAX];

  if (argc == 2 && strcmp(argv[1], "--help") == 0)
    usage(0);
  if (argc != 4)
    usage(1);

  pagesz = atoi(argv[2]);
  if (pagesz == 0 || pagesz % 2 || pagesz < 64 || pagesz > 256)
  {
    fprintf(stderr, "vfs-concat: Invalid page size: %d.\n", pagesz);
    return 1;
  }

  if ((f = fopen(argv[1], "rb")) == NULL)
  {
    fprintf(stderr, "vfs-concat: Unable to read %s.\n", argv[1]);
    return 1;
  }

  image_len = fread(buf_image, 1, MAX_IMAGE_SIZE, f);
  fclose(f);

  strcpy(filename_gz, argv[3]);
  strcat(filename_gz, ".gz");

  fprintf(stderr, "%s\n", filename_gz);

  if ((f = fopen(filename_gz, "rb")) == NULL)
  {
    if ((f = fopen(argv[3], "rb")) == NULL)
    {
      fprintf(stderr, "vfs-concat: Unable to read %s.\n", argv[3]);
      return 1;
    }
  }

  file_len = fread(buf_file, 1, MAX_IMAGE_SIZE, f);
  fclose(f);

  fprintf(stderr, "vfs-concat: Lengths: image=%d, file=%d\n",
          image_len, file_len);

  fwrite(buf_image, 1, image_len, stdout);

  while (image_len % pagesz)
  {
    putchar(0xFF);
    image_len++;
  }

  while ((ptr = strchr(argv[3], '/')))
    argv[3] = ptr + 1;

  if (strlen(argv[3]) > VFS_INLINE_FNLEN)
  {
    fprintf(stderr, "vfs-concat: Filename %s is too long.\n", argv[3]);
    return 1;
  }


  putchar(VFS_INLINE_MAGIC);

  strncpy(node.s.fn, argv[3], VFS_INLINE_FNLEN);
  node.s.len = file_len;
  node.s.crc = crc_calc(node.raw, sizeof(node) - 1);

  fwrite(&node, sizeof(node), 1, stdout);
  fwrite(buf_file, 1, file_len, stdout);

  return 0;
}
