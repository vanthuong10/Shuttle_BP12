/*
 * manual.c
 *
 *  Created on: Feb 20, 2025
 *      Author: ADMIN-HPZ2
 */
#include "manual.h"
#include "cmsis_os.h"
#include "database.h"
#include "hydraulic.h"
#include "user_custom.h"
#include "mongoose.h"
#include "safety.h"
#include "display.h"

static bool manualTask_suspended_state = false;
uint64_t manual_timer[2];

osThreadId_t ManualTaskHandle;
const osThreadAttr_t ManualTask_attributes = {
  .name = "ManualTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal3,
};
int test_rps;
double test_speed;
static bool acc_low = false ;
void Manualtask(void *argument)
{
	app_data.TargetSpeed = 0.3 ;  // vận tốc mặc định 0.3 m/s
//	char qr_pre[32]; // mảng lưu qr cũ
//	strcpy(qr_pre, sensor_signal.qr_sensor->Tag); // Sao chép chuỗi
//	bool qr_is_change = false ;
//	static int delta_p = 0 ;
//	static int pluse = 0 ;
//	static bool acc_low = false ;
	for(;;)
	{
		db_shuttle_run.packageStatus = getPackageState();
		static int dir = 0 ;
		static double target_speed = 0;
		ButtonState button_state = app_data.mode ? app_data.buttonState : (ButtonState) sensor_signal.rf_button ;
		bool en_cylinder = !checkErrorHydraulic();
		switch ( button_state) {
			case  BUTTON_STATE_FORWARD:
				dir = SHUTTLE_FORWARD ;
				target_speed = app_data.TargetSpeed ;
				shuttleSetStatus(SHUTTLE_IS_RUNNING);
				break;
			case  BUTTON_STATE_REVERSE:
				dir = SHUTTLE_REVERSE ;
				target_speed = app_data.TargetSpeed ;
				shuttleSetStatus(SHUTTLE_IS_RUNNING);
				break;
			case  BUTTON_STATE_PALLET_UP:
				controlCylinder(CYLINDER_PALLET_UP, en_cylinder);
				shuttleSetStatus(SHUTTLE_IS_LIFT_PALLET);
				break;
			case  BUTTON_STATE_PALLET_DOWN:
				controlCylinder(CYLINDER_PALLET_DOWN, en_cylinder);
				shuttleSetStatus(SHUTTLE_IS_LOWER_PALLET);
				break;
			case  BUTTON_STATE_WHEEL_UP:
				controlCylinder(CYLINDER_WHEEL_UP, en_cylinder);
				shuttleSetStatus(SHUTTLE_IS_LIFT_WHEEL);
				break;
			case  BUTTON_STATE_WHEEL_DOWN:
				controlCylinder(CYLINDER_WHEEL_DOWN, en_cylinder);
				shuttleSetStatus(SHUTTLE_IS_LOWER_WHEEL);
				break;
			default:
				target_speed = 0;
				controlCylinder(CYLINDER_OFF, false);
				shuttleSetStatus(SHUTTLE_IS_WAITING_CMD);
				shuttleUnSetStatus(SHUTTLE_IS_RUNNING);
				shuttleUnSetStatus(SHUTTLE_IS_LIFT_PALLET);
				shuttleUnSetStatus(SHUTTLE_IS_LOWER_PALLET);
				shuttleUnSetStatus(SHUTTLE_IS_LIFT_WHEEL);
				shuttleUnSetStatus(SHUTTLE_IS_LOWER_WHEEL);
				break;
		}
		bool m_error = false;
		if(shuttle_error_table[0].errorState || shuttle_alarm_table[0].alarmState || shuttle_error_table[4].errorState || shuttle_error_table[2].errorState )
		{
			m_error = true ;
		}
//		// So sánh chuỗi hiện tại với trạng thái trước đó
//		if (strcmp(sensor_signal.qr_sensor->Tag, qr_pre) != 0) {
//			strcpy(qr_pre, sensor_signal.qr_sensor->Tag); // Cập nhật trạng thái trước đó
//			qr_is_change = true; // Có thay đổi
//		}
//		if (qr_is_change) {
//			pluse = 0;
//			delta_p = 0;
//			qr_is_change = false;
//		} // Thay đổi QR cập nhật lại xung
//		if (pluse == 0) {
//			pluse = sensor_signal.motor_parameter->PosActual;
//		}  // cập nhật xung ban đầu
//		delta_p = sensor_signal.motor_parameter->PosActual - pluse;
//		if (abs(delta_p) <= distanceToPulses(0.2)) {
//			if (!acc_low) {
//				SDOProfileAcc(0.01, MotorID[0]); // thay đổi gia tốc chậm
//				acc_low = true;
//			}
//		} else {
//			if (acc_low) {
//				SDOProfileAcc(SHUTTLE_ACC, MotorID[0]); // thay đổi gia tốc chậm
//				acc_low = false;
//			}
//		}
		if (!acc_low) {
			SDOProfileAcc(0.01, MotorID[0]); // thay đổi gia tốc chậm
			acc_low = true;
		}
		motorControl(true, m_error, dir, target_speed);
		uint64_t now = mg_millis();
		if (u_timer_expired(&manual_timer[0], 200, now)) {
			app_data.buttonState = 0 ;
		}
		osDelay(50);
	}
}
void resetManualMode()
{
	app_data.buttonState = 0 ;
	app_data.TargetSpeed = 0.3 ;  // vận tốc mặc định 0.3 m/s
	manual_timer[0] = 0 ;
}
void manualTaskInit()
{
	MG_DEBUG(("MANUAL TASK INIT \n"));
	if (ManualTaskHandle == NULL) {
		ManualTaskHandle = osThreadNew(Manualtask, NULL,
				&ManualTask_attributes);
	}
}
void manualTaskSupend()
{
	if (!manualTask_suspended_state) {
		acc_low = false;
		osThreadSuspend(ManualTaskHandle);
		manualTask_suspended_state = true ;
		MG_DEBUG(("MANUAL TASK SUPEND \n"));
	}
}

void manualTaskResume()
{
	if (manualTask_suspended_state) {
		osThreadResume(ManualTaskHandle);
		manualTask_suspended_state = false ;
		resetManualMode();
	}
	db_shuttle_run.shuttleMode = 1 ;
}
