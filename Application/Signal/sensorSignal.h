/*
 * sensorSignal.h
 *
 *  Created on: Feb 19, 2025
 *      Author: ADMIN-HPZ2
 */

#ifndef SIGNAL_SENSORSIGNAL_H_
#define SIGNAL_SENSORSIGNAL_H_
#ifdef __cplusplus
extern "C" {
#endif
#include "u_gpio.h"
#include "MCP3208.h"
#include "mongoose_glue.h"
#include "user_custom.h"
#include "QrReader.h"
#include "dalySmartBms.h"
#include "MotorControl.h"
#define TEMPERATURE_RUN_FAN 35
#define SHUTTLE_DEC 0.06  // m/s²
#define SHUTTLE_ACC 0.1 // m/s²
#define SHUTTLE_SLOW_ACC 0.005 // m/s²

typedef void (*ChangeMode)(uint8_t state);
typedef struct  {
	SENSOR_STATUS di_sensor ;
	float pressure_sensor ;
	struct BmsGet battery ;
	int rf_button ;
	KinCoParameter* motor_parameter;
	qrhandle *qr_sensor;
	uint8_t motor_error_state;
	uint8_t qr_connection_state;
}SensorSignal;
/*
 * 	uint8_t motor_error_sate;
 * 	0 - không có lỗi
 * 	1 - Lỗi kết nối
 * 	2 - Lỗi driver
 * */
extern SensorSignal sensor_signal ;
void sensorTaskInit() ;
void switch_mode_register_callback(ChangeMode cb);

#ifdef __cplusplus
}
#endif
#endif /* SIGNAL_SENSORSIGNAL_H_ */
