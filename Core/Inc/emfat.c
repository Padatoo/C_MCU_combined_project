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

#include "emfat.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_msc.h"
#include "usbd_storage_if.h"
#include "45dbxx.h"
#include "spi.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SECT              512
#define CLUST             4096
#define SECT_PER_CLUST    (CLUST / SECT)
#define SIZE_TO_NSECT(s)  ((s) == 0 ? 1 : ((s) + SECT - 1) / SECT)
#define SIZE_TO_NCLUST(s) ((s) == 0 ? 1 : ((s) + CLUST - 1) / CLUST)

#define CLUST_FREE     0x00000000
#define CLUST_RESERVED 0x00000001
#define CLUST_BAD      0x0FFFFFF7
#define CLUST_ROOT_END 0X0FFFFFF8
#define CLUST_EOF      0x0FFFFFFF

#define MAX_DIR_ENTRY_CNT 16
#define FILE_SYS_TYPE_OFF 82
#define BYTES_PER_SEC_OFF 11
#define SEC_PER_CLUS_OFF 13
#define RES_SEC_CNT_OFF 14
#define FAT_CNT_OFF 16
#define TOT_SEC_CNT_OFF 32
#define SEC_PER_FAT 36
#define ROOT_DIR_STRT_CLUS_OFF 44
#define FS_INFOSECTOR_OFF 48
#define BACKUP_BOOT_SEC_OFF 50
#define NXT_FREE_CLUS_OFF 492
#define FILE_SYS_TYPE_LENGTH 8
#define SHRT_FILE_NAME_LEN 11
#define STRT_CLUS_LOW_OFF 26
#define STRT_CLUS_HIGH_OFF 20
#define FILE_SIZE_OFF 28
#define ATTR_OFF 11
#define FILE_STAT_LEN 21
#define CHECK_SUM_OFF 13
#define FILE_NAME_SHRT_LEN 8
#define FILE_NAME_EXTN_LEN 3
#define LONG_FILE_NAME_LEN 255
#define LOW_CLUSWORD_MASK 0x0000FFFF
#define HIGH_CLUSWORD_MASK 0xFFFF0000
#define LONG_FNAME_MASK 0x0F
#define LAST_ORD_FIELD_SEQ 0x40
#define LFN_END_MARK 0xFFFF
#define LFN_TERM_MARK 0x0000
#define LFN_FIRST_OFF 0x01
#define LFN_SIXTH_OFF 0x0E
#define LFN_TWELVETH_OFF 0x1C
#define LFN_FIRST_SET_CNT 5
#define LFN_SEC_SET_CNT 6
#define LFN_THIRD_SET_CNT 2
#define LFN_FIRST_SET_LEN 10
#define LFN_SEC_SET_LEN 12
#define LFN_THIRD_SET_LEN 4
#define LFN_EMPTY_LEN 2
#define LFN_LEN_PER_ENTRY 13
#define FNAME_EXTN_SEP_OFF 6
#define FNAME_SEQ_NUM_OFF 7
#define BYTES_PER_CLUSTER_ENTRY 4
#define DIR_ENTRY_LEN 32
#define VOL_ID_LEN 4
#define VOL_LABEL_LEN 11
#define RESERV_LEN 12
#define FS_VER_LEN 2
#define OEM_NAME_LEN 8
#define JUMP_INS_LEN 3
#define MAX_FAT_CNT 2
#define SPACE_VAL 32
#define FILE_READ 0x01
#define FILE_WRITE 0X02
#define FILE_CREATE_NEW 0x04
#define FILE_CREATE_ALWAYS 0x08
#define FILE_APPEND 0x10
#define ATTR_READ 0x01
#define ATTR_HIDDEN 0x02
#define ATTR_SYSTEM 0x04
#define ATTR_VOL_LABEL 0x08
#define ATTR_DIR 0x10
#define ATTR_ARCHIVE 0x20
#define ATTR_LONG_FNAME 0x0F
#define FREE_DIR_ENTRY 0x00
#define DEL_DIR_ENTRY 0xE5
#define DOT_DIR_ENTRY 0x2E
#define ASCII_DIFF 32
#define FILE_SEEK_SET 0
#define FILE_SEEK_CUR 1
#define FILE_SEEK_END 2
#define DELIMITER '/'
#define EXTN_DELIMITER '.'
#define TILDE '~'
#define FULL_SHRT_NAME_LEN 13

#pragma pack(push, 1)
//entries[256] = {NULL};

uint16_t global_head_increment = 0; // Thats the increment of head/tail location of the ringbuffer,
//the place from which reading/writing will commence.
uint8_t last_session_number = 0;
uint8_t global_session_number = 0;
//uint8_t *session_pointer;



const char *autorun_file =
	"[autorun]\r\n"
	"label=Ascam Emfat\r\n"
	"ICON=icon.ico\r\n";



const char icon_file[ICON_SIZE] =
{
	0x00
};



/*void autorun_read_proc(uint8_t *dest, int size, uint32_t offset, size_t userdata)
{
	int len = 0;
	if (offset > AUTORUN_SIZE) return;
	if (offset + size > AUTORUN_SIZE)
		len = AUTORUN_SIZE - offset; else
		len = size;
	memcpy(dest, &autorun_file[offset], len);
}
*/
/*void icon_read_proc(uint8_t *dest, int size, uint32_t offset, size_t userdata)
{
	int len = 0;
	if (offset > ICON_SIZE) return;
	if (offset + size > ICON_SIZE)
		len = ICON_SIZE - offset; else
		len = size;
	memcpy(dest, &icon_file[offset], len);
}*/

void readme_read_proc(uint8_t *dest, int size, uint32_t offset, size_t userdata)
{
	size_t page = userdata + offset;
	const char * answer_temp = at45dbxx_read_page(0, page);
	char buffer[512];
	//char buffer_debug[512];
	char empty_char[] = "";
	strcpy(buffer, empty_char);
	strcat(buffer, answer_temp);

	char buffer_output[512] = "";
	//char buffer_debug[512] = "";
	
	
	
	for (int slot = 0; slot < 5; slot++){  ///  TEST 
		if (slot == 4) {
		char buffer_temp[104];
		strncpy(buffer_temp, buffer+6+(slot*102), 96);
		strncat(buffer_output, buffer_temp, 96);
		strncat(buffer_output, "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\n", 8);
		} else {
		char buffer_temp[102];
		strncpy(buffer_temp, buffer+6+(slot*102), 96);
		strncat(buffer_output, buffer_temp, 96);
		strncat(buffer_output, "\xFF\xFF\xFF\xFF\xFF\n", 6);
		}
	}
	
	            // TEST
	/*	uint16_t new_valid_increment;
		uint8_t left_temp_increment;
		uint8_t right_temp_increment;		
		
		uint16_t last_valid_increment = 0;
		uint16_t last_increment_diff = 0;
		uint16_t new_increment_diff;
							
		left_temp_increment = answer_temp[1]; // get page increment number from bytes [1] and [2] and combine them into uint16_t
		right_temp_increment = answer_temp[2];
		new_valid_increment = ((((uint16_t)left_temp_increment) << 8) | (uint16_t)right_temp_increment);
		
		new_increment_diff = ((new_valid_increment - last_valid_increment) % 65536); // compare difference between increments to find the
		
		uint8_t first_half = (uint8_t)(new_increment_diff >> 8);
		uint8_t second_half = (uint8_t)(new_increment_diff);
		
		char decimal_buffer[5];
		sprintf(decimal_buffer, "%05u", new_increment_diff);
		
		char second_decimal_buffer[5];
		sprintf(second_decimal_buffer, "%05u", new_valid_increment);
		
		last_increment_diff = new_increment_diff;
		last_valid_increment = new_valid_increment;
		
	for (int slot2 = 0; slot2 < 5; slot2++){   // DEBUG MODE
		if (slot2 == 4){
			char buffer_temp[104];
			strncpy(buffer_temp, buffer+(slot2*102), 102);
			strncat(buffer_debug, buffer_temp, 102);
			strncat(buffer_debug, "\xFF\n", 2);
		} else {
			char buffer_temp[102];
			strncpy(buffer_temp, buffer+(slot2*102), 102);  */
			/////////////////////////////////////////////
			/*buffer_temp[13] = decimal_buffer[0];
			buffer_temp[14] = decimal_buffer[1];
			buffer_temp[15] = decimal_buffer[2];
			buffer_temp[16] = decimal_buffer[3];
			buffer_temp[17] = decimal_buffer[4];
			buffer_temp[18] = ' ';
			buffer_temp[19] = second_decimal_buffer[0];
			buffer_temp[20] = second_decimal_buffer[1];
			buffer_temp[21] = second_decimal_buffer[2];
			buffer_temp[22] = second_decimal_buffer[3];
			buffer_temp[23] = second_decimal_buffer[4];
			buffer_temp[24] = ' ';*/
			///////////////////////////////////////////////////
			//buffer_temp[101] = '\n';
			//strncat(buffer_debug, buffer_temp, 102);
			//strncat(buffer_debug, "\n", 1);
		//}
	//}
	
	
	//////////////////////////////////////////////////
	memcpy(dest, buffer_output, sizeof(buffer_output));
		//memcpy(dest, buffer, sizeof(buffer));
			//memcpy(dest, buffer_debug, sizeof(buffer_debug));  //DEBUG BUFFER
/////////////////////////////////////////////////////
	
}

names_struct entry_name_array[100];

emfat_entry_t entries[100] =
{
	// name          dir    lvl offset  size    max_size    user  read               write  session
	{ "",            true,  0,  0,      0,          0,       0,    NULL,              NULL, NULL }, // root
  { "neNaiden.txt",  false, 1,  0,    8192*512,       8192*512,    0,    readme_read_proc,  NULL, NULL },
  /*{ "ERROR_2.txt", false, 1, 0, 512*2048,     512*2048,    4096,  readme_read_proc, NULL   },
  { "ERROR_3.txt	", false, 1, 0, 512*1024,     512*1024,    6144,  readme_read_proc, NULL   },
	{ "ERROR_4.txt	", false, 1, 0, 1024*512,     1024*512,    7168,  readme_read_proc, NULL   },
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

typedef struct
{
	uint8_t  status;          // 0x80 for bootable, 0x00 for not bootable, anything else for invalid
	uint8_t  StartAddrHead;   // head address of start of partition
	uint16_t StartAddrCylSec; // (AddrCylSec & 0x3F) for sector,  (AddrCylSec & 0x3FF) for cylendar
	uint8_t  PartType;
	uint8_t  EndAddrHead;     // head address of start of partition
	uint16_t EndAddrCylSec;   // (AddrCylSec & 0x3F) for sector,  (AddrCylSec & 0x3FF) for cylendar
	uint32_t StartLBA;        // linear address of first sector in partition. Multiply by sector size (usually 512) for real offset
	uint32_t EndLBA;          // linear address of last sector in partition. Multiply by sector size (usually 512) for real offset
} mbr_part_t;

typedef struct
{
	uint8_t    Code[440];
	uint32_t   DiskSig;  //This is optional
	uint16_t   Reserved; //Usually 0x0000
	mbr_part_t PartTable[4];
	uint8_t    BootSignature[2]; //0x55 0xAA for bootable
} mbr_t;

typedef struct
{
	uint8_t jump[JUMP_INS_LEN];
	uint8_t OEM_name[OEM_NAME_LEN];
	uint16_t bytes_per_sec;
	uint8_t sec_per_clus;
	uint16_t reserved_sec_cnt;
	uint8_t fat_cnt;
	uint16_t root_dir_max_cnt;
	uint16_t tot_sectors;
	uint8_t media_desc;
	uint16_t sec_per_fat_fat16;
	uint16_t sec_per_track;
	uint16_t number_of_heads;
	uint32_t hidden_sec_cnt;
	uint32_t tol_sector_cnt;
	uint32_t sectors_per_fat;
	uint16_t ext_flags;
	uint8_t fs_version[FS_VER_LEN];
	uint32_t root_dir_strt_cluster;
	uint16_t fs_info_sector;
	uint16_t backup_boot_sector;
	uint8_t reserved[RESERV_LEN];
	uint8_t drive_number;
	uint8_t reserved1;
	uint8_t boot_sig;
	uint8_t volume_id[VOL_ID_LEN];
	uint8_t volume_label[VOL_LABEL_LEN];
	uint8_t file_system_type[FILE_SYS_TYPE_LENGTH];
} boot_sector;

typedef struct
{
	uint8_t name[FILE_NAME_SHRT_LEN];
	uint8_t extn[FILE_NAME_EXTN_LEN];
	uint8_t attr;
	uint8_t reserved;
	uint8_t crt_time_tenth;
	uint16_t crt_time;
	uint16_t crt_date;
	uint16_t lst_access_date;
	uint16_t strt_clus_hword;
	uint16_t lst_mod_time;
	uint16_t lst_mod_date;
	uint16_t strt_clus_lword;
	uint32_t size;
} dir_entry;

typedef struct
{
	uint8_t ord_field;
	uint8_t fname0_4[LFN_FIRST_SET_LEN];
	uint8_t flag;
	uint8_t reserved;
	uint8_t chksum;
	uint8_t fname6_11[LFN_SEC_SET_LEN];
	uint8_t empty[LFN_EMPTY_LEN];
	uint8_t fname12_13[LFN_THIRD_SET_LEN];
} lfn_entry;

#pragma pack(pop)

bool emfat_init_entries(emfat_entry_t *entries)
{
	emfat_entry_t *e;
	int i, n;

	e = &entries[0];
	if (e->level != 0 || !e->dir || e->name == NULL) return false;

	e->priv.top = NULL;
	e->priv.next = NULL;
	e->priv.sub = NULL;
	e->priv.num_subentry = 0;

	n = 0;
	for (i = 1; entries[i].name != NULL; i++)
	{
		entries[i].priv.top = NULL;
		entries[i].priv.next = NULL;
		entries[i].priv.sub = NULL;
		entries[i].priv.num_subentry = 0;
		if (entries[i].level == n - 1)
		{
			if (n == 0) return false;
			e = e->priv.top;
			n--;
		}
		if (entries[i].level == n + 1)
		{
			if (!e->dir) return false;
			e->priv.sub = &entries[i];
			entries[i].priv.top = e;
			e = &entries[i];
			n++;
			continue;
		}
		if (entries[i].level == n)
		{
			if (n == 0) return false;
			e->priv.top->priv.num_subentry++;
			entries[i].priv.top = e->priv.top;
			e->priv.next = &entries[i];
			e = &entries[i];
			continue;
		}
		return false;
	}
	return true;
}

bool emfat_init(emfat_t *emfat, const char *label, emfat_entry_t *entries)
{
	uint32_t sect_per_fat;
	uint32_t clust;
	emfat_entry_t *e;
	int i;

	if (emfat == NULL || label == NULL || entries == NULL)
		return false;

	if (!emfat_init_entries(entries))
		return false;

	clust = 2;
	for (i = 0; entries[i].name != NULL; i++)
	{
		e = &entries[i];
		if (e->dir)
		{
			e->curr_size = 0;
			e->priv.first_clust = clust;
			e->priv.last_clust = clust + SIZE_TO_NCLUST(e->priv.num_subentry * sizeof(dir_entry)) - 1;
			e->priv.last_reserved = e->priv.last_clust;
		}
		else
		{
			e->priv.first_clust = clust;
			e->priv.last_clust = e->priv.first_clust + SIZE_TO_NCLUST(entries[i].curr_size) - 1;
			e->priv.last_reserved = e->priv.first_clust + SIZE_TO_NCLUST(entries[i].max_size) - 1;
		}
		clust = e->priv.last_reserved + 1;
	}
	clust -= 2;

	emfat->vol_label = label;
	emfat->priv.num_entries = i;
	emfat->priv.boot_sect = 62;
	emfat->priv.fat1_sect = emfat->priv.boot_sect + 1;
	emfat->priv.num_clust = clust;
	sect_per_fat = SIZE_TO_NSECT((uint64_t)emfat->priv.num_clust * 4);
	emfat->priv.fat2_sect = emfat->priv.fat1_sect + sect_per_fat;
	emfat->priv.root_sect = emfat->priv.fat2_sect + sect_per_fat;
	emfat->priv.entries = entries;
	emfat->priv.last_entry = entries;
	emfat->num_sectors = (uint16_t)(clust * 8 + emfat->priv.root_sect);
	//test
	//emfat->num_sectors = 65536;
	emfat->vol_size = (uint64_t)emfat->num_sectors * SECT;
	return true;
}

/*
void lba_to_chs(uint32_t lba, uint8_t *cl, uint8_t *ch, uint8_t *dh)
{
	int cylinder, head, sector;
	int sectors = 63, heads = 255, cylinders = 1024;

	sector = lba % sectors + 1;
	head = (lba / sectors) % heads;
	cylinder = lba / (sectors * heads);

	if (cylinder >= cylinders)
	{
		*cl = *ch = *dh = 0xff;
		return;
	}

	*cl = sector | ((cylinder & 0x300) >> 2);
	*ch = cylinder & 0xFF;
	*dh = head;
}
*/
//struct ringfs ring_structure;

void ascam_emfat_scan(struct ringfs *ring_structure, emfat_entry_t entries[], names_struct entry_name_array[])
{
	HAL_NVIC_DisableIRQ(OTG_FS_IRQn);
	HAL_NVIC_DisableIRQ(USART1_IRQn); //HAL_NVIC_EnableIRQ(USART1_IRQn);
	int page;
	uint8_t old_session_byte = 0;
	uint8_t new_session_byte;
	uint8_t session_counter = 0;
	
	uint16_t last_valid_increment;
	//uint16_t last_increment_diff = 0;
	uint16_t new_increment_diff;
	uint16_t max_increment_diff = 0;
	
	ring_structure->write.sector = 0;
	ring_structure->write.slot = 0;
	
	bool first_page_flag = true;
	
	uint32_t empty_slot_counter = 0;
	
	uint32_t size_counter = 0;
	uint32_t test_size_counter = 0;
	//session_pointer = &last_session_number;
	
	for(page = 0; page < 8192; page++){
		//// prepare buffer for work
		char read_page_buffer[512];
		//char empty_char[] = "";
		char valid_slot_byte = 0xF1;
		
		uint16_t new_valid_increment;
		uint8_t left_temp_increment;
		uint8_t right_temp_increment;
		//uint8_t temp_session_counter;
		//// get page
		const char *answer_from_at45 = at45dbxx_read_page(0, page);
		//// fill buffer
		strcpy(read_page_buffer, answer_from_at45);
	  //strcat(read_page_buffer, answer_from_at45);   // TEST
		if ((uint8_t)read_page_buffer[0] == 0x00) {    // ignore BAD pages
			continue;
		} else {
			left_temp_increment = read_page_buffer[1]; // get page increment number from bytes [1] and [2] and combine them into uint16_t
			right_temp_increment = read_page_buffer[2];
			new_valid_increment = ((((uint16_t)left_temp_increment) << 8) | (uint16_t)right_temp_increment);
			if ((first_page_flag == true) && (uint8_t)read_page_buffer[0] == 0xF1) {
				first_page_flag = false;
				last_valid_increment = new_valid_increment;
				global_head_increment = last_valid_increment + 4;
				global_session_number = read_page_buffer[3];
			}
			new_increment_diff = ((new_valid_increment - last_valid_increment) % 65536); // compare difference between increments to find the
			 // head/tail. biggest difference is the head/tail (end of old session and start of new session)
			if (new_increment_diff > max_increment_diff) {
				ring_structure->write.sector = page;
				global_head_increment = last_valid_increment + 4;
				global_session_number = last_session_number; // last session number used by the main write function to change the session number (check fotoboard)
				max_increment_diff = new_increment_diff;
			}
			last_valid_increment = new_valid_increment;
			if ((uint8_t)read_page_buffer[0] == 0xF1){
				last_session_number = read_page_buffer[3];
			}
		}
		//// check each slot on page (valid is -> {0xF1,0xFF,0xFF,0xFF});  one entry is 102 bytes, starting from 0.
		//size_t entry_size = magic_number_of_bytes + magic_size_of_entry;
		//size_counter += 1;
		
		for (int entry_slot = 0; entry_slot < 5; entry_slot++)
		{
		  if ((read_page_buffer[entry_slot * 102]) != valid_slot_byte) {   //MAGIC NUMBER OF 102 is total entry size
				empty_slot_counter += 1;
				continue;
			}
			//else if   // INSERT  CRC HERE
			else {
				new_session_byte = read_page_buffer[((entry_slot * 102) + 3)];  // checking for new session // MAGIC NUMBER OF 102 IS TOTAL ENTRY SIZE
				if (new_session_byte != old_session_byte) {
					session_counter += 1;
					size_counter = 1;
					empty_slot_counter = 0;
					old_session_byte = new_session_byte;
				  if ((session_counter == 0) || (session_counter == 100))	{
						session_counter = 1;
					}
					entries[session_counter] = default_entry;
					entries[session_counter].user_data = page;
					static char temp_char_buffer[12];
					sprintf(temp_char_buffer, "num%u.txt", new_session_byte);
					////////////
					entries[session_counter].session = new_session_byte;
					////////////
					strcpy(entry_name_array[session_counter].name, temp_char_buffer);
					entries[session_counter].name = entry_name_array[session_counter].name;
		      ///////////

				} else {
					test_size_counter += 1;
					size_counter = size_counter + 1; // compiler Bug here.
					
				}
					entries[session_counter].curr_size = (((size_counter)/5) * 2) + ((size_counter) * 102) + (empty_slot_counter * 512);  //  <<<<<-----  512 is a fixed  size of USB data packet. 
				//	End of data transfer will be signalled by packet size less then 512.
					entries[session_counter].max_size = entries[session_counter].curr_size;   //  5 is the number of entries on page.
			}
		}
		
	}
	// END->START+END HERE.
	if ((session_counter > 2) && (entries[session_counter].session == entries[1].session)) {
		entries[1].curr_size += entries[session_counter].curr_size;
		entries[1].max_size += entries[session_counter].max_size;
		entries[1].user_data = entries[session_counter].user_data;
		entries[session_counter].name = 0x00;
  }
//fs->write.sector = 2000;
//fs->write.slot = 0;
HAL_NVIC_EnableIRQ(OTG_FS_IRQn);
HAL_NVIC_EnableIRQ(USART1_IRQn);	
}


void read_mbr_sector(const emfat_t *emfat, uint8_t *sect)
{
	mbr_t *mbr;
	memset(sect, 0, SECT);
	mbr = (mbr_t *)sect;
	mbr->DiskSig = 0;
	mbr->Reserved = 0;
	mbr->PartTable[0].status = 0x00;
	mbr->PartTable[0].StartAddrHead = 0;
	mbr->PartTable[0].StartAddrCylSec = 0;
	mbr->PartTable[0].PartType = 0x0C;
	mbr->PartTable[0].EndAddrHead = 0;
	mbr->PartTable[0].EndAddrCylSec = 0;
	mbr->PartTable[0].StartLBA = emfat->priv.boot_sect;
	mbr->PartTable[0].EndLBA = emfat->num_sectors - emfat->priv.boot_sect;
//		((emfat->num_sectors + SECT_PER_CLUST - 1) / SECT_PER_CLUST) * SECT_PER_CLUST;
	mbr->BootSignature[0] = 0x55;
	mbr->BootSignature[1] = 0xAA;
}

void read_boot_sector(const emfat_t *emfat, uint8_t *sect)
{
	boot_sector *bs;
	memset(sect, 0, SECT);
	bs = (boot_sector *)sect;
	bs->jump[0] = 0xEB;
	bs->jump[1] = 0x58;
	bs->jump[2] = 0x90;
	memcpy(bs->OEM_name, "MSDOS5.0", 8);
	bs->bytes_per_sec = SECT;
	bs->sec_per_clus = 8;
	bs->reserved_sec_cnt = 1;
	bs->fat_cnt = 2;
	bs->root_dir_max_cnt = 0;
	bs->tot_sectors = 0;
	bs->media_desc = 0xF8;
	bs->sec_per_fat_fat16 = 0;
	bs->sec_per_track = 63;
	bs->number_of_heads = 0xFF;
	bs->hidden_sec_cnt = 62;
		bs->tol_sector_cnt = emfat->priv.root_sect + emfat->priv.num_clust * 8;
		bs->sectors_per_fat = emfat->priv.fat2_sect - emfat->priv.fat1_sect;
	bs->ext_flags = 0;
	bs->fs_version[0] = 0;
	bs->fs_version[1] = 0;
	bs->root_dir_strt_cluster = 2;
	bs->fs_info_sector = 0;
	bs->backup_boot_sector = 0;
	bs->drive_number = 128;
	bs->boot_sig = 0x29;
	bs->volume_id[0] = 148;
	bs->volume_id[1] = 14;
	bs->volume_id[2] = 13;
	bs->volume_id[3] = 8;
	memcpy(bs->volume_label, "NO NAME     ", 12);
	memcpy(bs->file_system_type, "FAT32   ", 8);
	sect[SECT - 2] = 0x55;
	sect[SECT - 1] = 0xAA;
}

#define IS_CLUST_OF(clust, entry) ((clust) >= (entry)->priv.first_clust && (clust) <= (entry)->priv.last_reserved)

emfat_entry_t *find_entry(const emfat_t *emfat, uint32_t clust, emfat_entry_t *nearest)
{
	if (nearest == NULL)
		nearest = emfat->priv.entries;

	if (nearest->priv.first_clust > clust)
		while (nearest >= emfat->priv.entries) // backward finding
		{
			if (IS_CLUST_OF(clust, nearest))
				return nearest;
			nearest--;
		}
	else
		while (nearest->name != NULL) // forward finding
		{
			if (IS_CLUST_OF(clust, nearest))
				return nearest;
			nearest++;
		}
	return NULL;
}

void read_fat_sector(emfat_t *emfat, uint8_t *sect, uint32_t index)
{
	emfat_entry_t *le;
	uint32_t *values;
	uint32_t count;
	uint32_t curr;

	values = (uint32_t *)sect;
	curr = index * 128;
	count = 128;

	if (curr == 0)
	{
		*values++ = CLUST_ROOT_END;
		*values++ = 0xFFFFFFFF;
		count -= 2;
		curr += 2;
	}

	le = emfat->priv.last_entry;
	while (count != 0)
	{
		if (!IS_CLUST_OF(curr, le))
		{
			le = find_entry(emfat, curr, le);
			if (le == NULL)
			{
				le = emfat->priv.last_entry;
				*values = CLUST_RESERVED;
				values++;
				count--;
				curr++;
				continue;
			}
		}
		if (le->dir)
		{
			if (curr == le->priv.last_clust)
				*values = CLUST_EOF; else
				*values = curr + 1;
		}
		else
		{
			if (curr == le->priv.last_clust)
				*values = CLUST_EOF; else
			if (curr > le->priv.last_clust)
				*values = CLUST_FREE; else
				*values = curr + 1;
		}
		values++;
		count--;
		curr++;
	}
	emfat->priv.last_entry = le;
}

void fill_entry(dir_entry *entry, const char *name, uint8_t attr, uint32_t clust, uint32_t size)
{
	int i, l, l1, l2;
	int dot_pos;

	memset(entry, 0, sizeof(dir_entry));

	l = strlen(name);
	dot_pos = -1;
	if ((attr & ATTR_DIR) == 0)
		for (i = l - 1; i >= 0; i--)
			if (name[i] == '.')
			{
				dot_pos = i;
				break;
			}
	if (dot_pos == -1)
	{
		l1 = l > FILE_NAME_SHRT_LEN ? FILE_NAME_SHRT_LEN : l;
		l2 = 0;
	}
	else
	{
		l1 = dot_pos;
		l1 = l1 > FILE_NAME_SHRT_LEN ? FILE_NAME_SHRT_LEN : l1;
		l2 = l - dot_pos - 1;
		l2 = l2 > FILE_NAME_EXTN_LEN ? FILE_NAME_EXTN_LEN : l2;
	}
	memset(entry->name, ' ', FILE_NAME_SHRT_LEN + FILE_NAME_EXTN_LEN);
	memcpy(entry->name, name, l1);
	memcpy(entry->extn, name + dot_pos + 1, l2);
	for (i = 0; i < FILE_NAME_SHRT_LEN + FILE_NAME_EXTN_LEN; i++)
		if (entry->name[i] >= 'a' && entry->name[i] <= 'z')
			entry->name[i] -= 0x20;

	entry->attr = attr;
	entry->reserved = 24;
	entry->strt_clus_hword = clust >> 16;
	entry->strt_clus_lword = clust;
	entry->size = size;
	return;
}

void fill_dir_sector(emfat_t *emfat, uint8_t *data, emfat_entry_t *entry, uint32_t rel_sect)
{
	dir_entry *de;
	uint32_t avail;

	memset(data, 0, SECT);
	de = (dir_entry *)data;
	avail = SECT;

	if (rel_sect == 0)
	// 1. first sector of directory
	{
		if (entry->priv.top == NULL)
		{
			fill_entry(de++, emfat->vol_label, ATTR_VOL_LABEL, 0, 0);
			avail -= sizeof(dir_entry);
		}
		else
		{
			fill_entry(de++, ".", ATTR_DIR | ATTR_READ, entry->priv.first_clust, 0);
			fill_entry(de++, "..", ATTR_DIR | ATTR_READ, entry->priv.top->priv.first_clust, 0);
			avail -= sizeof(dir_entry) * 2;
		}
		entry = entry->priv.sub;
	}
	else
	// 2. not a first sector
	{
		int n;
		n = rel_sect * (SECT / sizeof(dir_entry));
		n -= entry->priv.top == NULL ? 1 : 2;
		entry = entry->priv.sub;
		while (n > 0 && entry != NULL)
		{
			entry = entry->priv.next;
			n--;
		}
	}
	while (entry != NULL && avail >= sizeof(dir_entry))
	{
		if (entry->dir)
			fill_entry(de++, entry->name, ATTR_DIR | ATTR_READ, entry->priv.first_clust, 0); else
			fill_entry(de++, entry->name, ATTR_ARCHIVE | ATTR_READ, entry->priv.first_clust, entry->curr_size);
		entry = entry->priv.next;
		avail -= sizeof(dir_entry);
	}
}

void read_data_sector(emfat_t *emfat, uint8_t *data, uint32_t rel_sect)
{
	emfat_entry_t *le;
	uint32_t cluster;
	cluster = rel_sect/8 + 2; // + 2; // / 8 + 2;
	rel_sect = rel_sect % 8;

	le = emfat->priv.last_entry;
	if (!IS_CLUST_OF(cluster, le))
	{
		le = find_entry(emfat, cluster, le);
		if (le == NULL)
		{
			int i;
			for (i = 0; i < SECT / 4; i++)
				((uint32_t *)data)[i] = 0xEFBEADDE;
			return;
		}
		emfat->priv.last_entry = le;
	}
	if (le->dir)
	{
		fill_dir_sector(emfat, data, le, rel_sect);
		return;
	}
	if (le->readcb == NULL)
		memset(data, 0, SECT);
	else
	{
		uint32_t offset = cluster - le->priv.first_clust;
		offset = offset * CLUST + rel_sect * SECT;
		//le->readcb(data, SECT, offset + le->offset, le->user_data);   OLD
		le->readcb(data, SECT, le->offset + offset/SECT, le->user_data); // NEW
	}
	return;
}

void emfat_read(emfat_t *emfat, uint8_t *data, uint32_t sector, int num_sectors)
{
	while ((num_sectors) > 0)
	{
		if (sector >= emfat->priv.root_sect){
			read_data_sector(emfat, data, sector - emfat->priv.root_sect);
		}else
		if (sector == 0)
			read_mbr_sector(emfat, data);
		else
		if (sector == emfat->priv.boot_sect)
			read_boot_sector(emfat, data);
		else
		if (sector >= emfat->priv.fat1_sect && sector < emfat->priv.fat2_sect)
			read_fat_sector(emfat, data, sector - emfat->priv.fat1_sect);
		else
		if (sector >= emfat->priv.fat2_sect && sector < emfat->priv.root_sect)
			read_fat_sector(emfat, data, sector - emfat->priv.fat2_sect);
		else
			memset(data, 0, SECT);
		data += SECT;
		num_sectors--;
		sector++;
	}
}

void write_data_sector(emfat_t *emfat, const uint8_t *data, uint32_t rel_sect)
{
	emfat_entry_t *le;
	uint32_t cluster;
	cluster = rel_sect / 8 + 2;
	rel_sect = rel_sect % 8;

	le = emfat->priv.last_entry;
	if (!IS_CLUST_OF(cluster, le))
	{
		le = find_entry(emfat, cluster, le);
		if (le == NULL) return;
		emfat->priv.last_entry = le;
	}
	if (le->dir)
	{
		// TODO: handle changing a filesize
		return;
	}
	if (le->writecb != NULL)
		le->writecb(data, SECT, rel_sect * SECT + le->offset, le->user_data);
}

void write_fat_sector(emfat_t *emfat, const uint8_t *data, uint32_t rel_sect)
{
}

void emfat_write(emfat_t *emfat, const uint8_t *data, uint32_t sector, int num_sectors)
{
	while (num_sectors > 0)
	{
		if (sector >= emfat->priv.root_sect)
			write_data_sector(emfat, data, sector - emfat->priv.root_sect);
		else
		if (sector >= emfat->priv.fat1_sect && sector < emfat->priv.fat2_sect)
			write_fat_sector(emfat, data, sector - emfat->priv.fat1_sect);
		else
		if (sector >= emfat->priv.fat2_sect && sector < emfat->priv.root_sect)
			write_fat_sector(emfat, data, sector - emfat->priv.fat2_sect);
		data += SECT;
		num_sectors--;
		sector++;
	}
}
static USBD_HandleTypeDef hUsbDeviceFS;

void EMFAT_USB_DEVICE_Init(void)
{



  if (USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS) != USBD_OK)
  {
    Error_Handler();
  }
  if (USBD_RegisterClass(&hUsbDeviceFS, &USBD_MSC) != USBD_OK)
  {
    Error_Handler();
  }
  if (USBD_MSC_RegisterStorage(&hUsbDeviceFS, &STORAGE_fops) != USBD_OK)
  {
    Error_Handler();
  }
  if (USBD_Start(&hUsbDeviceFS) != USBD_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN USB_DEVICE_Init_PostTreatment */

  /* USER CODE END USB_DEVICE_Init_PostTreatment */
}

#ifdef __cplusplus
}
#endif
