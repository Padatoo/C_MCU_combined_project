#ifndef _45DBXX_H
#define _45DBXX_H

#include <stdint.h>
#include <stdbool.h>
#include "string.h"
//#include "emfat.h"
//#include "usbd_cdc_if.h"

typedef struct
{
	uint8_t		FlashSize_MBit;	
	uint16_t	PageSize;
	uint16_t	Pages;
	uint8_t		Shift;
}at45dbxx_t;


extern at45dbxx_t	at45dbxx;

const char*	at45dbxx_device_id(void);
bool					at45dbxx_init(void);
void 		at45dbxx_erase_chip(void);
void 					at45dbxx_erase_page(int page);
//*****************
void					at45dbxx_write_page(uint8_t message[50]);
//*****************
const char *	 at45dbxx_read_page(size_t read_size, size_t at45_page);
//*****************
void           at45dbxx_program_byte(int page_address, int byte_address, uint8_t data[], size_t size);
//user data 
void       at45dbxx_read_ring_header(int page_address, int byte_address, uint8_t msg[]);

#endif
