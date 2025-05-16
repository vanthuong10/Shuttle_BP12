/*
 * mb_display.h
 *
 *  Created on: May 16, 2025
 *      Author: ADMIN-HPZ2
 */

#ifndef OLED_LED_MB_DISPLAY_H_
#define OLED_LED_MB_DISPLAY_H_
#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <stdint.h>
#include "database.h"

#define OLED_ID 69
#define LED_START_ADD 20
#define LED_NUM_OF_REG 5
#define OLED_START_ADD 30
#define OLED_NUM_OF_REG 16
#define TIME_NEXT_SCREEN 4
#define OLED_PAGE_ADD 46

#define OLED_U32_DECODE(value, a, b, c, d) do { \
  (a) = (uint8_t) ((value) >> 24) & 255;     \
  (b) = (uint8_t) ((value) >> 16) & 255;     \
  (c) = (uint8_t) ((value) >> 8) & 255;      \
  (d) = (uint8_t) (value) & 255;             \
} while (0)


void mb_ledInit(UART_HandleTypeDef *uart);
void mb_setLed(uint16_t R, uint16_t G, uint16_t B, uint16_t timeout, uint16_t brightness);
void mb_setOled(struct DataInfo *info, struct DataRun *data_run);
void mb_setPage(uint16_t page);

#ifdef __cplusplus
}
#endif

#endif /* OLED_LED_MB_DISPLAY_H_ */
