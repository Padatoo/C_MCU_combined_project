/*
 * Copyright © 2014 Kosma Moczek <kosma@cloudyourcar.com>
 * This program is free software. It comes without any warranty, to the extent
 * permitted by applicable law. You can redistribute it and/or modify it under
 * the terms of the Do What The Fuck You Want To Public License, Version 2, as
 * published by Sam Hocevar. See the COPYING file for more details.
 */

#ifndef RINGFS_H
#define RINGFS_H

/**
 * @defgroup ringfs_api RingFS API
 * @{
 */

#include <stdint.h>
#include <stdio.h>
#include "45dbxx.h"


#define defined_sector_size 528
#define defined_sector_offset 0
#define defined_sector_count 8192

struct log_entry {
	int level;
	char message[16];
};
#define LOG_ENTRY_VERSION 1
static size_t magic_number_of_bytes = 6;
typedef long ssize_t;

extern struct ringfs ring_structure;

/**
 * Flash memory+parition descriptor.
 */


struct ringfs_flash_partition
{
    int sector_size;            /**< Sector size, in bytes. */
    int sector_offset;          /**< Partition offset, in sectors. */
    int sector_count;           /**< Partition size, in sectors. */

    /**
     * Erase a sector.
     * @param address Any address inside the sector.
     * @returns Zero on success, -1 on failure.
     */
    int (*sector_erase)(struct ringfs_flash_partition *flash, int address);
    /**
     * Program flash memory bits by toggling them from 1 to 0.
     * @param address Start address, in bytes.
     * @param data Data to program.
     * @param size Size of data.
     * @returns size on success, -1 on failure.
     */
    ssize_t (*program)(struct ringfs_flash_partition *flash, int page_address, int byte_address, uint8_t data[], size_t size);
    /**
     * Read flash memory.
     * @param address Start address, in bytes.
     * @param data Buffer to store read data.
     * @param size Size of data.
     * @returns size on success, -1 on failure.
     */
		ssize_t (*program_slot)(struct ringfs_flash_partition *flash, int page_address, int byte_address, uint8_t data[], size_t size);
		//********************************************************************************
    ssize_t (*read)(struct ringfs_flash_partition *flash, int address, uint8_t data[], size_t size);
};
static int at45_sector_erase(struct ringfs_flash_partition *flash, int address)
{
	  (void) flash;
		int page_address = address;
		at45dbxx_erase_page(page_address);
		return 0;
}	

static ssize_t at45_program(struct ringfs_flash_partition *flash, int page_address, int byte_address, uint8_t data[], size_t size)
{
	(void) flash;
	//int page_address = address / 528; //528;
	//int byte_address = address % 528; //% 528; need to set to something other then 0
	at45dbxx_program_byte(page_address, byte_address, data, size);
	return size;
}

static ssize_t at45_program_slot(struct ringfs_flash_partition *flash, int page_address, int byte_address, uint8_t data[], size_t size)
{
	(void) flash;
	//int page_address = address / 528;//address / 528;
	//int byte_address = address % 528;//address % 528; // % 528
	at45dbxx_program_byte(page_address, byte_address, data, 6);  // magic number of 6 is amount of bytes
	return size;
}
static  ssize_t at45_read(struct ringfs_flash_partition *flash, int address, uint8_t data[], size_t size)
{
	(void) flash;
	//int page_address = address / 528;
	//int byte_address = address % 528;
	
	//void answer_temp = at45dbxx_read_ring_page(page_address, byte_address, data);
	//uint8_t buffer[4] = {0};
  //	char empty_char[] = "";
	//strcpy(buffer, empty_char);
	//memcpy(data, answer_temp, 4);
	//*global_sector_status[0] = buffer[0];
	//uint8_t * ptr_to_buffer = buffer;
	//return *ptr_to_buffer;
	//memcpy(local_sector_status, buffer, sizeof(buffer));
	//fs->flash->program(fs->flash,
	return 1;
}	

static struct ringfs_flash_partition flash = {
	.sector_size = defined_sector_size,
	.sector_offset = defined_sector_offset,
	.sector_count = defined_sector_count,
	
	.sector_erase = at45_sector_erase,
	.program = at45_program,
	.program_slot = at45_program_slot,
	.read = at45_read,
};
/** @private */
struct ringfs_loc {
    uint16_t sector;
    uint8_t slot;
};

/**
 * RingFS instance. Should be initialized with ringfs_init() befure use.
 * Structure fields should not be accessed directly.
 * */
struct ringfs {
    /* Constant values, set once at ringfs_init(). */
    struct ringfs_flash_partition *flash;
    uint32_t version;
    int object_size;
    /* Cached values. */
    uint8_t slots_per_sector;

    /* Read/write pointers. Modified as needed. */
    //struct ringfs_loc read;
    struct ringfs_loc write;
    //struct ringfs_loc cursor;
};

/**
 * Initialize a RingFS instance. Must be called before the instance can be used
 * with the other ringfs_* functions.
 *
 * @param fs RingFS instance to be initialized.
 * @param flash Flash memory interface. Must be implemented externally.
 * @param version Object version. Should be incremented whenever the object's
 *                semantics or size change in a backwards-incompatible way.
 * @param object_size Size of one stored object, in bytes.
 * @returns Zero on success, -1 on failure.
 */
int ringfs_init(struct ringfs *fs, struct ringfs_flash_partition *flash, uint32_t version, int object_size);

/**
 * Append an object at the end of the ring. Deletes oldest objects as needed.
 *
 * @param fs Initialized RingFS instance.
 * @param object Object to be stored.
 * @returns Zero on success, -1 on failure.
 */
int ringfs_append(struct ringfs *fs, uint8_t object[], size_t size, uint8_t session);



/**
 * @}
 */

#endif

/* vim: set ts=4 sw=4 et: */
