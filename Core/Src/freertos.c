/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
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

#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "queue.h"
#include "semphr.h"

#include "audio.h"

#include "fatfs.h"
#include "sai.h"

#include "stm32h7xx_hal.h"

#include "ff.h"

#include <string.h>
#include <stdio.h>

#include "lvgl.h"
#include "gt911.h"

extern SPI_HandleTypeDef hspi1;
extern SAI_HandleTypeDef hsai_BlockB2;

extern uint8_t file_system_err;

extern uint32_t lv_timer_handler(void);

QueueHandle_t xQueue_filecopy = NULL;

SemaphoreHandle_t xSemaphore_spi = NULL;
SemaphoreHandle_t xSemaphore_sdcard = NULL;
SemaphoreHandle_t xSemaphore_usb = NULL;
SemaphoreHandle_t xQ = NULL;

osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
    .name = "defaultTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};

osThreadId_t lvglTaskHandle;
const osThreadAttr_t lvglTask_attributes = {
    .name = "LvglTask",
    .stack_size = 1024 * 10,
    .priority = (osPriority_t)osPriorityNormal,
};

TaskHandle_t audioTaskHandle = NULL;

void LVGLTimer(void * argument);

void StartDefaultTask(void *argument);
void audioTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void MX_FREERTOS_Init(void)
{

  xSemaphore_spi = xSemaphoreCreateMutex();
  xSemaphore_sdcard = xSemaphoreCreateMutex();
  xSemaphore_usb = xSemaphoreCreateMutex();

  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  lvglTaskHandle = osThreadNew(LVGLTimer, NULL, &lvglTask_attributes);
  
  // audioTaskHandle = osThreadNew(audioTask, NULL, &audioTask_attr);

  xTaskCreate ( audioTask, "audioTask", 1024, NULL, 24, &audioTaskHandle);

  
}

void audioTask (void *argument)
{
  xSemaphoreTake(xSemaphore_sdcard, portMAX_DELAY);
  if(sd_card_mount() == FR_OK)
  {
    file_system_err = 0;
  }
  else
  {
    file_system_err = 1;
  }
 

  while (1)
  {
    AudioProcess();
    vTaskSuspend (NULL);
    // osThreadSuspend(audioTaskHandle);
  }
  // HAL_StatusTypeDef HAL_SAI_Transmit_DMA(SAI_HandleTypeDef *hsai, uint8_t *pData, uint16_t Size);
}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */

void StartDefaultTask(void *argument)
{

  for (;;)
  {
		HAL_GPIO_TogglePin(LED_GPIO_Port,LED_Pin);
    gt911_StartConvesion();
    osDelay(33);
  }
}

void LVGLTimer(void * argument)
{
  for(;;)
  {
    lv_timer_handler();
    osDelay(2);
  }
}
