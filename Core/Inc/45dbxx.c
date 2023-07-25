#include "45dbxx.h"
#include "45dbxxConfig.h"
#include "spi.h"
#include "string.h"

#define	_45DBXX_DELAY(x)	HAL_Delay(x)



/* Read commands */
#define at45DB_RDMN          0xd2 /* Main Memory Page Read */
#define at45DB_RDARRY        0xe8 /* Continuous Array Read (Legacy Command) */
#define at45DB_RDARRAYLF     0x03 /* Continuous Array Read (Low Frequency) */
#define at45DB_RDARRAYHF     0x0b /* Continuous Array Read (High Frequency) */
#define at45DB_RDBF1LF       0xd1 /* Buffer 1 Read (Low Frequency) */
#define at45DB_RDBF2LF       0xd3 /* Buffer 2 Read (Low Frequency) */
#define at45DB_RDBF1         0xd4 /* Buffer 1 Read */
#define at45DB_RDBF2         0xd6 /* Buffer 2 Read */

/* Program and Erase Commands */
#define at45DB_WRBF1         0x84 /* Buffer 1 Write */
#define at45DB_WRBF2         0x87 /* Buffer 2 Write */
#define at45DB_BF1TOMNE      0x83 /* Buffer 1 to Main Memory Page Program with Built-in Erase */
#define at45DB_BF2TOMNE      0x86 /* Buffer 2 to Main Memory Page Program with Built-in Erase */
#define at45DB_BF1TOMN       0x88 /* Buffer 1 to Main Memory Page Program without Built-in Erase */
#define at45DB_BF2TOMN       0x89 /* Buffer 2 to Main Memory Page Program without Built-in Erase  */
#define at45DB_PGERASE       0x81 /* Page Erase */
#define at45DB_BLKERASE      0x50 /* Block Erase */
#define at45DB_SECTERASE     0x7c /* Sector Erase */
#define at45DB_CHIPERASE1    0xc7 /* Chip Erase - byte 1 */
#  define at45DB_CHIPERASE2  0x94 /* Chip Erase - byte 2 */
#  define at45DB_CHIPERASE3  0x80 /* Chip Erase - byte 3 */
#  define at45DB_CHIPERASE4  0x9a /* Chip Erase - byte 4 */
#define at45DB_MNTHRUBF1     0x82 /* Main Memory Page Program Through Buffer 1 */
#define at45DB_MNTHRUBF2     0x85 /* Main Memory Page Program Through Buffer 2 */

/* Protection and Security Commands */
#define at45DB_ENABPROT1     0x3d /* Enable Sector Protection - byte 1 */
#  define at45DB_ENABPROT2   0x2a /* Enable Sector Protection - byte 2 */
#  define at45DB_ENABPROT3   0x7f /* Enable Sector Protection - byte 3 */
#  define at45DB_ENABPROT4   0xa9 /* Enable Sector Protection - byte 4 */
#define at45DB_DISABPROT1    0x3d /* Disable Sector Protection - byte 1 */
#  define at45DB_DISABPROT2  0x2a /* Disable Sector Protection - byte 2 */
#  define at45DB_DISABPROT3  0x7f /* Disable Sector Protection - byte 3 */
#  define at45DB_DISABPROT4  0x9a /* Disable Sector Protection - byte 4 */
#define at45DB_ERASEPROT1    0x3d /* Erase Sector Protection Register - byte 1 */
#  define at45DB_ERASEPROT2  0x2a /* Erase Sector Protection Register - byte 2 */
#  define at45DB_ERASEPROT3  0x7f /* Erase Sector Protection Register - byte 3 */
#  define at45DB_ERASEPROT4  0xcf /* Erase Sector Protection Register - byte 4 */
#define at45DB_PROGPROT1     0x3d /* Program Sector Protection Register - byte 1 */
#  define at45DB_PROGPROT2   0x2a /* Program Sector Protection Register - byte 2 */
#  define at45DB_PROGPROT3   0x7f /* Program Sector Protection Register - byte 3 */
#  define at45DB_PROGPROT4   0xfc /* Program Sector Protection Register - byte 4 */
#define at45DB_RDPROT        0x32 /* Read Sector Protection Register */
#define at45DB_LOCKDOWN1     0x3d /* Sector Lockdown - byte 1 */
#  define at45DB_LOCKDOWN2   0x2a /* Sector Lockdown - byte 2 */
#  define at45DB_LOCKDOWN3   0x7f /* Sector Lockdown - byte 3 */
#  define at45DB_LOCKDOWN4   0x30 /* Sector Lockdown - byte 4 */
#define at45DB_RDLOCKDOWN    0x35 /* Read Sector Lockdown Register  */
#define at45DB_PROGSEC1      0x9b /* Program Security Register - byte 1 */
#  define at45DB_PROGSEC2    0x00 /* Program Security Register - byte 2 */
#  define at45DB_PROGSEC3    0x00 /* Program Security Register - byte 3 */
#  define at45DB_PROGSEC4    0x00 /* Program Security Register - byte 4 */
#define at45DB_RDSEC         0x77 /* Read Security Register */

/* Additional commands */
#define at45DB_MNTOBF1XFR    0x53 /* Main Memory Page to Buffer 1 Transfer */
#define at45DB_MNTOBF2XFR    0x55 /* Main Memory Page to Buffer 2 Transfer */
#define at45DB_MNBF1CMP      0x60 /* Main Memory Page to Buffer 1 Compare  */
#define at45DB_MNBF2CMP      0x61 /* Main Memory Page to Buffer 2 Compare */
#define at45DB_AUTOWRBF1     0x58 /* Auto Page Rewrite through Buffer 1 */
#define at45DB_AUTOWRBF2     0x59 /* Auto Page Rewrite through Buffer 2 */
#define at45DB_PWRDOWN       0xb9 /* Deep Power-down */
#define at45DB_resume        0xab /* resume from Deep Power-down */
#define at45DB_RDSR          0xd7 /* Status Register Read */
#define at45DB_RDDEVID       0x9f /* Manufacturer and Device ID Read */

#define at45DB_MANUFACTURER  0x1f /* Manufacturer ID: atmel */
#define at45DB_DEVID1_CAPMSK 0x1f /* Bits 0-4: Capacity */
#define at45DB_DEVID1_1MBIT  0x02 /* xxx0 0010 = 1Mbit at45DB011 */
#define at45DB_DEVID1_2MBIT  0x03 /* xxx0 0012 = 2Mbit at45DB021 */
#define at45DB_DEVID1_4MBIT  0x04 /* xxx0 0100 = 4Mbit at45DB041 */
#define at45DB_DEVID1_8MBIT  0x05 /* xxx0 0101 = 8Mbit at45DB081 */
#define at45DB_DEVID1_16MBIT 0x06 /* xxx0 0110 = 16Mbit at45DB161 */
#define at45DB_DEVID1_32MBIT 0x07 /* xxx0 0111 = 32Mbit at45DB321 */
#define at45DB_DEVID1_64MBIT 0x08 /* xxx0 1000 = 32Mbit at45DB641 */
#define at45DB_DEVID1_FAMMSK 0xe0 /* Bits 5-7: Family */
#define at45DB_DEVID1_DFLASH 0x20 /* 001x xxxx = Dataflash */
#define at45DB_DEVID1_at26DF 0x40 /* 010x xxxx = at26DFxxx series (Not supported) */
#define at45DB_DEVID2_VERMSK 0x1f /* Bits 0-4: MLC mask */
#define at45DB_DEVID2_MLCMSK 0xe0 /* Bits 5-7: MLC mask */

/* Status register bit definitions */
#define at45DB_SR_RDY       (1 << 7) /* Bit 7: RDY/ Not BUSY */
#define at45DB_SR_COMP      (1 << 6) /* Bit 6: COMP */
#define at45DB_SR_PROTECT   (1 << 1) /* Bit 1: PROTECT */
#define at45DB_SR_PGSIZE    (1 << 0) /* Bit 0: PAGE_SIZE */

//################################################################################################################

at45dbxx_t	at45dbxx;


//################################################################################################################
uint8_t	at45dbxx_Spi(uint8_t	Data)
{
	uint8_t ret=0;
	HAL_SPI_TransmitReceive(&_45DBXX_SPI,&Data,&ret,1,100);
	return ret;
}
//################################################################################################################
uint8_t at45dbxx_read_status(void) 
{ 
	uint8_t	status	= 0;
  HAL_GPIO_WritePin(_45DBXX_CS_GPIO,_45DBXX_CS_PIN,GPIO_PIN_RESET);
  at45dbxx_Spi(0xd7);
  status = at45dbxx_Spi(0x00);
  HAL_GPIO_WritePin(_45DBXX_CS_GPIO,_45DBXX_CS_PIN,GPIO_PIN_SET);
	return status; 
}
//################################################################################################################
void at45dbxx_wait_busy(void)
{
	uint8_t	status;
  HAL_GPIO_WritePin(_45DBXX_CS_GPIO,_45DBXX_CS_PIN,GPIO_PIN_RESET);
  at45dbxx_Spi(0xd7);
  status = at45dbxx_Spi(0x00);
  HAL_GPIO_WritePin(_45DBXX_CS_GPIO,_45DBXX_CS_PIN,GPIO_PIN_SET);
	uint32_t tickstart;
	tickstart = HAL_GetTick();
	while(((status & 0x80) == 0) && ((HAL_GetTick() - tickstart) < 200))  // timeout 200ms
	{                              //////////////////   Possible Endless Loop.
	  HAL_GPIO_WritePin(_45DBXX_CS_GPIO,_45DBXX_CS_PIN,GPIO_PIN_RESET);
		at45dbxx_Spi(0xd7);
		status = at45dbxx_Spi(0x00);
		HAL_GPIO_WritePin(_45DBXX_CS_GPIO,_45DBXX_CS_PIN,GPIO_PIN_SET);
	}
}
//################################################################################################################
void at45dbxx_resume(void) 
{
	HAL_GPIO_WritePin(_45DBXX_CS_GPIO,_45DBXX_CS_PIN,GPIO_PIN_RESET);	
	at45dbxx_Spi(at45DB_resume);
  HAL_GPIO_WritePin(_45DBXX_CS_GPIO,_45DBXX_CS_PIN,GPIO_PIN_SET);	
}
//################################################################################################################
void at45dbxx_power_down(void) 
{
	HAL_GPIO_WritePin(_45DBXX_CS_GPIO,_45DBXX_CS_PIN,GPIO_PIN_RESET);	
	at45dbxx_Spi(at45DB_PWRDOWN);
  HAL_GPIO_WritePin(_45DBXX_CS_GPIO,_45DBXX_CS_PIN,GPIO_PIN_SET);	
}
//################################################################################################################
bool		at45dbxx_init(void)
{
	HAL_GPIO_WritePin(_45DBXX_CS_GPIO,_45DBXX_CS_PIN,GPIO_PIN_SET);	
	while(HAL_GetTick() < 20)
		_45DBXX_DELAY(10);
	//uint8_t Temp0 = 0, Temp1 = 0,Temp2=0;
	//HAL_GPIO_WritePin(_45DBXX_CS_GPIO,_45DBXX_CS_PIN,GPIO_PIN_RESET);	
	//at45dbxx_Spi(0x9f);
	//Temp0=at45dbxx_Spi(0xa5);
	//Temp1=at45dbxx_Spi(0xa5);
  //HAL_GPIO_WritePin(_45DBXX_CS_GPIO,_45DBXX_CS_PIN,GPIO_PIN_SET);	
	//Temp2=at45dbxx_read_status();
	
	// at45DB321 528byte page uses shift = 10. 512byte page would use shift = 0.
	
	at45dbxx.FlashSize_MBit = 32;
	at45dbxx.Pages = 8192;
	at45dbxx.Shift = 10;
	at45dbxx.PageSize = 528;
	
	return true;
}
//################################################################################################################
void 		at45dbxx_erase_chip(void)
{
	at45dbxx_resume();
	at45dbxx_wait_busy();
	uint8_t erase_chip_cmd[4];

erase_chip_cmd[0] = 0xc7;
erase_chip_cmd[1] = 0x94;
erase_chip_cmd[2] = 0x80;
erase_chip_cmd[3] = 0x9a;
	HAL_GPIO_WritePin(_45DBXX_CS_GPIO,_45DBXX_CS_PIN,GPIO_PIN_RESET);
	HAL_SPI_Transmit(&_45DBXX_SPI, erase_chip_cmd, sizeof(erase_chip_cmd), 100);
	HAL_GPIO_WritePin(_45DBXX_CS_GPIO,_45DBXX_CS_PIN,GPIO_PIN_SET);		
	at45dbxx_wait_busy();	
}
//################################################################################################################
void 	at45dbxx_erase_page(int page)
{
	uint16_t erase_page = page;
	/////
	uint8_t erase_page_cmd[4];
	
	erase_page_cmd[0] = 0x81;
	erase_page_cmd[1] = (erase_page >> 6) & 0x7F;
	erase_page_cmd[2] = (erase_page << 2) & 0xFC;
	erase_page_cmd[3] = 0x00;
	
	//at45dbxx_resume();
	at45dbxx_wait_busy();     // Possible Endless Loop.
	
	
	HAL_GPIO_WritePin(_45DBXX_CS_GPIO,_45DBXX_CS_PIN,GPIO_PIN_RESET);

	HAL_SPI_Transmit(&_45DBXX_SPI, erase_page_cmd, sizeof(erase_page_cmd), 100);
	
  HAL_GPIO_WritePin(_45DBXX_CS_GPIO,_45DBXX_CS_PIN,GPIO_PIN_SET);
	
}
//################################################################################################################
void		at45dbxx_write_page(uint8_t message[50])
{
///////////////////////
	at45dbxx_wait_busy();  // Possible Endless Loop.
///////////////////////
	uint16_t page_address = 294;
	uint8_t wmsg[50] = "This is a test number ONE_1";


	uint8_t wcmd[4];
	wcmd[0] = 0x82;
	wcmd[1] = (page_address >> 6) & 0x7F;
	wcmd[2] = (page_address << 2) & 0xFC;
	wcmd[3] = 0x00;

	HAL_GPIO_WritePin(_45DBXX_CS_GPIO,_45DBXX_CS_PIN,GPIO_PIN_RESET);

	HAL_SPI_Transmit(&_45DBXX_SPI, wcmd, sizeof(wcmd),100);
	HAL_SPI_Transmit(&_45DBXX_SPI,(uint8_t*)wmsg, sizeof(wmsg),100);
	HAL_GPIO_WritePin(_45DBXX_CS_GPIO,_45DBXX_CS_PIN,GPIO_PIN_SET);

}
//############################################
//uint8_t CDC_Transmit_FS(uint8_t*, uint16_t);
//############################################
	const char*	at45dbxx_read_page(size_t README_SIZE, size_t at45_page)
{	
	///////////////////// 
	//at45dbxx_wait_busy();    // Possible Endless Loop.
	/////////////////////
	uint16_t page_Addr = at45_page;
  char rmsg[512] = {0};
	uint8_t rcmd[8];
	rcmd[0] = 0xD2;
	rcmd[1] = (page_Addr >> 6) & 0x7F;
	rcmd[2] = (page_Addr << 2) & 0xFC;
	rcmd[3] = 0x00;	
	rcmd[4] = 0x00;
	rcmd[5] = 0x00;
	rcmd[6] = 0x00;
	rcmd[7] = 0x00;

	HAL_GPIO_WritePin(_45DBXX_CS_GPIO,_45DBXX_CS_PIN,GPIO_PIN_RESET);
	
	HAL_SPI_Transmit(&_45DBXX_SPI, rcmd, sizeof(rcmd), 100);
	
	HAL_SPI_Receive(&_45DBXX_SPI, (uint8_t*)rmsg, sizeof(rmsg), 100);
  //HAL_SPI_Receive(&_45DBXX_SPI, (uint8_t*)rmsg, sizeof(rmsg), 100);
	HAL_GPIO_WritePin(_45DBXX_CS_GPIO,_45DBXX_CS_PIN,GPIO_PIN_SET);

	char *return_answer = rmsg;
	
	return return_answer;
}

 void at45dbxx_program_byte(int page_address, int byte_address, uint8_t data[], size_t size)
 {
	 
	 uint16_t clocked_page_address = page_address;
	 uint16_t clocked_byte_address = byte_address;
/////////////////////////
   at45dbxx_wait_busy();       // Possible Endless Loop.
/////////////////////////
	 uint8_t wcmd[4];
	 wcmd[0] = 0x02;
	 wcmd[1] = (clocked_page_address >> 6) & 0x7F;
	 wcmd[2] = ((clocked_page_address << 2) & 0xFC) | ((clocked_byte_address >> 8) & 0x03);
	 wcmd[3] =  clocked_byte_address & 0xFF;
	 
	 HAL_GPIO_WritePin(_45DBXX_CS_GPIO, _45DBXX_CS_PIN, GPIO_PIN_RESET);
	 
	 HAL_SPI_Transmit(&_45DBXX_SPI, wcmd, sizeof(wcmd), 100);
	 HAL_SPI_Transmit(&_45DBXX_SPI, data, size, 100);
	 //HAL_SPI_Receive(&_45DBXX_SPI, return_message, sizeof(return_message), 100 )   may need this for error handler later
		 
	 HAL_GPIO_WritePin(_45DBXX_CS_GPIO, _45DBXX_CS_PIN, GPIO_PIN_SET);
	 
 }
//################################################################################################################
 void at45dbxx_read_ring_header(int page_address, int byte_address, uint8_t msg[])
{
	/////////////////////
	at45dbxx_wait_busy();      // Possible Endless Loop.
	/////////////////////
	uint16_t clocked_page_address = page_address;
	uint16_t clocked_byte_address = byte_address;

	uint8_t return_msg[1];
	
	uint8_t read_cmd[8];  // Main Memory Page Read OPcode of 0xD2 is followed by 3 address bytes and 4 dummy (empty) bytes
	read_cmd[0] = 0xD2;
	read_cmd[1] = (clocked_page_address >> 6) & 0x7F;
  read_cmd[2] = ((clocked_page_address << 2) & 0xFC) | ((clocked_byte_address >> 8) & 0x03);
	read_cmd[3] = clocked_byte_address & 0xFF;
		
	read_cmd[4] = 0x00;
	read_cmd[5] = 0x00;
	read_cmd[6] = 0x00;
	read_cmd[7] = 0x00;
		
	HAL_GPIO_WritePin(_45DBXX_CS_GPIO,_45DBXX_CS_PIN,GPIO_PIN_RESET);  // CS_PIN to 0 to enable SPI
		
	HAL_SPI_Transmit(&_45DBXX_SPI, read_cmd, sizeof(read_cmd), 100);                // send command
	HAL_SPI_Receive(&_45DBXX_SPI, (uint8_t*)return_msg, sizeof(return_msg), 100);   // get answer
	HAL_GPIO_WritePin(_45DBXX_CS_GPIO,_45DBXX_CS_PIN,GPIO_PIN_SET);    //CS_PIN to 1 to disable SPI

	msg[0] = return_msg[0];
	//msg[1] = return_msg[1];
	//msg[2] = return_msg[2];
	//msg[3] = return_msg[3];

}


	uint8_t device_id_cmd = 0x9f;
	//char buffer[5];
	
	//HAL_GPIO_WritePin(_45DBXX_CS_GPIO,_45DBXX_CS_PIN,GPIO_PIN_RESET);
	//HAL_SPI_Transmit(&_45DBXX_SPI, rcmd, sizeof(rcmd), 100);
	
	//HAL_SPI_Receive(&_45DBXX_SPI, (uint8_t*)rmsg, sizeof(rmsg), 100);
	//HAL_GPIO_WritePin(_45DBXX_CS_GPIO,_45DBXX_CS_PIN,GPIO_PIN_SET);
	//HAL_Delay(50);
	const char*	at45dbxx_device_id(void)
{	
	/////////////////////
	//at45dbxx_wait_busy();          // Possible Endless Loop.
	/////////////////////
	//uint16_t page_Addr = at45_page;
  char rmsg[5] = {0};
	uint8_t rcmd[1];
	rcmd[0] = 0x9F;

	HAL_GPIO_WritePin(_45DBXX_CS_GPIO,_45DBXX_CS_PIN,GPIO_PIN_RESET);
	
	HAL_SPI_Transmit(&_45DBXX_SPI, rcmd, sizeof(rcmd), 100);
	HAL_SPI_Receive(&_45DBXX_SPI, (uint8_t*)rmsg, sizeof(rmsg), 100);

	HAL_GPIO_WritePin(_45DBXX_CS_GPIO,_45DBXX_CS_PIN,GPIO_PIN_SET);

	char *return_answer = rmsg;
	HAL_Delay(50);
	return return_answer;
}
