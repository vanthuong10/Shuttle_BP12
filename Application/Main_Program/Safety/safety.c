/*
 * safety.c
 *
 *  Created on: Feb 24, 2025
 *      Author: ADMIN-HPZ2
 */
#include "safety.h"
#include "cmsis_os.h"
#include "stdio.h"
#include "string.h"
#include "sensorSignal.h"
#include "hydraulic.h"
#include "display.h"

#define TAG_LIMIT_NUM 6
#define TAG_SKIP_NUM 7
#define TAG_SKIP_OBJ_NUM 6
#define DISTANCE_ERROR_SET 60
#define LOW_POWER_SET 25

const char tag_limit_front[TAG_LIMIT_NUM][32] = { "X0000Y0045",
												  "X0004Y0001",
};

const char tag_limit_back[TAG_LIMIT_NUM][32] = { "X0000Y0000",
												 "X0000Y0023",
												 "X0015Y0001",
												 "X0015Y0002"

};

const char tag_limit_left[TAG_LIMIT_NUM][32] = { "X0000Y0045",
												 "X0000Y0025",
												 "X0000Y0002",
												 "X0004Y0002",
												 "X0015Y0001",
												 "X0015Y0000"
};

const char tag_limit_right[TAG_LIMIT_NUM][32] = { "X0004Y0001",
												  "X0003Y0004",
												  "X0000Y0044",
												  "X0000Y0021",
												  "X0015Y0003",
												  "X0015Y0002"
};

const char tag_skip_error[TAG_SKIP_NUM][32]   = { "X0000Y0023",
												  "X0000Y0022",
												  "X0000Y0001",
												  "X0000Y0000",
												  "X0000Y0021",
												  "X0000Y0002",
												  "X0000Y0003"
};

const char tag_skip_error_obj[TAG_SKIP_OBJ_NUM][32]   = { "X0000Y0002",
														  "X0000Y0025",
														  "X0000Y0045",
														  "X0000Y0021",
														  "X0000Y0044",
														  "X0003Y0004"
};

static bool error_limit_tag_state = false ;
struct ShuttleErrorStatus check_limit_tag() {
	static struct ShuttleErrorStatus error = { false, 0, 0 };
	if(!error_limit_tag_state)
	{
		memset(&error, 0 , sizeof(error));
	}
	ShuttleAxis axis = getAxisShuttle();
	int dir = 0 ;

	if(axis == AXIS_X && sensor_signal.motor_parameter->TargetSpeed > 0)
	{
		dir = 4 ;
	}else if(axis == AXIS_X && sensor_signal.motor_parameter->TargetSpeed < 0)
	{
		dir = 2 ;
	}else if(axis == AXIS_Y && sensor_signal.motor_parameter->TargetSpeed > 0)
	{
		dir = 1 ;
	}else if(axis == AXIS_Y && sensor_signal.motor_parameter->TargetSpeed < 0)
	{
		dir = 3 ;
	}else
	{
		dir = 0 ;
	}

	for (int i = 0; i < TAG_LIMIT_NUM; i++) {

		switch (dir) {
			case 1:
				if(checkQrcode(sensor_signal.qr_sensor->Tag, tag_limit_front[i]) && sensor_signal.qr_sensor->distanceX > DISTANCE_ERROR_SET)
				{
					error_limit_tag_state = true ;
				}
				break;
			case 2:
				if(checkQrcode(sensor_signal.qr_sensor->Tag, tag_limit_right[i]) && sensor_signal.qr_sensor->distanceY < -DISTANCE_ERROR_SET)
				{
					error_limit_tag_state = true ;
				}
				break;
			case 3:
				if(checkQrcode(sensor_signal.qr_sensor->Tag, tag_limit_back[i]) && sensor_signal.qr_sensor->distanceX < -DISTANCE_ERROR_SET)
				{
					error_limit_tag_state = true ;
				}
				break;
			case 4:
				if(checkQrcode(sensor_signal.qr_sensor->Tag, tag_limit_left[i]) && sensor_signal.qr_sensor->distanceY > DISTANCE_ERROR_SET)
				{
					error_limit_tag_state = true ;
				}
				break;
			default:
				break;
		}
	}
	if(error_limit_tag_state)
	{
		error.errorState = true ;
		error.errortype  = 1 ;
		error.ErrorCode  = 1 ;
		server_cmd.adminCmd = 0 ;
	}
	return error ;
}

static char NO_ERROR_STATUS[10] =  "00";
static bool emg_state = false ;

osThreadId_t SaftyTaskHandle;
const osThreadAttr_t SafetyTask_attributes = {
  .name = "SafetyTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal3
};

static int pulse_safety;
static char mc_error[10];
struct ShuttleErrorStatus shuttle_error_table[8];
struct ShuttleAlarmStatus shuttle_alarm_table[8];
static bool shuttle_is_error = false ;
static int count_sensor_trigger[8];
static bool photoelectric_ss_state[4] = { false, false, false, false } ;
/**
 * @brief   Kiểm tra có thay đổi mã QR.
 * @return: true nếu có thay đổi
 */
char qr_pre[32]; // mảng lưu qr cũ
static bool isQrChanged(char* current) {
    static bool is_qr_Initialized = false; // Đánh dấu trạng thái ban đầu
    if (!is_qr_Initialized) {
        strcpy(qr_pre, current); // Sao chép chuỗi
        is_qr_Initialized = true;
        return false;
    }
    // So sánh chuỗi hiện tại với trạng thái trước đó
    if (strcmp(current, qr_pre) != 0) {
        strcpy(qr_pre, current); // Cập nhật trạng thái trước đó
        return true; // Có thay đổi
    }
    return false; // Không thay đổi
}
/**
 * @brief   Kiểm tra tín hiêu cảm biến tác động.
 * @return: true nếu có tác động đủ 3 lần
 */
bool sensorTrigger(int *tmp, DI_SENSOR sensor) {
	if( sensor == HIGH ) (*tmp) ++;
	if(*tmp >= 3) { return true ; }
	return false;
}
/**
 * @brief   kiểm tra hướng hiện tại Shuttle thông qua cảm biến nâng hạ bánh.
 * @return: hướng của shuttle hiện tại
 */
ShuttleAxis getAxisShuttle()
{
	if((bool)sensor_signal.di_sensor.UP_LIMIT_WH1 || (bool) sensor_signal.di_sensor.UP_LIMIT_WH2 )
	{
		return AXIS_X ;
	}
	if((bool)sensor_signal.di_sensor.DOWN_LIMIT_WH1 || (bool) sensor_signal.di_sensor.DOWN_LIMIT_WH2 )
	{
		return AXIS_Y ;
	}
	return NOMAL ;
}
/**
 * @brief    Tạo mã báo lỗi Shuttle.
 * @param errorType: phân nhóm lỗi ( io/system/..)
 * @param errorCode: mã số lỗi
 */
char* errorStatus(uint8_t errorType, uint16_t errorCode)
{
	memset(mc_error,'\0',sizeof(mc_error));
	snprintf(mc_error,10,"E%d%d", errorType, errorCode);
	return mc_error ;
}
/**
 * @brief    Tạo mã cảnh báo Shuttle.
 * @param errorType: phân nhóm cảnh báo ( io/system/..)
 * @param errorCode: mã số cảnh báo
 */
char* alarmStatus(uint8_t errorType, uint16_t errorCode)
{
	memset(mc_error,'\0',sizeof(mc_error));
	snprintf(mc_error,10,"W%d%d", errorType, errorCode);
	return mc_error ;
}
/**
 * @brief    Kiểm tra vật cản khi di chuyển trên ray trung chuyển.
 * @return:  trạng thái cảnh báo nếu có
 */
static struct ShuttleAlarmStatus checkObstacleAxisY()
{
	struct ShuttleAlarmStatus alarm = { false, 0, 0 };
	if(getAxisShuttle() != AXIS_Y || db_shuttle_run.shuttleMode == 1 || !db_shuttle_run.motor_is_run)  // Chế độ manual và hướng X không cần kiểm tra lỗi
	{
		count_sensor_trigger[0] = 0 ;
		count_sensor_trigger[1] = 0 ;
		return alarm;
	}
	if(sensor_signal.motor_parameter->TargetSpeed < 0)
	{
		for(int i = 0; i< TAG_SKIP_NUM; i++)
		{
			if (checkQrcode(sensor_signal.qr_sensor->Tag, tag_skip_error[i])) return alarm; // bỏ qua cảm biến vật cản tại 2 đầu
		}
		if(sensorTrigger(&count_sensor_trigger[0], sensor_signal.di_sensor.Y1_BARRIER))
		{
			photoelectric_ss_state[0] = true ;
		}
	}else if(sensor_signal.motor_parameter->TargetSpeed > 0)
	{
		if(sensorTrigger(&count_sensor_trigger[1], sensor_signal.di_sensor.Y2_BARRIER))
		{
			photoelectric_ss_state[0] = true ;
		}
	}
	if(photoelectric_ss_state[0])
	{
		alarm.alarmState = true ;
		alarm.alarmtype  = 1 ;
		alarm.alarmCode = 0 ;
	}
	return alarm;

}

/**
 * @brief    Kiểm tra vật cản khi di chuyển trên ray chứa hàng.
 * @return:  trạng thái cảnh báo nếu có
 */
static struct ShuttleAlarmStatus checkObstacleAxisX()
{
	struct ShuttleAlarmStatus alarm = { false, 0, 0 };
	if(db_shuttle_run.shuttleMode == 1 ) return alarm;  // Chế độ manual và hướng Y không cần kiểm tra lỗi
	for (int i = 0; i < TAG_SKIP_OBJ_NUM; i++) {
			if (checkQrcode(sensor_signal.qr_sensor->Tag, tag_skip_error_obj[i]))
			{
				photoelectric_ss_state[1] = false;
				return alarm; // bỏ qua cảm biến phát hiện pallet tại 2 đầu
			}
	}
	if(getAxisShuttle() != AXIS_X || db_shuttle_run.shuttleMode == 1 || !db_shuttle_run.motor_is_run )
	{
		count_sensor_trigger[2] = 0 ;
		count_sensor_trigger[3] = 0 ;
		return alarm;  // Hướng Y không kiểm tra
	}
	if(sensor_signal.motor_parameter->TargetSpeed < 0 )
	{
		if(sensorTrigger(&count_sensor_trigger[2], sensor_signal.di_sensor.X2_BARRIER))
		{
			photoelectric_ss_state[1] = true ;
		}
	}else if(sensor_signal.motor_parameter->TargetSpeed > 0)
	{
		if(sensorTrigger(&count_sensor_trigger[3], sensor_signal.di_sensor.X1_BARRIER))
		{
			photoelectric_ss_state[1] = true ;
		}
	}
	if(photoelectric_ss_state[1])
	{
		alarm.alarmState = true ;
		alarm.alarmtype  = 1 ;
		alarm.alarmCode = 1 ;
	}
	return alarm;
}
/**
 * @brief    Kiểm tra có pallet hàng trên ray chứa hàng.
 * @return:  trạng thái cảnh báo nếu có
 */
static struct ShuttleAlarmStatus checkObjectAxisX()
{
	struct ShuttleAlarmStatus alarm = { false, 0, 0 };
	for (int i = 0; i < TAG_SKIP_OBJ_NUM; i++) {
		if (checkQrcode(sensor_signal.qr_sensor->Tag, tag_skip_error_obj[i]))
		{
			photoelectric_ss_state[2] = false;
			return alarm; // bỏ qua cảm biến phát hiện pallet tại 2 đầu
		}

	}
	if(getAxisShuttle() != AXIS_X || db_shuttle_run.shuttleMode == 1 || (bool) sensor_signal.di_sensor.DOWN_LIMIT_PK1
			|| (bool) sensor_signal.di_sensor.DOWN_LIMIT_PK2 || !db_shuttle_run.motor_is_run)
	{
		count_sensor_trigger[4] = 0 ;
		count_sensor_trigger[5] = 0 ;
		return alarm ;  // hướng Y không kiểm tra
	}

	if ((bool) sensor_signal.di_sensor.UP_LIMIT_PK1 || (bool) sensor_signal.di_sensor.UP_LIMIT_PK2) {
		if (sensor_signal.motor_parameter->TargetSpeed < 0) {
			if (sensorTrigger(&count_sensor_trigger[4],
					sensor_signal.di_sensor.X2_PACKAGE)) {
				photoelectric_ss_state[2] = true;
			}
		} else if (sensor_signal.motor_parameter->TargetSpeed > 0) {
			if (sensorTrigger(&count_sensor_trigger[5],
					sensor_signal.di_sensor.X1_PACKAGE)) {
				photoelectric_ss_state[2] = true;
			}
		}
	}
	if(photoelectric_ss_state[2])
	{
		alarm.alarmState = true;
		alarm.alarmtype = 1;
		alarm.alarmCode = 2;
	}
	return alarm;
}
/**
 * @brief    Kiểm tra lỗi lệch pallet hàng ( có tín hiệu chạy nhưng pallet bị lệch ).
 * @return:  trạng thái cảnh báo nếu có
 */
static struct ShuttleAlarmStatus checkPalletPositionAlarm()
{
	struct ShuttleAlarmStatus alarm = { false, 0, 0 };
	if(db_shuttle_run.shuttleMode == 1) return alarm;  // Chế độ manual không cần kiểm tra lỗi
	if(sensor_signal.di_sensor.UP_LIMIT_PK1 == LOW && sensor_signal.di_sensor.UP_LIMIT_PK2 == LOW) return alarm;  // không nâng pallet
	if((sensor_signal.motor_parameter->TargetSpeed != 0) && db_shuttle_run.packageStatus == 2)
	{
		alarm.alarmState = true ;
		alarm.alarmtype  = 2 ;
	}
	return alarm;
}

static struct ShuttleAlarmStatus checkEmgButton()
{
	struct ShuttleAlarmStatus alarm = { false, 0, 0 };
	if((bool)sensor_signal.di_sensor.EMG_BUTTON || app_data.emg == 1)
	{
		alarm.alarmState = true;
		alarm.alarmtype = 0;
		alarm.alarmCode = 1;
		emg_state = true ;
		hydraulicEmg();
		server_cmd.adminCmd = 0 ;
	}
	return alarm ;
}

/**
 * @brief    Kiểm tra dung lượng pin shuttle.
 * @return:  trạng thái cảnh báo nếu có
 */
static struct ShuttleAlarmStatus checkLowPower()
{
	struct ShuttleAlarmStatus alarm = { false, 0, 0 };
	if(sensor_signal.battery.SOC <= LOW_POWER_SET)
	{
		alarm.alarmState = true;
		alarm.alarmtype = 3;
		alarm.alarmCode = 0;
	}
	return alarm ;
}

/**
 * @brief   Kiểm tra lỗi động cơ .
 * @return:  trạng thai lỗi nếu có
 */
static struct ShuttleErrorStatus checkMotorError()
{
	struct ShuttleErrorStatus error = { false, 0, 0 };
	if(sensor_signal.motor_error_state == 0) return error ;  /// không có lỗi
	error.errorState = true ;
	error.errortype  = 0 ;
	if(sensor_signal.motor_error_state == 1)
	{
		error.ErrorCode  = 1 ;
		HAL_GPIO_WritePin(outputGpio.qStop.Port, outputGpio.qStop.gpioPin, GPIO_PIN_SET);
		server_cmd.adminCmd = 0 ;
	}else
	{
		error.ErrorCode  = sensor_signal.motor_parameter->Error_code  ;
	}
	return error;
}
/**
 * @brief   Kiểm tra lỗi kết nối đầu đọc QR .
 * @return:  trạng thai lỗi nếu có
 */
static struct ShuttleErrorStatus checkQrConnectionError()
{
	struct ShuttleErrorStatus error = { false, 0, 0 };
	if(sensor_signal.qr_connection_state == 0) return error ;  /// không có lỗi
	error.errorState = true ;
	error.errortype  = 1 ;
	if(sensor_signal.motor_error_state == 1)
	{
		error.ErrorCode  = 0 ;
		server_cmd.adminCmd = 0 ;
	}
	return error;
}
///**
// * @brief   Kiểm tra Shttle đọc được mã dừng khẩn .
// * @return:  trạng thai lỗi nếu đọc được mã dừng khẩn
// */
//static struct ShuttleErrorStatus checkTagEmg()
//{
//	struct ShuttleErrorStatus error = { false, 0, 0 };
//	if((bool) db_shuttle_run.shuttleMode) return error;  // Chế độ manual không cần kiểm tra lỗi
//	if(checkQrcode(sensor_signal.qr_sensor->Tag, TAG_EMG))
//	{
//		error.errorState = true ;
//		error.errortype  = 1 ;
//		error.ErrorCode  = 1 ;
//	}
//	return error ;
//}
/**
 * @brief   Kiểm tra quá tải Shuttle thông qua áp suất thủy lực .
 * @return:  trạng thai lỗi nếu có
 */
static bool overLoadState = false ;
static struct ShuttleErrorStatus checkOverLoad()
{
	struct ShuttleErrorStatus error = { false, 0, 0 };
	if(hydraulicGetState() != CYLINDER_PALLET_UP) return error ;  /// nếu không phải nâng pallet return
	if(!overLoadState && ((bool) sensor_signal.di_sensor.UP_LIMIT_PK1 ||(bool) sensor_signal.di_sensor.UP_LIMIT_PK2 )) return error ;
	if(sensor_signal.pressure_sensor >= PRESSURE_MAX || overLoadState)
	{
		error.errorState = true ;
		error.errortype  = 2 ;
		error.ErrorCode  = 1 ;
		setHydraulicOverload(); // set lỗi quá tải dừng hệ thống thủy lực
		overLoadState = true ;
	}
	return error ;
}
/**
 * @brief   Kiểm tra quá thời gian nâng hạ mà chưa tác động cảm biến .
 * @return:  trạng thai lỗi nếu có
 */
static struct ShuttleErrorStatus checkOverTimeHydraulic()
{
	struct ShuttleErrorStatus error = { false, 0, 0 };
	if(checkErrorHydraulic())
	{
		error.errorState = true ;
		error.errortype  = 2 ;
		error.ErrorCode  = 2 ;
	}
	return error ;
}
/**
 * @brief   Kiểm tra lệch vị trí mã Qr.
 * @param p :  Biến lưu xung trước đó.
 *            1. Kiểm tra hướng Shuttle để tính giá trị xung
 *            2. Nếu p=0 cập nhật giá trị xung hiện tại
 *            3. Tính độ lệch 2 xung
 *            4. Nếu đúng mã thì reset lại giá trị
 *            5. Kiểm tra nếu lệch thì trả về lỗi.
 */
static struct ShuttleErrorStatus checkOverDistanceQr(int *p) {
	struct ShuttleErrorStatus error = { false, 0, 0 };
	static int delta_p = 0 ;
	ShuttleAxis axis = getAxisShuttle();
	if (*p == 0) { *p = sensor_signal.motor_parameter->PosActual; return error ; }  // cập nhật xung ban đầu
	delta_p = sensor_signal.motor_parameter->PosActual - *p;
	if(isQrChanged(sensor_signal.qr_sensor->Tag)) { *p = 0 ; delta_p = 0 ;  return error ;  } // Thay đổi QR cập nhật lại xung
	if(axis == AXIS_X)
	{
		if(abs(delta_p) > distanceToPulses(DISTANCE_QR_X + 0.15))
		{
			error.errorState = true ;
			error.errortype  = 1 ;
			error.ErrorCode  = 2 ;
			server_cmd.adminCmd = 0 ;
		}
	}else if(axis == AXIS_Y)
	{
		if(abs(delta_p) > distanceToPulses(DISTANCE_QR_Y + 0.2))
		{
			error.errorState = true ;
			error.errortype  = 1 ;
			error.ErrorCode  = 3 ;
			server_cmd.adminCmd = 0 ;
		}
	}
	return error ;
}
/**
 * @brief   Kiểm tra tất cả lỗi và cảnh báo từ Shuttle .
 * @return:  Mã lỗi để gửi tới server.
 */
static char* getShuttleErrorStatus()
{
	shuttle_error_table[0] = checkMotorError();
	shuttle_error_table[1] = checkQrConnectionError();
	shuttle_error_table[2] = check_limit_tag();
	shuttle_error_table[3] = checkOverLoad();
	shuttle_error_table[4] = checkOverDistanceQr(&pulse_safety);
	shuttle_error_table[5] = checkOverTimeHydraulic();
	shuttle_alarm_table[0] = checkEmgButton() ;
	shuttle_alarm_table[1] = checkPalletPositionAlarm();
	shuttle_alarm_table[2] = checkObjectAxisX();
	shuttle_alarm_table[3] = checkObstacleAxisX();
	shuttle_alarm_table[4] = checkObstacleAxisY();
	shuttle_alarm_table[5] = checkLowPower();

	for(int i=0; i<=7; i++)
	{
		if(shuttle_error_table[i].errorState)
		{
			shuttle_is_error = true ;
			ControlBuzzer(GPIO_PIN_SET); // bật còi báo
			shuttleSetStatus(SHUTTLE_IS_ERROR);
			return errorStatus(shuttle_error_table[i].errortype, shuttle_error_table[i].ErrorCode);
		}
	}
	for(int y=0; y<=7; y++)
		{
			if(shuttle_alarm_table[y].alarmState)
			{
				if(shuttle_alarm_table[5].alarmState)
				{
					// no acction
				}else
				{
					shuttle_is_error = true ;
					shuttleSetStatus(SHUTTLE_IS_ERROR);
				}
				ControlBuzzer(GPIO_PIN_SET); // bật còi báo
				return alarmStatus(shuttle_alarm_table[y].alarmtype, shuttle_alarm_table[y].alarmCode);
			}
		}
	shuttle_is_error = false;
	shuttleUnSetStatus(SHUTTLE_IS_ERROR);
	ControlBuzzer(GPIO_PIN_RESET);
	return NO_ERROR_STATUS;
}

static void safetyTask(void *argument)
{
	memset(shuttle_error_table,'\0', sizeof(shuttle_error_table));
	memset(shuttle_alarm_table,'\0', sizeof(shuttle_alarm_table));
	for(;;)
	{
		db_shuttle_info.errorStatus = getShuttleErrorStatus();
		if((bool)app_data.resetErrors || sensor_signal.di_sensor.RF_RESET)
		{
			printf("Reset shuttle\n");
			if(shuttle_error_table[0].errorState || emg_state )
			{
				emg_state = false ;
				motorErrorReset();
				HAL_GPIO_WritePin(outputGpio.qStop.Port, outputGpio.qStop.gpioPin, GPIO_PIN_RESET);
			}
			if(shuttle_error_table[4].errorState) pulse_safety = 0 ; // reset xung encoder
			app_data.resetErrors = 0 ;
			sensor_signal.motor_error_state = 0 ;
			sensor_signal.qr_connection_state = 0 ;
			memset(shuttle_error_table,'\0', sizeof(shuttle_error_table));
			memset(count_sensor_trigger, 0,  sizeof(count_sensor_trigger));
			memset(photoelectric_ss_state, 0, sizeof(photoelectric_ss_state));
			resetErrorHydraulic(); // reset lỗi thủy lực
			overLoadState = false ;
			error_limit_tag_state = false;
		}
		osDelay(20/portTICK_PERIOD_MS);
	}

}

void safetyTaskInit()
{
	SaftyTaskHandle = osThreadNew(safetyTask, NULL, &SafetyTask_attributes);
}
bool shuttleErrorState()
{
	return shuttle_is_error;
}
