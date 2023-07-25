/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define EVENTA_Pin GPIO_PIN_13
#define EVENTA_GPIO_Port GPIOC
#define EVENTB_Pin GPIO_PIN_14
#define EVENTB_GPIO_Port GPIOC
#define cs__Pin GPIO_PIN_4
#define cs__GPIO_Port GPIOA
#define case1_Pin GPIO_PIN_4
#define case1_GPIO_Port GPIOC
#define cs_at45__Pin GPIO_PIN_0
#define cs_at45__GPIO_Port GPIOB
#define cs_at45_GPIO_Port GPIOB
#define cs_at45_Pin GPIO_PIN_0
#define shot_B_Pin GPIO_PIN_12
#define shot_B_GPIO_Port GPIOB
#define shtorka_B_Pin GPIO_PIN_13
#define shtorka_B_GPIO_Port GPIOB
#define shot_A_Pin GPIO_PIN_14
#define shot_A_GPIO_Port GPIOB
#define shtorka_A_Pin GPIO_PIN_15
#define shtorka_A_GPIO_Port GPIOB
#define Flash_Light_Pin GPIO_PIN_3
#define Flash_Light_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */
static size_t magic_size_of_entry = 96;
#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
