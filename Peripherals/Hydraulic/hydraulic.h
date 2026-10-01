/*
 * hydraulic.h
 *
 *  Created on: Feb 20, 2025
 *      Author: ADMIN-HPZ2
 */

#ifndef HYDRAULIC_H_
#define HYDRAULIC_H_
#ifdef __cplusplus
extern "C" {
#endif
#include "stdint.h"
#include "stdbool.h"
#include "main.h"

#define PUMP_RX_BUFFER_SIZE 16
#define PUMP_ID 1

#define DBLS_REG_CONTROL_STATUS 0x8106U
#define DBLS_REG_STARTUP_TORQUE 0x8109U
#define DBLS_REG_ACCEL_DECEL    0x810BU
#define DBLS_REG_SPEED_CMD      0x8110U
#define DBLS_REG_SAVE_PARAMS    0x81FFU /* Ghi 0xFFFF để driver lưu các thanh ghi 0x81xx vào flash */
#define DBLS_REG_FAULT_CODE     0x820FU

#define DBLS_SAVE_PARAMS_VALUE  0xFFFFU
#define DBLS_MODE_MASK          0xFF00U /* Byte cao của 0x8106 = chế độ làm việc */

/* Reg 0x8106: High byte = Operation Mode (0x07 = Hall, internal control, internal speed, closed-loop)
 *             Low byte  = Control Status (Bit0 EN Run, Bit1 F/R Reverse, Bit2 BK Brake) */
#define DBLS_CONTROL_RUN_FORWARD 0x0701U
#define DBLS_CONTROL_RUN_REVERSE 0x0703U
#define DBLS_CONTROL_BRAKE_STOP  0x0704U /* BK=1: phanh điện, hiện không dùng */
#define DBLS_CONTROL_FREE_STOP   0x0700U /* EN=0, BK=0: dừng tự nhiên (motor trôi theo quán tính) */

#define DBLS_SPEED_RPM_DEFAULT 3500U
/* Reg 0x810B: High byte = Deceleration 1..0xFF, Low byte = Acceleration 1..0xFF, 25 RPM per unit.
 * Giá trị càng lớn ramp càng nhanh -> 0xFFFF là tăng/giảm tốc nhanh nhất (mặc định driver 0xC8C8). */
#define DBLS_ACCEL_DECEL_VALUE 0xFFFFU
/* Reg 0x8109: mô-men khởi động, mặc định driver 0x00C0. Tăng ~50% để bơm khởi động dứt khoát khi còn áp dư trong đường dầu. */
#define DBLS_STARTUP_TORQUE_VALUE 0x0120U

#define HYDRAULIC_ERROR_NONE          0U
#define HYDRAULIC_ERROR_OVERLOAD      1U
#define HYDRAULIC_ERROR_OVERTIME      2U
#define HYDRAULIC_ERROR_DRIVER_STALL  10U
#define HYDRAULIC_ERROR_DRIVER_AVG_OC 11U
#define HYDRAULIC_ERROR_DRIVER_HALL   12U
#define HYDRAULIC_ERROR_DRIVER_UV     13U
#define HYDRAULIC_ERROR_DRIVER_OV     14U
#define HYDRAULIC_ERROR_DRIVER_PEAKOC 15U
#define HYDRAULIC_ERROR_DRIVER_HWOC   16U
#define HYDRAULIC_ERROR_DRIVER_OT     17U
#define HYDRAULIC_ERROR_DRIVER_COMM   18U

typedef enum {
	CYLINDER_OFF ,
	CYLINDER_PALLET_UP ,
	CYLINDER_WHEEL_UP ,
	CYLINDER_PALLET_DOWN ,
	CYLINDER_WHEEL_DOWN
}CylinderState;

struct DriverPump
{
	uint8_t rxData[PUMP_RX_BUFFER_SIZE];
	UART_HandleTypeDef *Serial;
	uint8_t txData[8];

};

void hydraulicTaskInit();
void pumpInit(UART_HandleTypeDef *uart);
void hydraulicUartRxEventCallback(UART_HandleTypeDef *uart, uint16_t size);
void hydraulicUartErrorCallback(UART_HandleTypeDef *uart);
void hydraulicDriverPoll(void);
void configCylinderLimitSensor(uint8_t* limitUpPallet1, uint8_t* limitUpPallet2, uint8_t* limitDownPallet1, uint8_t* limitDownPallet2,
							   uint8_t* limitUpWheel1, uint8_t* limitUpWheel2, uint8_t* limitDownWheel1, uint8_t* limitDownWheel2) ;

bool controlCylinder(CylinderState cmd, bool en ) ;
bool checkErrorHydraulic();
uint16_t hydraulicGetErrorCode();
void resetErrorHydraulic();
void setHydraulicOverload();
void hydraulicEmg();
CylinderState hydraulicGetState();
#ifdef __cplusplus
}
#endif
#endif /* HYDRAULIC_H_ */
