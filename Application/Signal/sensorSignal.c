/*
 * sensorSignal.c
 *
 *  Created on: Feb 19, 2025
 *      Author: ADMIN-HPZ2
 */
#include "sensorSignal.h"
#include "cmsis_os.h"
#include "hydraulic.h"
#include "CanBus.h"
#include "mb_display.h"
SensorSignal sensor_signal ;
osThreadId_t IOTaskHandle;
ChangeMode change_mode = NULL ;
extern SPI_HandleTypeDef hspi2;
extern UART_HandleTypeDef huart7;
extern UART_HandleTypeDef huart6;
extern UART_HandleTypeDef huart4;
extern FDCAN_HandleTypeDef hfdcan1;
static uint64_t input_timer[4] = { 0, 0 , 0 ,0 };

const osThreadAttr_t IOTask_attributes = {
  .name = "IOTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal1
};
/**
 * @brief Scale tín hiệu đầu vào .
 * @param value: giá trị đầu vào
 * @param fromLow: giá trị đầu vào nhỏ nhất
 * @param fromHigh: giá trị đầu vào lớn nhất
 * @param toLow: giá trị đầu ra nhỏ nhất
 * @param ftoHigh: giá trị đầu ra lớn nhất
 * @return: giá trị đã được scale
 *
 */
float mapValue(float value, float fromLow, float fromHigh, float toLow, float toHigh) {
  return (value - fromLow) * (toHigh - toLow) / (fromHigh - fromLow) + toLow;
}
/**
 * @brief : Đọc giá trị cảm biến áp suất .
 * @param value_ADC: giá trị đầu vào
 * @return: giá trị cảm biến
 *
 */
float getPressure(uint16_t value_ADC)
{
	return mapValue(value_ADC, 0, 4095, 0, 250) ;
}


/**
 * @brief : lấy trạng thái nút nhấn điều khiển từ xa
 *
 */
static void getRFButtonState()
{
	uint16_t rf_reg = 0 ;
	rf_reg = sensor_signal.di_sensor.RF_FORWARD == HIGH ? uSetBit( rf_reg, 0) : uClearBit( rf_reg, 0) ;
	rf_reg = sensor_signal.di_sensor.RF_REVERSE == HIGH ? uSetBit( rf_reg, 1) : uClearBit( rf_reg, 1) ;
	rf_reg = sensor_signal.di_sensor.RF_PALLET_UP == HIGH ? uSetBit( rf_reg, 2) : uClearBit( rf_reg, 2) ;
	rf_reg = sensor_signal.di_sensor.RF_PALLET_DOWN == HIGH ? uSetBit( rf_reg, 3) : uClearBit( rf_reg, 3) ;
	rf_reg = sensor_signal.di_sensor.RF_WHEEL_UP == HIGH ? uSetBit( rf_reg, 4) : uClearBit( rf_reg, 4) ;
	rf_reg = sensor_signal.di_sensor.RF_WHEEL_DOWN == HIGH ? uSetBit( rf_reg, 5) : uClearBit( rf_reg, 5) ;
	rf_reg = sensor_signal.di_sensor.RF_RESET == HIGH ? uSetBit( rf_reg, 6) : uClearBit( rf_reg, 6) ;
	switch (rf_reg) {
		case 1:
			sensor_signal.rf_button = 1;
			break;
		case 2:
			sensor_signal.rf_button = 2;
			break;
		case 4:
			sensor_signal.rf_button = 3;
			break;
		case 8:
			sensor_signal.rf_button = 4;
			break;
		case 16:
			sensor_signal.rf_button = 5;
			break;
		case 32:
			sensor_signal.rf_button = 6;
			break;
		case 64:
			sensor_signal.rf_button = 7;
			break;
		default:
			sensor_signal.rf_button = 0;
			break;
	}

}
/**
 * @brief : lấy thông số Pin
 *
 */
void getDataBms()
{
    sensor_signal.battery.volt = bmsGetVoltage();
    sensor_signal.battery.SOC = bmsGetSOC();
    sensor_signal.battery.SOH = bmsGetSOH();
	sensor_signal.battery.charge_status = bmsGetChargeStaus();
	sensor_signal.battery.curent = bmsGetCrurrent();
	sensor_signal.battery.cycle_time = bmsGetCycleTime();
	sensor_signal.battery.energy = bmsGetEnergy();
	sensor_signal.battery.power = bmsGetPower();
	sensor_signal.battery.remaining_capacity = bmsGetRemaningCapacity();
	sensor_signal.battery.temprature = bmsGetAmbientTemperature();
	bmsRequestData(0x81);
}
/**
 * @brief : Kiểm tra có mất kết nối với động cơ
 *
 */
static bool checkMotorConnecting()
{
	uint64_t time_now = mg_millis();
	if (u_timer_expired(&input_timer[1], 1000, time_now))
	{
		if(Kincoparam[0].flagMotorConnected)
		{
			Kincoparameter.flagMotorConnected = false ;
			Kincoparam[0].flagMotorConnected = false ;
		}
		else
		{
			return false ;
		}
	}
	return true ;
}
/**
 * @brief : Kiểm tra có mất kết nối với đầu đọc QR
 *
 */
static bool checkQrReaderConnecting()
{
	uint64_t time_now = mg_millis();
	if (u_timer_expired(&input_timer[2], 1000, time_now))
	{
		if(qr.hearbeat)
		{
			qr.hearbeat = false ;
		}
		else
		{
			return false ;
		}
	}
	return true ;
}

/**
 * @brief : Hàm cấu hình các thông số khởi động ban đầu cho động cơ
 * @return: true nếu kết nối thành công, false nếu thất bại
 */
static bool motorStarting()
{
	uint16_t timeout = 0;
	sensor_signal.motor_parameter = &Kincoparam[0] ;
	while (!Kincoparam[0].flagMotorConnected && timeout < 3000) // Đợi drive khởi động
	{
		timeout++;
		osDelay(5);
	}
	if (timeout >= 3000) {
		// Báo lỗi nếu sau 5s động cơ chưa khởi động
		return false ;
	}
	setHeartBeatTime(50, MotorID[0]);
	SetOperationMode(3, MotorID[0]);
	SDOProfileDec(SHUTTLE_DEC, MotorID[0]);
	SDOProfileAcc(SHUTTLE_ACC, MotorID[0]);
	NMTmanagement(Open, MotorID[0]);
	printf("SERVO CONNECTED\n");
	return true ;
}
/**
 * @brief : Hàm khởi động truyền thông CAN kết nối QR code
 * @return: true nếu kết nối thành công, false nếu thất bại
 */
static bool qrReaderStarting()
{
	uint16_t timeout = 0;
	while (!qr.hearbeat && timeout < 8000) // Đợi đầu đọc qr code khởi động
	{
		timeout++;
		osDelay(5);
	}
	if (timeout >= 8000) {
		// Báo lỗi nếu sau 40s đầu đọc chưa khởi động
		return false ;
	}
	NMTmanagement(Open, 0x0A);
	printf("QR CODE CONNECTED\n");
	return true ;
}
/**
 * @brief : Xử lý ngắt UART .
 * @param *huart: con trỏ ngắt uart
 * @param Size: kích thước dữ liệu nhận
 */

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	if(huart->Instance == UART7)
	{
		bmsContinute();
	}

}

void switch_mode_register_callback(ChangeMode cb)
{
	change_mode = cb ;
}
/**
 * @brief : Xử lý ngắt truyền thông CAN.
 * @param *hfdcan: con trỏ ngắt CAN
 * @param RxFifo0ITs kích thước dữ liệu nhận
 */
static CAN_COM canOpen ;
static char qr_buffer_mes[12];
void canOpenCallBack(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
	static uint32_t canId;
	canId = canOpen.RxHeader.Identifier;
	memcpy(qr_buffer_mes, canOpen.Can_rxData, 8);
	if ((canId >= 0x018A && canId <= 0x018E) || canId == 0x70A)
	{
		qrEventData(canId, (uint8_t*) qr_buffer_mes);
	}else
	{
		kincoRecieverCallback(); // hàm nhận dữ liệu của động cơ
	}
}
void canOpenInit()
{
	  FDCan_begin(&canOpen, &hfdcan1, 0 );
	  CanCofigfilter(&canOpen, 0x11, 0x11);
	  canReciverCallBack(canOpenCallBack) ;
	  Can_Start(&canOpen, MotorID[0]);
}
void IOControl(void *argument)
{
	mcp3208.begin(&hspi2,GPIOI,GPIO_PIN_0);
	canOpenInit();
	qrInit();
	MotorInit(&canOpen);
	sensor_signal.qr_sensor = &qr ;
	bmsInit(&huart7);
	pumpInit(&huart4);
	mb_ledInit(&huart6);
	configCylinderLimitSensor(&sensor_signal.di_sensor.UP_LIMIT_PK1, &sensor_signal.di_sensor.UP_LIMIT_PK2, &sensor_signal.di_sensor.DOWN_LIMIT_PK1, &sensor_signal.di_sensor.DOWN_LIMIT_PK2,
							  &sensor_signal.di_sensor.UP_LIMIT_WH1, &sensor_signal.di_sensor.UP_LIMIT_WH2, &sensor_signal.di_sensor.DOWN_LIMIT_WH1, &sensor_signal.di_sensor.DOWN_LIMIT_WH2);
	if(!motorStarting()) { sensor_signal.motor_error_state = 1 ; }
	if(!qrReaderStarting()) {sensor_signal.qr_connection_state = 1 ;}

	for(;;){
		uint64_t now = mg_millis();
		DigitalSensorRead(&sensor_signal.di_sensor);
		change_mode(sensor_signal.di_sensor.SelectMode);
		if(sensor_signal.motor_parameter->Error_code != 0) { sensor_signal.motor_error_state = 2 ; }
		if(!checkMotorConnecting()) { sensor_signal.motor_error_state = 1 ; }
		if(!checkQrReaderConnecting()){sensor_signal.qr_connection_state = 1 ; }
		getRFButtonState();
		if (u_timer_expired(&input_timer[0], 300, now)) {   // 300 ms đọc giá trị áp suất và pin 1 lần

			getDataBms();
			sensor_signal.pressure_sensor = getPressure(mcp3208.readChannel(0));
		}
		if(sensor_signal.battery.temprature >= TEMPERATURE_RUN_FAN)  // bật quạt khi quá giới hạn nhiệt độ
		{
			ControlFan(GPIO_PIN_SET);
		}else if(sensor_signal.battery.temprature <= TEMPERATURE_RUN_FAN - 3) // tắt quạt sau khi giảm 3 độ
		{
			ControlFan(GPIO_PIN_RESET);
		}
		osDelay(50/portTICK_PERIOD_MS);
	}
}

void sensorTaskInit()
{
	IOTaskHandle = osThreadNew(IOControl, NULL, &IOTask_attributes);
}
