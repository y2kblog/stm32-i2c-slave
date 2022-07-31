/* Include system header files -----------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

/* Include user header files -------------------------------------------------*/
// Peripherals
#include "i2c.h"

#include "common.h"

/* Exported macros -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private types -------------------------------------------------------------*/
/* Private enum tag ----------------------------------------------------------*/
/* Private struct/union tag --------------------------------------------------*/
/* Imported variables --------------------------------------------------------*/
extern TaskHandle_t I2cMasterTaskHandle;

/* Exported variables --------------------------------------------------------*/
/* Imported variables --------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Exported functions prototypes----------------------------------------------*/
bool I2CDevChk(I2C_HandleTypeDef *, uint8_t);

/* Imported function prototypes ----------------------------------------------*/
extern void StartI2cMasterTask(void *);

/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
void StartI2cMasterTask(void *argument)
{
	while(!initializedRtos) vTaskDelay(1);

	HAL_StatusTypeDef status;
	static char strBuf[100];
	uint8_t data[2] = {I2C_WRITE_VALUE_1, I2C_WRITE_VALUE_2};

	vTaskDelay(500);	// Wait until I2C slave initialization
	user_puts("StartI2cMasterTask\n\n");


	user_puts("M\tDevice ready check\n");
	status = HAL_I2C_IsDeviceReady(I2C_MASTER_HANDLE_PTR, I2C_SLAVE_DEV_ADDR << 1, 3, 1000);
	if(status != HAL_OK) {
		sprintf(strBuf, "HAL_I2C_IsDeviceReady error:%d\n", status);
		user_puts(strBuf);
	} else {
		user_puts("M\tI2C device is ready\n");
	}
	vTaskDelay(100);


	user_puts("\nM\tDevice existence check\n");
	if(I2CDevChk(I2C_MASTER_HANDLE_PTR, I2C_SLAVE_DEV_ADDR << 1)) {
		user_puts("M\tFound I2C device\n");
	} else {
		user_puts("M\tNot found I2C device\n");
	}
	vTaskDelay(100);


	user_puts("\nM\tMem Read 1byte start\n");
	status = HAL_I2C_Mem_Read_IT(I2C_MASTER_HANDLE_PTR, I2C_SLAVE_DEV_ADDR << 1, I2C_REG_READ_1, I2C_MEMADD_SIZE_8BIT, data, 1);
	if(status != HAL_OK) {
		sprintf(strBuf, "HAL_I2C_Mem_Read_IT error:%d\n", status);
		user_puts(strBuf);
	}
	ulTaskNotifyTake(pdTRUE, 1000);
	sprintf(strBuf, "M\tMem Read:0x%02X. %s\n", data[0], data[0] == I2C_READ_VALUE_1 ? "OK" : "NG");
	user_puts(strBuf);
	vTaskDelay(100);


	user_puts("\nM\tMem Read 2bytes start\n");
	status = HAL_I2C_Mem_Read_DMA(I2C_MASTER_HANDLE_PTR, I2C_SLAVE_DEV_ADDR << 1, I2C_REG_READ_1, I2C_MEMADD_SIZE_8BIT, data, 2);
	if(status != HAL_OK) {
		sprintf(strBuf, "HAL_I2C_Mem_Read_IT error:%d\n", status);
		user_puts(strBuf);
	}
	ulTaskNotifyTake(pdTRUE, 1000);
	sprintf(strBuf, "M\tMem Read:0x%02X, 0x%02X. %s\n", data[0], data[1], data[0] == I2C_READ_VALUE_1 && data[1] == I2C_READ_VALUE_2 ? "OK" : "NG");
	user_puts(strBuf);
	vTaskDelay(100);


	user_puts("\nM\tMem Write 1byte start\n");
	memset(writtenData, 0, I2C_WRITTENDATA_SIZE);
	data[0] = I2C_WRITE_VALUE_1;
	status = HAL_I2C_Mem_Write_DMA(I2C_MASTER_HANDLE_PTR, I2C_SLAVE_DEV_ADDR << 1, I2C_REG_WRITE_1, I2C_MEMADD_SIZE_8BIT, data, 1);
	if(status != HAL_OK) {
		sprintf(strBuf, "HAL_I2C_Mem_Write_IT error:%d\n", status);
		user_puts(strBuf);
	}
	ulTaskNotifyTake(pdTRUE, 1000);
	sprintf(strBuf, "M\tMem write, %s\n", writtenData[0] == I2C_WRITE_VALUE_1 ? "OK" : "NG");
	user_puts(strBuf);
	vTaskDelay(100);


	user_puts("\nM\tMem Write 2bytes start\n");
	memset(writtenData, 0, I2C_WRITTENDATA_SIZE);
	data[0] = I2C_WRITE_VALUE_1; data[1] = I2C_WRITE_VALUE_2;
	status = HAL_I2C_Mem_Write_DMA(I2C_MASTER_HANDLE_PTR, I2C_SLAVE_DEV_ADDR << 1, I2C_REG_WRITE_1, I2C_MEMADD_SIZE_8BIT, data, 2);
	if(status != HAL_OK) {
		sprintf(strBuf, "HAL_I2C_Mem_Write_IT error:%d\n", status);
		user_puts(strBuf);
	}
	ulTaskNotifyTake(pdTRUE, 1000);
	sprintf(strBuf, "M\tMem write2, %s\n", writtenData[0] == I2C_WRITE_VALUE_1 && writtenData[1] == I2C_WRITE_VALUE_2 ? "OK" : "NG");
	user_puts(strBuf);


	user_puts("\nTests end!\n");

	for(;;) {
		vTaskDelay(100);
	}
}


bool I2CDevChk(I2C_HandleTypeDef *hi2c, uint8_t address)
{
    HAL_StatusTypeDef res = HAL_I2C_Master_Transmit(hi2c, address, (uint8_t*)0x00, 0, 1000);
    if(res == HAL_OK)
        return true;
    else
    {
        if(res != HAL_ERROR)
            printf("I2C device check error : status = %d\r\n", res);
        return false;
    }
}


void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	user_puts("ISR(M)\tMT\n");
}


void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	user_puts("ISR(M)\tMemTxCplt\n");
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	vTaskNotifyGiveFromISR(I2cMasterTaskHandle, &xHigherPriorityTaskWoken );
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}


void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	user_puts("ISR(M)\tMemRxCplt\n");
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	vTaskNotifyGiveFromISR(I2cMasterTaskHandle, &xHigherPriorityTaskWoken );
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/* Private functions ---------------------------------------------------------*/

/***************************************************************END OF FILE****/
