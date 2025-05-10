/*
 * Oled_Config.h
 *
 *  Created on: Oct 16, 2024
 *      Author: tranngoctoan
 */

#ifndef LIB_OLED_OLED_CONFIG_H_
#define LIB_OLED_OLED_CONFIG_H_

#include "main.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "GUI_Paint.h"
#include "ImageData.h"
#include "math.h"
#include "cmsis_os.h"
#if defined(__STM32F4xx_HAL_H)
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_uart.h"
#include "stm32f4xx_hal_i2c.h"
#elif defined(__STM32F1xx_HAL_H)
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_uart.h"
#include "stm32f1xx_hal_i2c.h"
#elif defined(__STM32G4xx_HAL_H)
#include "stm32G4xx_hal.h"
#include "stm32G4xx_hal_uart.h"
#include "stm32G4xx_hal_i2c.h"
#elif defined(__STM32H7xx_HAL_H)
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_uart.h"
#include "stm32h7xx_hal_i2c.h"
#elif defined(STM32H7xx_HAL_H)
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_gpio.h"
#include "stm32h7xx_hal_uart.h"
#include "stm32h7xx_hal_i2c.h"
#endif



#define USE_SPI_4W 		0
#define USE_IIC 		1
#define USE_IIC_SOFT	0

#define I2C_ADR	0X3C //Hardware setting 0X3C or 0X3D

#define IIC_CMD		0X00
#define IIC_RAM		0X40

#define OLED_2IN42_WIDTH  64//OLED width
#define OLED_2IN42_HEIGHT 128 //OLED height

extern uint8_t *BlackImage;
extern uint8_t case_oled;
extern uint8_t aa;
typedef struct _DEF_OLED {
	I2C_HandleTypeDef *hi2c;
} DEF_OLED;

extern I2C_HandleTypeDef hi2c2; // Khai báo handle I2C từ main.c
extern DMA_HandleTypeDef hdma_i2c2_tx; // Khai báo handle DMA từ main.c
/********************************************************************************/
void init_i2c(I2C_HandleTypeDef *hi2c);
//void init_i2c(I2C_HandleTypeDef hi2c);
uint8_t Driver_Delay_ms(uint32_t xms);
void Driver_Delay_us(uint32_t xus);
void OLED_2in42_Init(I2C_HandleTypeDef *hi2c);
void OLED_2in42_Clear(void);
void OLED_2in42_Display(const uint8_t *Image);
void OLED_Error_Callback(bool flag);
void SSD1309_UpdateScreen(void);
void SSD1309_Fill(uint8_t color);
void SSD1309_DrawPixel(uint8_t x, uint8_t y, uint8_t color);
void Init_BlackImage(void);
void Init_WhiteImage(void);
#endif /* LIB_OLED_OLED_CONFIG_H_ */
