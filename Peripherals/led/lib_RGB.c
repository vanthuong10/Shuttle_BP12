/*
 * lib_RGB.c
 *
 *  Created on: Oct 23, 2024
 *      Author: PHAN THANH DANH
 */
#include "main.h"
#include "math.h"
#include "stm32h743xx.h"
#include "stm32h7xx_it.h"
#include "lib_RGB.h"

struct RGB_Config rgbConfig ;

#define MAX_LED 2
#define USE_BRIGHTNESS 1

uint8_t LED_Data[MAX_LED][4];
uint8_t LED_Mod[MAX_LED][4];
int datasentflag = 0;

uint16_t pwmData[(24 * MAX_LED) + 50];
void Reset_LED(void) {//Hàm cấu hình màu led
	for (int i = 0; i < MAX_LED; i++) {
		LED_Data[i][0] = i;
		LED_Data[i][1] = 0;
		LED_Data[i][2] = 0;
		LED_Data[i][3] = 0;
	}
}



void WS2812_Send(void) {//Hàm gửi dữ liệu đến SMT32
	uint32_t indx = 0;
	uint32_t status;
	for (int i = 0; i < MAX_LED; i++) {
#if USE_BRIGHTNESS
		status =
				((LED_Mod[i][1] << 16) | (LED_Mod[i][2] << 8) | (LED_Mod[i][3]));
#else
		status = ((LED_Data[i][1] << 16) | (LED_Data[i][2] << 8)
				| (LED_Data[i][3]));
#endif

		for (int i = 23; i >= 0; i--) {
			if (status & (1 << i)) {
				pwmData[indx] = (rgbConfig.tim->Instance->ARR * 2) / 3;  // 2/3 of 90
			}

			else
				pwmData[indx] = (rgbConfig.tim->Instance->ARR) / 3;  // 1/3 of 90

			indx++;
		}
	}
	for (int i = 0; i < 50; i++) {
		pwmData[indx] = 0;
		indx++;
	}

	HAL_TIM_PWM_Start_DMA(rgbConfig.tim, rgbConfig.Channel, (uint32_t*) pwmData, indx);
	while (!datasentflag) {
	};
	datasentflag = 0;
}



void Led_status(LEDSTATUS type) { //Hàm hiển thị trạng thái hoạt động của Shuttle. Cú pháp: Led_status(X); trong đó: X có thể là RUN, STOP, LIFT, BLOCK, EMG, ERROR_WIFI
	switch (type) {
	case LED_STATE_RUN:
		Setled(0, 0, 254, 500, 250);  //Green
		break;
	case LED_STATE_STOP:
		Setled(242, 5, 135, 500, 250);
		break;
	case LED_STATE_LIFT:
		Setled(0, 200, 200, 50, 250);  //Blue
		break;
	case LED_STATE_BLOCK:
		Setled(245, 2, 51, 100, 255);
		break;
	case LED_STATE_EMG:
		Setled(254, 0, 0, 50, 250);
		break;
	case LED_STATE_ERORR_WIFI:
		Setled(255, 0, 0, 100, 250);
		break;
	case LED_STATE_SLOW:
		Setled(255, 3, 247, 100, 250);
		break;
	default:
		break ;
	}
}



void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim) {
	HAL_TIM_PWM_Stop_DMA(rgbConfig.tim, rgbConfig.Channel);
	datasentflag = 1;
}




uint32_t previousTime = 0;
uint8_t ledState = 0;

void CHARGE(uint8_t percent) {//hàm hiển thị phần trăm pin. cú pháp: " CHARGE(X); trong đó: X là phần trăm PIN
	if (percent >= 0 && percent <= 25) {
		//nhay 1 cuc PIN
		static uint32_t previousTime = 0;   // Lưu thời gian trước đó
		static uint8_t ledIndex = 0;        // LED hiện tại đang xử lý
		static uint8_t state = 0;           // Trạng thái của hiệu ứng

		uint32_t currentTime = HAL_GetTick();   // Lấy thời gian hiện tại

		switch (state) {
		case 0:
			if (currentTime - previousTime >= 800) {
				Setled_charge(ledIndex);
				previousTime = currentTime;
				ledIndex++;
				if (ledIndex == 4) {
					state = 1;
				}
			}
			break;
		case 1:
			Reset_LED();
			if (currentTime - previousTime >= 800) {
				state = 0;
				ledIndex = -1;
			}

			break;
		}
	} else if (percent > 25 && percent <= 50) {
		//nhay 2 cuc PIN
		Setled_charge(0);

		static uint32_t previousTime = 0;   // Lưu thời gian trước đó
		static uint8_t ledIndex = 1;        // LED hiện tại đang xử lý
		static uint8_t state = 0;           // Trạng thái của hiệu ứng

		uint32_t currentTime = HAL_GetTick();   // Lấy thời gian hiện tại

		switch (state) {
		case 0:
			if (currentTime - previousTime >= 1000) {
				Setled_charge(ledIndex);
				previousTime = currentTime;
				ledIndex++;
				if (ledIndex == 4) {
					state = 1;
				}

			}
			break;
		case 1:

			if (currentTime - previousTime >= 1000) {
				Reset_LED();
				state = 0;
				ledIndex = 0;
			}

			break;
		}

	} else if (percent > 50 && percent <= 75) {
		//nhay 3 cuc PIN
		for (int i = 0; i < 2; i++) {
			Setled_charge(i);
		}

		static uint32_t previousTime = 0;   // Lưu thời gian trước đó
		static uint8_t ledIndex = 2;        // LED hiện tại đang xử lý
		static uint8_t state = 0;           // Trạng thái của hiệu ứng

		uint32_t currentTime = HAL_GetTick();   // Lấy thời gian hiện tại

		switch (state) {
		case 0:
			if (currentTime - previousTime >= 1000) {
				Setled_charge(ledIndex);
				previousTime = currentTime;
				ledIndex++;
				if (ledIndex == 4) {
					state = 1;
				}
			}
			break;
		case 1:

			if (currentTime - previousTime >= 1000) {
				Reset_LED();
				state = 0;
				ledIndex = 1;
			}

			break;
		}
	} else if (percent > 75 && percent < 100) {

		//nhay 4 cuc PIN
		for (int i = 0; i < 3; i++) {
			Setled_charge(i);
		}

		static uint32_t previousTime = 0;   // Lưu thời gian trước đó
		static uint8_t ledIndex = 1;        // LED hiện tại đang xử lý
		static uint8_t state = 0;           // Trạng thái của hiệu ứng

		uint32_t currentTime = HAL_GetTick();   // Lấy thời gian hiện tại

		switch (state) {
		case 0:
			if (currentTime - previousTime >= 1000) {
				Setled_charge(ledIndex);
				previousTime = currentTime;
				ledIndex++;
				if (ledIndex == 4) {
					state = 1;
				}
			}
			break;
		case 1:

			if (currentTime - previousTime >= 1000) {
				Reset_LED();
				state = 0;
				ledIndex = 2;
			}

			break;
		}
	} else {
		static uint32_t previousTime = 0;   // Lưu thời gian trước đó
		static uint8_t ledstates = 1;        // LED hiện tại đang xử lý
		uint32_t currentTime = HAL_GetTick();   // Lấy thời gian hiện tại
		if (currentTime - previousTime >= 500) {
			previousTime = currentTime;
			if (ledstates == 1) {
				for (int i = 0; i < MAX_LED; i++) {
					LED_Data[i][0] = i;
					LED_Data[i][1] = 200;
					LED_Data[i][2] = 0;
					LED_Data[i][3] = 200;

				}
				Set_Brightness(255);
				WS2812_Send();
				ledstates = 0;
			} else {
				for (int i = 0; i < MAX_LED; i++) {
					LED_Data[i][0] = i;
					LED_Data[i][1] = 200;
					LED_Data[i][2] = 200;
					LED_Data[i][3] = 0;
				}
				Set_Brightness(255);
				WS2812_Send();
				ledstates = 1;  // LED tắt
			}

		}

	}
}


void Setled_charge(uint32_t i) { //Hàm cấu hình màu LED hiển thị khi Shuttle đang sạc, để thay đổi màu LED thì thay đổi các giá trị 200/0/200
	LED_Data[i][0] = i;
	LED_Data[i][1] = 200;
	LED_Data[i][2] = 0;
	LED_Data[i][3] = 200;
	Set_Brightness(30);
	WS2812_Send();
}

void Setled(uint8_t R, uint8_t B, uint8_t G, uint32_t time, uint32_t Brightness) { //Hàm cấu hình màu LED khi Shuttle hoạt động. Cú pháp: "Setled(R, B, G, Time, Brightness);" trong đó: R,B,G là mã màu LED cần hiển thị, Time: thời gian LED nháy, Brightness: độ sáng của LED.
	if (time == 0) {
		for (int i = 0; i < MAX_LED; i++) {
			LED_Data[i][0] = i;
			LED_Data[i][1] = R;
			LED_Data[i][2] = G;
			LED_Data[i][3] = B;

		}
		Set_Brightness(Brightness);
		WS2812_Send();
	} else {
		int currentTime = HAL_GetTick(); // Lấy thời gian hiện tại
		if (currentTime - previousTime >= time) {
			previousTime = currentTime;  // Cập nhật thời gian trước đó

			if (ledState == 0) {
				for (int i = 0; i < MAX_LED; i++) {
					LED_Data[i][0] = i;
					LED_Data[i][1] = R;
					LED_Data[i][2] = G;
					LED_Data[i][3] = B;
				}
				Set_Brightness(Brightness);
				WS2812_Send();
				ledState = 1;  // LED bật sáng
			} else {
				for (int i = 0; i < MAX_LED; i++) {
					LED_Data[i][0] = i;
					LED_Data[i][1] = 0;
					LED_Data[i][2] = 0;
					LED_Data[i][3] = 0;
				}
				Set_Brightness(Brightness);
				WS2812_Send();
				ledState = 0;  // LED tắt
			}
		}
	}

}


void Set_Brightness(int brightness)  // 0-255 Hàm chỉnh độ sáng, cú pháp: Set_Brightness(X); trong đó: X chạy trong khoảng từ 0 đến 255.
{
#if USE_BRIGHTNESS
	if (brightness > 255)
		brightness = 255;
	for (int i = 0; i < MAX_LED; i++) {
		LED_Mod[i][0] = LED_Data[i][0];
		for (int j = 1; j < 4; j++) {
			LED_Mod[i][j] = (LED_Data[i][j] * brightness) / 255;
		}
	}
#endif
}


void LedRGBInit(TIM_HandleTypeDef *htim,   uint32_t Channel)
{
	rgbConfig.tim = htim ;
	rgbConfig.Channel = Channel ;
}




















