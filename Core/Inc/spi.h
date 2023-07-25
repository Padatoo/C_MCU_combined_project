/**
  ******************************************************************************
  * @file    spi.h
  * @brief   This file contains all the function prototypes for
  *          the spi.c file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SPI_H__
#define __SPI_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f1xx_hal.h"
#include "main.h"
#include "integer.h"

static __INLINE uint8_t SPI_Send(SPI_TypeDef* SPIx, uint8_t data) {
				uint32_t start_timer;
				start_timer = HAL_GetTick();
        SPIx->DR = data;      
        while ((!(SPIx->SR & SPI_SR_RXNE)) && ((HAL_GetTick() - start_timer) < 200)); // 
        while (((SPIx->SR & SPI_SR_BSY))  && ((HAL_GetTick() - start_timer) < 200));  // 200 ms timeout for combined while loops
        return SPIx->DR;
}

void SPI_SendMulti(SPI_TypeDef* SPIx, uint8_t* dataOut, uint8_t* dataIn, uint32_t count);

void SPI_WriteMulti(SPI_TypeDef* SPIx, const BYTE* dataOut, uint32_t count);

void SPI_ReadMulti(SPI_TypeDef* SPIx, uint8_t *dataIn, uint8_t dummy, uint32_t count);

static __INLINE uint16_t SPI_Send16(SPI_TypeDef* SPIx, uint16_t data) {
				uint32_t start_timer;
				start_timer = HAL_GetTick();
        SPIx->DR = data;      
        while ((!(SPIx->SR & SPI_SR_RXNE)) && ((HAL_GetTick() - start_timer) < 200)); //
        while (((SPIx->SR & SPI_SR_BSY)) && ((HAL_GetTick() - start_timer) < 200));  // 200 ms combined timeout for both while loops
         
	return SPIx->DR;
}	

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern SPI_HandleTypeDef hspi1;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_SPI1_Init(void);

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __SPI_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
