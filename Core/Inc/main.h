/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
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
#include "stm32h7xx_hal.h"

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
// void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai);
// void HAL_SAI_RxCpltCallback (SAI_HandleTypeDef *hsai);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC

#define LCD_DISP_Pin GPIO_PIN_3
#define LCD_DISP_GPIO_Port GPIOG

#define TP_INT_Pin GPIO_PIN_7
#define TP_INT_GPIO_Port GPIOH

#define TP_RST_Pin GPIO_PIN_11
#define TP_RST_GPIO_Port GPIOD

#define LCD_BL_Pin GPIO_PIN_6
#define LCD_BL_GPIO_Port GPIOH

#define TP_INTH7_Pin GPIO_PIN_7
#define TP_INTH7_GPIO_Port GPIOH

#define KEY1_Pin 			GPIO_PIN_3
#define KEY1_Port 		GPIOI

#define KEY2_Pin 			GPIO_PIN_4
#define KEY2_Port 		GPIOH

#define KEY3_Pin 			GPIO_PIN_14
#define KEY3_Port 		GPIOH

#define AUDIO_CS_Pin 			GPIO_PIN_3
#define AUDIO_CS_Port 		GPIOD

#define AUDIO_OUTSEL_Pin 			GPIO_PIN_1
#define AUDIO_OUTSEL_Port 		GPIOA

/* Bootloader info */
#pragma pack(1)
typedef struct
{
  uint32_t appKey;
  uint32_t hwKey;
  uint32_t appVersion;
  uint8_t date[12];
  uint8_t time[9];
  uint8_t res[3];
}AppInfo_t;
#pragma pack(0)

// #define ENABLE_SD_DMA_CACHE_MAINTENANCE 1
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
