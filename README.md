# ShuttleV1.1

## Provisioning network and device ID

One firmware image is used for every device. A new device starts with IP
`10.14.64.20`, mask `255.255.254.0`, gateway `10.14.64.1`, and device ID
`001`. Configure one device at a time at `http://10.14.64.20/config` using an
administrator account. Saving writes two CRC-protected records to Flash and
reboots the device with the new IP, mask, gateway, and ID.

Do not connect several unconfigured devices to the same network: they share
the factory-default IP address.
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

