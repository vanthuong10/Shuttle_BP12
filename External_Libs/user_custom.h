/*
 * user_custom.h
 *
 *  Created on: Feb 20, 2025
 *      Author: ADMIN-HPZ2
 */

#ifndef USER_CUSTOM_H_
#define USER_CUSTOM_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "stdlib.h"
#include "stdint.h"
#include "stdbool.h"

uint16_t crc16_modbus(const uint8_t *data, size_t length);
bool u_timer_expired(uint64_t *t, uint64_t prd, uint64_t now);
bool uReadBit(uint16_t value, uint8_t position);
uint16_t uSetBit(uint16_t value, uint8_t position);
uint16_t uClearBit(uint16_t value, uint8_t position);
bool detectFlagRisingEdge(bool currentState, bool *st);
bool detectFlagFallingEdge(bool currentState, bool *st);
bool toggleValueState(double *p ,double vl);
#ifdef __cplusplus
}
#endif
#endif /* USER_CUSTOM_H_ */
