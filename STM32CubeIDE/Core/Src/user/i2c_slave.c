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
// I2C direction
#define I2C_DIR_SLAVE_TO_MASTER		I2C_DIRECTION_RECEIVE
#define I2C_DIR_MASTER_TO_SLAVE		I2C_DIRECTION_TRANSMIT

/* Private types -------------------------------------------------------------*/
/* Private enum tag ----------------------------------------------------------*/
enum {
	I2C_STATE_NONE,
	I2C_STATE_READY,
	I2C_STATE_SLAVE_ADDR_MATCHED,
	I2C_STATE_REG_ADDR_RECEIVED,
	I2C_STATE_SLAVE_TO_MASTER,
	I2C_STATE_MASTER_TO_SLAVE,
	I2C_STATE_LISTEN_MODE_END,
	I2C_STATE_ERROR_OCCURED,
};

/* Private struct/union tag --------------------------------------------------*/
/* Imported variables --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Imported variables --------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static int i2c_state = I2C_STATE_NONE;
static uint8_t i2cTransferDirection;	// I2C_DIR_MASTER_TO_SLAVE or I2C_DIR_SLAVE_TO_MASTER
static uint8_t i2c_regAddr = 0;
static uint8_t i2c_wdata = 0;

/* Exported functions prototypes----------------------------------------------*/
/* Imported function prototypes ----------------------------------------------*/
extern void StartI2cSlaveTask(void *);

/* Private function prototypes -----------------------------------------------*/
static uint8_t getSlaveToMasterData(uint8_t);
static void setMasterToSlaveData(uint8_t, uint8_t);

/* Exported functions --------------------------------------------------------*/
void StartI2cSlaveTask(void *argument)
{
	while(!initializedRtos) vTaskDelay(1);

	HAL_StatusTypeDef status;
	static char strBuf[100];
	HAL_I2C_StateTypeDef i2cStatus;

	for(;;)
	{
		// Reinitialize I2C
		i2cStatus = HAL_I2C_GetState(I2C_SLAVE_HANDLE_PTR);
		if(i2cStatus == HAL_I2C_STATE_LISTEN || i2cStatus == HAL_I2C_STATE_BUSY_TX_LISTEN || i2cStatus == HAL_I2C_STATE_BUSY_RX_LISTEN) {
			status = HAL_I2C_DisableListen_IT(I2C_SLAVE_HANDLE_PTR);
			if(status != HAL_OK) {
				sprintf(strBuf, "0x%02X,0x%02X\n", status, i2cStatus);
				user_puts(strBuf);
			}
		}
		HAL_I2C_DeInit(I2C_SLAVE_HANDLE_PTR);
		HAL_I2C_Init(I2C_SLAVE_HANDLE_PTR);
		i2c_state = I2C_STATE_READY;
		if(HAL_I2C_GetState(I2C_SLAVE_HANDLE_PTR) != HAL_I2C_STATE_LISTEN) {
			status = HAL_I2C_EnableListen_IT(I2C_SLAVE_HANDLE_PTR);
			if(status != HAL_OK) {
				sprintf(strBuf, "HAL_I2C_EnableListen_IT error:0x%02X\n", status);
				user_puts(strBuf);
			}
		} else {
			sprintf(strBuf, "HAL_I2C_GetState error:0x%02X\n", HAL_I2C_GetState(I2C_SLAVE_HANDLE_PTR));
			user_puts(strBuf);
		}

		// I2C slave device sequence
		bool seqence_end = false;
		while(seqence_end == false)
		{
			vTaskDelay(1);
			switch(i2c_state)
			{
			case I2C_STATE_ERROR_OCCURED:
				seqence_end = true;
				break;
			case I2C_STATE_LISTEN_MODE_END:
				seqence_end = true;
				break;
			case I2C_STATE_NONE:
			case I2C_STATE_READY:
			case I2C_STATE_SLAVE_ADDR_MATCHED:
			case I2C_STATE_REG_ADDR_RECEIVED:
			case I2C_STATE_SLAVE_TO_MASTER:
			case I2C_STATE_MASTER_TO_SLAVE:
			default:
				break;
			}
		}
	}
}


void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode)
{
	static uint8_t i2cBuf;
	static char strBuf[50];
	if (AddrMatchCode != (I2C_SLAVE_DEV_ADDR << 1)) {
		user_puts("ISR(S)\tAddr match error");
		return;
	}
	i2cTransferDirection = TransferDirection;
	switch(i2c_state)
	{
	case I2C_STATE_READY:
		i2c_state = I2C_STATE_SLAVE_ADDR_MATCHED;
		if(i2cTransferDirection == I2C_DIR_SLAVE_TO_MASTER) {
			// In the initial state, the next communication direction must be Tx mode ??
			user_puts("ISR(S)\tAddrCallback error(I2C_STATE_READY)\n");
			return;
		}

		// Read register address
		HAL_I2C_Slave_Seq_Receive_IT(I2C_SLAVE_HANDLE_PTR, &i2c_regAddr, 1, I2C_FIRST_FRAME);
		user_puts("ISR(S)\tREADY -> SLAVE_ADDR_MATCHED\n");
		break;

	case I2C_STATE_REG_ADDR_RECEIVED:
		// start MemRead
		i2c_state = I2C_STATE_SLAVE_TO_MASTER;

		// When AddressCallback is called after receiving a register address, it must be Rx mode for restart ??
		if(i2cTransferDirection == I2C_DIR_MASTER_TO_SLAVE) {
			user_puts("ISR(S)\tAddrCallback error(I2C_STATE_REG_ADDR_RECEIVED)\n");
			return;
		}

		i2cBuf = getSlaveToMasterData(i2c_regAddr);
		sprintf(strBuf, "ISR(S)\tREG_RECEIVED -> SLAVE_TO_MASTER,0x%02X\n", i2cBuf);
		HAL_I2C_Slave_Seq_Transmit_IT(I2C_SLAVE_HANDLE_PTR, &i2cBuf, 1, I2C_NEXT_FRAME);
		i2c_regAddr++;
		user_puts(strBuf);
		break;

	default:
		sprintf(strBuf, "ISR(S)\tAddrCallback error,%d\n", i2c_state);
		user_puts(strBuf);
		break;
	}
}


void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	static char strBuf[50];
	switch(i2c_state)
	{
	case I2C_STATE_SLAVE_ADDR_MATCHED:
		// Complete to read register address
		i2c_state = I2C_STATE_REG_ADDR_RECEIVED;
		sprintf(strBuf, "ISR(S)\tSLAVE_ADDR_MATCHED -> REG_RECEIVED,0x%02X\n", i2c_regAddr);
		user_puts(strBuf);

		// From here, branch to MemWrite mode and MemRead mode

		// Waiting to receive data for MemWrite mode
		HAL_I2C_Slave_Seq_Receive_IT(I2C_SLAVE_HANDLE_PTR, &i2c_wdata, 1, I2C_NEXT_FRAME);
		break;

	case I2C_STATE_REG_ADDR_RECEIVED:
		// MemWrite mode: first byte
		i2c_state = I2C_STATE_MASTER_TO_SLAVE;
		sprintf(strBuf, "ISR(S)\tREG_RECEIVED -> MASTER_TO_SLAVE,%02X(%02X)\n", i2c_wdata, i2c_regAddr);
		setMasterToSlaveData(i2c_regAddr, i2c_wdata);
		i2c_regAddr++;
		user_puts(strBuf);

		// Wait for second byte to be read
		HAL_I2C_Slave_Seq_Receive_IT(I2C_SLAVE_HANDLE_PTR, &i2c_wdata, 1, I2C_NEXT_FRAME);
		break;

	case I2C_STATE_MASTER_TO_SLAVE:
		// MemWrite mode:　After 2nd bytes
		sprintf(strBuf, "ISR(S)\tMASTER_TO_SLAVE,%02X(%02X)\n", i2c_wdata, i2c_regAddr);
		setMasterToSlaveData(i2c_regAddr, i2c_wdata);
		i2c_regAddr++;
		user_puts(strBuf);

		// Wait for 3rd and subsequent bytes to be read
		HAL_I2C_Slave_Seq_Receive_IT(I2C_SLAVE_HANDLE_PTR, &i2c_wdata, 1, I2C_NEXT_FRAME);
		break;

	default:
		sprintf(strBuf, "ISR(S)\tSlaveRxCplt error,%d\n", i2c_state);
		user_puts(strBuf);
		break;
	}
}


void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	static uint8_t i2cBuf;
	static char strBuf[50];
	switch(i2c_state)
	{
	case I2C_STATE_SLAVE_TO_MASTER:
		// MemRead mode: After 2nd bytes
		i2cBuf = getSlaveToMasterData(i2c_regAddr);
		HAL_I2C_Slave_Seq_Transmit_IT(I2C_SLAVE_HANDLE_PTR, &i2cBuf, 1, I2C_NEXT_FRAME);
		i2c_regAddr++;
		sprintf(strBuf, "ISR(S)\tSLAVE_TO_MASTER,0x%02X\n", i2cBuf);
		user_puts(strBuf);
		break;

	default:
		sprintf(strBuf, "ISR(S)\tSlaveTxCplt error,%d\n", i2c_state);
		user_puts(strBuf);
		break;
	}
}


void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c)
{
	static char strBuf[50];
	switch(i2c_state)
	{
	case I2C_STATE_SLAVE_TO_MASTER:
		// Communication completes
		i2c_state = I2C_STATE_LISTEN_MODE_END;
		HAL_I2C_DisableListen_IT(I2C_SLAVE_HANDLE_PTR);
		user_puts("ISR(S)\tSLAVE_TO_MASTER -> LISTEN_MODE_END\n");
		break;

	case I2C_STATE_MASTER_TO_SLAVE:
		// Communication completes
		i2c_state = I2C_STATE_LISTEN_MODE_END;
		HAL_I2C_DisableListen_IT(I2C_SLAVE_HANDLE_PTR);
		user_puts("ISR(S)\tMASTER_TO_SLAVE -> LISTEN_MODE_END\n");
		break;

	case I2C_STATE_ERROR_OCCURED:
		i2c_state = I2C_STATE_LISTEN_MODE_END;
		user_puts("ISR(S)\tERROR -> LISTEN_MODE_END\n");
		break;

	default:
		sprintf(strBuf, "ISR(S)\tListenCplt error,%d\n", i2c_state);
		user_puts(strBuf);
		break;
	}
}


void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{

	i2c_state = I2C_STATE_ERROR_OCCURED;
	if(HAL_I2C_GetError(I2C_SLAVE_HANDLE_PTR) == HAL_I2C_ERROR_AF) {
		// In case of ACKF error, terminate communication
		HAL_I2C_DisableListen_IT(I2C_SLAVE_HANDLE_PTR);
//		user_puts("ISR(S)\tACKF Error\n");
		return;
	}
	static char strBuf[50];
	sprintf(strBuf, "ISR(S)\tError,%d\n", (int)(HAL_I2C_GetError(I2C_SLAVE_HANDLE_PTR)));
	user_puts(strBuf);
}

/* Private functions ---------------------------------------------------------*/
static void setMasterToSlaveData(uint8_t regAddr, uint8_t data)
{
	switch(regAddr)
	{
	case I2C_REG_WRITE_1:
		writtenData[0] = data;
		break;
	case I2C_REG_WRITE_2:
		writtenData[1] = data;
		break;
	case I2C_REG_READ_1:
	case I2C_REG_READ_2:
	default:
		writtenData[0] = 0xFF;
		writtenData[1] = 0xFF;
	}
}


static uint8_t getSlaveToMasterData(uint8_t regAddr)
{
	uint8_t data;
	switch(regAddr)
	{
	case I2C_REG_READ_1:
		data = I2C_READ_VALUE_1;
		break;
	case I2C_REG_READ_2:
		data = I2C_READ_VALUE_2;
		break;
	case I2C_REG_WRITE_1:
	case I2C_REG_WRITE_2:
	default:
		data = 0xFF;
	}
	return data;
}
/***************************************************************END OF FILE****/
