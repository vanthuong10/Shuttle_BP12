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
static const uint64_t TIMER_LIMIT_HYDRAULIC = 7000 ; // 7000 ms
static const uint64_t TIMER_DELAY_OFF_PUMP = 100 ; // 100 ms
static const uint64_t TIMER_DELAY_ON_HYDRAULIC = 2000 ; // 2000 ms
static uint64_t timer_error_hydarulic = 0 ;
static uint64_t timer_wait_hydraulic = 0 ;
static uint64_t timer_delay_hydraulic = 0 ;

static bool hydraulic_wait = false ;
static bool hydraulic_is_overtime = false ;
static bool hydraulic_is_overload = false;
static bool hydraulic_emg = false;
static volatile bool hydraulic_driver_comm_error = false;
static bool triger_flag[4];
static bool complete_flag[2];
static CylinderState hydraulic_state = CYLINDER_OFF ;
static struct DriverPump driverPump;
static osMutexId_t pumpUartMutexHandle;
static volatile uint8_t pump_fault_code = 0;
static volatile uint32_t pump_last_fault_rx_ms = 0;
static volatile uint32_t pump_last_fault_request_ms = 0;
static uint32_t pump_command_generation = 0;
static uint32_t pump_pending_stop_generation = 0;
static volatile bool pump_is_running = false;
static uint16_t pump_last_control_value = DBLS_CONTROL_FREE_STOP;

struct HydraulicTableControl
{
	uint8_t valve1 ;
	uint8_t valve2 ;
	uint16_t control ;
	uint8_t run ;
};

const struct HydraulicTableControl wheel_up_state     = {0, 1, DBLS_CONTROL_RUN_REVERSE, 1} ;
const struct HydraulicTableControl wheel_down_state   = {0, 1, DBLS_CONTROL_RUN_FORWARD, 1} ;
const struct HydraulicTableControl pallet_up_state    = {1, 0, DBLS_CONTROL_RUN_FORWARD, 1} ;
const struct HydraulicTableControl pallet_down_state  = {1, 0, DBLS_CONTROL_RUN_REVERSE, 1} ;
const struct HydraulicTableControl stop_motor_state   = {0, 0, DBLS_CONTROL_FREE_STOP, 0} ;

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

}CylinderSetState;

static bool pumpSet(uint16_t add_reg, uint16_t val);
static void pumpRead(void);

/**
 * @brief    Khởi tạo kết nối tới driver.
 * @param uart: cấu hình uart
 * @param dma: cấu hình dma
 */
static uint32_t hydraulicNowMs(void)
{
	return HAL_GetTick();
}

static bool pumpLockUart(uint32_t timeout_ms)
{
	if(pumpUartMutexHandle == NULL || osKernelGetState() != osKernelRunning)
	{
		return true;
	}
	return osMutexAcquire(pumpUartMutexHandle, timeout_ms) == osOK;
}

static void pumpUnlockUart(void)
{
	if(pumpUartMutexHandle != NULL && osKernelGetState() == osKernelRunning)
	{
		osMutexRelease(pumpUartMutexHandle);
	}
}

static void pumpAppendCrc(uint8_t *data, uint16_t size_without_crc)
{
	uint16_t crc = crc16_modbus(data, size_without_crc);
	data[size_without_crc] = crc & 0xFF;
	data[size_without_crc + 1] = (crc >> 8) & 0xFF;
}

static void pumpStartRx(void)
{
	if(driverPump.Serial != NULL)
	{
		HAL_UARTEx_ReceiveToIdle_IT(driverPump.Serial, driverPump.rxData, PUMP_RX_BUFFER_SIZE);
	}
}

void pumpInit(UART_HandleTypeDef *uart)
{
	driverPump.Serial = uart ;
	pump_last_fault_rx_ms = hydraulicNowMs();
	if(pumpUartMutexHandle == NULL)
	{
		pumpUartMutexHandle = osMutexNew(NULL);
	}
	pumpStartRx();
	pumpSet(DBLS_REG_ACCEL_DECEL, DBLS_ACCEL_DECEL_VALUE);
	pumpSet(DBLS_REG_SPEED_CMD, DBLS_SPEED_RPM_DEFAULT);
	if(pumpSet(DBLS_REG_CONTROL_STATUS, DBLS_CONTROL_FREE_STOP))
	{
		pump_last_control_value = DBLS_CONTROL_FREE_STOP;
	}
	HAL_GPIO_WritePin(outputGpio.valveL3.Port, outputGpio.valveL3.gpioPin, GPIO_PIN_RESET);
}

/**
 * @brief    Gửi tín hiệu điều khiển bơm bằng truyền thông modbus.
 * 			 sử dụng Funtion code: 0x06 Writing single Register
 * @param add_reg: địa chỉ thanh ghi
 * @param val: giá trị set
 */
static bool pumpSet(uint16_t add_reg, uint16_t val)
{
	if(driverPump.Serial == NULL || !pumpLockUart(50))
	{
		if(pump_is_running)
		{
			hydraulic_driver_comm_error = true;
		}
		return false;
	}
	driverPump.txData[0] = PUMP_ID;
	driverPump.txData[1] = 0x06;
	driverPump.txData[2] = (add_reg >> 8) & 0xFF;
	driverPump.txData[3] = add_reg & 0xFF;
	driverPump.txData[4] = (val >> 8) & 0xFF;
	driverPump.txData[5] = val & 0xFF;
	pumpAppendCrc(driverPump.txData, 6);
	HAL_StatusTypeDef status = HAL_UART_Transmit(driverPump.Serial, driverPump.txData, 8, 100);
	pumpUnlockUart();
	if(status != HAL_OK)
	{
		if(pump_is_running)
		{
			hydraulic_driver_comm_error = true;
		}
		return false;
	}
	return true;
}

/**
 * @brief    Gửi tín hiệu request data bằng truyền thông modbus.
 * 			 sử dụng Funtion code: 0x03 Read holding Register
 */
static void pumpRead()
{
	if(driverPump.Serial == NULL || !pumpLockUart(50))
	{
		if(pump_is_running)
		{
			hydraulic_driver_comm_error = true;
		}
		return;
	}
	driverPump.txData[0] = PUMP_ID;
	driverPump.txData[1] = 0x03;
	driverPump.txData[2] = (DBLS_REG_FAULT_CODE >> 8) & 0xFF;
	driverPump.txData[3] = DBLS_REG_FAULT_CODE & 0xFF;
	driverPump.txData[4] = 0x00;
	driverPump.txData[5] = 0x01;
	pumpAppendCrc(driverPump.txData, 6);
	pump_last_fault_request_ms = hydraulicNowMs();
	if(HAL_UART_Transmit(driverPump.Serial, driverPump.txData, 8, 100) != HAL_OK && pump_is_running)
	{
		hydraulic_driver_comm_error = true;
	}
	pumpUnlockUart();
}

static bool pumpWriteControl(uint16_t control)
{
	if(control == pump_last_control_value && control == DBLS_CONTROL_FREE_STOP && !pump_is_running)
	{
		return true;
	}
	if(!pumpSet(DBLS_REG_CONTROL_STATUS, control))
	{
		if(control == DBLS_CONTROL_FREE_STOP || pump_is_running)
		{
			hydraulic_driver_comm_error = true;
		}
		return false;
	}
	pump_last_control_value = control;
	return true;
}

static void pumpRun(uint16_t control)
{
	pump_command_generation++;
	if(!pump_is_running || pump_last_control_value != control)
	{
		// Driver không giữ thanh ghi tốc độ (chỉ lưu khi ghi 0xFFFF vào 0x81FF), mặc định về 0 RPM.
		// Set lại tốc độ trước khi cấp lệnh chạy để bơm luôn chạy đúng DBLS_SPEED_RPM_DEFAULT.
		if(!pumpSet(DBLS_REG_SPEED_CMD, DBLS_SPEED_RPM_DEFAULT))
		{
			return;
		}
		if(!pumpWriteControl(control))
		{
			return;
		}
		pump_last_fault_rx_ms = hydraulicNowMs();
		hydraulic_driver_comm_error = false;
	}
	pump_is_running = true;
}

static void pumpScheduleStop(void)
{
	pump_pending_stop_generation = pump_command_generation;
	if(pumpSemaphoreHandle != NULL)
	{
		osSemaphoreRelease(pumpSemaphoreHandle);
	}
}

static uint16_t hydraulicMapDriverFault(uint8_t fault)
{
	if(fault & 0x01U) return HYDRAULIC_ERROR_DRIVER_STALL;
	if(fault & 0x02U) return HYDRAULIC_ERROR_DRIVER_AVG_OC;
	if(fault & 0x04U) return HYDRAULIC_ERROR_DRIVER_HALL;
	if(fault & 0x08U) return HYDRAULIC_ERROR_DRIVER_UV;
	if(fault & 0x10U) return HYDRAULIC_ERROR_DRIVER_OV;
	if(fault & 0x20U) return HYDRAULIC_ERROR_DRIVER_PEAKOC;
	if(fault & 0x40U) return HYDRAULIC_ERROR_DRIVER_HWOC;
	if(fault & 0x80U) return HYDRAULIC_ERROR_DRIVER_OT;
	return HYDRAULIC_ERROR_NONE;
}

static void pumpGetDriverErrorSnapshot(uint8_t *fault_code, bool *comm_error)
{
	uint32_t primask = __get_PRIMASK();
	__disable_irq();
	*fault_code = pump_fault_code;
	*comm_error = hydraulic_driver_comm_error;
	__set_PRIMASK(primask);
}

static void pumpUpdateCommTimeout(uint32_t now)
{
	uint32_t primask = __get_PRIMASK();
	__disable_irq();
	if(pump_is_running && pump_last_fault_request_ms != 0U && (now - pump_last_fault_rx_ms) > 1000U)
	{
		hydraulic_driver_comm_error = true;
	}
	__set_PRIMASK(primask);
}

void hydraulicUartRxEventCallback(UART_HandleTypeDef *uart, uint16_t size)
{
	if(driverPump.Serial == NULL || uart->Instance != driverPump.Serial->Instance)
	{
		return;
	}
	if(size >= 5U)
	{
		uint16_t crc = crc16_modbus(driverPump.rxData, size - 2U);
		uint16_t rx_crc = driverPump.rxData[size - 2U] | ((uint16_t) driverPump.rxData[size - 1U] << 8);
		if(crc == rx_crc && driverPump.rxData[0] == PUMP_ID)
		{
			if(driverPump.rxData[1] == 0x03U && size >= 7U && driverPump.rxData[2] == 0x02U)
			{
				pump_fault_code = driverPump.rxData[4];
				pump_last_fault_rx_ms = hydraulicNowMs();
				hydraulic_driver_comm_error = false;
			}
			else if((driverPump.rxData[1] & 0x80U) != 0U && pump_is_running)
			{
				hydraulic_driver_comm_error = true;
			}
		}
		else if(pump_is_running)
		{
			hydraulic_driver_comm_error = true;
		}
	}
	pumpStartRx();
}

void hydraulicDriverPoll(void)
{
	uint32_t now = hydraulicNowMs();
	pumpRead();
	pumpUpdateCommTimeout(now);
}
/**
 * @brief    Xuất tín hiệu điều khiển xylanh.
 * @param state: bảng trạng thái điều khiển xylanh
 */

void hydraulicSetState(struct HydraulicTableControl state) {
	if(state.run == 1U)
    {
        HAL_GPIO_WritePin(outputGpio.valveL1.Port, outputGpio.valveL1.gpioPin, (GPIO_PinState) state.valve1);
		HAL_GPIO_WritePin(outputGpio.valveL2.Port, outputGpio.valveL2.gpioPin, (GPIO_PinState) state.valve2);
		HAL_GPIO_WritePin(outputGpio.valveL3.Port, outputGpio.valveL3.gpioPin, GPIO_PIN_RESET);
		pumpRun(state.control);
		triger_flag[1] = true ;
    }else
    {
		// Khóa van ngay để chốt tải tại vị trí, sau TIMER_DELAY_OFF_PUMP mới cắt bơm
		HAL_GPIO_WritePin(outputGpio.valveL1.Port, outputGpio.valveL1.gpioPin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(outputGpio.valveL2.Port, outputGpio.valveL2.gpioPin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(outputGpio.valveL3.Port, outputGpio.valveL3.gpioPin, GPIO_PIN_RESET);
		pumpScheduleStop();
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
	uint8_t fault_code = 0;
	bool comm_error = false;
	pumpGetDriverErrorSnapshot(&fault_code, &comm_error);
	if(!en || hydraulic_is_overload || hydraulic_is_overtime || fault_code != 0U || comm_error) {
		hydraulicSetState(stop_motor_state);
		hydraulic_state = CYLINDER_OFF;
		//MG_DEBUG(("TẮT THỦY LỰC \n"));
		return false ;
	}
	if(u_timer_expired(&timer_wait_hydraulic, 1000, now))	{ hydraulic_wait = true ; }
	if(!hydraulic_wait && sensor_signal.di_sensor.SelectMode) return false ; //chế độ manual không chờ
	switch (cmd) {
		case CYLINDER_OFF:
			hydraulicSetState(stop_motor_state);
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
		// báo lỗi sau 7s nếu không tác động cảm biến
		hydraulic_is_overtime = true ;
	}

	if(state)
	{
		hydraulic_state = cmd;
	}else
	{
		hydraulicSetState(stop_motor_state);
		hydraulic_state = CYLINDER_OFF;
	}
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
	uint8_t fault_code = 0;
	bool comm_error = false;
	pumpGetDriverErrorSnapshot(&fault_code, &comm_error);
	return hydraulic_is_overtime || hydraulic_is_overload || hydraulic_emg || fault_code != 0U || comm_error ;
}

uint16_t hydraulicGetErrorCode()
{
	uint8_t fault_code = 0;
	bool comm_error = false;
	pumpGetDriverErrorSnapshot(&fault_code, &comm_error);
	if(hydraulic_is_overload) return HYDRAULIC_ERROR_OVERLOAD;
	if(hydraulic_is_overtime) return HYDRAULIC_ERROR_OVERTIME;
	if(comm_error) return HYDRAULIC_ERROR_DRIVER_COMM;
	return hydraulicMapDriverFault(fault_code);
}

void resetErrorHydraulic(){
	hydraulic_is_overtime = false ;
	hydraulic_is_overload = false ;
	hydraulic_emg = false ;
	hydraulic_driver_comm_error = false;
	pump_fault_code = 0;
	if(pumpWriteControl(DBLS_CONTROL_FREE_STOP))
	{
		pump_is_running = false;
		triger_flag[1] = false;
	}else
	{
		hydraulic_driver_comm_error = true;
		pumpScheduleStop();
	}
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
			uint32_t stop_generation = pump_pending_stop_generation;
			osDelay(TIMER_DELAY_OFF_PUMP);
			if(triger_flag[1] && pump_is_running && stop_generation == pump_command_generation)
			{
				if(pumpWriteControl(DBLS_CONTROL_FREE_STOP))
				{
					pump_is_running = false;
					triger_flag[1] = false;
					hydraulic_state = CYLINDER_OFF;
				}else
				{
					hydraulic_driver_comm_error = true;
					pumpScheduleStop();
				}
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
