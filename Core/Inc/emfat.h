/*
The MIT License (MIT)

Copyright (c) 2015 by Sergey Fetisov <fsenok@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/*
version: 1.0 (4.01.2015)
*/

#ifndef EMFAT_H
#define EMFAT_H

#define AUTORUN_SIZE 50
#define README_SIZE  512
#define ICON_SIZE    1

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "stm32f1xx.h"
#include "stm32f1xx_hal.h"
#include "usbd_def.h"
#include "StorageMode.h"
#include "ringfs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*emfat_readcb_t)(uint8_t *dest, int size, uint32_t offset, size_t userdata);
typedef void (*emfat_writecb_t)(const uint8_t *data, int size, uint32_t offset, size_t userdata);

typedef struct emfat_entry emfat_entry_t;

typedef struct entry_name names_struct;

struct entry_name
{
	char name[12];
};

struct emfat_entry
{
	const char            *name;
	bool            dir;
	int             level ;
	uint32_t        offset;
	uint32_t        curr_size;
	uint32_t        max_size;
	size_t          user_data;
	emfat_readcb_t  readcb;
	emfat_writecb_t writecb;
	uint8_t         session;
	struct
	{
		uint32_t       first_clust;
		uint32_t       last_clust;
		uint32_t       last_reserved;
		uint32_t       num_subentry;
		emfat_entry_t *top;
		emfat_entry_t *sub;
		emfat_entry_t *next;
	} priv;
};

typedef struct
{
	uint64_t    vol_size;
	uint16_t    num_sectors;
	const char *vol_label;
	struct
	{
		uint16_t       boot_sect;
		uint16_t       fat1_sect;
		uint16_t       fat2_sect;
		uint16_t       root_sect;
		uint16_t       num_clust;
		emfat_entry_t *entries;
		emfat_entry_t *last_entry;
		int            num_entries;
	} priv;
} emfat_t;

extern emfat_t emfat;

bool emfat_init(emfat_t *emfat, const char *label, emfat_entry_t *entries);
void emfat_read(emfat_t *emfat, uint8_t *data, uint32_t sector, int num_sectors);
void emfat_write(emfat_t *emfat, const uint8_t *data, uint32_t sector, int num_sectors);

void autorun_read_proc(uint8_t *dest, int size, uint32_t offset, size_t userdata);
void icon_read_proc(uint8_t *dest, int size, uint32_t offset, size_t userdata);
void readme_read_proc(uint8_t *dest, int size, uint32_t offset, size_t userdata);

void EMFAT_USB_DEVICE_Init(void);

static names_struct entry_name_array[256];
static emfat_entry_t default_entry = {NULL, false, 1, 0, 512, 512, 0, readme_read_proc, NULL, NULL};

void ascam_emfat_scan(struct ringfs *fs, emfat_entry_t array[], names_struct entry_name_array[]);

static emfat_entry_t entries[80] =
{
	// name          dir    lvl offset  size    max_size    user  read               write  session
	{ "",            true,  0,  0,      0,          0,       0,    NULL,              NULL, NULL }, // root
  { "06_jun00.txt",  false, 1,  0,    512*8000,       512*8000,    0,    readme_read_proc,  NULL, NULL },
  /*{ "page_1.txt", false, 1, 0, 512*4100,     512*4100,    4000,  readme_read_proc, NULL   },
  { "page_2.txt	", false, 1, 0, README_SIZE*1000,     README_SIZE*1000,    2000,  readme_read_proc, NULL   },
	{ "page_3.txt	", false, 1, 0, 2000*512,     2000*512,    3000,  readme_read_proc, NULL   },
	{ "page_4.txt	", false, 1, 0, 512*2000,     512*2000,    5000,  readme_read_proc, NULL   },
  { "page_5.txt	", false, 1, 0, 512*1100,     512*1100,    7000,  readme_read_proc, NULL   },
	{ "page_6.txt	", false, 1, 0, 512*1000,     512*1000,    8000,  readme_read_proc, NULL   },
	//{ "page_7.txt	", false, 1, 0, 512,     512,    8000,  readme_read_proc, NULL   },
	{NULL},
{ "4100un.txt",  false, 1,  0,      README_SIZE,     README_SIZE,    4101,    readme_read_proc,  NULL },
{ "4101un.txt",  false, 1,  0,      README_SIZE,     README_SIZE,    4100,    readme_read_proc,  NULL },
{ "4099un.txt",  false, 1,  0,      README_SIZE,     README_SIZE,    4099,    readme_read_proc,  NULL },
{ "17_jun.txt",  false, 2,  0,      1024*1024,     1024*1024,    0,    readme_read_proc,  NULL },
{ "18_jun.txt",  false, 2,  0,      1024*1024,     1024*1024,    0,    readme_read_proc,  NULL },
{ "19_jun.txt",  false, 2,  0,      1024*1024,     1024*1024,    0,    readme_read_proc,  NULL },
{ "20_jun.txt",  false, 2,  0,      1024*1024,     1024*1024,    0,    readme_read_proc,  NULL },
{ "21_jun.txt",  false, 2,  0,      1024*1024,     1024*1024,    0,    readme_read_proc,  NULL },
{ "22_jun.txt",  false, 2,  0,      1024*1024,     1024*1024,    0,    readme_read_proc,  NULL },*/// drivers/readme.txt
};


#ifdef __cplusplus
}
#endif

#endif
