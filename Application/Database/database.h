/*
 * database.h
 *
 *  Created on: Feb 15, 2025
 *      Author: ADMIN-HPZ2
 */

#ifndef DATABASE_DATABASE_H_
#define DATABASE_DATABASE_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "stdbool.h"
#include "stdint.h"
#include "sensorSignal.h"

#define SHUTTLE_ID "002"
#define TOPIC_HANDLE  "shuttle/handle/002"
#define TOPIC_RUN     "shuttle/run/002"
#define STEP_MAX 20

typedef enum {
	SHUTTLE_STOP ,
	SHUTTLE_FORWARD ,
	SHUTTLE_RIGHT ,
	SHUTTLE_REVERSE ,
	SHUTTLE_LEFT ,
}ShttleDirState;

/* CẤU TRÚC DỮ LIỆU CHO GIAO TIẾP VỚI APP MOBILE */
typedef enum {
    BUTTON_STATE_FORWARD = 1,
    BUTTON_STATE_REVERSE,
    BUTTON_STATE_PALLET_UP,
    BUTTON_STATE_PALLET_DOWN,
    BUTTON_STATE_WHEEL_UP,
    BUTTON_STATE_WHEEL_DOWN,
} ButtonState;

typedef struct {
	float TargetSpeed;
	ButtonState buttonState;
	bool UpdateVlEvent;
	uint16_t emg;
	uint16_t brake;
	uint16_t mode;
	uint16_t resetErrors;
} AppDataHandle;
extern AppDataHandle app_data ;
/*************************************************/

/* CẤU TRÚC DỮ LIỆU CHO GIAO TIẾP VỚI SERVER */
typedef enum {
    SHUTTLE_IS_ERROR = 1 ,
	SHUTTLE_IS_LIFT_PALLET,
	SHUTTLE_IS_LOWER_PALLET,
	SHUTTLE_IS_LIFT_WHEEL,
	SHUTTLE_IS_LOWER_WHEEL,
	SHUTTLE_IS_SLOW,
	SHUTTLE_IS_RUNNING,
	SHUTTLE_IS_WAITING_CMD
} ShuttleStatus;
typedef struct {
	int direction;
	int proceed;
	char Qrcode[16];
}Step;

typedef struct {
	bool newMission;
	bool newAdCmd;
	uint8_t totalStep;
	Step cmd_step[STEP_MAX] ;
	uint8_t adminCmd;
}ServerCommand;
extern ServerCommand server_cmd ;
/*************************************************/
/* DATABASE DỮ LIỆU HOẠT ĐỘNG CỦA SHUTTLE */
typedef enum {
	NOMAL,
	AXIS_X,
	AXIS_Y
}ShuttleAxis;

struct DataRun {
	uint8_t shuttleMode;
	uint8_t targetDir; // 0 stop / 1 front /2 right /3 behind 4/left
	uint8_t packageStatus;
	uint8_t shuttleCurrentStatus;
	uint16_t statusReg;
	uint8_t cmdComplete;       // status mission
	int missionComplete;   // total mission
	bool motor_is_run;
};

extern struct DataRun db_shuttle_run;

struct DataInfo {
	int charge_cycle;
	int qrDistanceX;
	int qrDistanceY;
	float qrAngle;
	float batteryPercentage ;
	float Remaining_capacity;
	float Speed;
	float Voltage;
	float Current;
	float bmsTemperature ;
	int power;
	char *errorStatus;
	char ip[20];
	char *no;
	uint8_t chargeDischargeStatus;
	uint8_t currentStep;
	uint8_t palletLiftingstatus ;
	uint32_t ip32U;
};

extern struct DataInfo db_shuttle_info;
/*************************************************/

void databaseInit();
void lockCommandFromServer();
void unlockCommandFromServer();
void shuttleSetStatus(ShuttleStatus status);
ShuttleStatus shuttleGetStatus();
void shuttleUnSetStatus(ShuttleStatus status);
uint8_t getPackageState();
#ifdef __cplusplus
}
#endif

#endif /* DATABASE_DATABASE_H_ */
