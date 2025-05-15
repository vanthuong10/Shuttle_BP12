/*
 * update_data.c
 *
 *  Created on: Feb 21, 2025
 *      Author: ADMIN-HPZ2
 */
#include "database.h"
#include "update_data.h"
#include "cmsis_os.h"
#include "safety.h"

osThreadId_t SyncTaskHandle;
osMutexId_t jsonCreateMutex;
const osThreadAttr_t SyncTask_attributes = {
  .name = "SyncTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal2,
};

static void getInfoBatery()
{
	db_shuttle_info.batteryPercentage = sensor_signal.battery.SOC;
	db_shuttle_info.charge_cycle      = sensor_signal.battery.cycle_time;
	db_shuttle_info.Remaining_capacity = sensor_signal.battery.remaining_capacity;
	db_shuttle_info.chargeDischargeStatus = sensor_signal.battery.charge_status;
	db_shuttle_info.Voltage				  = sensor_signal.battery.volt;
	db_shuttle_info.Current				  = sensor_signal.battery.curent;
	db_shuttle_info.bmsTemperature        = sensor_signal.battery.temprature;
	db_shuttle_info.power				  = sensor_signal.battery.power;
	db_shuttle_info.Current				  = sensor_signal.battery.curent;
}
static void getInfoQrReader()
{
	db_shuttle_info.qrDistanceX = sensor_signal.qr_sensor->distanceX ;
	db_shuttle_info.qrDistanceY = sensor_signal.qr_sensor->distanceY ;
	db_shuttle_info.qrAngle     = sensor_signal.qr_sensor->angle ;
}
void ws_transmit_timer(void *arg)
{
	getInfoQrReader();
	db_shuttle_info.Speed = rpmToSpeed(abs(sensor_signal.motor_parameter->SpeedReal));
	ShuttleAxis axis = getAxisShuttle();
	struct mg_mgr *mgr = (struct mg_mgr *) arg;
	struct mg_connection *c;
	for (c = mgr->conns; c != NULL; c = c->next) {
		if (c->is_websocket) {
	  // Prevent stale connections to grow infinitely
		if (c->send.len > 2048) continue;
		/*********** GỬI DỮ LIỆU VẬN HÀNH TỚI APP MOBILE*/
		mg_ws_printf(c, WEBSOCKET_OP_TEXT, "{ %m:%m, %m:%m, %m:%d, %m:%d, %m:%d, %m:%.4f, %m:%.4f, %m:%.4f, %m:%.4f, %m:%d, %m:%d, %m:%d, %m:%m, %m:%.4f, %m:%d, %m:%m, %m:%d, %m:%d, %m:%.4f, %m:%d, %m:%d, %m:%.4f, %m:%.4f}" ,
		MG_ESC("no"), MG_ESC(db_shuttle_info.no),
		MG_ESC("ip"), MG_ESC(db_shuttle_info.ip),
		MG_ESC("shuttleMode"),db_shuttle_run.shuttleMode,
		MG_ESC("chargeCycle"),db_shuttle_info.charge_cycle,
		MG_ESC("axis"), axis,
		MG_ESC("voltage"),db_shuttle_info.Voltage,
		MG_ESC("current"),db_shuttle_info.Current,
		MG_ESC("batteryCapacity"),db_shuttle_info.Remaining_capacity,
		MG_ESC("batteryPercentage"),db_shuttle_info.batteryPercentage,
		MG_ESC("chargeDischargeStatus"),db_shuttle_info.chargeDischargeStatus,
		MG_ESC("shuttleStatus"), 1,
		MG_ESC("power"),db_shuttle_info.power,
		MG_ESC("errorCode"),MG_ESC(db_shuttle_info.errorStatus),
		MG_ESC("speed"),db_shuttle_info.Speed,
		MG_ESC("commandComplete"),db_shuttle_run.cmdComplete,
		MG_ESC("qrCode"), MG_ESC(sensor_signal.qr_sensor->Tag),
		MG_ESC("distanceX"),db_shuttle_info.qrDistanceX,
		MG_ESC("distanceY"),db_shuttle_info.qrDistanceY,
		MG_ESC("angle"),db_shuttle_info.qrAngle,
		MG_ESC("packageStatus"), db_shuttle_run.packageStatus,
		MG_ESC("missionCompleted"), db_shuttle_run.missionComplete,
		MG_ESC("temperature"),db_shuttle_info.bmsTemperature,
		MG_ESC("pressure"),sensor_signal.pressure_sensor );

		/*********** GỬI TÍN HIỆU CẢM BIẾN TỚI APP MOBILE*/
		mg_ws_printf(c, WEBSOCKET_OP_TEXT,"{ %m:%d, %m:%d, %m:%d, %m:%d, %m:%d, %m:%d, %m:%d, %m:%d, %m:%d, %m:%d, %m:%d, %m:%d, %m:%d, %m:%d, %m:%d, %m:%d, %m:%d, %m:%d, %m:%d, %m:%d, %m:%d, %m:%d, %m:%d, %m:%d, %m:%d, %m:%d, %m:%d, %m:%d, %m:%d, %m:%d }",
		MG_ESC("RearSensor"), sensor_signal.di_sensor.Y1_BARRIER,
		MG_ESC("FrontSensor"),sensor_signal.di_sensor.Y2_BARRIER,
		MG_ESC("RightObjectSensor"), sensor_signal.di_sensor.X2_PACKAGE,
		MG_ESC("RightObstacleSensor"), sensor_signal.di_sensor.X2_BARRIER,
		MG_ESC("LeftObjectSensor"), sensor_signal.di_sensor.X1_PACKAGE,
		MG_ESC("LeftObstacleSensor"), sensor_signal.di_sensor.X1_BARRIER,
		MG_ESC("LeftPalletSensor"), sensor_signal.di_sensor.S1_ISPALLET,
		MG_ESC("RightPalletSensor"), sensor_signal.di_sensor.S2_ISPALLET,
		MG_ESC("FrontRailSensor"), sensor_signal.di_sensor.S1_RAIL_FRONT,
		MG_ESC("RightRailSensor"), sensor_signal.di_sensor.S2_RAIL_RIGHT,
		MG_ESC("RearRailSensor"), sensor_signal.di_sensor.S3_RAIL_BEHIND,
		MG_ESC("LeftRailSensor"), sensor_signal.di_sensor.S4_RAIL_LEFT,
		MG_ESC("PalletLiftLimit1"),sensor_signal.di_sensor.UP_LIMIT_PK1,
		MG_ESC("PalletLowerLimit1"), sensor_signal.di_sensor.DOWN_LIMIT_PK1,
		MG_ESC("WheelLiftLimit1"), sensor_signal.di_sensor.UP_LIMIT_WH1,
		MG_ESC("WheelLowerLimit1"), sensor_signal.di_sensor.DOWN_LIMIT_WH1,
		MG_ESC("PalletLiftLimit2"),sensor_signal.di_sensor.UP_LIMIT_PK2,
		MG_ESC("PalletLowerLimit2"), sensor_signal.di_sensor.DOWN_LIMIT_PK2,
		MG_ESC("WheelLiftLimit2"), sensor_signal.di_sensor.UP_LIMIT_WH2,
		MG_ESC("WheelLowerLimit2"), sensor_signal.di_sensor.DOWN_LIMIT_WH2,
		MG_ESC("RfLiftPallet"), sensor_signal.di_sensor.RF_PALLET_UP,
		MG_ESC("RfLowerPallet"), sensor_signal.di_sensor.RF_PALLET_DOWN,
		MG_ESC("RfLiftShuttle"), sensor_signal.di_sensor.RF_WHEEL_UP ,
		MG_ESC("RfLowerShuttle"), sensor_signal.di_sensor.RF_WHEEL_DOWN ,
		MG_ESC("RfResetShuttle"), sensor_signal.di_sensor.RF_RESET,
		MG_ESC("RfShuttleForward"), sensor_signal.di_sensor.RF_FORWARD,
		MG_ESC("RfShuttleReverse"), sensor_signal.di_sensor.RF_REVERSE,
		MG_ESC("SelectMode"), sensor_signal.di_sensor.SelectMode ,
		MG_ESC("BrakeButton"), sensor_signal.di_sensor.OPEN_BRAKE ,
		MG_ESC("EmergencyStop"), sensor_signal.di_sensor.EMG_BUTTON);
		}
	}
}

static void inforUpdatetask(void *argument)
{

	struct mg_tcpip_if *mif = (struct mg_tcpip_if *) g_mgr.priv ;
	db_shuttle_info.no = SHUTTLE_ID ;
	db_shuttle_run.statusReg = 0 ;
	static char tmp_buf[32];
	for(;;)
	{
		mg_snprintf(db_shuttle_info.ip, sizeof(db_shuttle_info.ip), "%M", mg_print_ip4, &mif->ip);
		if(!checkQrcode(sensor_signal.qr_sensor->Tag, TAG_EMG)) // đọc mã cấm không gửi lên server
		{
			strcpy(tmp_buf,sensor_signal.qr_sensor->Tag);
		}
		getInfoBatery();
		if(db_shuttle_run.shuttleMode == 0)  // cập nhật trạng thái chỉ khi ở chế độ Auto
		{
			db_shuttle_run.shuttleCurrentStatus = shuttleGetStatus();
		}else
		{
			db_shuttle_run.shuttleCurrentStatus = 0 ;
		}
		char *buf = mg_mprintf("{ %m:%m, %m:%m, %m:%d, %m:%d, %m:%.4f, %m:%.4f, %m:%.4f, %m:%.4f, %m:%d, %m:%d, %m:%d, %m:%m, %m:%.4f, %m:%d, %m:%m, %m:%d, %m:%d, %m:%.4f, %m:%.4f}",
					MG_ESC("no"), MG_ESC(db_shuttle_info.no),
					MG_ESC("ip"), MG_ESC(db_shuttle_info.ip),
					MG_ESC("shuttleMode"),db_shuttle_run.shuttleMode,
					MG_ESC("chargeCycle"),db_shuttle_info.charge_cycle,
					MG_ESC("voltage"),db_shuttle_info.Voltage,
					MG_ESC("current"),db_shuttle_info.Current,
					MG_ESC("batteryCapacity"),db_shuttle_info.Remaining_capacity,
					MG_ESC("batteryPercentage"),db_shuttle_info.batteryPercentage,
					MG_ESC("chargeDischargeStatus"),db_shuttle_info.chargeDischargeStatus,
					MG_ESC("shuttleStatus"), db_shuttle_run.shuttleCurrentStatus,
					MG_ESC("power"),db_shuttle_info.power,
					MG_ESC("errorCode"),MG_ESC(db_shuttle_info.errorStatus),
					MG_ESC("speed"),db_shuttle_info.Speed,
					MG_ESC("commandComplete"),db_shuttle_run.cmdComplete,
					MG_ESC("qrCode"), MG_ESC(tmp_buf),
					MG_ESC("packageStatus"), db_shuttle_run.packageStatus,
					MG_ESC("missionCompleted"), db_shuttle_run.missionComplete,
					MG_ESC("temperature"),db_shuttle_info.bmsTemperature,
					MG_ESC("pressure"),sensor_signal.pressure_sensor);
			struct mg_str json = mg_str(buf);
			mqtt_publish(json, SELECT_INFO_TOPIC);
		free(buf);
		osDelay(300/ portTICK_PERIOD_MS);
	}
}

void dataUpdateTaskInit()
{
	  SyncTaskHandle = osThreadNew(inforUpdatetask,NULL, &SyncTask_attributes);
}

