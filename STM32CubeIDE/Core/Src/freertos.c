/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include "usart.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for I2cMasterTask */
osThreadId_t I2cMasterTaskHandle;
uint32_t I2cMasterTaskBuffer[ 1200 ];
osStaticThreadDef_t I2cMasterTaskControlBlock;
const osThreadAttr_t I2cMasterTask_attributes = {
  .name = "I2cMasterTask",
  .cb_mem = &I2cMasterTaskControlBlock,
  .cb_size = sizeof(I2cMasterTaskControlBlock),
  .stack_mem = &I2cMasterTaskBuffer[0],
  .stack_size = sizeof(I2cMasterTaskBuffer),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for I2cSlaveTask */
osThreadId_t I2cSlaveTaskHandle;
uint32_t I2cSlaveTaskBuffer[ 1200 ];
osStaticThreadDef_t I2cSlaveTaskControlBlock;
const osThreadAttr_t I2cSlaveTask_attributes = {
  .name = "I2cSlaveTask",
  .cb_mem = &I2cSlaveTaskControlBlock,
  .cb_size = sizeof(I2cSlaveTaskControlBlock),
  .stack_mem = &I2cSlaveTaskBuffer[0],
  .stack_size = sizeof(I2cSlaveTaskBuffer),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for UsbSendTask */
osThreadId_t UsbSendTaskHandle;
uint32_t UsbSendTaskBuffer[ 2000 ];
osStaticThreadDef_t UsbSendTaskControlBlock;
const osThreadAttr_t UsbSendTask_attributes = {
  .name = "UsbSendTask",
  .cb_mem = &UsbSendTaskControlBlock,
  .cb_size = sizeof(UsbSendTaskControlBlock),
  .stack_mem = &UsbSendTaskBuffer[0],
  .stack_size = sizeof(UsbSendTaskBuffer),
  .priority = (osPriority_t) osPriorityLow,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartI2cMasterTask(void *argument);
void StartI2cSlaveTask(void *argument);
void StartUsbSendTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void vApplicationTickHook(void);
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);
void vApplicationMallocFailedHook(void);
void vApplicationDaemonTaskStartupHook(void);

/* USER CODE BEGIN 3 */

/* USER CODE END 3 */

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN 5 */

/* USER CODE END 5 */

/* USER CODE BEGIN DAEMON_TASK_STARTUP_HOOK */

/* USER CODE END DAEMON_TASK_STARTUP_HOOK */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of I2cMasterTask */
  I2cMasterTaskHandle = osThreadNew(StartI2cMasterTask, NULL, &I2cMasterTask_attributes);

  /* creation of I2cSlaveTask */
  I2cSlaveTaskHandle = osThreadNew(StartI2cSlaveTask, NULL, &I2cSlaveTask_attributes);

  /* creation of UsbSendTask */
  UsbSendTaskHandle = osThreadNew(StartUsbSendTask, NULL, &UsbSendTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartI2cMasterTask */
/**
  * @brief  Function implementing the I2cMasterTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartI2cMasterTask */
__weak void StartI2cMasterTask(void *argument)
{
  /* USER CODE BEGIN StartI2cMasterTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartI2cMasterTask */
}

/* USER CODE BEGIN Header_StartI2cSlaveTask */
/**
* @brief Function implementing the I2cSlaveTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartI2cSlaveTask */
__weak void StartI2cSlaveTask(void *argument)
{
  /* USER CODE BEGIN StartI2cSlaveTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartI2cSlaveTask */
}

/* USER CODE BEGIN Header_StartUsbSendTask */
/**
* @brief Function implementing the UsbSendTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartUsbSendTask */
__weak void StartUsbSendTask(void *argument)
{
  /* USER CODE BEGIN StartUsbSendTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartUsbSendTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

