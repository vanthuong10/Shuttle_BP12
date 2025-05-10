/*
 * user_custom.c
 *
 *  Created on: Feb 20, 2025
 *      Author: ADMIN-HPZ2
 */
#include "user_custom.h"

uint16_t crc16_modbus(const uint8_t *data, size_t length) {
    uint16_t crc = 0xFFFF; // Giá trị khởi tạo

    for (size_t i = 0; i < length; i++) {
        crc ^= data[i]; // XOR byte dữ liệu với CRC

        for (uint8_t bit = 0; bit < 8; bit++) {
            if (crc & 0x0001) { // Kiểm tra bit thấp nhất
                crc = (crc >> 1) ^ 0xA001; // XOR với đa thức
            } else {
                crc >>= 1; // Dịch phải
            }
        }
    }
    return crc; // Trả về giá trị CRC 16-bit
}

// t: expiration time, prd: period, now: current time. Return true if expired
bool u_timer_expired(uint64_t *t, uint64_t prd, uint64_t now) {
  if (now + prd < *t) *t = 0;                    // Time wrapped? Reset timer
  if (*t == 0) *t = now + prd;                   // Firt poll? Set expiration
  if (*t > now) return false;                    // Not expired yet, return
  *t = (now - *t) > prd ? now + prd : *t + prd;  // Next expiration time
  return true;                                   // Expired, return true
}

bool uReadBit(uint16_t value, uint8_t position) {
    return (value & (1 << position)) != 0;
}
uint16_t uSetBit(uint16_t value, uint8_t position) {
    return value | (1 << position);
}

uint16_t uClearBit(uint16_t value, uint8_t position) {
    return value & ~(1 << position);
}

/**
 * @brief Hàm phát hiện xung cạnh lên của 1 cờ.
 *
 * Hàm nhận vào trạng thái hiện tại của cờ
 * và trả về true nếu phát hiện sự chuyển từ trạng thái xung cạnh lên.
 *
 * @param currentState Trạng thái hiện tại của cờ.
 * @return true nếu phát hiện xung cạnh lên, false nếu không.
 */
bool detectFlagRisingEdge(bool currentState)
{
    static bool lastState = false;
    bool risingEdge = (currentState && !lastState);
    lastState = currentState;
    return risingEdge;
}
/**
 * @brief Hàm phát hiện xung cạnh xuống của 1 cờ.
 *
 * Hàm nhận vào trạng thái hiện tại của cờ
 * và trả về true nếu phát hiện sự chuyển từ trạng thái xung cạnh xuống.
 *
 * @param currentState Trạng thái hiện tại của cờ.
 * @return true nếu phát hiện xung cạnh xuống, false nếu không.
 */
bool detectFlagFallingEdge(bool currentState)
{
    static bool lastState = false;
    bool fallingEdge = (!currentState && lastState);
    lastState = currentState;
    return fallingEdge;
}
// p : disappear memory vl: current value
bool toggleValueState(double *p ,double vl)
{
	if(vl != *p)
	{
		*p = vl ;
		return true ;
	}
	return false ;
}
