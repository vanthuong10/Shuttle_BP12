/*
 * mb_display.c
 *
 *  Created on: May 16, 2025
 *      Author: ADMIN-HPZ2
 */
#include "mb_display.h"
#include "main.h"
#include "user_custom.h"


static struct DisplayHandle {
	uint8_t tx_data[32];
	UART_HandleTypeDef *oled_Serial;
}displayHandle;

void mb_setLed(uint16_t R, uint16_t G, uint16_t B, uint16_t timeout, uint16_t brightness)
{
	displayHandle.tx_data[0] = OLED_ID ;
	displayHandle.tx_data[1] = 0x10 ; // Function code
	displayHandle.tx_data[2] = (LED_START_ADD >> 8) & 0xFF ;  // MSB of register address
	displayHandle.tx_data[3] = LED_START_ADD & 0xFF;          // LSB of register address
	displayHandle.tx_data[4] = (LED_NUM_OF_REG >> 8) & 0xFF;  // MSB of number of registers
	displayHandle.tx_data[5] = LED_NUM_OF_REG & 0xFF;         // LSB of number of registers
	displayHandle.tx_data[6] = 10 ;                 		  //Byte count (5 Registers would need 10 bytes )
	// Data
	displayHandle.tx_data[7]  = (R >> 8) & 0xFF ;
	displayHandle.tx_data[8]  = R & 0xFF ;
	displayHandle.tx_data[9]  = (G >> 8) & 0xFF ;
	displayHandle.tx_data[10] = G & 0xFF ;
	displayHandle.tx_data[11] = (B >> 8) & 0xFF ;
	displayHandle.tx_data[12] = B & 0xFF ;
	displayHandle.tx_data[13] = (timeout >> 8) & 0xFF ;
	displayHandle.tx_data[14] = timeout & 0xFF ;
	displayHandle.tx_data[15] = (brightness >> 8) & 0xFF ;
	displayHandle.tx_data[16] = brightness & 0xFF ;
    // Calculate CRC
	uint16_t crc = crc16_modbus(displayHandle.tx_data, 17);   // Compute CRC for the first 17 bytes
	displayHandle.tx_data[17] = crc & 0xFF ;
	displayHandle.tx_data[18] = (crc >> 8) & 0xFF ;
	HAL_UART_Transmit(displayHandle.oled_Serial, displayHandle.tx_data, 19, 100);
}

void mb_setOled(struct DataInfo *info, struct DataRun *data_run)
{
	displayHandle.tx_data[0] = OLED_ID ;
	displayHandle.tx_data[1] = 0x10 ; // Function code
	displayHandle.tx_data[2] = (OLED_START_ADD >> 8) & 0xFF ;  // MSB of register address
	displayHandle.tx_data[3] = OLED_START_ADD & 0xFF;          // LSB of register address
	displayHandle.tx_data[4] = (OLED_NUM_OF_REG >> 8) & 0xFF;  // MSB of number of registers
	displayHandle.tx_data[5] = OLED_NUM_OF_REG & 0xFF;         // LSB of number of registers
	displayHandle.tx_data[6] = 32 ;                 		  //Byte count (16 Registers would need 32 bytes )
	// Chuyển đổi giá trị
	uint16_t batteryPercent = (uint16_t)(info->batteryPercentage * 100); // Ví dụ: 75.50% -> 7550
	uint16_t bmsTemp = (uint16_t)(info->bmsTemperature * 100);           // Ví dụ: 25.50°C -> 2550
	uint16_t voltage = (uint16_t)(info->Voltage * 100);                 // Ví dụ: 12.34V -> 1234
	uint16_t current = (uint16_t)(info->Current * 100);                 // Ví dụ: 1.23A -> 123
	uint8_t ip[4];
	OLED_U32_DECODE(info->ip32U,ip[0],ip[1], ip[2], ip[3]);
	// Data
	displayHandle.tx_data[7]  = (batteryPercent >> 8) & 0xFF ;
	displayHandle.tx_data[8]  = batteryPercent & 0xFF ;
	displayHandle.tx_data[9]  = ((uint16_t)info->chargeDischargeStatus >> 8) & 0xFF ;
	displayHandle.tx_data[10] = (uint16_t)info->chargeDischargeStatus & 0xFF ;
	displayHandle.tx_data[11] = (bmsTemp >> 8) & 0xFF ;
	displayHandle.tx_data[12] = bmsTemp & 0xFF ;
	displayHandle.tx_data[13] = (voltage >> 8) & 0xFF ;
	displayHandle.tx_data[14] = voltage & 0xFF ;
	displayHandle.tx_data[15] = (current >> 8) & 0xFF ;
	displayHandle.tx_data[16] = current & 0xFF ;
	displayHandle.tx_data[17] = 0 ;
	displayHandle.tx_data[18] = 0x1 ;
	displayHandle.tx_data[19] = ((uint16_t)data_run->shuttleMode >> 8) & 0xFF ;
	displayHandle.tx_data[20] = (uint16_t)data_run->shuttleMode & 0xFF ;
	displayHandle.tx_data[21] = ((uint16_t)TIME_NEXT_SCREEN >> 8) & 0xFF ;
	displayHandle.tx_data[22] = (uint16_t)TIME_NEXT_SCREEN & 0xFF ;
	displayHandle.tx_data[23] = ip[0] ;
	displayHandle.tx_data[24] = ip[1] ;
	displayHandle.tx_data[25] = ip[2] ;
	displayHandle.tx_data[26] = ip[3] ;
	displayHandle.tx_data[27] = db_shuttle_info.errorStatus[0] ;
	displayHandle.tx_data[28] = db_shuttle_info.errorStatus[1] ;
	displayHandle.tx_data[29] = db_shuttle_info.errorStatus[2] ;
	displayHandle.tx_data[30] = db_shuttle_info.errorStatus[3] ;
	displayHandle.tx_data[31] = db_shuttle_info.errorStatus[4] ;
	displayHandle.tx_data[32] = db_shuttle_info.errorStatus[5] ;
	displayHandle.tx_data[33] = db_shuttle_info.errorStatus[6] ;
	displayHandle.tx_data[34] = db_shuttle_info.errorStatus[7] ;
	displayHandle.tx_data[35] = db_shuttle_info.no[0] ;
	displayHandle.tx_data[36] = db_shuttle_info.no[1] ;
	displayHandle.tx_data[37] = db_shuttle_info.no[2] ;
	displayHandle.tx_data[38] = db_shuttle_info.no[3] ;
	// Calculate CRC
	uint16_t crc = crc16_modbus(displayHandle.tx_data, 39);   // Compute CRC for the first 16 bytes
	displayHandle.tx_data[39] = crc & 0xFF ;
	displayHandle.tx_data[40] = (crc >> 8) & 0xFF ;
	HAL_UART_Transmit(displayHandle.oled_Serial, displayHandle.tx_data, 41, 100);
}

void mb_setPage(uint16_t page)
{
	displayHandle.tx_data[0] = OLED_ID ;
	displayHandle.tx_data[1] = 0x06 ; // Function code
	displayHandle.tx_data[2] = (OLED_PAGE_ADD >> 8) & 0xFF ;  // MSB of register address
	displayHandle.tx_data[3] = OLED_PAGE_ADD & 0xFF;          // LSB of register address
	displayHandle.tx_data[4] = (page >> 8) & 0xFF;  // MSB of number of data
	displayHandle.tx_data[5] = page & 0xFF;         // LSB of number of data
    // Calculate CRC
    uint16_t crc = crc16_modbus(displayHandle.tx_data, 6);   // Compute CRC for the first 6 bytes
    displayHandle.tx_data[6] = crc & 0xFF;                  // CRC Low Byte
    displayHandle.tx_data[7] = (crc >> 8) & 0xFF;           // CRC High Byte
	HAL_UART_Transmit(displayHandle.oled_Serial, displayHandle.tx_data, 8, 100);
}

void mb_ledInit(UART_HandleTypeDef *uart)
{
	displayHandle.oled_Serial = uart ;
}
