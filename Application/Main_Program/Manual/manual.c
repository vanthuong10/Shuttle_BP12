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
void Manualtask(void *argument)
{
	app_data.TargetSpeed = 0.2 ;  // vận tốc mặc định 0.2 m/s
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
	app_data.TargetSpeed = 0.2 ;  // vận tốc mặc định 0.2 m/s
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
