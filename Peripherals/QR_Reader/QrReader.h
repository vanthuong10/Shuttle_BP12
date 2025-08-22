/*
 * QrReader.h
 *
 *  Created on: Nov 11, 2024
 *      Author: THUONG NGUYEN
 */

#ifndef QRREADER_H_
#define QRREADER_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "stdbool.h"
#include "CanBus.h"
#define TOTAL_FRAMES 5
#define FRAME_DATA_SIZE 7
#define QR_SIZE (TOTAL_FRAMES * FRAME_DATA_SIZE)


typedef struct
{
	int distanceX;
	int distanceY;
	float angle ;
	char* Tag;
	bool hearbeat;
}qrhandle;
extern qrhandle qr ;

void qrInit();
void qrEventData(uint32_t id, uint8_t *mes);
bool checkQrcode(char* Qrcode, const char* qrcmd);
#ifdef __cplusplus
}
#endif

#endif /* QRREADER_H_ */
