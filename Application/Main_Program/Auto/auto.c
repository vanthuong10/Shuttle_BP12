/*
 * auto.c
 *
 *  Created on: Feb 25, 2025
 *      Author: ADMIN-HPZ2
 */
#include "database.h"
#include "auto.h"
#include "cmsis_os.h"
#include "hydraulic.h"
#include "user_custom.h"
#include "mongoose.h"
#include "safety.h"
#include "display.h"

static uint64_t timer_auto;
static bool autoTask_suspended_state = false;
osThreadId_t AutoTaskHandle;
const osThreadAttr_t AutoTask_attributes = {
  .name = "AutoTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal3
};

/* DỮ LIỆU CHO QUÁ TRÌNH CHẠY TỰ ĐỘNG SHUTTLE */
typedef enum {
	NO_CHANGE,
	CHANGE_AXIS_X,
	CHANGE_AXIS_Y
}ChangeAxisCmd;

typedef enum {
	SPEED_ZERO,
	SPEED_VERRY_LOW,
	SPEED_LOW,
	SPEED_NOMAL,
	SPEED_HIGH
}ShuttleSpeedCommand;

struct AMotorHandle {
	double targetSpeed;
	double dec;
	bool en ;
	bool Error ;
	bool zeroSpeed;
	uint8_t drirection;
};
static struct AMotorHandle motorHandle = {0,0,false, false, false, 0 } ;
struct CmdStatus
{
	bool complete ;  // cờ báo hoàn thành nhiệm vụ
	bool mission;    // Cờ báo có nhiệm vụ
	uint8_t totalStep;  // Tổng số lệnh
	uint8_t currentStep; // Lệnh hiện tại
	uint16_t speedReg;  // thanh ghi CMD Speed
};
static struct CmdStatus cmdstatus = {.currentStep = 0, .complete = false} ;

/*************************************************/

/**
 * @brief   Set tốc độ trong thanh ghi cmd tốc độ.
*/
static void aSetSpeed(ShuttleSpeedCommand cmd)
{
	cmdstatus.speedReg = uSetBit(cmdstatus.speedReg, (uint8_t) cmd);
}
/**
 * @brief   ReSet tốc độ trong thanh ghi cmd tốc độ.
*/
static void aUnSetSpeed(ShuttleSpeedCommand cmd)
{
	cmdstatus.speedReg = uClearBit(cmdstatus.speedReg, (uint8_t) cmd);
}
/**
 * @brief   Đọc giá trị thanh ghi CMD speed để chọn tốc độ động cơ.
*/
static void aSelectSpeed()
{
    static const float speedTable[] = {
        [SPEED_ZERO]      = SPEED_LEVEL_0,   // Bit 0
        [SPEED_VERRY_LOW] = SPEED_LEVEL_1,   // Bit 1
        [SPEED_LOW]       = SPEED_LEVEL_2,   // Bit 2
        [SPEED_NOMAL]     = SPEED_LEVEL_3,   // Bit 3
        [SPEED_HIGH]      = SPEED_LEVEL_4    // Bit 4
    };
    for (int i = SPEED_ZERO; i <= SPEED_HIGH; i++) {
        if (uReadBit(cmdstatus.speedReg, i)) {
            motorHandle.targetSpeed = speedTable[i];
            if(i == SPEED_VERRY_LOW)
            {
            	shuttleSetStatus(SHUTTLE_IS_SLOW);
            }else
            {
            	shuttleUnSetStatus(SHUTTLE_IS_SLOW);
            }
            break; // Thoát khi tìm thấy bit thấp nhất được đặt
        }
    }
}
/**
 * @brief   Xóa nhiệm vụ.
 */
static void clearMission()
{
	memset(&cmdstatus, 0 , sizeof(struct CmdStatus));  // clear status command
	memset(server_cmd.cmd_step, '\0', sizeof(server_cmd.cmd_step));	   // clear mission
	unlockCommandFromServer();  // sẵn sàng nhận nhiệm vụ mới
}

/**
 * @brief   Set các cờ hoàn thành nhiệm vụ và báo về server.
 */
static void missionComplete(int state)
{
	cmdstatus.complete = true;
	cmdstatus.mission = false;
	db_shuttle_run.cmdComplete = cmdstatus.complete;
	char buf[64];
	memset(buf, 0,sizeof(buf));
	mg_snprintf(buf, 64,"{ %m:%d }",
				MG_ESC("status"), state);
	struct mg_str json = mg_str(buf);
	mqtt_publish(json, SELECT_COMPLETE_TOPIC);
}

static struct DBTest {
	int stepCmd ;
	uint64_t timerAutoTest ;
	bool yQrPass ;
	bool xQrPass ;
}db_test;

static void reset_db_test() {
    memset(&db_test, 0, sizeof(db_test));
}
static void reset_motorhandle()
{
	memset(&motorHandle, 0 , sizeof(motorHandle));
}


/**
 * @brief   Thực hiện đổi bánh di chuyển.
 * @param axis: hướng cần chuyển
 * @retval: true  nếu hoàn tất chuyển hướng
 * 			false nếu đang chuyển hướng
 */

typedef struct {
    bool flag1;
    bool flag2;
    bool flag3;
    bool flag4;
} FlagState;

static FlagState flag_change_dir = {0,0,0,0} ;
static bool changeDirection(ShuttleAxis axis)
{
	/*
	 * flag1 : xQrPass
	 * flag2 : yQrPass
	 * flag3 : Change_acc
	 * */
    aSetSpeed(SPEED_VERRY_LOW);  // đi chậm dò mã
    if(axis == AXIS_X)
    {
    	if(abs(qr.distanceX) <= 3)
    	{
    		flag_change_dir.flag1 = true ; // nếu lệch 3 mm
    		flag_change_dir.flag4 = false ;
    	}else
    	{
			if(!flag_change_dir.flag4) // nếu chưa thay đổi gia tốc
			{
				flag_change_dir.flag4 = SDOProfileAcc(SHUTTLE_SLOW_ACC, MotorID[0]); // thay đổi gia tốc chậm
			}else
			{
				if(sensor_signal.qr_sensor->distanceX >3) {motorHandle.drirection = 3;} // chạy lùi
				else if (sensor_signal.qr_sensor->distanceX <-3) {motorHandle.drirection = 1;} // chạy tiến
			}
    	}
	    if (flag_change_dir.flag1)
	    {
	    	motorHandle.en = false;
	    	shuttleSetStatus(SHUTTLE_IS_LIFT_WHEEL);
	    	if(controlCylinder(CYLINDER_WHEEL_UP, true))  // nâng bánh thành công
	    	{
	    		controlCylinder(CYLINDER_OFF, false);     // dừng xyanh
	    		aUnSetSpeed(SPEED_VERRY_LOW) ; // bỏ trạng thái đi chậm
	    		shuttleUnSetStatus(SHUTTLE_IS_LIFT_WHEEL);
	    		SDOProfileAcc(SHUTTLE_ACC, MotorID[0]); // thay đổi gia tốc mặc định
	    		motorHandle.en = true;
	    		flag_change_dir.flag1 = false;
	    		return true ;
	    	}
	    }
    }else
    {
    	if(abs(qr.distanceY) <= 3) {
    		flag_change_dir.flag2 = true ; // nếu lệch 3 mm
    		flag_change_dir.flag3 = false ;
    	}else
    	{
			if(!flag_change_dir.flag3) // nếu chưa thay đổi gia tốc
			{
				flag_change_dir.flag3 = SDOProfileAcc(SHUTTLE_SLOW_ACC, MotorID[0]); // thay đổi gia tốc chậm
			}else
			{
				if(sensor_signal.qr_sensor->distanceY >3) {motorHandle.drirection = 2;} // qua phải
				else if (sensor_signal.qr_sensor->distanceY <-3) {motorHandle.drirection = 4;} // qua trái
			}
    	}
	    if (flag_change_dir.flag2)
	    {
	    	motorHandle.en = false;
	    	shuttleSetStatus(SHUTTLE_IS_LOWER_WHEEL);
	    	if(controlCylinder(CYLINDER_WHEEL_DOWN, true))
	    	{
	    		aUnSetSpeed(SPEED_VERRY_LOW) ; // bỏ trạng thái đi chậm
	    		controlCylinder(CYLINDER_OFF, false);     // dừng xyanh
	    		shuttleUnSetStatus(SHUTTLE_IS_LOWER_WHEEL);
	    		SDOProfileAcc(SHUTTLE_ACC, MotorID[0]); // thay đổi gia tốc mặc định
	    		motorHandle.en = true;
	    		flag_change_dir.flag2 = false;
	    		return true ;
	    	}
	    }
    }
    return false ;
}

/**
 * @brief   Thực hiện lấy Pallet.
 */
static FlagState flag_get_pack = {0,0,0,0} ;

static bool aGetPack()
{
	/*
	 * flag1 : yQrPass
	 * flag2 : Change_acc
	 * */

	aSetSpeed(SPEED_VERRY_LOW);  // đi chậm dò mã
	    ShuttleAxis current_axis  = getAxisShuttle() ;
	    switch ((int)current_axis) {
			case AXIS_X:
				if(abs(qr.distanceY) <= 1)
				{
					flag_get_pack.flag1 = true ; // nếu lệch 3 mm
					flag_get_pack.flag2 = false ;
				}else
				{
					if (!flag_get_pack.flag2) // nếu chưa thay đổi gia tốc
					{
						flag_get_pack.flag2 = SDOProfileAcc(SHUTTLE_SLOW_ACC, MotorID[0]); // thay đổi gia tốc chậm
					} else {
						if(sensor_signal.qr_sensor->distanceY >1) {motorHandle.drirection = 2;} // qua phải
						else if (sensor_signal.qr_sensor->distanceY <-1) {motorHandle.drirection = 4;} // qua trái
					}
				}
				break ;
			case AXIS_Y:
				if(abs(qr.distanceX) <=1)
				{
					flag_get_pack.flag3 = true ; // nếu lệch 3mm
					flag_get_pack.flag4 = false ;
				}else {
					if(!flag_get_pack.flag4) // nếu chưa thay đổi gia tốc
					{
						flag_get_pack.flag4 = SDOProfileAcc(SHUTTLE_SLOW_ACC, MotorID[0]); // thay đổi gia tốc chậm
					}else {
						if(sensor_signal.qr_sensor->distanceX >1) {motorHandle.drirection = 3;} // chạy lùi
						else if (sensor_signal.qr_sensor->distanceX <-1) {motorHandle.drirection = 1;} // chạy tiến
					}
				}
				break;
	    }
	if (flag_get_pack.flag1 || flag_get_pack.flag3 || motorHandle.drirection == 0) {
		motorHandle.en = false;  // dừng motor
		shuttleSetStatus(SHUTTLE_IS_LIFT_PALLET);
		if (controlCylinder(CYLINDER_PALLET_UP, true))  // nâng pallet
		{
			aUnSetSpeed(SPEED_VERRY_LOW); // bỏ trạng thái đi chậm
			shuttleUnSetStatus(SHUTTLE_IS_LIFT_PALLET);
			controlCylinder(CYLINDER_OFF, false);     // dừng xyanh
			motorHandle.en = true;
			flag_get_pack.flag1 = false;
    		SDOProfileAcc(SHUTTLE_ACC, MotorID[0]); // thay đổi gia tốc mặc định
			return true;
		}
	}
	return false ;
}

/**
 * @brief   Thực hiện trả Pallet.
 */
static FlagState flag_put_pack = {0,0,0,0} ;
static bool aPutPack()
{
	/*
	 * flag1 : yQrPass
	 * flag2 : Change_acc
	 * */

    aSetSpeed(SPEED_VERRY_LOW);  // đi chậm dò mã
    ShuttleAxis current_axis  = getAxisShuttle() ;
    switch ((int)current_axis) {
		case AXIS_X:
			if(abs(qr.distanceY) <= 1)
			{
				flag_put_pack.flag1 = true ; // nếu lệch 3 mm
				flag_put_pack.flag2 = false ;
			}else
			{
				if (!flag_put_pack.flag2) // nếu chưa thay đổi gia tốc
				{
					flag_put_pack.flag2 = SDOProfileAcc(SHUTTLE_SLOW_ACC, MotorID[0]); // thay đổi gia tốc chậm
				} else {
					if(sensor_signal.qr_sensor->distanceY >1) {motorHandle.drirection = 2;} // qua phải
					else if (sensor_signal.qr_sensor->distanceY <-1) {motorHandle.drirection = 4;} // qua trái
				}
			}
			break ;
		case AXIS_Y:
			if(abs(qr.distanceX) <=1)
			{
				flag_put_pack.flag3 = true ; // nếu lệch 3mm
				flag_put_pack.flag4 = false ;
			}else {
				if(!flag_put_pack.flag4) // nếu chưa thay đổi gia tốc
				{
					flag_put_pack.flag4 = SDOProfileAcc(SHUTTLE_SLOW_ACC, MotorID[0]); // thay đổi gia tốc chậm
				}else {
					if(sensor_signal.qr_sensor->distanceX >1) {motorHandle.drirection = 3;} // chạy lùi
					else if (sensor_signal.qr_sensor->distanceX <-1) {motorHandle.drirection = 1;} // chạy tiến
				}
			}
			break;
    }

	if (flag_put_pack.flag1 || flag_put_pack.flag3 || motorHandle.drirection == 0) {
		motorHandle.en = false;  // dừng motor
		shuttleSetStatus(SHUTTLE_IS_LOWER_PALLET);
		if (controlCylinder(CYLINDER_PALLET_DOWN, true))  // hạ pallet
		{
			aUnSetSpeed(SPEED_VERRY_LOW); // bỏ trạng thái đi chậm
			controlCylinder(CYLINDER_OFF, false);     // dừng xyanh
			shuttleUnSetStatus(SHUTTLE_IS_LOWER_PALLET);
			motorHandle.en = true;
			flag_put_pack.flag1 = false;
			flag_put_pack.flag3 = false;
			SDOProfileAcc(SHUTTLE_ACC, MotorID[0]); // thay đổi gia tốc mặc định
			return true;
		}
	}
	return false ;
}
/**
 * @brief   Thực hiện đi chậm 1/2 đoạn đường.
 * 		khi di chuyển 2 mã qr kế cận
 */
static FlagState flag_run_near_qr = {0,0,0,0} ;
static bool aSlowHalfRoad()
{
	aUnSetSpeed(SPEED_VERRY_LOW);
	static int pulse = 0 ;
	static int delta_p = 0 ;
	bool over_qr = false ;
	if(checkQrcode(server_cmd.cmd_step[cmdstatus.currentStep + 1].Qrcode, sensor_signal.qr_sensor->Tag))  // nếu Qr bằng mã qr kế tiếp
	{
		over_qr = true ;
	}
	if(!flag_run_near_qr.flag1) { pulse = 0 ; delta_p = 0 ; flag_run_near_qr.flag1 = true ; }
	if (pulse == 0) { pulse = sensor_signal.motor_parameter->PosActual; return false ; }  // cập nhật xung ban đầu
	delta_p = sensor_signal.motor_parameter->PosActual - pulse;
	if(abs(delta_p) >= distanceToPulses(DISTANCE_QR_X /3.3 ) || over_qr )
	{
		aSetSpeed(SPEED_VERRY_LOW);
		flag_run_near_qr.flag1 = false ;
		return true ;
	}
	return false ;
}

/**
 * @brief   Thực hiện đi chậm.
 * 		- Shuttle sẽ đi chậm đến khi nhận được mã mới
 */
static bool aSlowSpeed()
{
	aSetSpeed(SPEED_LOW);  // đi chậm
	return true ;
}
/**
 * @brief   Thực hiện đi rất chậm.
 */
static bool aVerySlowSpeed()
{
    aSetSpeed(SPEED_VERRY_LOW);  // đi chậm dò mã
    aUnSetSpeed(SPEED_LOW);
	return true ;
}
/**
 * @brief   Thực hiện dừng đúng vị trí
 */
static FlagState flag_stop_shuttle = {0,0,0,0} ;
static bool aStop()
{
	/*
	 * flag1 : xQrPass
	 * flag2 : yQrPass
	 * flag3 : change_acc
	 * */
	aSetSpeed(SPEED_VERRY_LOW);  // đi chậm dò mã
	ShuttleAxis current_axis  = getAxisShuttle() ;
	switch ((int)current_axis) {
		case AXIS_X:
			if(abs(qr.distanceY) <= 1)
			{
				flag_stop_shuttle.flag2 = true ;
			}else{
				if(!flag_stop_shuttle.flag3)
				{
					flag_stop_shuttle.flag3 = SDOProfileAcc(SHUTTLE_SLOW_ACC, MotorID[0]); // thay đổi gia tốc chậm
				}else{
					if(sensor_signal.qr_sensor->distanceY >1) {motorHandle.drirection = 2;} // qua phải
					else if (sensor_signal.qr_sensor->distanceY <-1) {motorHandle.drirection = 4;} // qua trái
				}
			}
			break;
		case AXIS_Y:
			if(abs(qr.distanceX) <= 1)
			{
				flag_stop_shuttle.flag1 = true ;
			}else
			{
				if(!flag_stop_shuttle.flag4)
				{
					flag_stop_shuttle.flag4 = SDOProfileAcc(SHUTTLE_SLOW_ACC, MotorID[0]); // thay đổi gia tốc chậm
				}else{
	    			if(sensor_signal.qr_sensor->distanceX >1) {motorHandle.drirection = 3;} // chạy lùi
	    			else if (sensor_signal.qr_sensor->distanceX <-1) {motorHandle.drirection = 1;} // chạy tiến
				}
			}
			break;
		default:
			motorHandle.en = false ;
			return true ;
			break;
	}
	if(flag_stop_shuttle.flag2 || flag_stop_shuttle.flag1)
	{
		motorHandle.en = false ;
		flag_stop_shuttle.flag2 = false ;
		flag_stop_shuttle.flag1 = false ;
		flag_stop_shuttle.flag3 = false ;
		flag_stop_shuttle.flag4 = false ;
		aUnSetSpeed(SPEED_VERRY_LOW);
		aSetSpeed(SPEED_ZERO) ;		 // Vận tốc về 0
		SDOProfileAcc(SHUTTLE_ACC, MotorID[0]); // thay đổi gia tốc mặc định
		return true ;
	}

	return false ;
}
/**
 * @brief   Thực hiện từng hành động.
 * @param action: hành động được yêu cầu
 * @retval : true done
 */
static FlagState flag_take_action = {0,0,0,0} ;
static bool aTakeAction(int action)
{
	/*
	 * flag1: reset_timer
	 * flag2: en_action
	 * */
	bool state = false;
	switch (action) {
		case 10:
		    aUnSetSpeed(SPEED_VERRY_LOW);
		    aUnSetSpeed(SPEED_LOW);
		    state = true ;
		    break;
		case 11:
			state = aGetPack();
			break;
		case 12:
			state = aPutPack();
			break;
		case 13:
			state = aSlowSpeed();
			break;
		case 14:
			state = aVerySlowSpeed();
			break;
		case 15:
			state = aStop();
			break;
		case 16:
			state = aSlowHalfRoad();
			break;
		default:
			state = true;
			break;
	}
	return state ;
}
/**
 * @brief   Thực hiện từng step.
 * @param return: true nếu hoàn thành/ false nếu chưa hoàn thành
 */
static FlagState flag_run_step = {0,0,0,0};
static bool runStep(char* targetQr, int targetDir, int proceed)
{
	/*
	 * flag1 chuyển hướng thành công
	 * flag2 báo chuyển hướng
	 * flag3 running
	 * */
	ShuttleAxis target_axis ;
    if(checkQrcode(targetQr, sensor_signal.qr_sensor->Tag))  flag_run_step.flag3 = true ; // nếu đúng mã QR
    if(flag_run_step.flag3)
    {
    	if(targetDir == NOMAL) { // nếu hướng là 0 thì dừng
    	    	aSetSpeed(SPEED_ZERO);
    	    	flag_run_step.flag1 = true ;
    	    }
    	    else
    	    {
    			ShuttleAxis current_axis  = getAxisShuttle() ;
    			target_axis = (targetDir == 1) || (targetDir == 3) ? AXIS_Y : AXIS_X ;
    			if((current_axis == target_axis) && !flag_run_step.flag2) // Nếu đúng hướng và trước đó chưa chuyển rail
    			{
    				flag_run_step.flag1 = true ;
    				motorHandle.drirection = targetDir ;
    			}else
    			{
    				flag_run_step.flag2 = true ; // bật cờ chuyển rail
    				if(changeDirection(target_axis))  // đợi chuyển rail thành công
    				{
    					flag_run_step.flag2 = false ; // reset cờ chuyển rail
    					MG_DEBUG(("CHUYỂN RAIL \n"));
    				}
    			}
    	    }
    	    if(flag_run_step.flag1)
    	    {
    	    	if(aTakeAction(proceed))
    	    	{
    	    		flag_run_step.flag1 = false ;
    	    		motorHandle.drirection = targetDir ;
    	        	aUnSetSpeed(SPEED_ZERO);
    	        	flag_run_step.flag3 = false;
    	        	return true;
    	    	}
    	    }
    }
    return false;
}
/**
 * @brief  Khi nhận lệnh cmd = 0 ; shuttle sẽ dừng và chưa kịp xóa các cờ trong quá trình thực thi
 * 		   điều này gây sai lệch cho lần chạy tiếp theo nên cần reset về trạng thái ban đầu.
 */
static void aResetFlag()
{
	size_t size_flag = sizeof(FlagState) ;
	memset(&flag_change_dir,'\0', size_flag); // reset cờ chuyển hướng
	memset(&flag_get_pack,'\0', size_flag); // reset cờ lấy pallet
	memset(&flag_put_pack,'\0', size_flag); // reset cờ thả pallet
	memset(&flag_run_step,'\0', size_flag); // reset cờ chạy theo bước
	memset(&flag_take_action,'\0', size_flag); // reset cờ thực hiện hành động
	memset(&flag_stop_shuttle,'\0', size_flag); // reset cờ hành động dừng shuttle
	memset(&flag_run_near_qr,'\0', size_flag);  // reset cờ di chuyển 2 mã gần nhau
	flag_take_action.flag1 = true ; // reset thời gian
	cmdstatus.speedReg = 0 ; // reset thanh ghi tốc độ

}

/**
 * @brief   Chương trình tự động với tốc độ mặc định.
 * @param return: true nếu hoàn thành lệnh/ false nếu chưa hoàn thành hoặc không có lệnh
 */
static bool autoModeNomal()
{
	if (server_cmd.newMission && !cmdstatus.mission) // nếu có nhiệm vụ mới và shuttle đang không có nhiệm vụ
	{
		cmdstatus.mission = true;      // get mission
		server_cmd.newMission = false; // reset cờ nhiệm vụ mới
		cmdstatus.complete = false;    // reset cờ hoàn thành
		motorHandle.en = true ;        // cho phép chạy động cơ
		aUnSetSpeed(SPEED_ZERO);       // reset tốc độ động cơ
		cmdstatus.totalStep = server_cmd.totalStep;
		lockCommandFromServer() ;     // Từ chối nhận thêm nhiệm vụ
		shuttleSetStatus(SHUTTLE_IS_RUNNING);
		shuttleUnSetStatus(SHUTTLE_IS_WAITING_CMD);
		MG_DEBUG(("NHẬN NHIỆM VỤ MỚI \n"));
	}
	if (!cmdstatus.mission || cmdstatus.complete) {  return true ; }
	bool stepDone = runStep(server_cmd.cmd_step[cmdstatus.currentStep].Qrcode,
								server_cmd.cmd_step[cmdstatus.currentStep].direction,
								server_cmd.cmd_step[cmdstatus.currentStep].proceed);
	if(stepDone){
		cmdstatus.currentStep++;
		if (cmdstatus.currentStep >= cmdstatus.totalStep){
			missionComplete(1);  // gửi thông báo tới server
		    db_shuttle_run.missionComplete ++ ;  // tăng số lệnh hoàn thành
		    reset_motorhandle();
		    shuttleUnSetStatus(SHUTTLE_IS_RUNNING);
		    shuttleSetStatus(SHUTTLE_IS_WAITING_CMD);
			clearMission(); 	// xóa nhiệm vụ
			return true ;
		}
	}
	return false ;
}

/**
 * @brief   Task thực thi chương trình chạy tự động.
 */

void Autotask(void *argument)
{
	db_test.stepCmd = 0 ;
	reset_motorhandle();
	shuttle_started();
	for(;;)
	{
		switch (server_cmd.adminCmd) {
			case 0:          /** lệnh dừng shuttle **/
				aSetSpeed(SPEED_ZERO) ;		 // Vận tốc về 0
				shuttleUnSetStatus(SHUTTLE_IS_RUNNING);
				shuttleSetStatus(SHUTTLE_IS_WAITING_CMD);
				if(rpmToSpeed(sensor_signal.motor_parameter->SpeedReal) <= 0.1) // chờ vận tốc về 0
				{
					clearMission();
					motorHandle.en = false ;     // off động cơ
					cmdstatus.complete = true ;  // done mission
					cmdstatus.mission = false;   // no mission
					server_cmd.newMission = false ; // reset flag get mission
					aResetFlag();  // reset các cờ phục vụ chạy auto
					aSetSpeed(SPEED_ZERO) ;		 // Vận tốc về 0
				}
				break;
			case 1: /* lệnh chạy shuttle mặc định*/
				autoModeNomal();
				aUnSetSpeed(SPEED_LOW);  // bỏ trạng thái đi chậm
				break;
			case 2: /* lệnh shuttle di chuyển chậm */
				autoModeNomal();
				aSetSpeed(SPEED_LOW); // đi chậm
				break;
			default:
				break;
		}
		if (server_cmd.adminCmd != 0) {
			db_shuttle_run.packageStatus = getPackageState();
			switch (db_shuttle_run.packageStatus) {
			case 0:
				aSetSpeed(SPEED_HIGH);
				aUnSetSpeed(SPEED_NOMAL);
				break;
			case 1:
				aSetSpeed(SPEED_NOMAL);
				aUnSetSpeed(SPEED_HIGH);
				break;
			default:
				aSetSpeed(SPEED_NOMAL);
				aUnSetSpeed(SPEED_HIGH);
				break;
			}
		}
		aSelectSpeed();  // Set tốc độ động cơ
		motorHandle.Error = shuttleErrorState();
		db_shuttle_info.currentStep = cmdstatus.currentStep ;
		uint64_t now = mg_millis();
		if(u_timer_expired(&timer_auto, 40, now))
		{
			motorControl(motorHandle.en, motorHandle.Error, motorHandle.drirection, motorHandle.targetSpeed);
		}
		osDelay(10);

	}
}

void autoTaskInit()
{
	if (AutoTaskHandle == NULL) {
		AutoTaskHandle = osThreadNew(Autotask, NULL,
				&AutoTask_attributes);
	}
	MG_DEBUG(("AUTO TASK INIT \n"));
}

void autoTaskSupend()
{
	server_cmd.adminCmd = 0;
	if (!autoTask_suspended_state) {
		missionComplete(0);
		osThreadSuspend(AutoTaskHandle);
		autoTask_suspended_state = true ;
		MG_DEBUG(("AUTO TASK SUPEND \n"));
	}
}

void autoTaskResume()
{
	if (autoTask_suspended_state) {
		osThreadResume(AutoTaskHandle);
		autoTask_suspended_state = false ;
		reset_db_test();
		reset_motorhandle();
	}
	db_shuttle_run.shuttleMode = 0 ;
}
