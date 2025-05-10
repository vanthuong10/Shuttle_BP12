/*
 * hydraulic.h
 *
 *  Created on: Feb 20, 2025
 *      Author: ADMIN-HPZ2
 */

#ifndef HYDRAULIC_H_
#define HYDRAULIC_H_
#ifdef __cplusplus
extern "C" {
#endif
#include "stdint.h"
#include "stdbool.h"
#include "main.h"

#define PUMP_RX_BUFFER_SIZE 16
#define PUMP_ID 1
#define PUMP_START_ADD 0x41
#define PUMP_NUMBER_OF_REGISTER 5

#define PUMP_SET_INPUT_VOLTAGE_REG 31U
#define PUMP_SET_CURRENT_REG 32U
#define PUMP_SET_ACC_REG 33U
#define PUMP_SET_MOTOR_VOLTAGE_REG 34U
#define PUMP_SET_TIME_OFF_REG 35U
#define PUMP_ENABLE_REG 36U
#define PUMP_RESET_ERROR_REG 37U

typedef enum {
	CYLINDER_OFF ,
	CYLINDER_PALLET_UP ,
	CYLINDER_WHEEL_UP ,
	CYLINDER_PALLET_DOWN ,
	CYLINDER_WHEEL_DOWN
}CylinderState;

struct DriverPump
{
	uint8_t rxData[PUMP_RX_BUFFER_SIZE];
	UART_HandleTypeDef *Serial;
	uint8_t txData[8];

};

void hydraulicTaskInit();
void pumpInit(UART_HandleTypeDef *uart);
void configCylinderLimitSensor(uint8_t* limitUpPallet1, uint8_t* limitUpPallet2, uint8_t* limitDownPallet1, uint8_t* limitDownPallet2,
							   uint8_t* limitUpWheel1, uint8_t* limitUpWheel2, uint8_t* limitDownWheel1, uint8_t* limitDownWheel2) ;

bool controlCylinder(CylinderState cmd, bool en ) ;
bool checkErrorHydraulic();
void resetErrorHydraulic();
void setHydraulicOverload();
void hydraulicEmg();
CylinderState hydraulicGetState();
#ifdef __cplusplus
}
#endif
#endif /* HYDRAULIC_H_ */
