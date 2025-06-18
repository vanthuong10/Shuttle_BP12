/*
 * database.c
 *
 *  Created on: Feb 15, 2025
 *      Author: ADMIN-HPZ2
 */
#include "database.h"
#include "cmsis_os.h"
#include "event_groups.h"
#include "mongoose_glue.h"


#define MQTTEVENTMES (1 << 0)
#define WSEVENTMES (1 << 1)
#define WEBEVENTMES (1 << 2)
static EventGroupHandle_t event_eth ;
struct DataRun db_shuttle_run;
struct DataInfo db_shuttle_info;

AppDataHandle app_data ;
/* BIẾN CHỨA DỮ LIỆU WS */
typedef struct {
	struct mg_connection *con;
	struct mg_ws_message *wsm ;
} WsData;
/* BIẾN CHỨA DỮ LIỆU MQTT */
typedef struct {
	struct mg_connection *con;
	struct mg_mqtt_message *mqttm ;
} MqttData;
/****************************************** */
static WsData ws_data;
static MqttData mqtt_data ;

void websocketCallback(struct mg_connection *con, struct mg_ws_message *wsm )
{
	ws_data.con = con;
	ws_data.wsm = wsm;
	xEventGroupSetBits(event_eth, WSEVENTMES);
}
void mqttCallback(struct mg_connection *con, struct mg_mqtt_message *mqttm )
{
	mqtt_data.con = con;
	mqtt_data.mqttm = mqttm;
	xEventGroupSetBits(event_eth, MQTTEVENTMES);
}
/* XỬ LÝ DỮ LiỆU NHẬN TỪ APP */
typedef struct {
    const char *command;
    void (*action)(void);
} CommandAction;

// Khai báo các hành động
static void handleRequestFromApp() { /*appTranmiterCallback(ws_data.con, &ws_data.wsm->data); */ }
static void handleAppConnected() { mg_ws_send(ws_data.con, ws_data.wsm->data.buf, ws_data.wsm->data.len, WEBSOCKET_OP_TEXT); }
static void handlePalletUp() { app_data.buttonState = BUTTON_STATE_PALLET_UP ; }
static void handlePalletDown() { app_data.buttonState = BUTTON_STATE_PALLET_DOWN ;}
static void handleWheelUp() { app_data.buttonState = BUTTON_STATE_WHEEL_UP ;}
static void handleWheelDown() { app_data.buttonState = BUTTON_STATE_WHEEL_DOWN ;}
static void handleLeft() { app_data.buttonState = BUTTON_STATE_FORWARD; }
static void handleRight() { app_data.buttonState = BUTTON_STATE_REVERSE; }
static void handleForward() {app_data.buttonState = BUTTON_STATE_FORWARD ; }
static void handleReverse() { app_data.buttonState = BUTTON_STATE_REVERSE; }
static void handleEmg() { sscanf(ws_data.wsm->data.buf, "EMG:%hu", &app_data.emg); }
static void handleBrake() { sscanf(ws_data.wsm->data.buf, "Brake:%hu", &app_data.brake);  }
static void handleTargetSpeed() { sscanf(ws_data.wsm->data.buf, "Target Speed:%f" , &app_data.TargetSpeed); }
static void handleMode() { sscanf(ws_data.wsm->data.buf, "Mode:%hu" , &app_data.mode); }
static void handleResetError() { sscanf(ws_data.wsm->data.buf, "ResetError:%hu" , &app_data.resetErrors);  }

// Bảng ánh xạ lệnh với hàm xử lý
static CommandAction commandTable[] = {
    {"request from app", handleRequestFromApp},
    {"oke", handleAppConnected},
    {"Pallet Up", handlePalletUp},
    {"Pallet Down", handlePalletDown},
    {"Wheel Up", handleWheelUp},
    {"Wheel Down", handleWheelDown},
    {"Left", handleLeft},
    {"Right", handleRight},
    {"Forward", handleForward},
    {"Reverse", handleReverse}
};
static CommandAction commandGetValue[] =
{
	{"EMG:", handleEmg},
	{"Brake:", handleBrake},
	{"Target Speed:", handleTargetSpeed},
	{"Mode:", handleMode},
	{"ResetError:", handleResetError}
};
// Số lượng lệnh trong bảng
#define COMMAND_ACTION_COUNT (sizeof(commandTable) / sizeof(commandTable[0]))
#define COMMAND_GET_DATA_COUNT (sizeof(commandGetValue) / sizeof(commandGetValue[0]))

static void processingDataApp() {
    for (size_t i = 0; i < COMMAND_ACTION_COUNT; i++) {
        if (mg_strcmp(ws_data.wsm->data, mg_str(commandTable[i].command)) == 0) {
            commandTable[i].action();  // Gọi hàm xử lý
            return;
        }
    }

    for (size_t y = 0; y < COMMAND_GET_DATA_COUNT; y++) {
        if (strstr(ws_data.wsm->data.buf,commandGetValue[y].command) != NULL) {
            mg_ws_send(ws_data.con, ws_data.wsm->data.buf, ws_data.wsm->data.len, WEBSOCKET_OP_TEXT);
            commandGetValue[y].action();  // Gọi hàm xử lý
            MG_DEBUG(("RECEIVED %.*s\n", ws_data.wsm->data.len, ws_data.wsm->data.buf));
           return;
        }
    }

}

/****************************************** */

/* XỬ LÝ DỮ LiỆU NHẬN TỪ SERVER */
ServerCommand server_cmd ;
static bool lock_cmd_state = false;

void lockCommandFromServer()
{
	lock_cmd_state = true ;
}
void unlockCommandFromServer()
{
	lock_cmd_state = false ;
}

static void resetCommand()
{
	memset(server_cmd.cmd_step,'\0',sizeof(server_cmd.cmd_step));
}

static void processingDataTopicHandle() {
	double a;
	resetCommand();
	if(!mg_json_get_num(mqtt_data.mqttm->data, "$.totalStep", &a)) return;  // return nếu không đúng định dạng json
	server_cmd.totalStep = (int) a;
//	printf("TotalStep: %d \n", server_cmd.totalStep);
	if(server_cmd.totalStep >= STEP_MAX) { resetCommand(); return; } // return nếu gửi quá lệnh
	for (int i = 0; i < server_cmd.totalStep; i++) {
		char *jpath = mg_mprintf("$.step%d", i + 1);
		char *str = mg_json_get_str(mqtt_data.mqttm->data, jpath);
		if(str == NULL) { resetCommand(); return; } // return nếu không đúng định dạng json
		struct mg_str vl = mg_str(str);
//		printf("step%d %.*s\n", i + 1, vl.len, vl.buf);
		sscanf(vl.buf, "%[^>]>%d:%d", server_cmd.cmd_step[i].Qrcode, &server_cmd.cmd_step[i].direction,
				&server_cmd.cmd_step[i].proceed);
//		char *buf = mg_mprintf("Qrcode:%m\ndirection:%d\nprocced:%d\n",
//						       MG_ESC(server_cmd.cmd_step[i].Qrcode), server_cmd.cmd_step[i].direction,
//							   server_cmd.cmd_step[i].proceed);
//		printf(buf);
//		free(buf);
		free(jpath);
		free(str);
	}
	server_cmd.newMission = true ;
}

static void processingDataTopicRun()
{
	double a;
	if(mg_json_get_num(mqtt_data.mqttm->data, "$.status", &a)){
		server_cmd.newAdCmd = true ;
		server_cmd.adminCmd = (int) a;
	}
}
/****************************************** */
/* SET TRẠNG THÁI SHUTTLE */
void shuttleSetStatus(ShuttleStatus status)
{
	db_shuttle_run.statusReg = uSetBit(db_shuttle_run.statusReg, (uint8_t) status);
}

void shuttleUnSetStatus(ShuttleStatus status)
{
	db_shuttle_run.statusReg = uClearBit(db_shuttle_run.statusReg, (uint8_t) status);
}
ShuttleStatus shuttleGetStatus()
{
	ShuttleStatus stt = 0;
    for (int i = SHUTTLE_IS_ERROR; i <= SHUTTLE_IS_WAITING_CMD; i++) {
        if (uReadBit(db_shuttle_run.statusReg, i)) {
        	stt = (ShuttleStatus) i;
            break; // Thoát khi tìm thấy bit thấp nhất được đặt
        }
    }
    return stt;
}

/**
 * @brief   Kiểm tra trạng thái pallet hàng của Shuttle.
 * @return: 0 Không có pallet
 * 			1 Có pallet
 * 			2 lệch pallet
 */
uint8_t getPackageState()
{
	if( (sensor_signal.di_sensor.S1_ISPALLET == HIGH)  && (sensor_signal.di_sensor.S2_ISPALLET == HIGH)) return 1 ;
	if( (sensor_signal.di_sensor.S1_ISPALLET == LOW)  && (sensor_signal.di_sensor.S2_ISPALLET == LOW)) return 0 ;
    return 2 ;
}
/****************************************** */
osThreadId_t DataEthTaskHandle;
const osThreadAttr_t DataEthTask_attributes = {
  .name = "DataEthTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};

static void dataProcessTask(void *argument)
{
	EventBits_t event_bits;
	for(;;)
	{
		event_bits = xEventGroupWaitBits(event_eth,    /* The event group handle. */
		                                 MQTTEVENTMES | WSEVENTMES | WEBEVENTMES,        /* The bit pattern the event group is waiting for. */
		                                 pdTRUE,         /* will be cleared automatically. */
		                                 pdFALSE,        /* Don't wait for both bits, either bit unblock task. */
		                                 portMAX_DELAY); /* Block indefinitely to wait for the condition to be met. */

		if ((event_bits & (MQTTEVENTMES | WSEVENTMES | WEBEVENTMES)) == (MQTTEVENTMES | WSEVENTMES | WEBEVENTMES))
		{
		   //PRINTF("Both bits are set.");
		} else if ((event_bits & WSEVENTMES) == WSEVENTMES)
        {
        	processingDataApp();

        }else if ((event_bits & MQTTEVENTMES) == MQTTEVENTMES)
        {
        	struct mg_mqtt_opts pub_opts;
        	pub_opts.qos = 1;
        	pub_opts.message = mqtt_data.mqttm->data;
        	pub_opts.topic = mg_str(topicBff[1]);
    	    MG_INFO(("%lu RECEIVED %.*s <- %.*s", mqtt_data.con->id, (int) mqtt_data.mqttm->data.len,
    	    		mqtt_data.mqttm->data.buf, (int) mqtt_data.mqttm->topic.len, mqtt_data.mqttm->topic.buf));
    	    if(strncmp(mqtt_data.mqttm->topic.buf, TOPIC_HANDLE,18) == 0){
    	    	mg_mqtt_pub(mqtt_data.con, &pub_opts);
    	    	if(!lock_cmd_state)
    	    	{
        	    	processingDataTopicHandle();
    	    	}

    	    }else if (strncmp(mqtt_data.mqttm->topic.buf, TOPIC_RUN,15) == 0){
    	    	mg_mqtt_pub(mqtt_data.con, &pub_opts);
    	    	processingDataTopicRun();
    	    }

        }else
        {

        }

	}
}
void databaseInit()
{
	event_eth = xEventGroupCreate();
	DataEthTaskHandle = osThreadNew(dataProcessTask, NULL, &DataEthTask_attributes);
}

