/*
 * Oled_PalletShuttle.h
 *
 *  Created on: Nov 30, 2024
 *      Author: tranngoctoan
 */

#ifndef LIB_OLED_OLED_PALLETSHUTTLE_H_
#define LIB_OLED_OLED_PALLETSHUTTLE_H_

#include "Oled_Config.h"

typedef struct {
    uint8_t *ip;            // Địa chỉ IP
    uint8_t *errcode;       // Mã lỗi
    uint8_t *code_number;   // Số mã code
    uint8_t battery;           // Mức pin
    uint8_t charging_state;    // Trạng thái sạc
    float temp;                // Nhiệt độ
    float volt;                // Điện áp
    float current;             // Dòng điện
    uint8_t signal_connect;    // Trạng thái kết nối
    uint8_t mode;              // Chế độ hoạt động
    uint8_t timedelay;         // Thời gian trễ
} Oled;


void DrawBatteryPage(uint8_t battery, const char *batteryStr, const char *voltStr, const char *currentStr, const char *tempStr, uint8_t signal_connect, uint8_t mode);
void DrawNumberPage(uint8_t battery, uint8_t *code_number, uint8_t *ip, uint8_t signal_connect, uint8_t mode);
void DrawBatteryStatus(uint8_t battery, const char *batteryStr, uint8_t charging_state);
void DrawHeaderStatus(uint8_t battery, uint8_t signal_connect, uint8_t mode);
void Start_oled(void);
void Waiting_oled(void);
void Home_oled(uint8_t *ip, uint8_t *code_number, uint8_t battery, float temp, float volt, float current, uint8_t timedelay, uint8_t signal_connect, uint8_t mode);
void Charging_oled(uint8_t battery, float temp, float volt, float current, uint8_t charging_state, uint8_t signal_connect, uint8_t mode);
void CaseError_oled(uint8_t battery,uint8_t *errcode, uint8_t *code_number, uint8_t *ip, uint8_t signal_connect, uint8_t mode);
void DrawBattery_ver(uint8_t Xstart, uint8_t Ystart, uint8_t Width, uint8_t Height,uint8_t battery, uint8_t charging);
void DrawBattery_hoz(uint8_t Xstart, uint8_t Ystart, uint8_t Width, uint8_t Height,uint8_t battery, uint8_t charging);
void EyeAnimation(uint16_t centerX, uint16_t centerY, uint16_t eyeRadius, uint16_t pupilRadius, uint16_t eyeColor, uint16_t pupilColor, int steps);

#endif /* LIB_OLED_OLED_PALLETSHUTTLE_H_ */
