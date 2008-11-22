
/*
 * Copyright (c) 2006-2008 by Roland Riegel <feedback@roland-riegel.de>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <string.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include "fat.h"
#include "fat_config.h"
#include "partition.h"
#include "sd_raw.h"
#include "sd_raw_config.h"
#include "uart.h"

#define DEBUG 1

/**
 * \mainpage MMC/SD card example application
 *
 * This project is a small test application which implements read and write
 * support for MMC, SD and SDHC cards.
 *
 * It includes
 * - low-level \link sd_raw MMC, SD and SDHC read/write routines \endlink
 * - \link partition partition table support \endlink
 * - a simple \link fat FAT16/FAT32 read/write implementation \endlink
 *
 * \section circuit The circuit
 * The curcuit board is a self-made and self-soldered board consisting of a single
 * copper layer and standard DIL components, except of the MMC/SD card connector.
 *
 * The connector is soldered to the bottom side of the board. It has a simple
 * eject button which, when a card is inserted, needs some space beyond the connector
 * itself. As an additional feature the connector has two electrical switches
 * to detect wether a card is inserted and wether this card is write-protected.
 * 
 * I used two microcontrollers during development, the Atmel ATmega8 with 8kBytes
 * of flash, and its pin-compatible alternative, the ATmega168 with 16kBytes flash.
 * The first one is the one I started with, but when I implemented FAT16 write
 * support, I ran out of flash space and switched to the ATmega168. For FAT32, an
 * ATmega328 is required.
 * 
 * \section pictures Pictures
 * \image html pic01.jpg "The circuit board used to implement and test this application."
 * \image html pic02.jpg "The MMC/SD card connector on the soldering side of the circuit board."
 *
 * \section software The software
 * The software is written in pure standard ANSI-C. Sure, it might not be the
 * smallest or the fastest one, but I think it is quite flexible.
 *
 * I implemented a simple command prompt which is accessible via the UART at 9600 Baud. With
 * commands similiar to the Unix shell you can browse different directories, read and write
 * files, create new ones and delete them again. Not all commands are available in all
 * software configurations.
 * - <tt>cat \<file\></tt>\n
 *   Writes a hexdump of \<file\> to the terminal.
 * - <tt>cd \<directory\></tt>\n
 *   Changes current working directory to \<directory\>.
 * - <tt>disk</tt>\n
 *   Shows card manufacturer, status, filesystem capacity and free storage space.
 * - <tt>init</tt>\n
 *   Reinitializes and reopens the memory card.
 * - <tt>ls</tt>\n
 *   Shows the content of the current directory.
 * - <tt>mkdir \<directory\></tt>\n
 *   Creates a directory called \<directory\>.
 * - <tt>rm \<file\></tt>\n
 *   Deletes \<file\>.
 * - <tt>sync</tt>\n
 *   Ensures all buffered data is written to the card.
 * - <tt>touch \<file\></tt>\n
 *   Creates \<file\>.
 * - <tt>write \<file\> \<offset\></tt>\n
 *   Writes text to \<file\>, starting from \<offset\>. The text is read
 *   from the UART, line by line. Finish with an empty line.
 *
 * \htmlonly
 * <p>
 * The following table shows some typical code sizes in bytes, using the 20061101 release with malloc()/free():
 * </p>
 *
 * <table border="1" cellpadding="2">
 *     <tr>
 *         <th>layer</th>
 *         <th>code size</th>
 *         <th>static RAM usage</th>
 *     </tr>
 *     <tr>
 *         <td>MMC/SD (read-only)</td>
 *         <td align="right">1576</td>
 *         <td align="right">0</td>
 *     </tr>
 *     <tr>
 *         <td>MMC/SD (read-write)</td>
 *         <td align="right">2202</td>
 *         <td align="right">517</td>
 *     </tr>
 *     <tr>
 *         <td>Partition</td>
 *         <td align="right">418</td>
 *         <td align="right">0</td>
 *     </tr>
 *     <tr>
 *         <td>FAT16 (read-only)</td>
 *         <td align="right">3834</td>
 *         <td align="right">0</td>
 *     </tr>
 *     <tr>
 *         <td>FAT16 (read-write)</td>
 *         <td align="right">7932</td>
 *         <td align="right">0</td>
 *     </tr>
 * </table>
 *
 * <p>
 * The static RAM in the read-write case is used for buffering memory card
 * access. Without this buffer, implementation would have been much more complicated.
 * </p>
 * 
 * <p>
 * Please note that the numbers above do not include the C library functions
 * used, e.g. malloc()/free() and some string functions. These will raise the
 * numbers somewhat if they are not already used in other program parts.
 * </p>
 * 
 * <p>
 * When opening a partition, filesystem, file or directory, a little amount
 * of dynamic RAM is used, as listed in the following table. Alternatively,
 * the same amount of static RAM can be used.
 * </p>
 *
 * <table border="1" cellpadding="2">
 *     <tr>
 *         <th>descriptor</th>
 *         <th>dynamic/static RAM</th>
 *     </tr>
 *     <tr>
 *         <td>partition</td>
 *         <td align="right">17</td>
 *     </tr>
 *     <tr>
 *         <td>filesystem</td>
 *         <td align="right">26</td>
 *     </tr>
 *     <tr>
 *         <td>file</td>
 *         <td align="right">51</td>
 *     </tr>
 *     <tr>
 *         <td>directory</td>
 *         <td align="right">47</td>
 *     </tr>
 * </table>
 * 
 * \endhtmlonly
 *
 * \section adaptation Adapting the software to your needs
 * The only hardware dependent part is the communication
 * layer talking to the memory card. The other parts like partition table and FAT
 * support are completely independent, you could use them even for managing
 * Compact Flash cards or standard ATAPI hard disks.
 *
 * By changing the MCU* variables in the Makefile, you can use other Atmel
 * microcontrollers or different clock speeds. You might also want to change
 * the configuration defines in the files fat_config.h, partition_config.h,
 * sd_raw_config.h and sd-reader_config.h. For example, you could disable
 * write support completely if you only need read support.
 * 
 * \section bugs Bugs or comments?
 * If you have comments or found a bug in the software - there might be some
 * of them - you may contact me per mail at feedback@roland-riegel.de.
 *
 * \section acknowledgements Acknowledgements
 * Thanks go to Ulrich Radig, who explained on his homepage how to interface
 * MMC cards to the Atmel microcontroller (http://www.ulrichradig.de/).
 * I adapted his work for my circuit. Although this is a very simple
 * solution, I had no problems using it.
 * 
 * \section copyright Copyright 2006-2008 by Roland Riegel
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation (http://www.gnu.org/copyleft/gpl.html).
 * At your option, you can alternatively redistribute and/or modify the following
 * files under the terms of the GNU Lesser General Public License version 2.1
 * as published by the Free Software Foundation (http://www.gnu.org/copyleft/lgpl.html):
 * - byteordering.c
 * - byteordering.h
 * - fat.c
 * - fat.h
 * - fat_config.h
 * - partition.c
 * - partition.h
 * - partition_config.h
 * - sd_raw.c
 * - sd_raw.h
 * - sd_raw_config.h
 * - sd-reader_config.h
 */

static uint8_t read_line(char* buffer, uint8_t buffer_length);
static uint32_t strtolong(const char* str);
static uint8_t find_file_in_dir(struct fat_fs_struct* fs, struct fat_dir_struct* dd, const char* name, struct fat_dir_entry_struct* dir_entry);
static struct fat_file_struct* open_file_in_dir(struct fat_fs_struct* fs, struct fat_dir_struct* dd, const char* name); 
static uint8_t print_disk_info(const struct fat_fs_struct* fs);

int main()
{
    /* we will just use ordinary idle mode */
    set_sleep_mode(SLEEP_MODE_IDLE);

    /* setup uart */
    uart_init();

    while(1)
    {
        /* setup sd card slot */
        if(!sd_raw_init())
        {
#if DEBUG
            uart_puts_p(PSTR("MMC/SD initialization failed\n"));
#endif
            continue;
        }

        /* open first partition */
        struct partition_struct* partition = partition_open(sd_raw_read,
                                                            sd_raw_read_interval,
                                                            sd_raw_write,
                                                            sd_raw_write_interval,
                                                            0
                                                           );

        if(!partition)
        {
            /* If the partition did not open, assume the storage device
             * is a "superfloppy", i.e. has no MBR.
             */
            partition = partition_open(sd_raw_read,
                                       sd_raw_read_interval,
                                       sd_raw_write,
                                       sd_raw_write_interval,
                                       -1
                                      );
            if(!partition)
            {
#if DEBUG
                uart_puts_p(PSTR("opening partition failed\n"));
#endif
                continue;
            }
        }

        /* open file system */
        struct fat_fs_struct* fs = fat_open(partition);
        if(!fs)
        {
#if DEBUG
            uart_puts_p(PSTR("opening filesystem failed\n"));
#endif
            continue;
        }

        /* open root directory */
        struct fat_dir_entry_struct directory;
        fat_get_dir_entry_of_path(fs, "/", &directory);

        struct fat_dir_struct* dd = fat_open_dir(fs, &directory);
        if(!dd)
        {
#if DEBUG
            uart_puts_p(PSTR("opening root directory failed\n"));
#endif
            continue;
        }
        
        /* print some card information as a boot message */
        print_disk_info(fs);

        /* provide a simple shell */
        char buffer[24];
        while(1)
        {
            /* print prompt */
            uart_putc('>');
            uart_putc(' ');

            /* read command */
            char* command = buffer;
            if(read_line(command, sizeof(buffer)) < 1)
                continue;

            /* execute command */
            if(strcmp_P(command, PSTR("init")) == 0)
            {
                break;
            }
            else if(strncmp_P(command, PSTR("cd "), 3) == 0)
            {
                command += 3;
                if(command[0] == '\0')
                    continue;

                /* change directory */
                struct fat_dir_entry_struct subdir_entry;
                if(find_file_in_dir(fs, dd, command, &subdir_entry))
                {
                    struct fat_dir_struct* dd_new = fat_open_dir(fs, &subdir_entry);
                    if(dd_new)
                    {
                        fat_close_dir(dd);
                        dd = dd_new;
                        continue;
                    }
                }

                uart_puts_p(PSTR("directory not found: "));
                uart_puts(command);
                uart_putc('\n');
            }
            else if(strcmp_P(command, PSTR("ls")) == 0)
            {
                /* print directory listing */
                struct fat_dir_entry_struct dir_entry;
                while(fat_read_dir(dd, &dir_entry))
                {
                    uint8_t spaces = sizeof(dir_entry.long_name) - strlen(dir_entry.long_name) + 4;

                    uart_puts(dir_entry.long_name);
                    uart_putc(dir_entry.attributes & FAT_ATTRIB_DIR ? '/' : ' ');
                    while(spaces--)
                        uart_putc(' ');
                    uart_putdw_dec(dir_entry.file_size);
                    uart_putc('\n');
                }
            }
            else if(strncmp_P(command, PSTR("cat "), 4) == 0)
            {
                command += 4;
                if(command[0] == '\0')
                    continue;
                
                /* search file in current directory and open it */
                struct fat_file_struct* fd = open_file_in_dir(fs, dd, command);
                if(!fd)
                {
                    uart_puts_p(PSTR("error opening "));
                    uart_puts(command);
                    uart_putc('\n');
                    continue;
                }

                /* print file contents */
                uint8_t buffer[8];
                uint32_t offset = 0;
                while(fat_read_file(fd, buffer, sizeof(buffer)) > 0)
                {
                    uart_putdw_hex(offset);
                    uart_putc(':');
                    for(uint8_t i = 0; i < 8; ++i)
                    {
                        uart_putc(' ');
                        uart_putc_hex(buffer[i]);
                    }
                    uart_putc('\n');
                    offset += 8;
                }

                fat_close_file(fd);
            }
            else if(strcmp_P(command, PSTR("disk")) == 0)
            {
                if(!print_disk_info(fs))
                    uart_puts_p(PSTR("error reading disk info\n"));
            }
#if FAT_WRITE_SUPPORT
            else if(strncmp_P(command, PSTR("rm "), 3) == 0)
            {
                command += 3;
                if(command[0] == '\0')
                    continue;
                
                struct fat_dir_entry_struct file_entry;
                if(find_file_in_dir(fs, dd, command, &file_entry))
                {
                    if(fat_delete_file(fs, &file_entry))
                        continue;
                }

                uart_puts_p(PSTR("error deleting file: "));
                uart_puts(command);
                uart_putc('\n');
            }
            else if(strncmp_P(command, PSTR("touch "), 6) == 0)
            {
                command += 6;
                if(command[0] == '\0')
                    continue;

                struct fat_dir_entry_struct file_entry;
                if(!fat_create_file(dd, command, &file_entry))
                {
                    uart_puts_p(PSTR("error creating file: "));
                    uart_puts(command);
                    uart_putc('\n');
                }
            }
            else if(strncmp_P(command, PSTR("write "), 6) == 0)
            {
                command += 6;
                if(command[0] == '\0')
                    continue;

                char* offset_value = command;
                while(*offset_value != ' ' && *offset_value != '\0')
                    ++offset_value;

                if(*offset_value == ' ')
                    *offset_value++ = '\0';
                else
                    continue;

                /* search file in current directory and open it */
                struct fat_file_struct* fd = open_file_in_dir(fs, dd, command);
                if(!fd)
                {
                    uart_puts_p(PSTR("error opening "));
                    uart_puts(command);
                    uart_putc('\n');
                    continue;
                }

                int32_t offset = strtolong(offset_value);
                if(!fat_seek_file(fd, &offset, FAT_SEEK_SET))
                {
                    uart_puts_p(PSTR("error seeking on "));
                    uart_puts(command);
                    uart_putc('\n');

                    fat_close_file(fd);
                    continue;
                }

                /* read text from the shell and write it to the file */
                uint8_t data_len;
                while(1)
                {
                    /* give a different prompt */
                    uart_putc('<');
                    uart_putc(' ');

                    /* read one line of text */
                    data_len = read_line(buffer, sizeof(buffer));
                    if(!data_len)
                        break;

                    /* write text to file */
                    if(fat_write_file(fd, (uint8_t*) buffer, data_len) != data_len)
                    {
                        uart_puts_p(PSTR("error writing to file\n"));
                        break;
                    }
                }

                fat_close_file(fd);
            }
            else if(strncmp_P(command, PSTR("mkdir "), 6) == 0)
            {
                command += 6;
                if(command[0] == '\0')
                    continue;

                struct fat_dir_entry_struct dir_entry;
                if(!fat_create_dir(dd, command, &dir_entry))
                {
                    uart_puts_p(PSTR("error creating directory: "));
                    uart_puts(command);
                    uart_putc('\n');
                }
            }
#endif
#if SD_RAW_WRITE_BUFFERING
            else if(strcmp_P(command, PSTR("sync")) == 0)
            {
                if(!sd_raw_sync())
                    uart_puts_p(PSTR("error syncing disk\n"));
            }
#endif
            else
            {
                uart_puts_p(PSTR("unknown command: "));
                uart_puts(command);
                uart_putc('\n');
            }
        }

        /* close file system */
        fat_close(fs);

        /* close partition */
        partition_close(partition);
    }
    
    return 0;
}

uint8_t read_line(char* buffer, uint8_t buffer_length)
{
    memset(buffer, 0, buffer_length);

    uint8_t read_length = 0;
    while(read_length < buffer_length - 1)
    {
        uint8_t c = uart_getc();

        if(c == 0x08 || c == 0x7f)
        {
            if(read_length < 1)
                continue;

            --read_length;
            buffer[read_length] = '\0';

            uart_putc(0x08);
            uart_putc(' ');
            uart_putc(0x08);

            continue;
        }

        uart_putc(c);

        if(c == '\n')
        {
            buffer[read_length] = '\0';
            break;
        }
        else
        {
            buffer[read_length] = c;
            ++read_length;
        }
    }

    return read_length;
}

uint32_t strtolong(const char* str)
{
    uint32_t l = 0;
    while(*str >= '0' && *str <= '9')
        l = l * 10 + (*str++ - '0');

    return l;
}

uint8_t find_file_in_dir(struct fat_fs_struct* fs, struct fat_dir_struct* dd, const char* name, struct fat_dir_entry_struct* dir_entry)
{
    while(fat_read_dir(dd, dir_entry))
    {
        if(strcmp(dir_entry->long_name, name) == 0)
        {
            fat_reset_dir(dd);
            return 1;
        }
    }

    return 0;
}

struct fat_file_struct* open_file_in_dir(struct fat_fs_struct* fs, struct fat_dir_struct* dd, const char* name)
{
    struct fat_dir_entry_struct file_entry;
    if(!find_file_in_dir(fs, dd, name, &file_entry))
        return 0;

    return fat_open_file(fs, &file_entry);
}

uint8_t print_disk_info(const struct fat_fs_struct* fs)
{
    if(!fs)
        return 0;

    struct sd_raw_info disk_info;
    if(!sd_raw_get_info(&disk_info))
        return 0;

    uart_puts_p(PSTR("manuf:  0x")); uart_putc_hex(disk_info.manufacturer); uart_putc('\n');
    uart_puts_p(PSTR("oem:    ")); uart_puts((char*) disk_info.oem); uart_putc('\n');
    uart_puts_p(PSTR("prod:   ")); uart_puts((char*) disk_info.product); uart_putc('\n');
    uart_puts_p(PSTR("rev:    ")); uart_putc_hex(disk_info.revision); uart_putc('\n');
    uart_puts_p(PSTR("serial: 0x")); uart_putdw_hex(disk_info.serial); uart_putc('\n');
    uart_puts_p(PSTR("date:   ")); uart_putw_dec(disk_info.manufacturing_month); uart_putc('/');
                                   uart_putw_dec(disk_info.manufacturing_year); uart_putc('\n');
    uart_puts_p(PSTR("size:   ")); uart_putdw_dec(disk_info.capacity / 1024 / 1024); uart_puts_p(PSTR("MB\n"));
    uart_puts_p(PSTR("copy:   ")); uart_putw_dec(disk_info.flag_copy); uart_putc('\n');
    uart_puts_p(PSTR("wr.pr.: ")); uart_putw_dec(disk_info.flag_write_protect_temp); uart_putc('/');
                                   uart_putw_dec(disk_info.flag_write_protect); uart_putc('\n');
    uart_puts_p(PSTR("format: ")); uart_putw_dec(disk_info.format); uart_putc('\n');
    uart_puts_p(PSTR("free:   ")); uart_putdw_dec(fat_get_fs_free(fs)); uart_putc('/');
                                   uart_putdw_dec(fat_get_fs_size(fs)); uart_putc('\n');

    return 1;
}

#if FAT_DATETIME_SUPPORT
void get_datetime(uint16_t* year, uint8_t* month, uint8_t* day, uint8_t* hour, uint8_t* min, uint8_t* sec)
{
    *year = 2007;
    *month = 1;
    *day = 1;
    *hour = 0;
    *min = 0;
    *sec = 0;
}
#endif


