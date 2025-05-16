# ShuttleV1.1
* CẤU HÌNH THÔNG SỐ CHO SHUTTLE
- Cấu hình mạng truyền thông tại file mongoose_glue.c

struct INTERNET_CONFIG tcpConfig = { .ip   = MG_U32(10,14,64,12) ,//MG_U32(10,14,16,34) ,
									 .mask = MG_U32(255,255,254,0),
									 .gw   = MG_U32(10,14,64,1),
									 .mqttBroker = "mqtt://10.14.64.11:1991",
									 .mqttUser =   "thaco" ,
									 .mqttPass =   "thaco1234",
									 .s_sub_handle = "shuttle/handle",
									 .s_sub_run = "shuttle/run",
									 .s_sub_admin = "shuttle/admin",
									 .s_pub_info = "shuttle/information",
									 .s_pub_report = "shuttle/report" ,
									 .s_pub_complete = "shuttle/completeMission" ,
									 .no = "002"  };
								
- Cấu hình Topic truyền nhận dữ liệu với server tại database.h

#define SHUTTLE_ID "002"
#define TOPIC_HANDLE  "shuttle/handle/002"
#define TOPIC_RUN     "shuttle/run/002"

- Cấu hình vận tốc và gia tốc Shuttle tại sensorSignal.h

#define SHUTTLE_DEC 0.06  // m/s²
#define SHUTTLE_ACC 0.1 // m/s²
#define SHUTTLE_SLOW_ACC 0.005 // m/s²
+ Cấu hình bật tắt quạt 

#define TEMPERATURE_RUN_FAN 35

