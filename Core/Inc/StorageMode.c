#include "StorageMode.h"
#include "usbd_msc.h"


#define STORAGE_LUN_NBR                  1 
#define USBD_STD_INQUIRY_LENGTH         36
emfat_t emfat;

static int8_t  STORAGE_Inquirydata_FS[] = 
{
	0x00, 0x80, 0x02, 0x02,
  (USBD_STD_INQUIRY_LENGTH - 5),
  0x00, 0x00, 0x00,
  'A', 'S', 'C', 'A', 'M', ' ', ' ', ' ', // Manufacturer : 8 bytes
  'V', 'I', 'R', 'T', 'U', 'A', 'L', ' ', // Product      : 16 Bytes
  'F', 'A', 'T', ' ', ' ', ' ', ' ', ' ', //
  ' ', ' ', ' ' ,' ',                     // Version      : 4 Bytes
};

int8_t STORAGE_Init_FS(uint8_t lun)
{
	return (USBD_OK);
}

int8_t STORAGE_GetCapacity_FS(uint8_t lun, uint32_t *block_num, uint16_t *block_size)
{
  *block_num = 8192;
	*block_size = 512;
  return (USBD_OK);
}

int8_t STORAGE_IsReady_FS(uint8_t lun)
{
  return (USBD_OK);
}

int8_t STORAGE_IsWriteProtected_FS(uint8_t lun)
{
	return (USBD_OK);
}

int8_t STORAGE_Read_FS(
	uint8_t lun,        // logical unit number
	uint8_t *buf,       // Pointer to the buffer to save data
	uint32_t blk_addr,  // address of 1st block to be read
	uint16_t blk_len)   // nmber of blocks to be read
{
	emfat_read(&emfat, buf, blk_addr, blk_len);
	return (USBD_OK);
}

int8_t STORAGE_Write_FS(uint8_t lun,
	uint8_t *buf,
	uint32_t blk_addr,
	uint16_t blk_len)
{
	emfat_write(&emfat, buf, blk_addr, blk_len);
	return (USBD_OK);
}

int8_t STORAGE_GetMaxLun_FS(void)
{
  return (STORAGE_LUN_NBR - 1);
}




USBD_StorageTypeDef STORAGE_fops =
{
  STORAGE_Init_FS,
  STORAGE_GetCapacity_FS,
  STORAGE_IsReady_FS,
  STORAGE_IsWriteProtected_FS,
  STORAGE_Read_FS,
  STORAGE_Write_FS,
  STORAGE_GetMaxLun_FS,
  (int8_t *)STORAGE_Inquirydata_FS,
};





void storage_mode_init(void)
{
	
}
