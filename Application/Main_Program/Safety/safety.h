/*
 * safety.h
 *
 *  Created on: Feb 24, 2025
 *      Author: ADMIN-HPZ2
 */

#ifndef MAIN_PROGRAM_SAFETY_SAFETY_H_
#define MAIN_PROGRAM_SAFETY_SAFETY_H_
#ifdef __cplusplus
extern "C" {
#endif
#include "stdint.h"
#include "stdbool.h"
#include "database.h"

#define TAG_EMG "X9999Y9999"
#define PRESSURE_MAX 80 // bar
#define DISTANCE_QR_X 1.35 //m
#define DISTANCE_QR_Y 1.45 //m

struct ShuttleErrorStatus {
	bool errorState ;
	uint8_t errortype;
	uint16_t ErrorCode;
};

struct ShuttleAlarmStatus {
	bool alarmState ;
	uint8_t alarmtype;
	uint16_t alarmCode;
};

extern struct ShuttleErrorStatus shuttle_error_table[8];
extern struct ShuttleAlarmStatus shuttle_alarm_table[8];

void safetyTaskInit();
ShuttleAxis getAxisShuttle();
bool shuttleErrorState();
bool shuttleAlarmState();
#ifdef __cplusplus
}
#endif
#endif /* MAIN_PROGRAM_SAFETY_SAFETY_H_ */
