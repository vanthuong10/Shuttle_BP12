/*
 * display.c
 *
 *  Created on: Mar 17, 2025
 *      Author: ADMIN-HPZ2
 */

#include "display.h"
#include "cmsis_os.h"
#include "stdio.h"
#include "user_custom.h"
#include "database.h"

extern TIM_HandleTypeDef htim1;
osThreadId_t displayTaskHandle;
static LEDSTATUS led_status;
const osThreadAttr_t displayTask_attributes = {
  .name = "displayTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t)  osPriorityNormal1 ,
};

static void displaytask(void *argument)
{
	LedRGBInit(&htim1, TIM_CHANNEL_3);
	for(;;)
	{
		uint8_t shuttle_state = shuttleGetStatus();
		switch (shuttle_state) {
			case SHUTTLE_IS_ERROR:
				led_status = LED_STATE_EMG ;
				break;
			case SHUTTLE_IS_LIFT_PALLET:
				led_status = LED_STATE_LIFT ;
				break;
			case SHUTTLE_IS_LOWER_PALLET:
				led_status = LED_STATE_LIFT ;
				break;
			case SHUTTLE_IS_LIFT_WHEEL:
				led_status = LED_STATE_LIFT ;
				break;
			case SHUTTLE_IS_LOWER_WHEEL:
				led_status = LED_STATE_LIFT ;
				break;
			case SHUTTLE_IS_SLOW:
				led_status = LED_STATE_SLOW ;
				break;
			case SHUTTLE_IS_RUNNING:
				led_status = LED_STATE_RUN ;
				break;
			case SHUTTLE_IS_WAITING_CMD:
				led_status = LED_STATE_STOP ;
				break;
			default:
				led_status = LED_STATE_STOP ;
				break;
		}
		Led_status(led_status);
		osDelay(100/portTICK_PERIOD_MS);
	}
}

void displayInit()
{
	displayTaskHandle = osThreadNew(displaytask,NULL, &displayTask_attributes);
}


