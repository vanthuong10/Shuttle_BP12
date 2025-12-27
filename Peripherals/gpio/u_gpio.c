/*
 * u_gpio.c
 *
 *  Created on: Feb 19, 2025
 *      Author: ADMIN-HPZ2
 */
#include "u_gpio.h"

/* @brief     Configure output signal pins
*
*            Edit Port and gpioPin parameters according to hardware connection
*/
DoHandle outputGpio = {
    .resetDriver = { .Port = GPIOF, .gpioPin = GPIO_PIN_5  },  // Y0  PF5  – Reset driver
    .fan         = { .Port = GPIOF, .gpioPin = GPIO_PIN_3  },  // Y1  PF3  – Quạt làm mát
    .buzzer      = { .Port = GPIOI, .gpioPin = GPIO_PIN_11 },  // Y2  PI11 – Còi (buzzer)
    .charger     = { .Port = GPIOI, .gpioPin = GPIO_PIN_10 },  // Y3  PI10 – Sạc/charger
    .pump        = { .Port = GPIOF, .gpioPin = GPIO_PIN_14 },  // Y31 PF14 – Bơm
    .valveL1     = { .Port = GPIOC, .gpioPin = GPIO_PIN_13 },  // Y4  PC13 – Van L1
    .valveL2     = { .Port = GPIOI, .gpioPin = GPIO_PIN_8  },  // Y5  PI8  – Van L2
    .valveL3     = { .Port = GPIOG, .gpioPin = GPIO_PIN_15 },  // Y6  PG15 – Van L3
    .qStop       = { .Port = GPIOK, .gpioPin = GPIO_PIN_4  },  // Y7  PK4  – Ngắt khẩn (Q-Stop)
};


/* @brief     Configure input signal pins
*
*            Edit Port and gpioPin parameters according to hardware connection
*/

GPIO_UserHandle PalletSensor1       = { .Port = GPIOH, .gpioPin = GPIO_PIN_5  }; // X0  PH5  – Cảm biến pallet 1
GPIO_UserHandle PackageSensorX1     = { .Port = GPIOA, .gpioPin = GPIO_PIN_0  }; // X1  PA0  – Cảm biến kiện X1
GPIO_UserHandle BarrierSensorX1     = { .Port = GPIOB, .gpioPin = GPIO_PIN_0  }; // X2  PB0  – Rào cản X1
// X3 PJ3  – bỏ trống
GPIO_UserHandle BarrierSensorY1     = { .Port = GPIOJ, .gpioPin = GPIO_PIN_4  }; // X4  PJ4  – Rào cản Y1
GPIO_UserHandle UpPalletSwitch1     = { .Port = GPIOG, .gpioPin = GPIO_PIN_1  }; // X5  PG1  – Hành trình pallet 1 trên
GPIO_UserHandle DownPalletSwitch1   = { .Port = GPIOH, .gpioPin = GPIO_PIN_3  }; // X6  PH3  – Hành trình pallet 1 dưới
// X7 PH2  – bỏ trống
GPIO_UserHandle UpWheelSwitch2      = { .Port = GPIOF, .gpioPin = GPIO_PIN_4  }; // X8  PF4  – Hành trình bánh 2 trên
GPIO_UserHandle DownWheelSwitch2    = { .Port = GPIOC, .gpioPin = GPIO_PIN_2  }; // X9  PC2  – Hành trình bánh 2 dưới
// X10 PC3 – bỏ trống
GPIO_UserHandle UpWheelSwitch1      = { .Port = GPIOJ, .gpioPin = GPIO_PIN_0  }; // X11 PJ0  – Hành trình bánh 1 trên
GPIO_UserHandle DownWheelSwitch1    = { .Port = GPIOG, .gpioPin = GPIO_PIN_0  }; // X12 PG0  – Hành trình bánh 1 dưới
GPIO_UserHandle PalletSensor2       = { .Port = GPIOF, .gpioPin = GPIO_PIN_11 }; // X13 PF11 – Cảm biến pallet 2
GPIO_UserHandle PackageSensorX2     = { .Port = GPIOJ, .gpioPin = GPIO_PIN_2  }; // X14 PJ2  – Cảm biến kiện X2
GPIO_UserHandle BarrierSensorX2     = { .Port = GPIOB, .gpioPin = GPIO_PIN_1  }; // X15 PB1 – Rào cản X2
// X16 PI15 – bỏ trống
GPIO_UserHandle BarrierSensorY2     = { .Port = GPIOH, .gpioPin = GPIO_PIN_4  }; // X17 PH4  – Rào cản Y2
GPIO_UserHandle UpPalletSwitch2     = { .Port = GPIOJ, .gpioPin = GPIO_PIN_6  }; // X18 PJ6  – Hành trình pallet 2 trên
GPIO_UserHandle DownPalletSwitch2   = { .Port = GPIOD, .gpioPin = GPIO_PIN_15 }; // X19 PD15 – Hành trình pallet 2 dưới
GPIO_UserHandle RfResetError        = { .Port = GPIOH, .gpioPin = GPIO_PIN_12 }; // X20 PH12 – RF reset lỗi
GPIO_UserHandle EmgSwitch           = { .Port = GPIOJ, .gpioPin = GPIO_PIN_1  }; // X28 PJ5  – Nút dừng khẩn
GPIO_UserHandle RfForwardSwitch     = { .Port = GPIOE, .gpioPin = GPIO_PIN_15 }; // X22 PE15 – RF tiến
GPIO_UserHandle RfReverseSwitch     = { .Port = GPIOE, .gpioPin = GPIO_PIN_12 }; // X23 PE12 – RF lùi
GPIO_UserHandle RfUpPalletSwitch    = { .Port = GPIOH, .gpioPin = GPIO_PIN_6  }; // X24 PH6  – RF nâng pallet
GPIO_UserHandle RfDownPalletSwitch  = { .Port = GPIOK, .gpioPin = GPIO_PIN_5  }; // X25 PK5  – RF hạ pallet
GPIO_UserHandle RfUpWheelSwitch     = { .Port = GPIOG, .gpioPin = GPIO_PIN_10 }; // X26 PG10 – RF nâng bánh
GPIO_UserHandle RfDownWheelSwitch   = { .Port = GPIOB, .gpioPin = GPIO_PIN_2  }; // X27 PB2  – RF hạ bánh
GPIO_UserHandle OpenBrake           = { .Port = GPIOJ, .gpioPin = GPIO_PIN_1  }; // X28 PJ1  – Mở phanh
// X29 PF12 – chưa gán
GPIO_UserHandle OverTemmpature      = { .Port = GPIOF, .gpioPin = GPIO_PIN_13 }; // X30 PF13 – Báo quá nhiệt
GPIO_UserHandle SwitchSelectMode    = { .Port = GPIOB, .gpioPin = GPIO_PIN_10 }; // X31 PB10 – Chọn chế độ

// InRail chuyển sang X32..X35
GPIO_UserHandle InRailFront         = { .Port = GPIOB, .gpioPin = GPIO_PIN_11 }; // X32 PB11 – Ray trước
GPIO_UserHandle InRailRight         = { .Port = GPIOH, .gpioPin = GPIO_PIN_9  }; // X33 PH9  – Ray phải
GPIO_UserHandle InRailBehind        = { .Port = GPIOD, .gpioPin = GPIO_PIN_11 }; // X34 PD11 – Ray sau
GPIO_UserHandle InRailLeft          = { .Port = GPIOJ, .gpioPin = GPIO_PIN_7  }; // X35 PJ7  – Ray trái


static SENSOR_STATUS inputSignal ;
/**
 * @brief Đọc tín hiệu digital đầu vào.
 * @param sensor: Con trỏ nhận dữ liệu trả về
 */
void DigitalSensorRead(SENSOR_STATUS *sensor)
{
	inputSignal.SelectMode		=  HAL_GPIO_ReadPin(SwitchSelectMode.Port, SwitchSelectMode.gpioPin);
	inputSignal.S1_ISPALLET 	=  HAL_GPIO_ReadPin(PalletSensor1.Port, PalletSensor1.gpioPin);
	inputSignal.S2_ISPALLET 	=  HAL_GPIO_ReadPin(PalletSensor2.Port, PalletSensor2.gpioPin);
//	inputSignal.X1_BARRIER  	=  HAL_GPIO_ReadPin(BarrierSensorX1.Port, BarrierSensorX1.gpioPin) == 0 ? HIGH : LOW;
//	inputSignal.X2_BARRIER  	=  HAL_GPIO_ReadPin(BarrierSensorX2.Port, BarrierSensorX2.gpioPin) == 0 ? HIGH : LOW;
//	inputSignal.Y1_BARRIER  	=  HAL_GPIO_ReadPin(BarrierSensorY1.Port, BarrierSensorY1.gpioPin) == 0 ? HIGH : LOW;
//	inputSignal.Y2_BARRIER  	=  HAL_GPIO_ReadPin(BarrierSensorY2.Port, BarrierSensorY2.gpioPin) == 0 ? HIGH : LOW;
//	inputSignal.X1_PACKAGE  	=  HAL_GPIO_ReadPin(PackageSensorX1.Port, PackageSensorX1.gpioPin) == 0 ? HIGH : LOW;
//	inputSignal.X2_PACKAGE  	=  HAL_GPIO_ReadPin(PackageSensorX2.Port, PackageSensorX2.gpioPin) == 0 ? HIGH : LOW;

	inputSignal.X1_BARRIER  	=  LOW;
	inputSignal.X2_BARRIER  	=  LOW;
	inputSignal.Y1_BARRIER  	=  LOW;
	inputSignal.Y2_BARRIER  	=  LOW;
	inputSignal.X1_PACKAGE  	=  LOW;
	inputSignal.X2_PACKAGE  	=  LOW;

	inputSignal.S1_RAIL_FRONT   =  HAL_GPIO_ReadPin(InRailFront.Port, InRailFront.gpioPin);
	inputSignal.S2_RAIL_RIGHT   =  HAL_GPIO_ReadPin(InRailRight.Port, InRailRight.gpioPin);
	inputSignal.S3_RAIL_BEHIND  =  HAL_GPIO_ReadPin(InRailBehind.Port, InRailBehind.gpioPin);
	inputSignal.S4_RAIL_LEFT   	=  HAL_GPIO_ReadPin(InRailLeft.Port, InRailLeft.gpioPin);
	inputSignal.DOWN_LIMIT_WH1 	=  HAL_GPIO_ReadPin(DownWheelSwitch1.Port, DownWheelSwitch1.gpioPin);
	inputSignal.UP_LIMIT_WH1 	=  HAL_GPIO_ReadPin(UpWheelSwitch1.Port, UpWheelSwitch1.gpioPin);
	inputSignal.DOWN_LIMIT_WH2 	=  HAL_GPIO_ReadPin(DownWheelSwitch2.Port, DownWheelSwitch2.gpioPin);
	inputSignal.UP_LIMIT_WH2 	=  HAL_GPIO_ReadPin(UpWheelSwitch2.Port, UpWheelSwitch2.gpioPin);
	inputSignal.UP_LIMIT_PK2    =  HAL_GPIO_ReadPin(UpPalletSwitch2.Port, UpPalletSwitch2.gpioPin);
	inputSignal.DOWN_LIMIT_PK2  =  HAL_GPIO_ReadPin(DownPalletSwitch2.Port, DownPalletSwitch2.gpioPin);
	inputSignal.UP_LIMIT_PK1    =  HAL_GPIO_ReadPin(UpPalletSwitch1.Port, UpPalletSwitch1.gpioPin);
	inputSignal.DOWN_LIMIT_PK1  =  HAL_GPIO_ReadPin(DownPalletSwitch1.Port, DownPalletSwitch1.gpioPin);
	inputSignal.EMG_BUTTON 		=  HAL_GPIO_ReadPin(EmgSwitch.Port, EmgSwitch.gpioPin) == 0 ? HIGH : LOW;
	inputSignal.RF_FORWARD 		=  HAL_GPIO_ReadPin(RfForwardSwitch.Port, RfForwardSwitch.gpioPin);
	inputSignal.RF_REVERSE		=  HAL_GPIO_ReadPin(RfReverseSwitch.Port, RfReverseSwitch.gpioPin);
	inputSignal.RF_PALLET_UP 	=  HAL_GPIO_ReadPin(RfUpPalletSwitch.Port, RfUpPalletSwitch.gpioPin);
	inputSignal.RF_PALLET_DOWN  =  HAL_GPIO_ReadPin(RfDownPalletSwitch.Port, RfDownPalletSwitch.gpioPin);
	inputSignal.RF_WHEEL_UP 	=  HAL_GPIO_ReadPin(RfUpWheelSwitch.Port, RfUpWheelSwitch.gpioPin);
	inputSignal.RF_WHEEL_DOWN 	=  HAL_GPIO_ReadPin(RfDownWheelSwitch.Port, RfDownWheelSwitch.gpioPin);
	inputSignal.RF_RESET        =  HAL_GPIO_ReadPin(RfResetError.Port, RfResetError.gpioPin);
	inputSignal.OPEN_BRAKE		= HAL_GPIO_ReadPin(OpenBrake.Port, OpenBrake.gpioPin);
	inputSignal.PUMP_OVER_TEMPERATURE = HAL_GPIO_ReadPin(OverTemmpature.Port, OverTemmpature.gpioPin);
	*sensor = inputSignal ;
}
/**
 * @brief Reset ngõ ra .
 * @param: Đặt tất cả trạng thái ngõ ra về GPIO_PIN_RESET
 */
void ResetOutput()
{
	HAL_GPIO_WritePin(outputGpio.buzzer.Port, outputGpio.buzzer.gpioPin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(outputGpio.charger.Port, outputGpio.charger.gpioPin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(outputGpio.fan.Port, outputGpio.fan.gpioPin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(outputGpio.pump.Port, outputGpio.pump.gpioPin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(outputGpio.resetDriver.Port, outputGpio.resetDriver.gpioPin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(outputGpio.valveL1.Port, outputGpio.valveL1.gpioPin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(outputGpio.valveL2.Port, outputGpio.valveL2.gpioPin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(outputGpio.valveL3.Port, outputGpio.valveL3.gpioPin, GPIO_PIN_RESET);
}

/**
 * @brief Điều khiển ngõ ra Buzzer.
 * @param state: Trạng thái cần đặt cho Buzzer (GPIO_PIN_SET hoặc GPIO_PIN_RESET)
 */
void ControlBuzzer(GPIO_PinState state)
{
    HAL_GPIO_WritePin(outputGpio.buzzer.Port, outputGpio.buzzer.gpioPin, state);
}

/**
 * @brief Điều khiển ngõ ra Charger.
 * @param state: Trạng thái cần đặt cho Charger (GPIO_PIN_SET hoặc GPIO_PIN_RESET)
 */
void ControlCharger(GPIO_PinState state)
{
    HAL_GPIO_WritePin(outputGpio.charger.Port, outputGpio.charger.gpioPin, state);
}

/**
 * @brief Điều khiển ngõ ra Fan.
 * @param state: Trạng thái cần đặt cho Fan (GPIO_PIN_SET hoặc GPIO_PIN_RESET)
 */
void ControlFan(GPIO_PinState state)
{
    HAL_GPIO_WritePin(outputGpio.fan.Port, outputGpio.fan.gpioPin, state);
}

/**
 * @brief Điều khiển ngõ ra Pump.
 * @param state: Trạng thái cần đặt cho Pump (GPIO_PIN_SET hoặc GPIO_PIN_RESET)
 */
void ControlPump(GPIO_PinState state)
{
    HAL_GPIO_WritePin(outputGpio.pump.Port, outputGpio.pump.gpioPin, state);
}

/**
 * @brief Điều khiển ngõ ra Reset Driver.
 * @param state: Trạng thái cần đặt cho Reset Driver (GPIO_PIN_SET hoặc GPIO_PIN_RESET)
 */
void ControlResetDriver(GPIO_PinState state)
{
    HAL_GPIO_WritePin(outputGpio.resetDriver.Port, outputGpio.resetDriver.gpioPin, state);
}

/**
 * @brief Điều khiển ngõ ra Valve L1.
 * @param state: Trạng thái cần đặt cho Valve L1 (GPIO_PIN_SET hoặc GPIO_PIN_RESET)
 */
void ControlValveL1(GPIO_PinState state)
{
    HAL_GPIO_WritePin(outputGpio.valveL1.Port, outputGpio.valveL1.gpioPin, state);
}

/**
 * @brief Điều khiển ngõ ra Valve L2.
 * @param state: Trạng thái cần đặt cho Valve L2 (GPIO_PIN_SET hoặc GPIO_PIN_RESET)
 */
void ControlValveL2(GPIO_PinState state)
{
    HAL_GPIO_WritePin(outputGpio.valveL2.Port, outputGpio.valveL2.gpioPin, state);
}

/**
 * @brief Điều khiển ngõ ra Valve L3.
 * @param state: Trạng thái cần đặt cho Valve L3 (GPIO_PIN_SET hoặc GPIO_PIN_RESET)
 */
void ControlValveL3(GPIO_PinState state)
{
    HAL_GPIO_WritePin(outputGpio.valveL3.Port, outputGpio.valveL3.gpioPin, state);
}
