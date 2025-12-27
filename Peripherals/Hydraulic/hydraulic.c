/*
 * hydraulic.c
 *
 *  Created on: Feb 20, 2025
 *      Author: ADMIN-HPZ2
 */
#include "hydraulic.h"
#include "u_gpio.h"
#include "user_custom.h"
#include "cmsis_os.h"
#include "stdio.h"
#include "mongoose.h"
#include "sensorSignal.h"

static osSemaphoreId_t pumpSemaphoreHandle; // Semaphore cho bơm
static const uint64_t TIMER_LIMIT_HYDRAULIC = 6000 ; // 6000 ms
static const uint64_t TIMER_DELAY_OFF_PUMP = 300 ; // 500 ms
static const uint64_t TIMER_DELAY_ON_HYDRAULIC = 2000 ; // 1000 ms
static uint64_t timer_error_hydarulic = 0 ;
static uint64_t timer_wait_hydraulic = 0 ;
static uint64_t timer_delay_hydraulic = 0 ;

static bool hydraulic_wait = false ;
static bool hydraulic_is_overtime = false ;
static bool hydraulic_is_overload = false;
static bool hydraulic_emg = false;
static bool triger_flag[4];
static bool complete_flag[2];
static CylinderState hydraulic_state = CYLINDER_OFF ;
static struct DriverPump driverPump;

struct HydraulicTableControl
{
	uint8_t valve1 ;
	uint8_t valve2 ;
	uint8_t valve3 ;
	uint8_t pump ;
};

const struct HydraulicTableControl wheel_up_state     = {0, 0, 1, 1} ;
const struct HydraulicTableControl wheel_down_state   = {1, 0, 1, 1} ;
const struct HydraulicTableControl pallet_up_state    = {1, 1, 0, 1} ;
const struct HydraulicTableControl pallet_down_state  = {0, 1, 0, 1} ;
const struct HydraulicTableControl free_all_state     = {0, 0, 0, 0} ;

static uint8_t* lmss_pallet_up_1 ;
static uint8_t* lmss_pallet_up_2 ;
static uint8_t* lmss_pallet_down_1 ;
static uint8_t* lmss_pallet_down_2 ;
static uint8_t* lmss_wheel_up_1 ;
static uint8_t* lmss_wheel_up_2 ;
static uint8_t* lmss_wheel_down_1 ;
static uint8_t* lmss_wheel_down_2 ;

typedef struct
{
	struct HydraulicTableControl wheel_up ;
	struct HydraulicTableControl wheel_down ;
	struct HydraulicTableControl pallet_up ;
	struct HydraulicTableControl pallet_down ;
	struct HydraulicTableControl free_all ;

}CylinderSetState;

/**
 * @brief    Khởi tạo kết nối tới driver.
 * @param uart: cấu hình uart
 * @param dma: cấu hình dma
 */
void pumpInit(UART_HandleTypeDef *uart)
{
	 driverPump.Serial = uart ;
	 HAL_UARTEx_ReceiveToIdle_IT(uart, driverPump.rxData, PUMP_RX_BUFFER_SIZE);
}

/**
 * @brief    Gửi tín hiệu điều khiển bơm bằng truyền thông modbus.
 * 			 sử dụng Funtion code: 0x06 Writing single Register
 * @param add_reg: địa chỉ thanh ghi
 * @param val: giá trị set
 */
void pumpSet(uint16_t add_reg, uint16_t val)
{
	driverPump.txData[0] = PUMP_ID;
	driverPump.txData[1] = 0x06;  // Function code
	driverPump.txData[2] = (add_reg >> 8) & 0xFF;  // MSB of register address
	driverPump.txData[3] = add_reg & 0xFF;         // LSB of register address
	driverPump.txData[4] = (val >> 8) & 0xFF;  // MSB of number of registers
	driverPump.txData[5] = val & 0xFF;         // LSB of number of registers

    // Calculate CRC
    uint16_t crc = crc16_modbus(driverPump.txData, 6);   // Compute CRC for the first 6 bytes
    driverPump.txData[6] = crc & 0xFF;                  // CRC Low Byte
    driverPump.txData[7] = (crc >> 8) & 0xFF;           // CRC High Byte
    HAL_UART_Transmit(driverPump.Serial, driverPump.txData, 8, 100);
}

/**
 * @brief    Gửi tín hiệu request data bằng truyền thông modbus.
 * 			 sử dụng Funtion code: 0x03 Read holding Register
 */
void pumpRead()
{
	driverPump.txData[0] = PUMP_ID;
	driverPump.txData[1] = 0x03;  // Function code
	driverPump.txData[2] = (PUMP_START_ADD>> 8) & 0xFF;  // MSB of register address
	driverPump.txData[3] = PUMP_START_ADD & 0xFF;         // LSB of register address
	driverPump.txData[4] = (PUMP_NUMBER_OF_REGISTER >> 8) & 0xFF;  // MSB of number of registers
	driverPump.txData[5] = PUMP_NUMBER_OF_REGISTER & 0xFF;         // LSB of number of registers

    // Calculate CRC
    uint16_t crc = crc16_modbus(driverPump.txData, 6);   // Compute CRC for the first 6 bytes
    driverPump.txData[6] = crc & 0xFF;                  // CRC Low Byte
    driverPump.txData[7] = (crc >> 8) & 0xFF;           // CRC High Byte
    HAL_UART_Transmit(driverPump.Serial, driverPump.txData, 8, 100);
}
/**
 * @brief    Xuất tín hiệu điều khiển xylanh.
 * @param state: bảng trạng thái điều khiển xylanh
 */

void hydraulicSetState(struct HydraulicTableControl state) {
    HAL_GPIO_WritePin(outputGpio.valveL2.Port, outputGpio.valveL2.gpioPin, (GPIO_PinState) state.valve2);
    HAL_GPIO_WritePin(outputGpio.valveL3.Port, outputGpio.valveL3.gpioPin, (GPIO_PinState) state.valve3);
    if(state.pump == 1)
    {
        HAL_GPIO_WritePin(outputGpio.valveL1.Port, outputGpio.valveL1.gpioPin, (GPIO_PinState) state.valve1);
		if (!triger_flag[1]) {
			pumpSet(PUMP_ENABLE_REG, 1);
			triger_flag[1] = true ;
		}
    }else
    {
    	osSemaphoreRelease(pumpSemaphoreHandle); // Giải phóng semaphore khi tắt bơm
    }
}

bool controlCylinder(CylinderState cmd, bool en )
{
	if(hydraulic_emg) en = false ;   // nếu emg không chạy bơm
	if(detectFlagRisingEdge(en, &triger_flag[0])) {
		// Reset lại timer mỗi lần chạy bơm
		timer_error_hydarulic = 0 ;
		timer_wait_hydraulic = 0 ;
		hydraulic_wait = false ;
		timer_delay_hydraulic = 0 ;
		complete_flag[0] = false;
	}

	bool state = false;
	uint64_t now = (uint64_t) ((osKernelGetTickCount() * 1000) / osKernelGetTickFreq());
	if(!en || hydraulic_is_overload || hydraulic_is_overtime) {
		hydraulicSetState(free_all_state);
		//MG_DEBUG(("TẮT THỦY LỰC \n"));
		return false ;
	}
	if(u_timer_expired(&timer_wait_hydraulic, 1000, now))	{ hydraulic_wait = true ; }
	if(!hydraulic_wait && sensor_signal.di_sensor.SelectMode) return false ; //chế độ manual không chờ
	switch (cmd) {
		case CYLINDER_OFF:
			hydraulicSetState(free_all_state);
			break;
		case CYLINDER_PALLET_UP:
			state = *lmss_pallet_up_1 == 0 || *lmss_pallet_up_2 == 0 ;
			bool temp1 = *lmss_pallet_up_1 == 1 || *lmss_pallet_up_2 == 1 ;
			if(temp1)
			{
				if(u_timer_expired(&timer_delay_hydraulic, TIMER_DELAY_ON_HYDRAULIC, now) || complete_flag[0])
				{
					state = false ;
					 complete_flag[0] = true ;
				}
			}
			if(state)
			{
				hydraulicSetState(pallet_up_state);
		    	//MG_DEBUG(("NÂNG PALLET \n"));
			}
			break;
		case CYLINDER_WHEEL_UP:
			state = *lmss_wheel_up_1 == 0 || *lmss_wheel_up_2 == 0 ;
			if(state)
			{
				hydraulicSetState(wheel_up_state);
		    	//MG_DEBUG(("NÂNG BÁNH XE \n"));
			}
			break;
		case CYLINDER_PALLET_DOWN:
			state = *lmss_pallet_down_1 == 0 || *lmss_pallet_down_2 == 0 ;
			bool temp2 = *lmss_pallet_down_1 == 1 || *lmss_pallet_down_2 == 1 ;
			if(temp2)
			{
				if(u_timer_expired(&timer_delay_hydraulic, TIMER_DELAY_ON_HYDRAULIC, now) ||  complete_flag[0])
				{
					state = false ;
					 complete_flag[0] = true ;
				}
			}
			if(state)
			{
				hydraulicSetState(pallet_down_state);
		    	//MG_DEBUG(("HẠ PALLET \n"));

			}
			break;
		case CYLINDER_WHEEL_DOWN:
			state = *lmss_wheel_down_1 == 0 || *lmss_wheel_down_2 == 0 ;
			if(state)
			{
				hydraulicSetState(wheel_down_state);
		    	//MG_DEBUG(("HẠ BÁNH XE \n"));
			}
			break;
		default:
			break;
	}

	if(u_timer_expired(&timer_error_hydarulic, TIMER_LIMIT_HYDRAULIC, now) && state)
	{
		// báo lỗi sau 5s nếu không tác động cảm biến
		hydraulic_is_overtime = true ;
	}

	if(!state) { hydraulicSetState(free_all_state); }
	return !state ;
}

void configCylinderLimitSensor(uint8_t* limitUpPallet1, uint8_t* limitUpPallet2, uint8_t* limitDownPallet1, uint8_t* limitDownPallet2 ,
							   uint8_t* limitUpWheel1, uint8_t* limitUpWheel2, uint8_t* limitDownWheel1, uint8_t* limitDownWheel2)
{
	lmss_pallet_down_1 = limitDownPallet1 ;
	lmss_pallet_down_2 = limitDownPallet2 ;
	lmss_pallet_up_1   = limitUpPallet1 ;
	lmss_pallet_up_2   = limitUpPallet2 ;
	lmss_wheel_down_1  = limitDownWheel1 ;
	lmss_wheel_down_2  = limitDownWheel2 ;
	lmss_wheel_up_1    = limitUpWheel1   ;
	lmss_wheel_up_2    = limitUpWheel2  ;
}

bool checkErrorHydraulic() {
	return hydraulic_is_overtime ;
}

void resetErrorHydraulic(){
	hydraulic_is_overtime = false ;
	hydraulic_is_overload = false ;
	hydraulic_emg = false ;
	pumpSet(PUMP_RESET_ERROR_REG, 1);
}
// hàm set quá tải shuttle
void setHydraulicOverload()
{
	hydraulic_is_overload = true ;
}
// hàm set quá tải shuttle
void hydraulicEmg()
{
	hydraulic_emg = true ;
}
CylinderState hydraulicGetState()
{
	return hydraulic_state ;
}
/************ TASK XỬ LÝ THUỶ LỰC*****************/
static void hydraulicOffTask(void *argument) {
    for (;;) {
        if (osSemaphoreAcquire(pumpSemaphoreHandle, osWaitForever) == osOK) {
            osDelay(TIMER_DELAY_OFF_PUMP); // Đợi 100ms
            HAL_GPIO_WritePin(outputGpio.valveL1.Port, outputGpio.valveL1.gpioPin, GPIO_PIN_RESET); // tắt van L1
            if(triger_flag[1])
            {
                pumpSet(PUMP_ENABLE_REG, 0);
                triger_flag[1] = false;
            }
        }
    }
}
osThreadAttr_t hydraulicTaskAttr = {
        .name = "pumpOffTask",
        .stack_size = 128 * 4,
        .priority = osPriorityAboveNormal2
};

osThreadId_t hydraulicTaskHandle;
void hydraulicTaskInit()
{
	pumpSemaphoreHandle = osSemaphoreNew(1, 0, NULL);
	hydraulicTaskHandle = osThreadNew(hydraulicOffTask, NULL, &hydraulicTaskAttr);
}
