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

#define PUMP_LOCK_TIMEOUT_MS     150U // chờ lấy quyền dùng bus RS485
#define PUMP_RESP_TIMEOUT_MS     50U  // chờ driver trả lời 1 frame
#define PUMP_FRAME_GAP_MS        3U   // khoảng lặng giữa 2 frame Modbus RTU (>= 1.75 ms)
#define PUMP_SAVE_DELAY_MS       200U // chờ driver ghi flash sau lệnh lưu 0x81FF
#define PUMP_CONFIG_MAX_RETRY    5U

typedef enum {
	PUMP_RESP_NONE = 0,  // không có phản hồi (timeout)
	PUMP_RESP_OK,
	PUMP_RESP_EXCEPTION
} PumpRespStatus;

static osSemaphoreId_t pumpRespSemaphoreHandle;
static volatile uint8_t pump_pending_func = 0;   // function code đang chờ phản hồi, 0 = không chờ
static volatile uint16_t pump_pending_reg = 0;
static volatile uint8_t pump_resp_status = PUMP_RESP_NONE;
static volatile uint16_t pump_resp_value = 0;
static volatile bool pump_config_pending = true; // chưa xác nhận được cấu hình đã lưu trong driver
static uint8_t pump_config_retry = 0;

typedef struct
{
	uint16_t reg ;
	uint16_t value ;
	uint16_t mask ;  // chỉ so sánh các bit này khi đọc lại
}PumpConfigItem;

/* Cấu hình cố định của driver, được lưu vào flash driver nên chỉ cần ghi 1 lần */
static const PumpConfigItem pump_config_table[] = {
	{ DBLS_REG_CONTROL_STATUS,  DBLS_CONTROL_FREE_STOP,    DBLS_MODE_MASK }, // chế độ 0x07: Hall, điều khiển + tốc độ qua RS485
	{ DBLS_REG_SPEED_CMD,       DBLS_SPEED_RPM_DEFAULT,    0xFFFFU },
	{ DBLS_REG_ACCEL_DECEL,     DBLS_ACCEL_DECEL_VALUE,    0xFFFFU },
	{ DBLS_REG_STARTUP_TORQUE,  DBLS_STARTUP_TORQUE_VALUE, 0xFFFFU },
};

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
static bool pumpConfigure(void);

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
	static const osMutexAttr_t pumpUartMutexAttr = { .name = "pumpUart", .attr_bits = osMutexRecursive };
	driverPump.Serial = uart ;
	pump_last_fault_rx_ms = hydraulicNowMs();
	if(pumpUartMutexHandle == NULL)
	{
		pumpUartMutexHandle = osMutexNew(&pumpUartMutexAttr);
	}
	if(pumpRespSemaphoreHandle == NULL)
	{
		pumpRespSemaphoreHandle = osSemaphoreNew(1, 0, NULL);
	}
	pumpStartRx();
	// Dừng bơm trước (phòng trường hợp MCU reset khi bơm đang chạy), sau đó mới kiểm tra cấu hình
	if(pumpSet(DBLS_REG_CONTROL_STATUS, DBLS_CONTROL_FREE_STOP))
	{
		pump_last_control_value = DBLS_CONTROL_FREE_STOP;
	}
	pump_config_retry = 1;
	pump_config_pending = !pumpConfigure();
	HAL_GPIO_WritePin(outputGpio.valveL3.Port, outputGpio.valveL3.gpioPin, GPIO_PIN_RESET);
}

/**
 * @brief    Gửi 1 frame Modbus RTU và chờ driver trả lời.
 * 			 RS485 half-duplex: phải chờ driver trả lời xong (hoặc timeout) và giữ khoảng lặng
 * 			 giữa 2 frame, nếu không frame sau sẽ đè lên phản hồi của frame trước.
 * @param func: 0x03 đọc / 0x06 ghi 1 thanh ghi
 * @param add_reg: địa chỉ thanh ghi
 * @param val: giá trị ghi (0x06) hoặc số thanh ghi cần đọc (0x03)
 * @param resp_status: trạng thái phản hồi (PumpRespStatus), có thể NULL
 * @param resp_value: giá trị đọc được (0x03), có thể NULL
 * @return   false nếu không gửi được frame
 */
static bool pumpTransaction(uint8_t func, uint16_t add_reg, uint16_t val, uint8_t *resp_status, uint16_t *resp_value)
{
	if(driverPump.Serial == NULL || !pumpLockUart(PUMP_LOCK_TIMEOUT_MS))
	{
		return false;
	}
	bool rtos_running = osKernelGetState() == osKernelRunning && pumpRespSemaphoreHandle != NULL;
	if(rtos_running)
	{
		while(osSemaphoreAcquire(pumpRespSemaphoreHandle, 0) == osOK) {} // bỏ tín hiệu cũ
	}
	driverPump.txData[0] = PUMP_ID;
	driverPump.txData[1] = func;
	driverPump.txData[2] = (add_reg >> 8) & 0xFF;
	driverPump.txData[3] = add_reg & 0xFF;
	driverPump.txData[4] = (val >> 8) & 0xFF;
	driverPump.txData[5] = val & 0xFF;
	pumpAppendCrc(driverPump.txData, 6);
	pump_resp_status = PUMP_RESP_NONE;
	pump_pending_reg = add_reg;
	pump_pending_func = func;
	HAL_StatusTypeDef status = HAL_UART_Transmit(driverPump.Serial, driverPump.txData, 8, 100);
	if(status == HAL_OK && rtos_running)
	{
		osSemaphoreAcquire(pumpRespSemaphoreHandle, PUMP_RESP_TIMEOUT_MS);
	}
	pump_pending_func = 0; // phản hồi đến muộn sẽ bị bỏ qua
	if(resp_status != NULL) *resp_status = pump_resp_status;
	if(resp_value != NULL) *resp_value = pump_resp_value;
	if(rtos_running)
	{
		osDelay(PUMP_FRAME_GAP_MS);
	}
	pumpUnlockUart();
	return status == HAL_OK;
}

/**
 * @brief    Gửi tín hiệu điều khiển bơm bằng truyền thông modbus.
 * 			 sử dụng Funtion code: 0x06 Writing single Register
 * 			 Driver không trả lời vẫn coi là đã gửi (mất kết nối được phát hiện qua polling mã lỗi).
 * @param add_reg: địa chỉ thanh ghi
 * @param val: giá trị set
 */
static bool pumpSet(uint16_t add_reg, uint16_t val)
{
	uint8_t resp = PUMP_RESP_NONE;
	if(!pumpTransaction(0x06, add_reg, val, &resp, NULL) || resp == PUMP_RESP_EXCEPTION)
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
 * @brief    Đọc 1 thanh ghi của driver (Funtion code 0x03).
 * @return   true nếu driver trả lời hợp lệ
 */
static bool pumpReadReg(uint16_t add_reg, uint16_t *value)
{
	uint8_t resp = PUMP_RESP_NONE;
	return pumpTransaction(0x03, add_reg, 0x0001, &resp, value) && resp == PUMP_RESP_OK;
}

/**
 * @brief    Gửi tín hiệu request mã lỗi driver bằng truyền thông modbus.
 * 			 sử dụng Funtion code: 0x03 Read holding Register
 */
static void pumpRead()
{
	pump_last_fault_request_ms = hydraulicNowMs();
	if(!pumpTransaction(0x03, DBLS_REG_FAULT_CODE, 0x0001, NULL, NULL) && pump_is_running)
	{
		hydraulic_driver_comm_error = true;
	}
}

/**
 * @brief    Đọc lại cấu hình trong driver, chỉ ghi các thanh ghi bị sai rồi lưu flash (0x81FF = 0xFFFF).
 * 			 Driver đã lưu đúng thì chỉ đọc, không ghi -> không tốn chu kỳ ghi flash mỗi lần khởi động
 * 			 và các lần chạy sau không cần set lại tốc độ/gia tốc.
 * @return   true nếu cấu hình trong driver đã đúng và đã được lưu
 */
static bool pumpConfigure(void)
{
	bool changed = false;
	for(uint8_t i = 0; i < sizeof(pump_config_table) / sizeof(pump_config_table[0]); i++)
	{
		const PumpConfigItem *item = &pump_config_table[i];
		uint16_t value = 0;
		if(!pumpReadReg(item->reg, &value)) return false;
		if((value & item->mask) == (item->value & item->mask)) continue;
		if(pump_is_running) return false; // không đổi cấu hình khi bơm đang chạy
		if(!pumpSet(item->reg, item->value)) return false;
		if(item->reg == DBLS_REG_CONTROL_STATUS)
		{
			pump_last_control_value = item->value;
		}
		if(!pumpReadReg(item->reg, &value) || (value & item->mask) != (item->value & item->mask)) return false;
		changed = true;
	}
	if(changed)
	{
		if(pump_is_running || !pumpLockUart(PUMP_LOCK_TIMEOUT_MS)) return false;
		// Giữ bus trong lúc driver ghi flash để không task nào gửi lệnh chen vào
		bool saved = pumpSet(DBLS_REG_SAVE_PARAMS, DBLS_SAVE_PARAMS_VALUE);
		if(saved)
		{
			osDelay(PUMP_SAVE_DELAY_MS);
			printf("Pump driver: da luu cau hinh vao flash\n");
		}
		pumpUnlockUart();
		return saved;
	}
	return true;
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
		// Tốc độ/gia tốc đã được lưu trong flash driver (pumpConfigure). Chỉ khi chưa xác nhận được
		// cấu hình thì mới set lại tốc độ trước khi chạy như cách cũ.
		if(pump_config_pending && !pumpSet(DBLS_REG_SPEED_CMD, DBLS_SPEED_RPM_DEFAULT))
		{
			return;
		}
		// Manual driver: đổi chiều F/R phải tắt EN trước
		if(pump_is_running && !pumpWriteControl(DBLS_CONTROL_FREE_STOP))
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

static void pumpUpdateCommTimeout(void)
{
	uint32_t primask = __get_PRIMASK();
	__disable_irq();
	uint32_t now = hydraulicNowMs(); // lấy sau khi khóa ngắt để không nhỏ hơn pump_last_fault_rx_ms
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
		if(crc == rx_crc && driverPump.rxData[0] == PUMP_ID && pump_pending_func != 0U)
		{
			uint8_t func = driverPump.rxData[1];
			bool done = false;
			if(func == 0x03U && pump_pending_func == 0x03U && size >= 7U && driverPump.rxData[2] == 0x02U)
			{
				pump_resp_value = ((uint16_t) driverPump.rxData[3] << 8) | driverPump.rxData[4];
				if(pump_pending_reg == DBLS_REG_FAULT_CODE)
				{
					pump_fault_code = driverPump.rxData[4];
					pump_last_fault_rx_ms = hydraulicNowMs();
					hydraulic_driver_comm_error = false;
				}
				pump_resp_status = PUMP_RESP_OK;
				done = true;
			}
			else if(func == 0x06U && pump_pending_func == 0x06U && size >= 8U)
			{
				pump_resp_status = PUMP_RESP_OK;
				done = true;
			}
			else if((func & 0x80U) != 0U)
			{
				if(pump_is_running)
				{
					hydraulic_driver_comm_error = true;
				}
				pump_resp_status = PUMP_RESP_EXCEPTION;
				done = true;
			}
			if(done)
			{
				pump_pending_func = 0;
				if(pumpRespSemaphoreHandle != NULL)
				{
					osSemaphoreRelease(pumpRespSemaphoreHandle);
				}
			}
		}
		else if(crc != rx_crc && pump_is_running)
		{
			hydraulic_driver_comm_error = true;
		}
	}
	pumpStartRx();
}

/**
 * @brief    Lỗi UART (overrun/noise...) làm HAL hủy nhận ReceiveToIdle -> bật lại để không mất kết nối driver.
 */
void hydraulicUartErrorCallback(UART_HandleTypeDef *uart)
{
	if(driverPump.Serial == NULL || uart->Instance != driverPump.Serial->Instance)
	{
		return;
	}
	pumpStartRx();
}

void hydraulicDriverPoll(void)
{
	if(pump_config_pending && !pump_is_running && pump_config_retry < PUMP_CONFIG_MAX_RETRY)
	{
		pump_config_retry++;
		pump_config_pending = !pumpConfigure();
	}
	pumpRead();
	pumpUpdateCommTimeout();
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
	// Driver có thể đã bị mất nguồn (EMG/lỗi) -> đọc lại cấu hình ở lần poll kế tiếp
	pump_config_pending = true;
	pump_config_retry = 0;
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
