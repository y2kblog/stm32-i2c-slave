/* Include system header files -----------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

/* Include user header files -------------------------------------------------*/
// Peripherals
#include "usart.h"
#include "i2c.h"

#include "common.h"
#include "builddate.h"

/* Exported macros -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
// UART
#define UART_INSTANCE			USART2
#define UART_HANDLE_PTR			&huart2
#define UART_BUFFER_SIZE		2000

/* Private types -------------------------------------------------------------*/
/* Private enum tag ----------------------------------------------------------*/
/* Private struct/union tag --------------------------------------------------*/
/* Imported variables --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
bool initializedRtos = false;
uint8_t writtenData[I2C_WRITTENDATA_SIZE] = {0};

/* Imported variables --------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static StreamBufferHandle_t UsbTxStreamHandle = NULL;
static bool usart_isBusy = false;

/* Exported functions prototypes----------------------------------------------*/
void user_puts(char *);

/* Imported function prototypes ----------------------------------------------*/
extern void StartUsbSendTask(void *);
extern void vApplicationDaemonTaskStartupHook(void);
extern void vApplicationStackOverflowHook(xTaskHandle, signed char *);
extern void vApplicationMallocFailedHook(void);

/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
void vApplicationDaemonTaskStartupHook(void)
{
	printf("start\n");
	printf("Build date: %s\n", getBuildDate());

	const size_t xTriggerLevel = 1;
	UsbTxStreamHandle = xStreamBufferCreate(UART_BUFFER_SIZE, xTriggerLevel);

	initializedRtos = true;
}


void HAL_I2C_AbortCpltCallback(I2C_HandleTypeDef *hi2c)
{
	if (hi2c->Instance == I2C_MASTER_INSTANCE)
	{
		user_puts("Master Abort\n");
	}
	if (hi2c->Instance == I2C_SLAVE_INSTANCE)
	{
		user_puts("Slave Abort\n");
	}
}


void vApplicationMallocFailedHook(void)
{
	const char strBuf[] = "vApplicationMallocFailedHook is called\n";
	HAL_UART_Transmit(UART_HANDLE_PTR, (const uint8_t *)strBuf, strlen(strBuf), 1000);
	HAL_Delay(1000);
	NVIC_SystemReset();
}


void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{
	static char strBuf[70];
	sprintf(strBuf, "vApplicationStackOverflowHook is called at %s\n", pcTaskName);
	HAL_UART_Transmit(UART_HANDLE_PTR, (const uint8_t *)strBuf, strlen(strBuf), 1000);
	HAL_Delay(1000);
	NVIC_SystemReset();
}


int _write(int fd, char *ptr, int len)
{
	UBaseType_t uxSavedInterruptStatus;
    bool runningFreeRTOS = (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) ? true : false;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	bool isInISR = false;
	if(runningFreeRTOS) isInISR = xPortIsInsideInterrupt();

	if(runningFreeRTOS && UsbTxStreamHandle != NULL) {
		if(isInISR)
			uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();
		else
			taskENTER_CRITICAL();

		size_t xBytesSent;
		xBytesSent = 0;
		while(len != xBytesSent) {
			if(isInISR) {
				xBytesSent += xStreamBufferSendFromISR(
					UsbTxStreamHandle,
					(void *)ptr,
					len,
					&xHigherPriorityTaskWoken);
			} else {
				xBytesSent += xStreamBufferSend(
					UsbTxStreamHandle,
					(void *)ptr,
					len,
					1000);
			}
		}

		if(isInISR)
			taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);
		else
			taskEXIT_CRITICAL();
	} else {
		if(runningFreeRTOS)
			taskENTER_CRITICAL();
		HAL_UART_Transmit(UART_HANDLE_PTR, (uint8_t*) ptr, len, 1000);
		if(runningFreeRTOS)
			taskEXIT_CRITICAL();
	}
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
    return len;
}


void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == UART_INSTANCE)
	{
		usart_isBusy = false;
	}
}


void StartUsbSendTask(void *argument)
{
	while(!initializedRtos) vTaskDelay(1);

	size_t xDataLengthBytes;
	size_t xReceivedBytes;
	static uint8_t usbTxBuffer[UART_BUFFER_SIZE];

	for(;;)
	{
		vTaskDelay(10);

		xDataLengthBytes = xStreamBufferBytesAvailable(UsbTxStreamHandle);
		if(xDataLengthBytes == 0)
			continue;	// No data

		if(usart_isBusy == true)
			continue;

		const TickType_t x10ms = pdMS_TO_TICKS(1000);
		xReceivedBytes = xStreamBufferReceive(
				UsbTxStreamHandle,
				(void *)usbTxBuffer,
				xDataLengthBytes,
				x10ms);

		usart_isBusy = true;
		HAL_UART_Transmit_DMA(UART_HANDLE_PTR, usbTxBuffer, xReceivedBytes);
	}
}


// print function in FreeRTOS
void user_puts(char *str)
{
	size_t len = strlen(str);

	if(xTaskGetSchedulerState() != taskSCHEDULER_RUNNING || UsbTxStreamHandle != NULL) {
		HAL_UART_Transmit(UART_HANDLE_PTR, (uint8_t*) str, len, 1000);
		return;
	}

	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	UBaseType_t uxSavedInterruptStatus;
	bool isInISR = xPortIsInsideInterrupt();
	if(isInISR)
		uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();
	else
		taskENTER_CRITICAL();

	if(isInISR) {
		xStreamBufferSendFromISR(
			UsbTxStreamHandle,
			(void *)str,
			len,
			&xHigherPriorityTaskWoken);
	} else {
		xStreamBufferSend(
			UsbTxStreamHandle,
			(void *)str,
			len,
			1000);
	}

	if(isInISR)
		taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);
	else
		taskEXIT_CRITICAL();
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

/* Private functions ---------------------------------------------------------*/
/***************************************************************END OF FILE****/
