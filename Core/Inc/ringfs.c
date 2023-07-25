/*
 * Copyright © 2014 Kosma Moczek <kosma@cloudyourcar.com>
 * This program is free software. It comes without any warranty, to the extent
 * permitted by applicable law. You can redistribute it and/or modify it under
 * the terms of the Do What The Fuck You Want To Public License, Version 2, as
 * published by Sam Hocevar. See the COPYING file for more details.
 */

/**
 * @defgroup ringfs_impl RingFS implementation
 * @details
 *
 * @{
 */

#include <ringfs.h>

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include "main.h"


/**
 * @defgroup sector
 * @{
 */


extern uint16_t global_head_increment;
struct ringfs ring_structure;

 uint8_t SECTOR_FREE[]       = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; //< Sector erased OR FREE   ///
 uint8_t SECTOR_VALID[]     = {0xF1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; //< Sector contains valid data. 

 uint8_t SECTOR_GARBAGE[] = {0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; //< BAD BLOCK OR GARBAGE DATA. //
//**************************  byte 0x00 NOT ALLOWED BY EITHER AT45DB321 OR HAL-SPI, HAD A NASTY BUG BECAUSE OF THAT. ********************************
 uint8_t SLOT_FREE[]   = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
 uint8_t SLOT_VALID[]    = {0xF1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
 uint8_t SLOT_GARBAGE[]  = {0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};


static int _sector_address(struct ringfs *ring_structure, int sector)
{
    return (ring_structure->flash->sector_offset + sector);     //* fs->flash->sector_size;
}

 void _sector_get_status(struct ringfs *ring_structure, int sector, uint8_t local_sector[])
{
    
	(void) flash;
  int page_address = sector;
	int byte_address = 0;

	at45dbxx_read_ring_header(page_address, byte_address, local_sector);

}

/*static int _sector_set_status(struct ringfs *fs, int sector, uint8_t status[])
{
    return fs->flash->program(fs->flash,
            _sector_address(fs, sector),
            status, magic_number_of_bytes);
}
*/

static int _slot_address(struct ringfs *ring_structure, struct ringfs_loc *loc)
{
    return (magic_number_of_bytes + ring_structure->object_size) * loc->slot;
}

static int _slot_set_status(struct ringfs *ring_structure, struct ringfs_loc *loc, uint8_t status[], uint8_t session)
{
	  uint8_t local_temp_status[6];
		//uint8_t default_increment_1 = 0xFF, default_increment_2 = 0xFF, default_session_byte = 0xFF; 
		uint8_t default_crc_byte_1 = 0xFF, default_crc_byte_2 = 0xFF;
	  ///// lines above is mainly for testing purpose
	global_head_increment += 1;
		uint16_t local_temp_increment = global_head_increment;
		if ((local_temp_increment < 0x0101) || (local_temp_increment == 0xFFFF)) { // try to avoid increment 0x0000 because that is marker of BAD block, and increment 0xffff because that is state of erased page
			local_temp_increment = 0x0101;
			global_head_increment = 0x0101;    // weird shit, I know, cba.
		}
	//(uint8_t)(entry_increment >> 8)
	//
	  local_temp_status[0] = status[0];
	  local_temp_status[1] = (uint8_t)(local_temp_increment >> 8); //default_increment_1;//(uint8_t)(entry_increment >> 8);
	  local_temp_status[2] = (uint8_t)(local_temp_increment); //default_increment_2;//(uint8_t)(entry_increment);
	  local_temp_status[3] = session;
	  local_temp_status[4] = default_crc_byte_1;
	  local_temp_status[5] = default_crc_byte_2;
    return ring_structure->flash->program(ring_structure->flash, _sector_address(ring_structure, loc->sector),  
            _slot_address(ring_structure, loc),
              local_temp_status, magic_number_of_bytes); 
}

/**
 * @}
 * @defgroup loc
 * @{
 */

bool _status_equal(uint8_t status_one[], uint8_t status_two[])
{
	int i;
	int size_of_sector_in_bytes = 1;
	for (i = 0; i < size_of_sector_in_bytes; i++) {
		if (status_one[i] != status_two[i]){
				return false;
		}
	}
	return true;
}	
/** Advance a location to the beginning of the next sector. */
static void _loc_advance_sector(struct ringfs *ring_structure, struct ringfs_loc *loc)
{
    loc->slot = 0;
    loc->sector++;
    if (loc->sector >= (ring_structure->flash->sector_count) - 60) {
        loc->sector = 0;
		}
}

//function to get slot location on page for the append session check
int _get_slot_loc(struct ringfs *ring_structure, struct ringfs_loc *loc)
{
	return loc->slot;
}
/** Advance a location to the next slot, advancing the sector too if needed. */
static void _loc_advance_slot(struct ringfs *ring_structure, struct ringfs_loc *loc)
{
    loc->slot++;
    if (loc->slot >= ring_structure->slots_per_sector)
        _loc_advance_sector(ring_structure, loc);
}



static void _conditional_sector_erase(struct ringfs *ring_structure, struct ringfs_loc *loc) {
	if ((loc->slot) == 0) {
		ring_structure->flash->sector_erase(ring_structure->flash, ring_structure->write.sector);
	}
}
	

/**
 * @}
 */

/* And here we go. */

int ringfs_init(struct ringfs *ring_structure, struct ringfs_flash_partition *flash, uint32_t version, int object_size)
{
    /* Copy arguments to instance. */
    ring_structure->flash = flash;
    ring_structure->version = version;
    ring_structure->object_size = object_size;

    /* Precalculate commonly used values. */
    ring_structure->slots_per_sector = (ring_structure->flash->sector_size) /
                           (magic_number_of_bytes + ring_structure->object_size);

	
		//fs->write.slot = 0;
		//fs->write.sector = 2000;
    return 0;
}


bool start_of_stm32 = true;
uint8_t old_session;

int ringfs_append(struct ringfs *ring_structure, uint8_t object[], size_t append_size, uint8_t session)
{

	  uint8_t new_session = session;
	if (start_of_stm32 == true) {
		start_of_stm32 = false;
		old_session = new_session;
	}
	else {
    if ((_get_slot_loc(ring_structure, &ring_structure->write) != 0) && (new_session != old_session)){
			_loc_advance_sector(ring_structure, &ring_structure->write);
			//_loc_advance_sector(fs, &fs->read);
			//_loc_advance_sector(fs, &fs->cursor);
		}
		old_session = new_session;
	}
		/*if current session is different from the previous session AND the cursor is not at start of the page, 
	   then go to start of the next page.*/
  
      /*   
		MUST ADD A CHECK FOR BAD BLOCKS HERE ACCORDING TO ALGORITHM
		
		MUST ADD A CHECK FOR BAD BLOCKS HERE ACCORDING TO ALGORITHM!!!
    }*/
	
	// erase 'sector' (page) when  write slot is 0 (start of page)
	   //fs->flash->sector_erase(fs->flash, fs->write.sector);
			//at45dbxx_erase_page(fs->write.sector);
		_conditional_sector_erase(ring_structure, &ring_structure->write);

		//fs->flash->sector_erase(fs->flash, fs->write.sector);

    // Write object. 
    ring_structure->flash->program(ring_structure->flash, ring_structure->write.sector,   // program_slot ?? OR?
           _slot_address(ring_structure, &ring_structure->write) + magic_number_of_bytes, // currently is 6 
            object, append_size);    
    // Commit write. 
    _slot_set_status(ring_structure, &ring_structure->write, SLOT_VALID, session);
    // Advance the write head. 
    _loc_advance_slot(ring_structure, &ring_structure->write);
//
    return 0;
}






// 

/**
 * @}
 */

/* vim: set ts=4 sw=4 et: */
