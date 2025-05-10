/*
 * lib_RGB.h
 *
 *  Created on: Sep 25, 2024
 *      Author: PHAN THANH DANH
 */

#ifndef INC_LIB_RGB_H_
#define INC_LIB_RGB_H_
#include "main.h"

struct RGB_Config {
	TIM_HandleTypeDef *tim ;
	uint32_t Channel ;
};

typedef enum {
	LED_STATE_EMG, LED_STATE_ERORR_WIFI, LED_STATE_BLOCK, LED_STATE_LIFT, LED_STATE_SLOW, LED_STATE_RUN, LED_STATE_STOP
} LEDSTATUS;

void LedRGBInit(TIM_HandleTypeDef *htim, uint32_t Channel) ;
void Reset_LED(void); //tắt tất cả các LED
void WS2812_Send(void); //Gửi giá trị LED
void Led_status(LEDSTATUS type); //Lựa chọn trạng thái màu LED
void Setled(uint8_t R, uint8_t B, uint8_t G, uint32_t time, uint32_t Brightness); //Hiệu ứng nháy
void Set_Brightness(int brightness); //Chỉnh độ sáng của LED, gia tri 0>>255
void CHARGE(uint8_t percent); //Lựa chọn trạng thái hiển thị phần trăm PIN khi sạc
void Setled_charge(uint32_t i); //Hiệu ứng nháy khi sạc
void Setled_charge1(uint32_t i); //Hiệu ứng nháy khi sạc


#endif /* INC_LIB_RGB_H_ */

