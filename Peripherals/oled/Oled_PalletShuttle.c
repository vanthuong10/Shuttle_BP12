/*
 * Oled_PalletShuttle.c
 *
 *  Created on: Nov 30, 2024
 *      Author: tranngoctoan
 */
#include "Oled_PalletShuttle.h"

/**
 * @brief Vẽ một lưới trên màn hình OLED.
 *
 * Hàm này tạo ra các đường kẻ ngang và dọc trên màn hình để tạo thành
 * một lưới ô vuông có kích thước được định nghĩa bởi `grid_size`.
 */
void drawGrid() {
    int grid_size = 13;  // Kích thước của mỗi ô là 12x12 pixel
    int screen_width = 128;
    int screen_height = 64;
    // Vẽ các đường dọc
    for (int x = 0; x <= screen_width; x += grid_size) {
        Paint_DrawLine(x, 0, x, screen_height, WHITE, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    }
    // Vẽ các đường ngang
    for (int y = 0; y <= screen_height; y += grid_size) {
        Paint_DrawLine(0, y, screen_width, y, WHITE, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    }
}
/**
 * @brief Vẽ trang hiển thị trạng thái pin.
 * @param battery Giá trị phần trăm pin.
 * @param batteryStr Chuỗi mô tả phần trăm pin.
 * @param voltStr Chuỗi hiển thị điện áp.
 * @param currentStr Chuỗi hiển thị dòng điện.
 * @param tempStr Chuỗi hiển thị nhiệt độ.
 * @param signal_connect Trạng thái kết nối (0: không kết nối, 1: kết nối).
 * @param mode Chế độ hiển thị (ví dụ: 0 - manual, 1 - auto).
 */
void DrawBatteryPage(uint8_t battery, const char *batteryStr, const char *voltStr, const char *currentStr, const char *tempStr, uint8_t signal_connect, uint8_t mode) {
    uint8_t loc_y_volt = 8;
    Paint_SelectImage(BlackImage);
    Paint_Clear(BLACK);
    DrawBattery_hoz(12, 15, 32, 15, battery, 0);
    Paint_DrawString_EN((battery == 100) ? 3 : (battery < 10) ? 15 : 11, 34, batteryStr, &Font27, WHITE, BLACK);
	Paint_DrawImage(gImage_volt, 62, 9, 16, 16, 0);
	Paint_DrawImage(gImage_current, 62, 28, 16, 16, 0);
	Paint_DrawImage(gImage_temp, 62, 47, 16, 16, 270);
	Paint_DrawString_EN(80, loc_y_volt, voltStr, &Font15, WHITE, BLACK);
	Paint_DrawString_EN(80, loc_y_volt + 19, currentStr, &Font15, WHITE, BLACK);
	Paint_DrawString_EN(80, loc_y_volt + 38, tempStr, &Font15, WHITE, BLACK);
    DrawHeaderStatus(battery, signal_connect,mode);
    Paint_DrawRectangle(60, 9, 60, 62, WHITE, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    OLED_2in42_Display(BlackImage);
}
/**
 * @brief Vẽ trang hiển thị thông tin số hiệu.
 * @param battery Giá trị phần trăm pin.
 * @param code_number Giá trị số hiệu.
 * @param codeStr Chuỗi mô tả số hiệu.
 * @param ip Địa chỉ IP dưới dạng mảng uint8_t.
 * @param signal_connect Trạng thái kết nối.
 * @param mode Chế độ hiển thị.
 */
void DrawNumberPage(uint8_t battery, uint8_t *code_number, uint8_t *ip, uint8_t signal_connect, uint8_t mode) {
    char ipFull[32];
    char codeStr[16];

    snprintf(ipFull, sizeof(ipFull), "IP:%s", ip);
    snprintf(codeStr, sizeof(codeStr), "%s", code_number);

    Paint_SelectImage(BlackImage);
    Paint_Clear(BLACK);

    UBYTE adjusted_x = (strlen((char *)code_number) < 2) ? 37 : 32;
    Paint_DrawString_EN(adjusted_x, 22, "No.", &Font12, WHITE, BLACK);
    Paint_DrawString_EN(adjusted_x + Font12.Width * 3, 10, codeStr, &Font27, WHITE, BLACK);
    Paint_DrawString_EN(13, 45, ipFull, &Font12, WHITE, BLACK);
    DrawHeaderStatus(battery, signal_connect, mode);
    OLED_2in42_Display(BlackImage);
}

/**
 * @brief Vẽ trạng thái pin (dọc hoặc ngang).
 * @param battery Giá trị phần trăm pin.
 * @param batteryStr Chuỗi mô tả phần trăm pin.
 * @param charging_state Trạng thái sạc (0: không sạc, 1: đang sạc).
 */
void DrawBatteryStatus(uint8_t battery, const char *batteryStr, uint8_t charging_state) {
    DrawBattery_hoz(12, 15, 32, 15, battery, charging_state);
    if (charging_state) {
        Paint_DrawImage(gImage_thunder, 49, 16, 12, 8, 90);
    }
    uint8_t posX = (battery == 100) ? 3 : (battery < 10) ? 15 : 11;
    Paint_DrawString_EN(posX, 34, batteryStr, &Font27, WHITE, BLACK);
}
/**
 * @brief Vẽ tiêu đề trạng thái.
 * @param battery Giá trị phần trăm pin.
 * @param signal_connect Trạng thái kết nối.
 * @param mode Chế độ hiển thị (ví dụ: 0 - manual, 1 - auto).
 */
void DrawHeaderStatus(uint8_t battery, uint8_t signal_connect, uint8_t mode) {
    // Vẽ biểu tượng kết nối
    Paint_DrawImage(signal_connect ? gImage_signal : gImage_nosignal, 1, 1, 16, 8, 0);

    // Vẽ hình ảnh mode
    const uint8_t *modeImage = (mode == 1) ? gImage_manual : gImage_auto;
    uint8_t pos_x_mode = 1 + 16 + 4; // Cách hình signal 16 điểm ảnh và thêm 4 điểm ảnh
    uint8_t pos_y_mode = 1;          // Cùng dòng với signal
    Paint_DrawImage(modeImage, pos_x_mode, pos_y_mode, 16, 8, 0);

    // Vẽ trạng thái pin
    uint8_t pos_x_battery = 112;
    uint8_t pos_y_battery = 1;
    Paint_DrawImage(gImage_battery, pos_x_battery, pos_y_battery, 16, 8, 0);

    uint8_t num_bars = battery / 25; // Xác định số ô cần vẽ dựa trên mức pin
    for (uint8_t i = 0; i < num_bars; i++) {
        Paint_DrawRectangle(
            pos_x_battery + 2 + (i * 3), pos_y_battery + 2,
            pos_x_battery + 3 + (i * 3), pos_y_battery + 6,
            WHITE, DOT_PIXEL_1X1, DRAW_FILL_FULL
        );
    }
}
/**
 * @brief Khởi tạo màn hình OLED.
 */
void Start_oled() {
	Paint_SelectImage(BlackImage);
	Paint_Clear(BLACK);
	Paint_DrawString_EN(36, 19, "THACO", &Font16, WHITE, BLACK);
	Paint_DrawString_EN(10, 37, "INDUSTRIES", &Font16, WHITE, BLACK);
	OLED_2in42_Display(BlackImage);
	if (Driver_Delay_ms(1000)) {
		case_oled = 1;
	}
}
/**
 * @brief Hiển thị màn hình chờ trên OLED.
 */
//void Waiting_oled() {
//	static int i = 0;
//	static char buffer[6] = "";
//
//	if (Driver_Delay_ms(50)) {
//		buffer[i++] = '>';
//		buffer[i] = '\0';
//		Paint_SelectImage(BlackImage);
//		Paint_Clear(BLACK);
//		Paint_DrawString_EN(27, 15, "WAITING", &Font16, WHITE, BLACK);
//		Paint_DrawString_EN(36, 33, "SETUP", &Font16, WHITE, BLACK);
//		Paint_DrawString_EN(34, 46, buffer, &Font16, WHITE, BLACK);
//		OLED_2in42_Display(BlackImage);
//		if (i == 5)
//			i = 0; // Reset i sau khi đủ 5 dấu '>'
//	}
//}
void Waiting_oled() {
    static int i = 0;
    static char buffer[6] = "";

    buffer[i++] = '>';
    buffer[i] = '\0';

    Paint_SelectImage(BlackImage);
    Paint_Clear(BLACK);
    Paint_DrawString_EN(27, 15, "WAITING", &Font16, WHITE, BLACK);
    Paint_DrawString_EN(36, 33, "SETUP", &Font16, WHITE, BLACK);
    Paint_DrawString_EN(34, 46, buffer, &Font16, WHITE, BLACK);
    OLED_2in42_Display(BlackImage);

    if (i == 5)
        i = 0; // Reset i sau khi đủ 5 dấu '>'

    osDelay(50); // Delay 50ms
}


void FloatToString(char* buffer, float value, const char* suffix) {
    int integerPart = (int)value;                      // Phần nguyên
    int decimalPart = (int)((value - integerPart) * 10); // Phần thập phân (1 chữ số)
    buffer[0] = integerPart / 10 + '0';                // Chữ số hàng chục
    buffer[1] = integerPart % 10 + '0';                // Chữ số hàng đơn vị
    buffer[2] = '.';                                   // Dấu thập phân
    buffer[3] = decimalPart + '0';                     // Phần thập phân
    strcpy(buffer + 4, suffix);                        // Thêm hậu tố
}
/**
 * @brief Hiển thị trang chính trên OLED.
 * @param ip Địa chỉ IP dưới dạng mảng uint8_t.
 * @param code_number Giá trị số hiệu.
 * @param battery Giá trị phần trăm pin.
 * @param temp Nhiệt độ.
 * @param volt Điện áp.
 * @param current Dòng điện.
 * @param timedelay Thời gian chờ giữa các lần cập nhật.
 * @param signal_connect Trạng thái kết nối.
 * @param mode Chế độ hiển thị (ví dụ: 0 - manual, 1 - auto).
 */
//void Home_oled(uint8_t *ip, uint8_t *code_number, uint8_t battery, float temp, float volt, float current, uint8_t timedelay, uint8_t signal_connect, uint8_t mode) {
//    static uint8_t state = 0;
//    char batteryStr[8], tempStr[16], voltStr[16], currentStr[16];
//    snprintf(batteryStr, sizeof(batteryStr), "%d%%", battery);
//    FloatToString(tempStr, temp, "`C");
//    FloatToString(voltStr, volt, "V");
//    FloatToString(currentStr, current, "A");
//
//    // Chuyển đổi giữa các trang sau khoảng thời gian
//    if (Driver_Delay_ms(timedelay * 1000)) {
//        if (state == 0) {
//            DrawBatteryPage(battery, batteryStr, voltStr, currentStr, tempStr, signal_connect, mode);
//            state = 1;
//        } else {
//            DrawNumberPage(battery, code_number, ip, signal_connect, mode);
//            state = 0;
//        }
//    }
//}

void Home_oled(uint8_t *ip, uint8_t *code_number, uint8_t battery, float temp, float volt, float current, uint8_t timedelay, uint8_t signal_connect, uint8_t mode) {
    static uint8_t state = 0;          // Trạng thái hiện tại (0: NumberPage, 1: BatteryPage)
    static uint32_t startTick = 0;    // Lưu thời điểm bắt đầu trang hiện tại
    char batteryStr[8], tempStr[16], voltStr[16], currentStr[16];

    // Chuyển đổi dữ liệu thành chuỗi
    snprintf(batteryStr, sizeof(batteryStr), "%d%%", battery);
    FloatToString(tempStr, temp, "`C");
    FloatToString(voltStr, volt, "V");
    FloatToString(currentStr, current, "A");

    // Lấy thời gian hiện tại
    uint32_t currentTick = HAL_GetTick();

    // Kiểm tra nếu đủ thời gian để chuyển trạng thái
    if ((currentTick - startTick) >= (timedelay * 1000)) {
        startTick = currentTick; // Reset lại thời gian
        state = (state == 0) ? 1 : 0; // Chuyển đổi trạng thái
    }

    // Gọi trang theo trạng thái hiện tại
    if (state == 0) {
        DrawNumberPage(battery, code_number, ip, signal_connect, mode);
    } else {
        DrawBatteryPage(battery, batteryStr, voltStr, currentStr, tempStr, signal_connect, mode);
    }
}


/**
 * @brief Hiển thị trang sạc trên OLED.
 * @param battery Giá trị phần trăm pin.
 * @param temp Nhiệt độ.
 * @param volt Điện áp.
 * @param current Dòng điện.
 * @param charging_state Trạng thái sạc.
 * @param signal_connect Trạng thái kết nối.
 * @param mode Chế độ hiển thị (ví dụ: 0 - manual, 1 - auto).
 */
void Charging_oled(uint8_t battery, float temp, float volt, float current, uint8_t charging_state,uint8_t signal_connect, uint8_t mode)
{
	char batteryStr[8],tempStr[16],voltStr[16],currentStr[16];
	uint8_t loc_y_volt=8;
	snprintf(batteryStr, sizeof(batteryStr), "%d%%", battery);
	FloatToString(tempStr, temp, "`C");
	FloatToString(voltStr, volt, "V");
	FloatToString(currentStr, current, "A");
	Paint_SelectImage(BlackImage);
	Paint_Clear(BLACK);
	DrawBatteryStatus(battery, batteryStr, charging_state);
	Paint_DrawImage(gImage_volt, 62, 9, 16, 16, 0);
	Paint_DrawImage(gImage_current, 62, 28, 16, 16, 0);
	Paint_DrawImage(gImage_temp, 62, 47, 16, 16, 270);
	Paint_DrawString_EN(80, loc_y_volt, voltStr, &Font15, WHITE, BLACK);
	Paint_DrawString_EN(80, loc_y_volt + 19, currentStr, &Font15, WHITE, BLACK);
	Paint_DrawString_EN(80, loc_y_volt + 38, tempStr, &Font15, WHITE, BLACK);
	Paint_DrawRectangle(60, 9, 60, 62, WHITE, DOT_PIXEL_1X1, DRAW_FILL_FULL);
	DrawHeaderStatus(battery, signal_connect,mode);
	OLED_2in42_Display(BlackImage);
}
/**
 * @brief Hiển thị trang lỗi trên OLED.
 * @param battery Giá trị phần trăm pin.
 * @param errcode Mảng mã lỗi.
 * @param code_number Số lượng mã lỗi.
 * @param ip Địa chỉ IP.
 * @param signal_connect Trạng thái kết nối.
 * @param mode Chế độ hiển thị (ví dụ: 0 - manual, 1 - auto).
 */
void CaseError_oled(uint8_t battery, uint8_t *errcode, uint8_t *code_number, uint8_t *ip, uint8_t signal_connect, uint8_t mode) {
    char codeStr[16];
    char ipFull[32];

    // Chuyển các giá trị thành chuỗi
    snprintf(codeStr, sizeof(codeStr), "No.%s", code_number); // Dùng chuỗi code_number thay vì số
    snprintf(ipFull, sizeof(ipFull), "IP:%s", ip); // Chuyển thẳng mảng ip

    Paint_SelectImage(BlackImage);
    Paint_Clear(BLACK);

    // Tính toán vị trí x để chuỗi `errcode` nằm ở giữa màn hình
    int errcodeWidth = strlen((char*)errcode) * Font27.Width; // Độ rộng của chuỗi mã lỗi với Font27
    int centerX = (128 - errcodeWidth) / 2; // Tọa độ x để căn giữa màn hình (128 là độ rộng màn hình)

    // Hiển thị mã lỗi ở giữa màn hình
    Paint_DrawString_EN(centerX, 18, (const char*) errcode, &Font27, WHITE, BLACK);

    // Hiển thị số thứ tự ở trên mã lỗi
    Paint_DrawString_EN(centerX, 6, codeStr, &Font12, WHITE, BLACK);

    // Hiển thị địa chỉ IP ở dưới cùng
    Paint_DrawString_EN(13, 45, ipFull, &Font12, WHITE, BLACK);

    // Tính toán vị trí y cho hình ảnh cảnh báo để cạnh đáy của nó ngang với cạnh đáy của mã lỗi
    int warningImageY = 18 + Font27.Height - 16 - 2; // 16 là chiều cao của hình ảnh
    Paint_DrawImage(gImage_warning, centerX - 20, warningImageY, 16, 16, 270); // Hình ảnh lệch trái để không che mã lỗi

    // Vẽ trạng thái chung
    DrawHeaderStatus(battery, signal_connect, mode);

    // Cập nhật màn hình
    OLED_2in42_Display(BlackImage);
}

/**
 * @brief Vẽ biểu tượng pin dạng đứng.
 * @param Xstart Tọa độ bắt đầu trên trục X.
 * @param Ystart Tọa độ bắt đầu trên trục Y.
 * @param Width Chiều rộng của biểu tượng.
 * @param Height Chiều cao của biểu tượng.
 * @param battery Giá trị phần trăm pin.
 * @param charging Trạng thái sạc.
 */
void DrawBattery_ver(uint8_t Xstart, uint8_t Ystart, uint8_t Width, uint8_t Height, uint8_t battery, uint8_t charging)
{
    // Vẽ đầu pin
    Paint_DrawRectangle(Xstart + (Width / 3), Ystart - 1, Xstart + (Width / 3) * 2, Ystart, WHITE, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    // Vẽ khung pin
    Paint_DrawRectangle(Xstart, Ystart, Xstart + Width, Ystart + Height, WHITE, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    // Vẽ các điểm ở 4 góc của khung pin
    Paint_DrawPoint(Xstart, Ystart, BLACK, DOT_PIXEL_1X1, DOT_FILL_AROUND);
    Paint_DrawPoint(Xstart, Ystart + Height, BLACK, DOT_PIXEL_1X1, DOT_FILL_AROUND);
    Paint_DrawPoint(Xstart + Width, Ystart, BLACK, DOT_PIXEL_1X1, DOT_FILL_AROUND);
    Paint_DrawPoint(Xstart + Width, Ystart + Height, BLACK, DOT_PIXEL_1X1, DOT_FILL_AROUND);
    // Chiều cao phần pin đầy dựa theo giá trị hiện tại của pin
    uint8_t filledHeight = (Height * (battery)) / 100;
    // Nếu pin đang sạc, nhấp nháy từ mức pin hiện tại lên 100% và quay lại mức pin hiện tại
    static uint8_t chargeLevel = 0;   // Mức sạc tạm thời
    static uint8_t increasing = 1;    // Biến kiểm tra hướng tăng hay giảm của mức sạc
    static uint8_t prevCharging = 0;  // Biến lưu trạng thái sạc trước đó
    // Nếu trạng thái sạc thay đổi từ 0 sang 1, khởi tạo chargeLevel bằng battery
    if (charging && !prevCharging) {
        chargeLevel = battery;
    }
    prevCharging = charging;  // Cập nhật trạng thái sạc hiện tại

    if (charging) {
        // Nếu pin đang sạc, tăng hoặc giảm mức sạc từ mức pin hiện tại
        if (increasing) {
            chargeLevel += 6;  // Tăng mức sạc
            if (chargeLevel >= 100) {
                increasing = 0;  // Khi đạt 100%, bắt đầu giảm
            }
        } else {
            chargeLevel = battery;  // Quay lại mức pin hiện tại
            increasing = 1;  // Khi trở về mức pin hiện tại, bắt đầu tăng lại
        }
        // Cập nhật chiều cao dựa trên mức sạc
        filledHeight = (Height * chargeLevel) / 100;
    }
    // Vẽ phần pin đầy dựa theo mức pin hoặc mức sạc và đảm bảo nó nằm trong khung
       if (battery > 0 || charging) {
           uint8_t topY = Ystart + Height - filledHeight;
           // Đảm bảo phần pin đầy không ra ngoài khung pin
           if (topY < Ystart + 1) {
               topY = Ystart + 1;
           }
           Paint_DrawRectangle(Xstart + 1, topY, Xstart + Width - 1, Ystart + Height, WHITE, DOT_PIXEL_1X1, DRAW_FILL_FULL);
       }
}
/**
 * @brief Vẽ biểu tượng pin dạng ngang.
 * @param Xstart Tọa độ bắt đầu trên trục X.
 * @param Ystart Tọa độ bắt đầu trên trục Y.
 * @param Width Chiều rộng của biểu tượng.
 * @param Height Chiều cao của biểu tượng.
 * @param battery Giá trị phần trăm pin.
 * @param charging Trạng thái sạc.
 */
void DrawBattery_hoz(uint8_t Xstart, uint8_t Ystart, uint8_t Width, uint8_t Height, uint8_t battery, uint8_t charging)
{
    // Vẽ đầu pin nằm ngang
    Paint_DrawRectangle(Xstart + Width, Ystart + (Height / 3), Xstart + Width + 1, Ystart + (Height / 3) * 2, WHITE, DOT_PIXEL_1X1, DRAW_FILL_FULL);

    // Vẽ khung pin nằm ngang
    Paint_DrawRectangle(Xstart, Ystart, Xstart + Width, Ystart + Height, WHITE, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);

    // Vẽ các điểm ở 4 góc của khung pin
    Paint_DrawPoint(Xstart, Ystart, BLACK, DOT_PIXEL_1X1, DOT_FILL_AROUND);
    Paint_DrawPoint(Xstart, Ystart + Height, BLACK, DOT_PIXEL_1X1, DOT_FILL_AROUND);
    Paint_DrawPoint(Xstart + Width, Ystart, BLACK, DOT_PIXEL_1X1, DOT_FILL_AROUND);
    Paint_DrawPoint(Xstart + Width, Ystart + Height, BLACK, DOT_PIXEL_1X1, DOT_FILL_AROUND);

    // Chiều rộng phần pin đầy dựa theo giá trị hiện tại của pin
    uint8_t filledWidth = (Width * (battery)) / 100;

    // Nếu pin đang sạc, nhấp nháy từ mức pin hiện tại lên 100% và quay lại mức pin hiện tại
    static uint8_t chargeLevel = 0;   // Mức sạc tạm thời
    static uint8_t increasing = 1;    // Biến kiểm tra hướng tăng hay giảm của mức sạc
    static uint8_t prevCharging = 0;  // Biến lưu trạng thái sạc trước đó

    // Nếu trạng thái sạc thay đổi từ 0 sang 1, khởi tạo chargeLevel bằng *battery
    if (charging && !prevCharging) {
        chargeLevel = battery;
    }
    prevCharging = charging;  // Cập nhật trạng thái sạc hiện tại

    if (charging) {
        // Nếu pin đang sạc, tăng hoặc giảm mức sạc từ mức pin hiện tại
        if (increasing) {
            chargeLevel += 6;  // Tăng mức sạc
            if (chargeLevel >= 100) {
                increasing = 0;  // Khi đạt 100%, bắt đầu giảm
            }
        } else {
            chargeLevel = battery;  // Quay lại mức pin hiện tại
            increasing = 1;  // Khi trở về mức pin hiện tại, bắt đầu tăng lại
        }
        // Cập nhật chiều rộng dựa trên mức sạc
        filledWidth = (Width * chargeLevel) / 100;
    }

    // Vẽ phần pin đầy dựa theo mức pin hoặc mức sạc và đảm bảo nó nằm trong khung
    if (battery > 0 || charging) {
        uint8_t rightX = Xstart + filledWidth;
        // Đảm bảo phần pin đầy không ra ngoài khung pin
        if (rightX > Xstart + Width - 1) {
            rightX = Xstart + Width - 1;
        }
        Paint_DrawRectangle(Xstart + 1, Ystart + 1, rightX, Ystart + Height, WHITE, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    }
}
void MovePupil(uint16_t centerX, uint16_t centerY, uint16_t eyeRadius, uint16_t pupilRadius, int offsetX, int offsetY, uint16_t pupilColor) {
    uint16_t pupilX = centerX + offsetX;
    uint16_t pupilY = centerY + offsetY;

    // Giới hạn con ngươi không di chuyển ra ngoài lòng trắng
    if (offsetX * offsetX + offsetY * offsetY <= (eyeRadius - pupilRadius) * (eyeRadius - pupilRadius)) {
        // Vẽ con ngươi mới
        Paint_DrawCircle(pupilX, pupilY, pupilRadius, pupilColor, DOT_PIXEL_DFT, DRAW_FILL_FULL);
    }
}
void DrawEye(uint16_t centerX, uint16_t centerY, uint16_t eyeRadius, uint16_t pupilRadius, uint16_t eyeColor, uint16_t pupilColor) {
    // Vẽ phần lòng trắng của mắt
    Paint_DrawCircle(centerX, centerY, eyeRadius, eyeColor, DOT_PIXEL_DFT, DRAW_FILL_FULL);

    // Vẽ con ngươi ở giữa
    Paint_DrawCircle(centerX, centerY, pupilRadius, pupilColor, DOT_PIXEL_DFT, DRAW_FILL_FULL);
}
void EyeAnimation(uint16_t centerX, uint16_t centerY, uint16_t eyeRadius, uint16_t pupilRadius, uint16_t eyeColor, uint16_t pupilColor, int steps) {
    int offsetX = -eyeRadius / 3; // Bắt đầu từ bên trái
    int offsetY = 0;  // Không di chuyển theo trục Y
    Paint_SelectImage(BlackImage);
    		Paint_Clear(BLACK);
    for (int i = 0; i < steps; i++) {
        // Xóa con ngươi trước đó
        DrawEye(centerX, centerY, eyeRadius, pupilRadius, eyeColor, WHITE);

        // Di chuyển con ngươi sang phải
        MovePupil(centerX, centerY, eyeRadius, pupilRadius, offsetX, offsetY, pupilColor);

        // Cập nhật offset để con ngươi di chuyển qua lại
        offsetX = (offsetX + 2) % (eyeRadius - pupilRadius);  // Điều chỉnh bước nhảy

//        osDelay(50); // Đợi một thời gian để thấy hiệu ứng chuyển động
        OLED_2in42_Display(BlackImage);
    }
}
