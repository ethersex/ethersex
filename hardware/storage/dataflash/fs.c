/*
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2008 by Stefan Siegl <stesie@brokenpipe.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (either version 2 or
 * version 3) as published by the Free Software Foundation.
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

#include <avr/io.h>
#include <stdlib.h>
#include <string.h>

#include "fs.h"

#ifdef DEBUG_FS
#include "core/debug.h"
#endif

/* debug */
#ifndef __AVR__

#include <assert.h>
#include <stdio.h>
#include <err.h>

#define _BV(x) (1<<(x))

uint8_t _crc_ibutton_update(uint8_t crc, uint8_t data)
{
    uint8_t i;

    crc = crc ^ data;
    for (i = 0; i < 8; i++)
    {
        if (crc & 0x01)
            crc = (crc >> 1) ^ 0x8C;
        else
            crc >>= 1;
    }

    return crc;
}

#else

#include <util/crc16.h>

#ifdef DEBUG_FS
# define printf  debug_printf
#else
# define printf(...)
#endif

#define assert(x)
#define PACKED
#define errx(...)

#endif /* */

fs_t fs;

/* structs */

/* 15 byte structure information, at the beginning of each page */
typedef struct {
    uint8_t unused:1;
    uint8_t root:1;
    uint8_t eof:1;
    uint8_t reserved:1;
    uint16_t next_inode:12; /* fs_inode_t is bigger, but that does not matter */
    uint16_t size;
} PACKED fs_page_t;

/* the file/directory pointer, 8 byte, used in node table in root node */
typedef struct {
    uint8_t unused:1;
    uint8_t file:1;
    uint8_t reserved:2;
    uint16_t inode:12;
    char name[FS_FILENAME];
} fs_node_t;

/* the root node, contains version */
typedef struct {
    fs_page_t page;
    fs_version_t version;
    df_page_t inodetable[FS_ROOTNODE_INODETABLE_SIZE];
    fs_node_t nodes[];
} PACKED fs_root_t;

/* inode datastructure, used in inode table pages */
typedef struct {
    uint8_t unused:1;
    uint8_t reserved:3;
    uint16_t page:12;
} PACKED fs_inodetable_node_t;

/* an inode table page */
typedef struct {
    fs_inodetable_node_t inodes[FS_INODES_PER_TABLE];
} PACKED fs_inodetable_t;

/* local prototypes */



/* public functions */

/* initialize fs structure, call fs_scan_root */
fs_status_t fs_init(void)
{

    printf("initalizing filesystem...\n");
    fs.chip = NULL;

    /* keep address of last free page, so that we can start searching for the
     * next free page at this address to provide wear-levelling */
    fs.last_free = 0;

    /* init free pages storage:
     * buffer 2 in dataflash is used as the free pages storage, each byte
     * represents 8 pages, if a bit is set, this page is known as free, so
     * initialilly fill buffer 2 with 0xff's */

    uint8_t b = 0xff;

    for (uint16_t i = 0; i < DF_PAGESIZE; i++)
        df_buf_write(fs.chip, DF_BUF2, &b, i, 1);

    /* scan for root node, if none could be founde, create one in page 0 */
    fs_status_t ret = fs_scan(&fs);

    if (ret != FS_OK) {
        printf("fs: error scannning dataflash: %s\r\n", ret);
        return ret;
    }

    /* mark used pages */
    fs_mark_used(&fs, fs.root);

    for (uint8_t i = 0; i < 16; i++) {
        // printf("inodetable %i at page %d\n", i, fs_get_inodetable(fs, i));
        fs_mark_used(&fs, fs_inodetable(&fs, i));
    }

    /* allocate temporary buffer */
    fs_node_t *node = malloc(sizeof(fs_node_t));

    if (node == NULL)
        return FS_MEM;

    printf("fs: nodes in root:\r\n");
    for (uint8_t i = 0; i < FS_NODES_IN_ROOT; i++) {

        df_flash_read(fs.chip, fs.root, node, FS_ROOTNODE_NODETABLE_OFFSET + i * sizeof(fs_node_t), sizeof(fs_node_t));

        if (node->unused == 0) {
#ifdef DEBUG_FS
            char name[7];
            strncpy(name, node->name, FS_FILENAME);
            name[FS_FILENAME] = 0;
#endif

            df_page_t page = fs_page(&fs, node->inode);

	    printf(" * %s: (index %d, file %d, inode 0x%04x, page 0x%04x)\r\n",
	    	   name, i, node->file, node->inode, page);

            while (page != 0xffff) {
                fs_mark_used(&fs, page);

		fs_page_t pagedata;
		df_flash_read (fs.chip, page, &pagedata, FS_STRUCTURE_OFFSET,
			       sizeof (fs_page_t));

		if (pagedata.eof)
		    break;

		page = fs_page (&fs, pagedata.next_inode);
		printf ("\t... continues in page 0x%04x (inode 0x%04x)\n",
			page, pagedata.next_inode);
	    }
        }

    }

    /* free temporarily buffer */
    free(node);

#ifdef DEBUG_FS_MARK
    printf("fs: used pages:\r\n");
    for (uint16_t i = 0; i < DF_PAGES; i++) {
        if (fs_used(&fs, i)) {
	    printf(" * %04x\r\n", i);
        }
    }
#endif

    printf("fs: root page is 0x%04x\n", fs.root);

    return FS_OK;

}

/* list files in directory, write filename to buffer, return FS_OK or FS_EOF if
 * no more, increment index to get next files */
fs_status_t fs_list(fs_t *fs, char *dir, char *buf, fs_index_t index)
{

    /* alloc temporary buffer */
    fs_node_t *node = malloc(sizeof(fs_node_t)+1);

    if (node == NULL)
        return FS_MEM;

    /* read node */
    df_flash_read(fs->chip,
                  fs->root,
                  node,
                  FS_ROOTNODE_NODETABLE_OFFSET+index * sizeof(fs_node_t),
                  sizeof(fs_node_t));

    /* copy name, terminate string */
    strncpy(buf, node->name, FS_FILENAME);
    buf[FS_FILENAME] = '\0';

    // printf("index %d, node->unused: %d\n", index, node->unused);

    /* if this is the last node, return EOF */
    if (node->unused) {
        free(node);
        return FS_EOF;
    } else {
        free(node);
        return FS_OK;
    }

}

fs_inode_t fs_get_inode(fs_t *fs, const char *file)
{

    fs_inode_t inode = 0xffff;
    fs_index_t index = 0;

    /* alloc temporary buffer */
    fs_node_t *node = malloc(sizeof(fs_node_t)+1);

    do {
        /* read node */
        df_flash_read(fs->chip,
                      fs->root,
                      node,
                      FS_ROOTNODE_NODETABLE_OFFSET+(index++) * sizeof(fs_node_t),
                      sizeof(fs_node_t));

        if (strncmp(node->name, file, FS_FILENAME) == 0) {
            inode = node->inode;
            break;
        }

    } while (!node->unused);

    free(node);
    return inode;

}

fs_size_t fs_read(fs_t *fs, fs_inode_t inode, void *buf, fs_size_t offset, fs_size_t length)
{

    uint8_t *b = (uint8_t *)buf;

    assert(length > 0);

    fs_size_t read = 0;

    /* load first page address */
    df_page_t pagenum = fs_page(fs, inode);

    printf("reading inode %d (starting at page %d): %d bytes starting at %d\n", inode, pagenum, length, offset);

    if (pagenum == 0xffff)
        return 0;

    fs_page_t page;

    while (offset >= FS_DATASIZE) {

        printf("\toffset > datasize\n");

        /* load page data */
        df_flash_read(fs->chip, pagenum, &page, FS_STRUCTURE_OFFSET, sizeof(fs_page_t));

        /* if this is already the end, there are no bytes to read... */
        if (page.eof) {
            printf("already at eof...\n");
            return 0;
        }

        /* extract next address */
        pagenum = fs_page(fs, page.next_inode);

        if (pagenum == 0xffff)
            return -1; /* bad page */

        printf("\tnext page is at %d\n", pagenum);
        offset -= FS_DATASIZE;

    }

    printf("remaining offset is %d\n", offset);

    /* load page data */
    df_flash_read(fs->chip, pagenum, &page, FS_STRUCTURE_OFFSET, sizeof(fs_page_t));

    printf("now we are at page %d (which is filled with %d bytes), %d bytes to read\n", pagenum, page.size, length);

    if (offset >= page.size) {
        printf("offset >= page.size\n");
        return 0;
    }

    while (1) {

        printf("\treading..., pagenum is %d\n", pagenum);

        /* load page */
        df_flash_read(fs->chip, pagenum, &page, FS_STRUCTURE_OFFSET, sizeof(fs_page_t));

        /* if this is the last page to read, return */
        if (length+offset <= page.size) {

            printf("\tlast page (but not eof), length %d, offset %d\n", length, offset);
            df_flash_read(fs->chip, pagenum, b, FS_DATA_OFFSET+offset, length+offset);
            read += length;
            return read;

        /* if this is eof, read up to eof and return */
        } else if (length+offset > page.size && page.eof) {
            length = page.size-offset;
            printf("\treached eof, reading %d bytes at offset %d\n", length, offset);
            df_flash_read(fs->chip, pagenum, b, FS_DATA_OFFSET+offset, length);
            read += length;
            return read;

        /* normal case, length+offset > page.size */
        } else {

            printf("\tnormal case, reading %d bytes, offset %d\n", FS_DATASIZE, offset);
            df_flash_read(fs->chip, pagenum, b, FS_DATA_OFFSET+offset, FS_DATASIZE-offset);
        }

        /* else if we have to touch another page */
        pagenum = fs_page(fs, page.next_inode);

        if (pagenum == 0xffff)
            return -1;

        df_size_t read_bytes = page.size-offset;

        read += read_bytes;
        length -= read_bytes;
        b += read_bytes;
        offset = 0;

    }

}

fs_status_t fs_write(fs_t *fs, fs_inode_t inode, void *buf, fs_size_t offset, fs_size_t length)
{

    printf("fs_write, inode %d, offset %ld, length %ld\n",
	   inode, offset, length);

    df_page_t pagenum, old_pagenum;
    fs_page_t page;

    /* check if this file contained data before */
    if ( (pagenum = fs_page(fs, inode)) != 0xffff) {

        /* while the current position is below the requested offset, seek */
        while (offset > FS_DATASIZE) {

            printf("\toffset > FS_DATASIZE, searching for next page\n");

            /* find next page */
            df_flash_read(fs->chip, pagenum, &page, FS_STRUCTURE_OFFSET, sizeof(fs_page_t));

            if (page.eof) {
                printf(" ************************* BAD SEEK *******************************\n");
                return FS_BADSEEK;
            }

            inode = page.next_inode;
            pagenum = fs_page(fs, inode);
            printf("\tnext inode is %d, next page is %d\n", page.next_inode, pagenum);

            offset -= FS_DATASIZE;
        }

        old_pagenum = pagenum;
        uint8_t eof = 0;

        do {
            printf("\tlength > 0 (%ld), old page is %d\n",
		   length, old_pagenum);

            /* allocate new page */
            if ( (pagenum = fs_new_page(fs)) == 0xffff)
                return FS_BADPAGE;

            printf("\tnew allocated page is %d\n", pagenum);

            /* update inode */
            fs_status_t ret = fs_update_inodetable(fs, inode, pagenum);

            if (ret != FS_OK)
                return ret;

            if (length+offset > FS_DATASIZE) {

                printf("\t\tlength+offset > FS_DATASIZE\n");

                /* load old page into buffer */
                df_buf_load(fs->chip, DF_BUF1, old_pagenum);
                df_wait(fs->chip);

                /* load structure */
                df_buf_read(fs->chip, DF_BUF1, &page, FS_STRUCTURE_OFFSET, sizeof(fs_page_t));

                /* remember if original file ended in this page */
                if (page.eof) {
                    printf("\t\toriginal file ended here\r\n");
                    eof = 1;
                }

                if (!page.eof)
                    /* save old pagenum */
                    old_pagenum = fs_page(fs, page.next_inode);

                page.unused = 0;
                page.eof = 0;
                page.root = 0;
                page.size = FS_DATASIZE;
                page.next_inode = fs_new_inode(fs);
                inode = page.next_inode;

                if (page.next_inode == 0xffff)
                    return FS_BADINODE;

                printf("\t\tnext inode is %d\n", page.next_inode);

                /* save structure */
                df_buf_write(fs->chip, DF_BUF1, &page, FS_STRUCTURE_OFFSET, sizeof(fs_page_t));

                printf("fs: writing1 into page 0x%04x\r\n", pagenum);

		if (FS_DATASIZE - offset) {
		    /* write data */
		    df_buf_write(fs->chip, DF_BUF1, buf, FS_DATA_OFFSET+offset,
				 FS_DATASIZE-offset);
		    length -= (FS_DATASIZE-offset);
		}

		df_buf_save(fs->chip, DF_BUF1, pagenum);
		df_wait(fs->chip);

            } else {

                printf("\t\tlength+offset <= FS_DATASIZE, length 0x%04lx, "
		       "offset 0x%04lx, eof %d, old page 0x%04x\n",
		       length, offset, eof, old_pagenum);

                /* load old page into buffer */
                df_buf_load(fs->chip, DF_BUF1, old_pagenum);
                df_wait(fs->chip);

                /* load structure */
                df_buf_read(fs->chip, DF_BUF1, &page, FS_STRUCTURE_OFFSET, sizeof(fs_page_t));
                page.unused = 0;
		if (eof)
		    page.eof = 1;
                page.root = 0;

		printf(", page.size: 0x%04x\r\n", page.size);

                if (eof || length+offset > page.size) {
                    page.size = length+offset;
                    printf("\t\tpage size must be updated to 0x%04x\r\n",
		           page.size);
                }

                printf("\t\tnew page length is %d\n", page.size);

                /* save structure */
                df_buf_write(fs->chip, DF_BUF1, &page, FS_STRUCTURE_OFFSET, sizeof(fs_page_t));

                printf("fs: writing2 into page 0x%04x\r\n", pagenum);

                /* write data */
                df_buf_write(fs->chip, DF_BUF1, buf, FS_DATA_OFFSET+offset, length);
                df_buf_save(fs->chip, DF_BUF1, pagenum);
                df_wait(fs->chip);

                length = 0;

            }

            offset = 0;
        } while (length > 0);

        printf("done writing\n");

        return FS_OK;

    } else {

        /* check for valid offset */
        if (offset != 0)
            return FS_BADSEEK;

        page.unused = 0;
        page.root = 0;

        printf("creating new file content, inode %d\n", inode);

        do {

            /* allocate new page */
            pagenum = fs_new_page(fs);
            if (pagenum == 0xffff)
                return FS_BADPAGE;

            printf("\tusing page %d\n", pagenum);

            /* update inode */
            fs_status_t ret = fs_update_inodetable(fs, inode, pagenum);

            if (ret != FS_OK)
                return ret;

            /* write data to buffer */
            if (length > FS_DATASIZE) {

                /* allocate next inode */
                page.eof = 0;
                page.root = 0;
                page.next_inode = fs_new_inode(fs);
                inode = page.next_inode;

                printf("\t next_inode %d\n", page.next_inode);

                if (page.next_inode == 0xffff)
                    return FS_BADINODE;

                page.size = FS_DATASIZE;

                /* write structure */
                df_buf_write(fs->chip, DF_BUF1, &page, FS_STRUCTURE_OFFSET, sizeof(fs_page_t));

                /* write data */
                df_buf_write(fs->chip, DF_BUF1, buf, FS_DATA_OFFSET, FS_DATASIZE);
                length -= FS_DATASIZE;

            } else {

                /* mark last page */
                page.eof = 1;
                page.size = length;
                page.root = 0;

                /* write structure */
                df_buf_write(fs->chip, DF_BUF1, &page, FS_STRUCTURE_OFFSET, sizeof(fs_page_t));

                /* write data */
                df_buf_write(fs->chip, DF_BUF1, buf, FS_DATA_OFFSET, length);
                length = 0;

            }

            /* program page */
            df_buf_save(fs->chip, DF_BUF1, pagenum);
            df_wait(fs->chip);

        } while (length > 0);

    }

    return FS_OK;

}



fs_status_t noinline fs_truncate(fs_t *fs, fs_inode_t inode, fs_size_t length)
{
    printf("fs_truncate, inode %d, length %ld\n", inode, length);

    df_page_t pagenum;
    fs_page_t page;

    pagenum = fs_page (fs, inode);
    if (pagenum == 0xffff)
	return FS_OK;		/* File hasn't contained data before. */

    /* Seek forward until we've reached the page that contains the offset
       we ought to truncate the file at. */
    while (length > FS_DATASIZE) {
	printf ("\tlength > FS_DATASIZE, trying next page.\n");

	/* find next page */
	df_flash_read (fs->chip, pagenum, &page, FS_STRUCTURE_OFFSET,
		       sizeof(fs_page_t));

	if (page.eof) {
	    printf (" *** BAD SEEK ***  (truncate length beyond file length)\n");
	    return FS_BADSEEK;
	}

	inode = page.next_inode;
	pagenum = fs_page (fs, inode);

	printf ("\tnext inode is %d, next page is %d\n", inode, pagenum);

	if (pagenum == 0xffff) {
	    printf ("\t\tEEEK! Bad page, out here.\n");
	    return -1;
	}

	length -= FS_DATASIZE;
    }

    printf ("remaining length is %ld\n", length);

    /* Load the page (structural) data. */
    df_flash_read (fs->chip, pagenum, &page, FS_STRUCTURE_OFFSET,
		   sizeof (fs_page_t));
    printf ("now we are at page %d (filled with %d bytes), truncate it at %d.\n",
	    pagenum, page.size, length);

    if (length > page.size) {
	printf ("length > page.size -> *** BADSEEK ***\n");
	return FS_BADSEEK;
    }

    if (length == page.size) {
	printf ("length == page.size -> nothing to do.\n");
	return FS_OK;
    }

    df_page_t new_pagenum;
    uint8_t found_eof;
    do {
	if ((new_pagenum = fs_new_page (fs)) == 0xffff)
	    return FS_BADPAGE;

	printf ("\tnew allocated page is %d\n", new_pagenum);

	/* update inode */
	fs_status_t ret = fs_update_inodetable (fs, inode, new_pagenum);

	if (ret != FS_OK)
	    return ret;

	/* Load the old page into buffer. */
	df_buf_load (fs->chip, DF_BUF1, pagenum);
	df_wait (fs->chip);

	/* Load the structure. */
	df_flash_read (fs->chip, pagenum, &page, FS_STRUCTURE_OFFSET,
		       sizeof (fs_page_t));

	if ((found_eof = page.eof)) {
	    printf ("\tThe file ended here.\n");
	}

	page.size = length;
	page.eof = 1;

	length = 0;

	/* Save the updated structure data to buffer and save the page. */
	df_buf_write (fs->chip, DF_BUF1, &page, FS_STRUCTURE_OFFSET,
		      sizeof (fs_page_t));
	df_buf_save (fs->chip, DF_BUF1, new_pagenum);
	df_wait (fs->chip);

    } while (0); /* (found_eof == 0); FIXME */


    return FS_OK;
}

fs_status_t fs_create(fs_t *fs, const char *name)
{

    /* search for a place for this filename in the table */
    fs_index_t index = 0, i = 0, max = 0;
    fs_node_t *node = malloc(sizeof(fs_node_t));

    if (node == NULL)
        return FS_MEM;

    while(1) {

        // printf("i = %d:", i);

        /* read node */
        df_flash_read(fs->chip,
                fs->root,
                node,
                FS_ROOTNODE_NODETABLE_OFFSET+i*sizeof(fs_node_t),
                sizeof(fs_node_t));

        if (node->unused) {
            // printf(", last node\n");
            max = i;
            break;
        }

        if (strncmp(node->name, name, FS_FILENAME) == 0) {
            // printf(" duplicate, EEK!\n");
            free(node);
            return FS_DUPLICATE;
        }

        if (strncmp(node->name, name, FS_FILENAME) < 0) {
            // printf(" strncmp() < 0)");
            index++;
        }

        // printf("...\n");

        i++;

    }

    printf("new file will be placed in index %d, max is %d\n", index, max);

    /* now i points to the node index, construct new node */
    strncpy(node->name, name, FS_FILENAME);
    node->unused = 0;
    node->file = 1;
    node->inode = fs_new_inode(fs);

    if (node->inode == 0xffff) {
        free(node);
        return FS_BADINODE;
    }

    /* load root node into BUF1 */
    df_buf_load(fs->chip, DF_BUF1, fs->root);
    df_wait(fs->chip);

    /* write node to BUF1 */
    df_buf_write(fs->chip,
                 DF_BUF1,
                 node,
                 FS_ROOTNODE_NODETABLE_OFFSET+index*sizeof(fs_node_t),
                 sizeof(fs_node_t));

    /* append nodes after this one */
    while (index < max) {

        printf("appending old node %d\n", index);

        /* read node */
        df_flash_read(fs->chip,
                fs->root,
                node,
                FS_ROOTNODE_NODETABLE_OFFSET+index*sizeof(fs_node_t),
                sizeof(fs_node_t));

        /* write node to BUF1 */
        df_buf_write(fs->chip,
                     DF_BUF1,
                     node,
                     FS_ROOTNODE_NODETABLE_OFFSET+(index+1)*sizeof(fs_node_t),
                     sizeof(fs_node_t));

        index++;

    }

    free(node);

    /* increment version and update checksum */
    return fs_increment(fs);

}

fs_status_t fs_remove(fs_t *fs, const char *name)
{

    /* search for this filename in the nodetable */
    fs_index_t index = 0xffff, i = 0, max = 0;
    fs_inode_t inode = 0xffff;
    fs_node_t *node = malloc(sizeof(fs_node_t));

    if (node == NULL) {
	printf ("fs_remove: not enough memory!");
        return FS_MEM;
    }

    while(1) {

        // printf("i = %d:", i);

        /* read node */
        df_flash_read(fs->chip,
                fs->root,
                node,
                FS_ROOTNODE_NODETABLE_OFFSET+i*sizeof(fs_node_t),
                sizeof(fs_node_t));

        if (node->unused) {
            max = i;
            break;
        }

        if (strncmp(node->name, name, FS_FILENAME) == 0) {
            index = i;
            inode = node->inode;
        }

        i++;

    }

    if (max == 0 || index > max) {
        printf("no such file\n");
        free(node);
        return FS_NOSUCHFILE;
    }

    printf("index = %d, i = %d, max = %d\n", index, i, max);

    /* load root node into BUF1 */
    df_buf_load(fs->chip, DF_BUF1, fs->root);
    df_wait(fs->chip);

    /* append other nodes */
    while (index < max-1) {

        printf("appending old node %d\n", index);

        /* read node */
        df_flash_read(fs->chip,
                fs->root,
                node,
                FS_ROOTNODE_NODETABLE_OFFSET+(index+1)*sizeof(fs_node_t),
                sizeof(fs_node_t));

        /* write node to BUF1 */
        df_buf_write(fs->chip,
                     DF_BUF1,
                     node,
                     FS_ROOTNODE_NODETABLE_OFFSET+index*sizeof(fs_node_t),
                     sizeof(fs_node_t));

        index++;

    }

    /* mark last node as unused */
    node->unused = 1;

    /* write node to BUF1 */
    df_buf_write(fs->chip,
                 DF_BUF1,
                 node,
                 FS_ROOTNODE_NODETABLE_OFFSET+(max-1)*sizeof(fs_node_t),
                 sizeof(fs_node_t));

    /* cleanup */
    free(node);

    /* increment version and update checksum */
    fs_status_t ret = fs_increment(fs);

    if (ret != FS_OK)
        return ret;

    /* remove inode */
    return fs_update_inodetable(fs, inode, 0xffff);

}

fs_size_t fs_size(fs_t *fs, fs_inode_t inode)
{

    fs_size_t size = 0;

    /* allocate space */
    fs_page_t *page = malloc(sizeof(fs_page_t));

    /* return -1 on memory error */
    if (page == NULL)
        return -1;

    while(1) {

        /* extract page */
        df_page_t pagenum = fs_page(fs, inode);

        /* if this inode is empty, this file is either corrupt or empty */
        if (pagenum == 0xffff) {
            printf("fs: invalid next inode, return size 0\r\n");
            size = 0;
            break;
        }

        /* else load page */
        df_flash_read(fs->chip, pagenum, page, FS_STRUCTURE_OFFSET, sizeof(fs_page_t));

        /* append size */
        size += page->size;

        printf("fs: size in inode 0x%04x is 0x%04x\r\n", inode, page->size);

        /* if this is the last page, we are done */
        if (page->eof) {
            printf("fs: last inode in this file, returning\r\n");
            break;
        }

        /* else extract next inode */
        inode = page->next_inode;
    }

    free(page);

    return size;

}

/* private functions */
fs_status_t fs_scan(fs_t *fs)
{

    printf("fs: scanning for root node\r\n");

    /* init fs structure */
    fs->version = 0;

    /* alloc temporary buffer */
    fs_root_t *page = malloc(sizeof(fs_root_t));

    if (page == NULL)
        return FS_MEM;

    for (df_page_t p = 0; p < DF_PAGES; p++) {
        df_flash_read(fs->chip, p, page, FS_STRUCTURE_OFFSET, sizeof(fs_root_t));

        if (page->page.unused == 0 && page->page.root == 1) {
            printf("fs: found root node in page 0x%04x\r\n", p);

            /* compute crc */
            uint8_t crc = 0;
            df_buf_load(fs->chip, DF_BUF1, p);
            df_wait(fs->chip);
            crc = fs_crc(fs, crc, DF_BUF1, FS_STRUCTURE_OFFSET, FS_CRC_LENGTH);

            uint8_t crc2;
            df_flash_read(fs->chip, p, &crc2, FS_CRC_OFFSET, 1);

            if (crc == crc2) {
                printf("fs: valid crc\r\n");

                if (page->version > fs->version) {
                    printf("fs: found newer version!\r\n");
                    fs->version = page->version;
                    fs->root = p;
                }
            }
#ifdef DEBUG_FS
            else {
                printf("fs: crc do not match: 0x%02x != 0x%02x\r\n",
		       crc, crc2);
            }
#endif
        }
    }

    /* free temporary buffer */
    free(page);

    if (fs->version >= FS_INITIAL_VERSION) {
        printf("fs: root node has been found, page 0x%04x, version 0x%04x\r\n",
	       fs->root, fs->version);
        return FS_OK;
    } else {
        printf("fs: no root node found, creating one in page 0\r\n");
        return fs_format(fs);
    }

}

fs_status_t fs_format(fs_t *fs)
{

    /* set new version */
    if (fs->version > 0)
        fs->version++;
    else
        fs->version = FS_INITIAL_VERSION;

    /* allocate temporary buffer */
    fs_root_t *root = malloc(sizeof(fs_root_t));

    if (root == NULL)
        return FS_MEM;

    /* fill buffer with empty root node */
    root->page.root = 1;
    root->page.eof = 1;
    root->page.unused = 0;
    root->version = fs->version;

    /* initialize and erase future inode tables */
    fs_page_t *inode_page = malloc(sizeof(fs_page_t));

    if (inode_page == NULL)
    {
        free(root);
        return  FS_MEM;
    }

    /* create empty page */
    inode_page->root = 0;
    inode_page->eof = 1;
    inode_page->unused = 0;
    inode_page->next_inode = 0x0fff;

    /* set structure information */
    df_buf_write(fs->chip, DF_BUF1, inode_page, 0, sizeof(fs_page_t));

    /* fill with 0xff */
    uint8_t b = 0xff;
    for (uint16_t i = 0; i < DF_PAGESIZE - FS_DATA_OFFSET - sizeof(fs_page_t); i++)
        df_buf_write(fs->chip, DF_BUF1, &b, FS_DATA_OFFSET+i, 1);

    /* write pages */
    for (uint8_t i = 0; i < 16; i++) {
        root->inodetable[i] = i+1;
        df_buf_save(fs->chip, DF_BUF1, i+1);
        df_wait(fs->chip);
    }

    /* write root node to buffer */
    df_buf_write(fs->chip, DF_BUF1, root, FS_STRUCTURE_OFFSET, sizeof(fs_root_t));

    /* free temporary buffer */
    free(root);

    /* allocate temporary buffer */
    fs_node_t *node = malloc(sizeof(fs_node_t));

    if (node == NULL)
        return FS_MEM;

    /* write empty node entries */
    node->unused = 1;
    node->inode = 0;
    node->file = 0;
    node->reserved = 0;
    node->name[0] = 0;
    for (uint8_t i = 0; i < FS_NODES_IN_ROOT; i++) {
        df_buf_write(fs->chip, DF_BUF1, node,
                FS_ROOTNODE_NODETABLE_OFFSET+i*sizeof(fs_node_t),
                sizeof(fs_node_t));
    }

    /* calculate crc */
    uint8_t crc = 0;
    crc = fs_crc(fs, crc, DF_BUF1, FS_STRUCTURE_OFFSET, FS_CRC_LENGTH);

    printf("fs: crc of new root page is 0x%02x\n", crc);

    /* write crc */
    df_buf_write(fs->chip, DF_BUF1, &crc, FS_CRC_OFFSET, 1);

    /* program root node */
    df_buf_save(fs->chip, DF_BUF1, 0);
    df_wait(fs->chip);

    /* set global pointers */
    fs->root = 0;

    /* free temporary buffer */
    free(node);

    return FS_OK;

}

df_page_t fs_new_page(fs_t *fs)
{

    df_page_t page = (fs->last_free + 1) % DF_PAGES;

    /* sequentially check pages, until a free one can be found */
    while (fs_used(fs, page) && page != fs->last_free)
        page = (page + 1) % DF_PAGES;

    // printf("last free page %d, new free page %d\n", fs->last_free, page);

    if (page == fs->last_free)
        /* no free page could be found */
        return 0xffff;
    else {
        /* free page is found */
        fs->last_free = page;

        return page;
    }

}

fs_inode_t fs_new_inode(fs_t *fs)
{

    /* sequentially check inodes, until a free one can be found */
    for (uint8_t i = 0; i < FS_ROOTNODE_INODETABLE_SIZE; i++) {
        df_page_t page = fs_inodetable(fs, i);

        // printf("checking table %d at page %d\n", i, page);

        /* read each inode table entry */
        for (uint16_t j = 0; j < FS_INODES_PER_TABLE; j++) {
            fs_inodetable_node_t inode;

            df_flash_read(fs->chip, page, &inode, FS_DATA_OFFSET + j * sizeof(fs_inode_t), sizeof(fs_inode_t));

            /* if this inode is unused, return the index */
            if (inode.unused)
                return i*FS_INODES_PER_TABLE+j;
        }
    }

    /* else return 0xffff */
    return 0xffff;

}

df_page_t fs_inodetable(fs_t *fs, uint8_t tableid)
{

    df_page_t page;

    df_flash_read(fs->chip, fs->root, &page,
            FS_ROOTNODE_INODETABLE_OFFSET + (tableid * sizeof(fs_inodetable_node_t)),
            sizeof(fs_inodetable_node_t));

#ifdef DEBUG_FS_INODETABLE
    printf("inodetable(%02x): root 0x%04x, page 0x%04x\n", tableid, fs->root, page);
#endif

    return page;

}

df_page_t fs_page(fs_t *fs, fs_inode_t inode)
{

    /* load address of the page which contains this inode */
    df_page_t page = fs_inodetable(fs, inode / FS_INODES_PER_TABLE);

    // printf("inode %d: table id is %d and table page address is %d\n", inode, inode / FS_INODES_PER_TABLE, page);

    /* load page address */
    fs_inodetable_node_t node;
    df_flash_read(fs->chip, page, &node, FS_DATA_OFFSET + (inode % FS_INODES_PER_TABLE) * sizeof(fs_inodetable_node_t), sizeof(fs_inodetable_node_t));

    // printf("inode %d: -> page %d\n", inode, node.page);

    if (node.unused)
        return 0xffff;
    else
        return node.page;

}

void fs_mark(fs_t *fs, df_page_t page, uint8_t is_free)
{

    uint8_t b;

#ifdef DEBUG_FS_MARK
    printf("fs: marking page 0x%04x as %s\n", page, 
           is_free ? "free" : "used");
#endif

    /* load byte first */
    df_buf_read(fs->chip, DF_BUF2, &b, page/8, 1);

#ifdef DEBUG_FS_MARK
    printf("fs: read byte at offset 0x%04x: 0x%02x\r\n", page/8, b);
#endif

    /* set bit and write byte */
    if (is_free)
        b |= _BV(page % 8);
    else
        b &= ~_BV(page % 8);

    df_buf_write(fs->chip, DF_BUF2, &b, page/8, 1);

}

uint8_t fs_used(fs_t *fs, df_page_t page)
{

    uint8_t b;

    /* load byte */
    df_buf_read(fs->chip, DF_BUF2, &b, page/8, 1);

    return !(b & _BV(page % 8));

}

uint8_t fs_crc(fs_t *fs, uint8_t crc, df_buf_t buf, df_size_t offset, df_size_t length)
{

    while (length-- > 0) {

        uint8_t data;
        df_buf_read(fs->chip, buf, &data, offset++, 1);
        crc = _crc_ibutton_update(crc, data);

    }

    return crc;

}

fs_status_t fs_increment(fs_t *fs)
{

    fs_root_t *root = malloc(sizeof(fs_root_t));

    /* read root structure */
    df_buf_read(fs->chip, DF_BUF1, root, FS_STRUCTURE_OFFSET, sizeof(fs_root_t));

    /* update version */
    fs->version++;
    root->version = fs->version;

    /* write root node to buffer */
    df_buf_write(fs->chip, DF_BUF1, root, FS_STRUCTURE_OFFSET, sizeof(fs_root_t));

    /* calculate crc */
    uint8_t crc = 0;
    crc = fs_crc(fs, crc, DF_BUF1, FS_STRUCTURE_OFFSET, FS_CRC_LENGTH);

    // printf("crc of new root page is 0x%x\n", crc);

    /* write crc */
    df_buf_write(fs->chip, DF_BUF1, &crc, FS_CRC_OFFSET, 1);

    df_page_t page = fs_new_page(fs);

    if (page == 0xffff) {
        free(root);
        return FS_BADPAGE;
    }

    /* write root node to flash */
    df_buf_save(fs->chip, DF_BUF1, page);
    df_wait(fs->chip);

    fs->root = page;

    free(root);
    return FS_OK;

}

fs_status_t fs_update_inodetable(fs_t *fs, fs_inode_t inode, df_page_t page)
{

    // printf("updating inodetable %d -> %d\n", inode, page);

    /* allocate new page for the inodetable */
    df_page_t new_page = fs_new_page(fs);

    if ( new_page == 0xffff)
        return FS_BADPAGE;

    // printf("new inodetable will live in page %d\n", new_page);

    fs_inodetable_node_t *node = malloc(sizeof(fs_inodetable_node_t));

    if (node == NULL)
        return FS_MEM;

    if (page == 0xffff)
        node->unused = 1;
    else {
        node->unused = 0;
        node->page = page;
    }

    // printf("inodetable %d lives in page %d, writing new inodetable to page %d\n", inode / FS_INODES_PER_TABLE, fs_inodetable(fs, inode / FS_INODES_PER_TABLE), new_page);

    // printf("inode index %d\n", inode % FS_INODES_PER_TABLE);

    /* load inodetable into BUF1, update inode, write inodetable */
    df_buf_load(fs->chip, DF_BUF1, fs_inodetable(fs, inode / FS_INODES_PER_TABLE));
    df_wait(fs->chip);
    df_buf_write(fs->chip,
                 DF_BUF1,
                 node,
                 FS_DATA_OFFSET + (inode % FS_INODES_PER_TABLE) * sizeof(fs_inodetable_node_t),
                 sizeof(fs_inodetable_node_t));
    df_buf_save(fs->chip, DF_BUF1, new_page);
    df_wait(fs->chip);

    free(node);

    /* load root node into BUF1 */
    df_buf_load(fs->chip, DF_BUF1, fs->root);
    df_wait(fs->chip);

    /* write inodetable address */
    df_buf_write(fs->chip,
                 DF_BUF1,
                 &new_page,
                 FS_ROOTNODE_INODETABLE_OFFSET+ (inode / FS_INODES_PER_TABLE) *sizeof(df_page_t),
                 sizeof(df_page_t));

    /* increment version and update checksum */
    return fs_increment(fs);

}


#if DEBUG_FS
void
fs_inspect_node(fs_t *fs, uint16_t p)
{
    printf ("root page is 0x%04x\n", fs->root);

    fs_root_t *root = malloc(sizeof(fs_root_t));

    df_flash_read(fs->chip, p, root, FS_STRUCTURE_OFFSET, sizeof(fs_root_t));

    printf ("Properties of page 0x%04x:\n", p);
    if (root->page.unused)
	printf ("\t* unused\n");
    if (root->page.root)
        printf ("\t* root\n");
    if (root->page.eof)
        printf ("\t* eof\n");

    printf ("\tnext_inode: 0x%04x, size: 0x%04x\n", root->page.next_inode,
	    root->page.size);

    if (root->page.root) {
	printf ("\tThis is a root node (version 0x%02x), inodetable:\n",
		root->version);

        for (uint8_t i = 0; i < FS_ROOTNODE_INODETABLE_SIZE; i++) {

            uint16_t page = fs_inodetable(fs, i);
	    printf ("\t\t* 0x%02x -> 0x%04x, func: 0x%04x\n", i,
		    root->inodetable[i], page);
        }

        printf ("\troot entries:\n");
        for (uint8_t i = 0; i < FS_NODES_IN_ROOT; i++) {

            fs_node_t *node = malloc(sizeof(fs_node_t));

            if (node == NULL) {
                printf ("\t\tNULL!\n");
                break;
            }

            df_flash_read(fs->chip, fs->root, node, FS_ROOTNODE_NODETABLE_OFFSET + i * sizeof(fs_node_t), sizeof(fs_node_t));

            if (!node->unused) {
                char name[FS_FILENAME+1];

                strncpy(name, node->name, FS_FILENAME);
                name[FS_FILENAME] = '\0';

		printf ("\t\t* 0x%02x: %s\n", i, name);
            }

	    free (node);
        }
    }

    free(root);
}


void
fs_inspect_inode(fs_t *fs, fs_inode_t p)
{
    printf ("Inspection of inode 0x%04x:\n", p);

    df_page_t pagenum = fs_page (fs, p);
    printf ("\t* pagenum = %04x\n", pagenum);

    if (pagenum == 0xffff)
	return;

    fs_inspect_node (fs, pagenum);
}

#endif	/* DEBUG_FS */

/*
  -- Ethersex META --
  header(hardware/storage/dataflash/fs.h)
  init(fs_init)
*/

