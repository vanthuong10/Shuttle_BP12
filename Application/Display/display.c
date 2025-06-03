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
#include "mb_display.h"

extern TIM_HandleTypeDef htim1;
osThreadId_t displayTaskHandle;
static LEDSTATUS led_status;
const osThreadAttr_t displayTask_attributes = {
  .name = "displayTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t)  osPriorityNormal1 ,
};

static bool showHomePage = false ;

static void displaytask(void *argument)
{
	LedRGBInit(&htim1, TIM_CHANNEL_3);
	for(;;)
	{
		uint8_t shuttle_state = shuttleGetStatus();
		switch (shuttle_state) {
			case SHUTTLE_IS_ERROR:
				led_status = LED_STATE_EMG ;
				mb_setLed(254, 0, 0, 50, 250);
				break;
			case SHUTTLE_IS_LIFT_PALLET:
				led_status = LED_STATE_LIFT ;
				mb_setLed(0, 200, 200, 50, 250);  //Blue
				break;
			case SHUTTLE_IS_LOWER_PALLET:
				led_status = LED_STATE_LIFT ;
				mb_setLed(0, 200, 200, 50, 250);  //Blue
				break;
			case SHUTTLE_IS_LIFT_WHEEL:
				led_status = LED_STATE_LIFT ;
				mb_setLed(0, 200, 200, 50, 250);  //Blue
				break;
			case SHUTTLE_IS_LOWER_WHEEL:
				led_status = LED_STATE_LIFT ;
				mb_setLed(0, 200, 200, 50, 250);  //Blue
				break;
			case SHUTTLE_IS_SLOW:
				led_status = LED_STATE_SLOW ;
				mb_setLed(255, 247, 3, 100, 250);
				break;
			case SHUTTLE_IS_RUNNING:
				led_status = LED_STATE_RUN ;
				mb_setLed(0, 254, 0, 500, 250);  //Green
				break;
			case SHUTTLE_IS_WAITING_CMD:
				led_status = LED_STATE_STOP ;
				mb_setLed(242, 135, 5, 500, 250);
				break;
			default:
				mb_setLed(0, 200, 0, 100, 250);
				break;
		}

		Led_status(led_status);
		if(showHomePage)
		{
			osDelay(100/portTICK_PERIOD_MS);
			mb_setPage(1);
		}
		osDelay(100/portTICK_PERIOD_MS);
		mb_setOled(&db_shuttle_info, &db_shuttle_run);
		osDelay(100/portTICK_PERIOD_MS);
	}
}

void displayInit()
{
	displayTaskHandle = osThreadNew(displaytask,NULL, &displayTask_attributes);
}

void shuttle_started()
{
	showHomePage = true ;

}
