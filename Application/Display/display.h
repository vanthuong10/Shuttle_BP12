/*
 * display.h
 *
 *  Created on: Mar 17, 2025
 *      Author: ADMIN-HPZ2
 */

#ifndef DISPLAY_DISPLAY_H_
#define DISPLAY_DISPLAY_H_
#ifdef __cplusplus
extern "C" {
#endif
#include "lib_RGB.h"

void displayInit();
void shuttleSetLed(LEDSTATUS lv);
void shuttle_started();

#ifdef __cplusplus
}
#endif
#endif /* DISPLAY_DISPLAY_H_ */
