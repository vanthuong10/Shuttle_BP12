/*
 * QrReader.c
 *
 *  Created on: Nov 11, 2024
 *      Author: THUONG NGUYEN
 */
#include "main.h"
#include "QrReader.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
qrhandle qr ;
uint8_t QrBuffer[QR_SIZE] = {0};
char qrTag[32];
void get_substring_between_newlines(const char *input, char *result);
void parse_string(const char *input, qrhandle *qr);
UART_HandleTypeDef *qruart;


#include "cmsis_os.h"
osThreadId_t qrTaskHandle;
const osThreadAttr_t qrTask_attributes = {
  .name = "qr Task",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityISR ,
};
osSemaphoreId_t semaphore_qr;

void threadQr(void *argument) {
    // Wait until the semaphore is released by another thread
	qr.Tag = "" ;
	for(;;)
	{
		if (osSemaphoreAcquire(semaphore_qr, osWaitForever) == osOK) {
			char* qrRead = pvPortMalloc(sizeof(char)*QR_SIZE);
			if(qrRead != NULL)
			{
				get_substring_between_newlines((char*)QrBuffer, qrRead);
				parse_string(qrRead, &qr);
				//printf("Received QRcode: %s\n", (char*) qrRead);
				vPortFree(qrRead);
			}
		}
	}
}

void qrInit()
{
	semaphore_qr = osSemaphoreNew(1, 0, NULL);
	qrTaskHandle = osThreadNew(threadQr, NULL, &qrTask_attributes);
}
/**
 * @brief   Hàm gọi khi ngắt truyền thông Can.
 * @param id : id thiết bị gửi
 * 	      --- Cấu hình QR ID= 10
 * 	      --- Chế độ hoạt dộng PDO
 * 	      --- Địa chỉ gửi bắt đầu là 0x018A - 0x018E
 * 	      --- Cấu hình Hearbeat 300 ms
 * @param mes: gói tin nhận được
 */

void qrEventData(uint32_t id, uint8_t *mes)
{
	if ((id >= 0x018A && id <= 0x018E)) {
		uint8_t index = id - 0x018A;
		memcpy(&QrBuffer[index * FRAME_DATA_SIZE], &mes[1], FRAME_DATA_SIZE);
		static uint8_t frameCount;
		if (++frameCount == TOTAL_FRAMES)
		{
		    frameCount = 0;
		    osSemaphoreRelease(semaphore_qr);
		}
	}else if(id == 0x70A)
	{
		qr.hearbeat = true ;
	}

}

void get_substring_between_newlines(const char *input, char *result) {
    result[0] = '\0';
    if (!input) { return; }
    const char *start_pos = strchr(input, 0x2);
    if (!start_pos) {  return; }
    start_pos++; // Bỏ qua ký tự 0x2
    const char *end_pos = strchr(start_pos, '\n');
    if (!end_pos) { return; }
    size_t len = end_pos - start_pos;
    if (len >= QR_SIZE) { len = QR_SIZE - 1; }
    strncpy(result, start_pos, len);
    result[len] = '\0';
}

void parse_string(const char *input, qrhandle *qr)
{
	if(input == NULL) return ;
	char* temp = pvPortMalloc(sizeof(char)*QR_SIZE);
	if(temp == NULL) return ;
    strcpy(temp, input);
    char *token;
    token = strtok(temp, ";");
    if (token != NULL)
    {
      strcpy(qrTag, token+2);
      qr->Tag = qrTag;
    }
    token = strtok(NULL, ";");
    if (token != NULL)
    {
      qr->distanceX = atoi(token);
    }
    token = strtok(NULL, ";");
    if (token != NULL)
    {
      qr->distanceY = atoi(token);
    }
    token = strtok(NULL, ";");
    if (token != NULL)
    {
      qr->angle = atof(token);
    }
    vPortFree(temp);

}

/**
 * @brief   Kiểm tra đúng mã QR.
 * @param Qrcode : mã QR hiện tại
 * 	      qrcmd  : mã QR từ server cần đọc
 * return: true nếu đúng mã/ flase nếu sai mã
 */
bool checkQrcode(char* Qrcode, char* qrcmd)
{
	return strcmp(qrcmd,Qrcode) == 0 ? true : false ;
}
