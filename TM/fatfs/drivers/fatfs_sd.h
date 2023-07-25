/*-----------------------------------------------------------------------/
/  Low level disk interface module include file   (C)ChaN, 2013          /
/-----------------------------------------------------------------------*/

#ifndef _DISKIO_DEFINED_SD
#define _DISKIO_DEFINED_SD

#define _USE_WRITE	1	/* 1: Enable disk_write function */
#define _USE_IOCTL	1	/* 1: Enable disk_ioctl fucntion */

#include "diskio.h"
#include "integer.h"

//#include "stm32fxxx_hal.h"
#include "tm_stm32_fatfs.h"
//#include "tm_stm32_spi.h"
//#include "tm_stm32_delay.h"

/* DMA for STM32F4xx and STM32F7xx 
#if defined(STM32F4xx) || defined(STM32F7xx)
#include "tm_stm32_spi_dma.h"
#define FATFS_DMA           1
#else*/
#define FATFS_DMA           0
//#endif

/* SPI settings */
#ifndef FATFS_SPI
#define FATFS_SPI							SPI1
#endif

/* CS pin settings */
#ifndef FATFS_CS_PIN		
#define FATFS_CS_PORT						GPIOA
#define FATFS_CS_PIN						GPIO_PIN_4
#endif

/* CS pin */
#define FATFS_CS_LOW						HAL_GPIO_WritePin(FATFS_CS_PORT, FATFS_CS_PIN,GPIO_PIN_RESET)
#define FATFS_CS_HIGH						HAL_GPIO_WritePin(FATFS_CS_PORT, FATFS_CS_PIN,GPIO_PIN_SET)

#endif

