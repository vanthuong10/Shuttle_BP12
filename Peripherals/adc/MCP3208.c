/*
 * MCP3208.c
 *
 *  Created on: Jul 25, 2024
 *      Author: ADMIN-HPZ2
 */
#include "MCP3208.h"

mcp3208_pin spiPin;

void _begin (SPI_HandleTypeDef* hspi, GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	spiPin._hspi = hspi;
	spiPin.GPIO_p = GPIOx;
	spiPin.GPIO_pin = GPIO_Pin;
}

uint16_t _readChannel(uint8_t pin) {
	uint8_t txData[3] = { 0, 0, 0 }; // Khởi tạo mảng txData để truyền dữ liệu
	uint8_t rxData[3] = { 0, 0, 0 }; // Khởi tạo mảng rxData để nhận dữ liệu
	txData[0] = ((pin >> 2) | 0b00000110);
	txData[1] = (pin << 6);
	HAL_GPIO_WritePin(spiPin.GPIO_p, spiPin.GPIO_pin, GPIO_PIN_RESET); // Thiết lập tín hiệu CS để bắt đầu truyền nhận
	HAL_SPI_TransmitReceive(spiPin._hspi, txData, rxData, 3, 100); // Truyền nhận dữ liệu
	HAL_GPIO_WritePin(spiPin.GPIO_p, spiPin.GPIO_pin, GPIO_PIN_SET); // Thiết lập tín hiệu CS để kết thúc truyền nhận
	uint16_t result = ((rxData[1] & 0x0F) << 8) | rxData[2];
	return result;
}

MCP3208 mcp3208 = { .begin = _begin , .readChannel =  _readChannel};
