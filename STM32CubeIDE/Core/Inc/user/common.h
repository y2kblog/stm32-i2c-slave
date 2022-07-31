/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __COMMON_H
#define __COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

/* Include system header files -----------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>

/* Include user header files -------------------------------------------------*/
// FreeRTOS
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"
#include "stream_buffer.h"
#include "main.h"
#include "cmsis_os.h"

/* Exported macros -----------------------------------------------------------*/
// I2C master peripheral paramters
#define I2C_MASTER_INSTANCE		I2C1
#define I2C_MASTER_HANDLE_PTR	&hi2c1

// I2C slave peripheral paramters
#define I2C_SLAVE_INSTANCE		I2C3
#define I2C_SLAVE_HANDLE_PTR	&hi2c3

// Test Register address
#define I2C_REG_READ_1			0xC1
#define I2C_REG_READ_2			0xC2
#define I2C_REG_WRITE_1			0xF1
#define I2C_REG_WRITE_2			0xF2

// Test Register value
#define I2C_READ_VALUE_1		0x0A
#define I2C_READ_VALUE_2		0x0B
#define I2C_WRITE_VALUE_1		0x1A
#define I2C_WRITE_VALUE_2		0x1B


#define I2C_WRITTENDATA_SIZE	2

/* Exported types ------------------------------------------------------------*/
/* Exported enum tag ---------------------------------------------------------*/
/* Exported struct/union tag -------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
extern bool initializedRtos;
extern uint8_t writtenData[];

/* Imported variables --------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Exported functions prototypes----------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
extern void user_puts(char *);

/* Private functions ---------------------------------------------------------*/
#ifdef __cplusplus
}
#endif

#endif /* __COMMON_H */
/***************************************************************END OF FILE****/
