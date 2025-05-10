/*
 * Oled_Config.c
 *
 *  Created on: Oct 16, 2024
 *      Author: tranngoctoan
 */

#include "Oled_Config.h"
#include "main.h"

uint8_t case_oled=0;
static DEF_OLED defineoled;
#define SSD1309_I2C_ADDR  0x3C
#define SSD1309_WIDTH     128
#define SSD1309_HEIGHT    64
#define SSD1309_DATA      0x40
#define SSD1309_CMD       0x00
static volatile bool i2c_dma_busy = false;
__attribute__((weak)) void OLED_Error_Callback(bool flag) {}
/*****************Define I2C*****************/
void init_i2c(I2C_HandleTypeDef *hi2c) {
	defineoled.hi2c = hi2c;
}

/********************************************************************************
function:	Delay function
note:
	Driver_Delay_ms(xms) : Delay x ms
	Driver_Delay_us(xus) : Delay x us
********************************************************************************/
uint8_t Driver_Delay_ms(uint32_t xms)
{
    static uint32_t startTick = 0;

    if (startTick == 0) {
        startTick = HAL_GetTick();
    } else if ((HAL_GetTick() - startTick) >= xms) {
        startTick = 0;
        return 1;
    }

    return 0;
}


//void Driver_Delay_us(uint32_t xus)
//{
//    // Kích hoạt DWT (Data Watchpoint and Trace) nếu chưa được kích hoạt
//    if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk)) {
//        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
//        DWT->CYCCNT = 0; // Reset counter
//        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk; // Kích hoạt counter
//    }
//
//    uint32_t startTick = DWT->CYCCNT;
//    uint32_t delayTicks = xus * (SystemCoreClock / 1000000); // Tính toán số chu kỳ
//
//    while ((DWT->CYCCNT - startTick) < delayTicks) {
//        // Chờ cho đến khi đủ số micro giây yêu cầu
//    }
//}
/********************************************************************************
function:	System Init
note:
	Initialize the communication method
********************************************************************************/
uint8_t System_Init(void)
{
#if USE_SPI_4W
	printf("USE_SPI_4W\r\n");
#elif USE_IIC
	printf("USE_IIC\r\n");
//	OLED_CS_0;
//	OLED_DC_0;
//	DC = 0,1 >> Address = 0x3c,0x3d
#elif USE_IIC_SOFT
	printf("USEI_IIC_SOFT\r\n");
	OLED_CS_0;
	OLED_DC_1;
	iic_init();

#endif
  return 0;
}
/********************************************************************************
function:	Hardware interface
note:
	    SPI4W_Write_Byte(value) :
		HAL library hardware SPI
		Register hardware SPI
		Gpio analog SPI
	    I2C_Write_Byte(value, cmd):
		HAL library hardware I2C
********************************************************************************/

void I2C_Write_Byte(uint8_t value, uint8_t Cmd)
{
	if (defineoled.hi2c == NULL) return;
	    uint8_t data[2] = {Cmd,value};
	    i2c_dma_busy = true;
	    if (HAL_I2C_Master_Transmit_DMA(defineoled.hi2c, SSD1309_I2C_ADDR << 1, data, 2) != HAL_OK) {
	        i2c_dma_busy = false;
	        OLED_Error_Callback(true);
	    }
	    uint32_t timeout = HAL_GetTick();
	    while (i2c_dma_busy && (HAL_GetTick() - timeout < 100));

}
/*******************************************************************************
function:
            Write register address and data
*******************************************************************************/
inline static void OLED_2in42_WriteReg(uint8_t Reg)
{
	I2C_Write_Byte(Reg,IIC_CMD);
}
inline static void OLED_2in42_WriteData(uint8_t Data)
{
	I2C_Write_Byte(Data,IIC_RAM);
}
///***************** I2C Write Command *****************/
//static void OLED_2in42_WriteReg(uint8_t cmd) {
//    if (defineoled.hi2c == NULL) return;
//    uint8_t data[2] = {SSD1309_CMD, cmd};
//    i2c_dma_busy = true;
//    if (HAL_I2C_Master_Transmit_DMA(defineoled.hi2c, SSD1309_I2C_ADDR << 1, data, 2) != HAL_OK) {
//        i2c_dma_busy = false;
//        OLED_Error_Callback(true);
//    }
//    uint32_t timeout = HAL_GetTick();
//    while (i2c_dma_busy && (HAL_GetTick() - timeout < 100));
//}
//
///***************** I2C Write Data *****************/
//static void SSD1309_WriteData(const uint8_t* data, uint16_t len) {
//    if (defineoled.hi2c == NULL) return;
//    tx_data[0] = SSD1309_DATA;
//    memcpy(&tx_data[1], data, len);
//    i2c_dma_busy = true;
//    if (HAL_I2C_Master_Transmit_DMA(defineoled.hi2c, SSD1309_I2C_ADDR << 1, tx_data, len + 1) != HAL_OK) {
//        i2c_dma_busy = false;
//        OLED_Error_Callback(true);
//    }
//    uint32_t timeout = HAL_GetTick();
//    while (i2c_dma_busy && (HAL_GetTick() - timeout < 100));
//}

/***************** I2C DMA Callback *****************/
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c) {
    if (defineoled.hi2c != NULL && hi2c->Instance == defineoled.hi2c->Instance) {
        i2c_dma_busy = false;
    }
}
/*******************************************************************************
function:
		Common register initialization
*******************************************************************************/
static void OLED_2in42_InitReg(void)
{
    OLED_2in42_WriteReg(0xAE);//--turn off oled panel
    OLED_2in42_WriteReg(0x00);//---set low column address
    OLED_2in42_WriteReg(0x10);//---set high column address
	OLED_2in42_WriteReg(0x20);
    OLED_2in42_WriteReg(0x00);
    OLED_2in42_WriteReg(0xFF);
    OLED_2in42_WriteReg(0xA6);
    OLED_2in42_WriteReg(0xA8);
    OLED_2in42_WriteReg(0x3F);
    OLED_2in42_WriteReg(0xD3);
    OLED_2in42_WriteReg(0x00);
    OLED_2in42_WriteReg(0xD5);
    OLED_2in42_WriteReg(0x80);
    OLED_2in42_WriteReg(0xD9);
    OLED_2in42_WriteReg(0x22);
    OLED_2in42_WriteReg(0xDA);
    OLED_2in42_WriteReg(0x12);
    OLED_2in42_WriteReg(0xDB);
    OLED_2in42_WriteReg(0x40);


}
/********************************************************************************
function:
            initialization
********************************************************************************/
void OLED_2in42_Init(I2C_HandleTypeDef *hi2c)
{
	defineoled.hi2c = hi2c;
    //Set the initialization register
    OLED_2in42_InitReg();
    //Turn on the OLED display
    OLED_2in42_WriteReg(0xaf);
    // 0.Create a new image cache
    Init_BlackImage();
}

/********************************************************************************
function:
			Clear screen
********************************************************************************/
void OLED_2in42_Clear() {
	UWORD i, j;
	for (i = 0; i < 8; i++) {
		/* set page address */
		OLED_2in42_WriteReg(0xB0 + i);
		/* set low column address */
		OLED_2in42_WriteReg(0x00);
		/* set high column address */
		OLED_2in42_WriteReg(0x10);
		for (j = 0; j < 128; j++) {
			/* write data */
			OLED_2in42_WriteData(0x00);
		}
	}
}
/********************************************************************************
function:	Update memory to OLED
********************************************************************************/
void OLED_2in42_Display(const UBYTE *Image) {
	UWORD page, column, temp;

	for (page = 0; page < 8; page++) {
		/* set page address */
		OLED_2in42_WriteReg(0xB0 + page);
		/* set low column address */
		OLED_2in42_WriteReg(0x00);
		/* set high column address */
		OLED_2in42_WriteReg(0x10);

		/* write data */
		for (column = 0; column < 128; column++) {
			temp = Image[(7 - page) + column * 8];
			OLED_2in42_WriteData(temp);
		}
	}
}
UBYTE *BlackImage = NULL; // Khởi tạo biến toàn cục

void Init_BlackImage() {
    UWORD Imagesize = ((OLED_2IN42_WIDTH % 8 == 0) ? (OLED_2IN42_WIDTH / 8) : (OLED_2IN42_WIDTH / 8 + 1)) * OLED_2IN42_HEIGHT;
    BlackImage = (UBYTE *)malloc(Imagesize);
    if (BlackImage == NULL) {
        return;
    }
    Paint_NewImage(BlackImage, OLED_2IN42_WIDTH, OLED_2IN42_HEIGHT, 270, BLACK);
}
UBYTE *WhiteImage = NULL; // Khởi tạo biến toàn cục cho hình ảnh trắng

void Init_WhiteImage() {
    UWORD Imagesize = ((OLED_2IN42_WIDTH % 8 == 0) ? (OLED_2IN42_WIDTH / 8) : (OLED_2IN42_WIDTH / 8 + 1)) * OLED_2IN42_HEIGHT;
    WhiteImage = (UBYTE *)malloc(Imagesize);
    if (WhiteImage == NULL) {
        return;
    }
    // Khởi tạo nền trắng cho hình ảnh
    Paint_NewImage(WhiteImage, OLED_2IN42_WIDTH, OLED_2IN42_HEIGHT, 270, WHITE);
}

