/*
 * dalySmartBms.c
 *
 *  Created on: Dec 3, 2024
 *      Author: ADMIN-HPZ2
 */
#include "dalySmartBms.h"
#include "user_custom.h"

struct DalyBms bms ;

void bmsInit(UART_HandleTypeDef *uart)
{
	 bms.bmsSerial = uart ;
	 HAL_UARTEx_ReceiveToIdle_IT(uart, bms.rxData, bmsRxbuffer);
}

void bmsContinute()
{
	HAL_UARTEx_ReceiveToIdle_IT(bms.bmsSerial, bms.rxData, bmsRxbuffer);
}

void bmsRequestData(uint8_t slave_id)
{
    bms.txData[0] = slave_id;
    bms.txData[1] = 0x03;  // Function code
    bms.txData[2] = (START_ADD >> 8) & 0xFF;  // MSB of register address
    bms.txData[3] = START_ADD & 0xFF;         // LSB of register address
    bms.txData[4] = (NUMBER_OF_REGISTER >> 8) & 0xFF;  // MSB of number of registers
    bms.txData[5] = NUMBER_OF_REGISTER & 0xFF;         // LSB of number of registers

    // Calculate CRC
    uint16_t crc = crc16_modbus(bms.txData, 6);   // Compute CRC for the first 6 bytes
    bms.txData[6] = crc & 0xFF;                  // CRC Low Byte
    bms.txData[7] = (crc >> 8) & 0xFF;           // CRC High Byte

    HAL_UART_Transmit(bms.bmsSerial, bms.txData, 8, 100);
}

float bmsGetVoltage()
{
    uint16_t result = ((uint16_t)bms.rxData[3] << 8) | bms.rxData[4];
    return (float)result * 0.1;
}

/*
 * 0.1A, 30000 Offset, the charging current is a negative number, and the discharge is a positive number, (30080-30000)*0.1=8A, which is the discharge current
 */
float bmsGetCrurrent()
{
    uint16_t result = (((uint16_t)bms.rxData[5] << 8) | bms.rxData[6]);
    if(result >= 35536)
    {
    	result = 0 ;
    }
    float current = (result - 30000) * 0.1;
    return current;
}

/*
 * 0.001,800/1000=80%
 */
float bmsGetSOC()
{
    uint16_t result = ((uint16_t)bms.rxData[7] << 8) | bms.rxData[8];
    return (float)result * 0.1;
}
/*
 * Heartbeat bag
 */
uint16_t bmsGetSOH()
{
    uint16_t result = ((uint16_t)bms.rxData[9] << 8) | bms.rxData[10];
    return result;
}

/*
 * 1°C, the temperature data is offset by 40. For example, when the temperature data is 40, the actual temperature = temperature data-40;
 */
float bmsGetAmbientTemperature()
{
    uint16_t result = ((uint16_t)bms.rxData[25] << 8) | bms.rxData[26];
    if(result < 40)
    {
    	return 0 ;
    }
    return (float)result - 40.0 ;
}
/*
 * 0 stationary, 1 charging, 2 discharging
 */
uint8_t bmsGetChargeStaus()
{
	uint16_t result = ((uint16_t)bms.rxData[35] << 8) | bms.rxData[36];
	return result ;
}

float bmsGetRemaningCapacity()
{
	uint16_t result = ((uint16_t)bms.rxData[41] << 8) | bms.rxData[42];
	return (float)result * 0.1 ;
}

/*
 * 0.1 AH
 */
uint16_t bmsGetCycleTime()
{
	uint16_t result = ((uint16_t)bms.rxData[43] << 8) | bms.rxData[44];
	return result;
}


uint16_t bmsGetPower()
{
	uint16_t result = ((uint16_t)bms.rxData[67] << 8) | bms.rxData[68];  /// units: W
	return result;
}

uint16_t bmsGetEnergy()
{
	uint16_t result = ((uint16_t)bms.rxData[69] << 8) | bms.rxData[70];  /// units: W/H
	return result;
}





