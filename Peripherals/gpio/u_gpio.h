/*
 * u_gpio.h
 *
 *  Created on: Feb 19, 2025
 *      Author: ADMIN-HPZ2
 */

#ifndef GPIO_U_GPIO_H_
#define GPIO_U_GPIO_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/////////////////////////////////////
typedef enum {
	LOW,
	HIGH,
	DI_ERROR
}DI_SENSOR;
////////////////////////////////////

////////////////////////////////

typedef struct {
	GPIO_TypeDef* Port;
	uint16_t gpioPin;
}GPIO_UserHandle;

///////////////////////////////////

typedef struct {
	DI_SENSOR Y1_BARRIER;
	DI_SENSOR Y2_BARRIER;
	DI_SENSOR X2_PACKAGE;
	DI_SENSOR X2_BARRIER;
	DI_SENSOR X1_PACKAGE;
	DI_SENSOR X1_BARRIER;
	DI_SENSOR S1_ISPALLET;
	DI_SENSOR S2_ISPALLET;
	DI_SENSOR S1_RAIL_FRONT;
	DI_SENSOR S2_RAIL_RIGHT;
	DI_SENSOR S3_RAIL_BEHIND;
	DI_SENSOR S4_RAIL_LEFT;
	DI_SENSOR UP_LIMIT_WH2;
	DI_SENSOR DOWN_LIMIT_WH2;
	DI_SENSOR UP_LIMIT_WH1;
	DI_SENSOR DOWN_LIMIT_WH1;
	DI_SENSOR UP_LIMIT_PK2;
	DI_SENSOR DOWN_LIMIT_PK2;
	DI_SENSOR UP_LIMIT_PK1;
	DI_SENSOR DOWN_LIMIT_PK1;
	DI_SENSOR RF_PALLET_UP;
	DI_SENSOR RF_PALLET_DOWN;
	DI_SENSOR RF_WHEEL_UP;
	DI_SENSOR RF_WHEEL_DOWN;
	DI_SENSOR RF_FORWARD;
	DI_SENSOR RF_REVERSE;
	DI_SENSOR RF_RESET ;
	DI_SENSOR PUMP_OVER_TEMPERATURE;
	DI_SENSOR OPEN_BRAKE;
	DI_SENSOR EMG_BUTTON;
	DI_SENSOR SelectMode;
}SENSOR_STATUS;


////////////////////////////////////
typedef struct {
	GPIO_UserHandle resetDriver ;
	GPIO_UserHandle fan ;
	GPIO_UserHandle buzzer ;
	GPIO_UserHandle charger ;
	GPIO_UserHandle pump ;
	GPIO_UserHandle valveL1 ;
	GPIO_UserHandle valveL2 ;
	GPIO_UserHandle valveL3 ;
	GPIO_UserHandle brake ;
}DoHandle;
extern DoHandle outputGpio ;
void DigitalSensorRead(SENSOR_STATUS *sensor);
void ResetOutput();
void ControlBuzzer(GPIO_PinState state);
void ControlCharger(GPIO_PinState state);
void ControlFan(GPIO_PinState state);
void ControlPump(GPIO_PinState state);
void ControlResetDriver(GPIO_PinState state);
void ControlValveL1(GPIO_PinState state);
void ControlValveL2(GPIO_PinState state);
void ControlValveL3(GPIO_PinState state);

#ifdef __cplusplus
}
#endif
#endif /* GPIO_U_GPIO_H_ */
