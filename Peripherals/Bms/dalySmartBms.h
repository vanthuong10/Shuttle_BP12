/*
 * dalySmartBms.h
 *
 *  Created on: Dec 3, 2024
 *      Author: ADMIN-HPZ2
 */

#ifndef LIB_BMS_DALY_H_
#define LIB_BMS_DALY_H_
#ifdef __cplusplus
extern "C" {
#endif
#include "stdbool.h"
#include "stdlib.h"
#include "stdint.h"
#include "main.h"
#define bmsRxbuffer 100
#define START_ADD 0x38
#define NUMBER_OF_REGISTER 37
struct DalyBms
{
	uint8_t rxData[bmsRxbuffer];
	UART_HandleTypeDef *bmsSerial;
	DMA_HandleTypeDef  *bmsDma;
	uint8_t txData[8];

};
struct BmsGet
{
	float volt;
	float curent ;
	float SOC ;
	float SOH ;
	float temprature;
	float remaining_capacity;
	uint16_t power;
	uint16_t energy;
	uint16_t cycle_time;
	uint8_t  charge_status;
};

uint16_t crc16_modbus(const uint8_t *data, size_t length);
void bmsRequestData(uint8_t slave_id);
void bmsContinute();
void bmsInit(UART_HandleTypeDef *uart);
float bmsGetVoltage();
float bmsGetCrurrent();
float bmsGetSOC();
uint16_t bmsGetSOH();
float bmsGetAmbientTemperature();
uint8_t bmsGetChargeStaus();
float bmsGetRemaningCapacity();
uint16_t bmsGetCycleTime();
uint16_t bmsGetPower();
uint16_t bmsGetEnergy();
#ifdef __cplusplus
}
#endif

#endif /* LIB_BMS_DALY_H_ */

