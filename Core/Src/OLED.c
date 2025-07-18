
#include "OLED.h"
#include "oledfont.h"

// 定义OLED显存缓冲区（128列 × 8页 × 8行 = 64像素高度）
static uint8_t oled_buffer[128 * 8] = {0};

/**
 * @brief OLED写入命令
 */
static void OLED_Write_Cmd(uint8_t cmd) {
    uint8_t buf[2];
    buf[0] = 0x00; // control byte
    buf[1] = cmd;

    HAL_I2C_Master_Transmit(&hi2c1, 0x78, buf, 2, 0xFFFF);
}

/**
 * @brief OLED写入数据
 */
static void OLED_Write_Dat(uint8_t dat) {
    uint8_t buf[2];
    buf[0] = 0x40; // control byte
    buf[1] = dat;

    HAL_I2C_Master_Transmit(&hi2c1, 0x78, buf, 2, 0xFFFF);
}

/**
 * @brief OLED设置显示位置
 */
void OLED_Set_Pos(uint8_t x, uint8_t y) {
    OLED_Write_Cmd(0xB0 + y);          // 设置页地址
    OLED_Write_Cmd(((x & 0xF0) >> 4) | 0x10); // 设置高列地址
    OLED_Write_Cmd((x & 0x0F) | 0x01);       // 设置低列地址
}

/**
 * @brief OLED清屏函数（清屏之后屏幕全为黑色）
 */
void OLED_Clear(void) {
    memset(oled_buffer, 0, sizeof(oled_buffer));
    OLED_Update_Screen(); // 立即刷新
}

/**
 * @brief OLED显示全开（所有像素点全亮）
 */
void OLED_On(void) {
    memset(oled_buffer, 0xFF, sizeof(oled_buffer));
    OLED_Update_Screen(); // 立即刷新
}

/**
 * @brief OLED专用pow函数
 * @param m - 底数
 * @param n - 指数
 * @return 计算结果 (m^n)
 */
static uint32_t oled_pow(uint8_t m, uint8_t n) {
    uint32_t result = 1; // 初始化结果为1（任何数的0次幂为1）
    while (n--) {        // 循环n次，每次将result乘以m
        result *= m;
    }
    return result;
}


void OLED_ShowChar(uint8_t x, uint8_t y, uint8_t chr, uint8_t size) {
    if (x >= 128 || y >= 8) return; // 边界检查

    uint8_t c = chr - ' '; // 计算字体数组索引
    uint8_t i;

    if (size == 16) {
        for (i = 0; i < 8; i++) {
            if (x + i < 128) {
                oled_buffer[y * 128 + x + i] |= F8X16[c * 16 + i]; // 上半部分
                if (y + 1 < 8 && x + i < 128) {
                    oled_buffer[(y + 1) * 128 + x + i] |= F8X16[c * 16 + i + 8]; // 下半部分
                }
            }
        }
    } else {
        for (i = 0; i < 6; i++) {
            if (x + i < 128) {
                oled_buffer[y * 128 + x + i] |= F6x8[c][i];
            }
        }
    }
}


void OLED_ShowString(uint8_t x, uint8_t y, char *chr, uint8_t size) {
    uint8_t j = 0;
    while (chr[j] != '\0') {
        OLED_ShowChar(x, y, chr[j], size);
        x += (size == 16) ? 8 : 6; // 计算下一个字符位置
        if (x > 120) {             // 边界换行
            x = 0;
            y += (size == 16) ? 2 : 1; // 根据字体高度换页
            if (y >= 8) break;         // 超出屏幕则终止
        }
        j++;
    }
}


void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size) {
    if (x >= 128 || y >= 8) return; // 边界检查

    uint8_t t, temp;
    uint8_t enshow = 0; // 前导零标志

    for (t = 0; t < len; t++) {
        temp = (num / oled_pow(10, len - t - 1)) % 10;

        // 前导零处理
        if (enshow == 0 && t < (len - 1)) {
            if (temp == 0) {
                OLED_ShowChar(x + (size / 2) * t, y, ' ', size); // 显示空格
                continue;
            } else {
                enshow = 1; // 开始显示数字
            }
        }

        OLED_ShowChar(x + (size / 2) * t, y, temp + '0', size); // 显示数字
    }
}


void OLED_ShowFloat(uint8_t x, uint8_t y, float num, uint8_t decimals, uint8_t size) {
    if (x >= 128 || y >= 8) return;

    char str_buf[16] = {0};  // 初始化缓冲区
    int32_t integer_part;
    int32_t decimal_part;
    uint8_t is_negative = 0;
    int32_t factor = 1;

    // 参数有效性检查
    decimals = decimals > 5 ? 5 : decimals;
    for(uint8_t i=0; i<decimals; i++) factor *= 10;

    // 四舍五入处理（整个数值）
    float rounded = num;
    if(decimals > 0) {
        rounded += (num < 0 ? -0.5f : 0.5f) / factor;
    } else {
        rounded += (num < 0 ? -0.5f : 0.5f);
    }

    // 处理负数
    if(rounded < 0) {
        is_negative = 1;
        rounded = -rounded;
    }

    // 分离整数和小数部分
    integer_part = (int32_t)rounded;
    float decimal_float = rounded - integer_part;
    decimal_part = (int32_t)(decimal_float * factor + 0.5f);  // 再次四舍五入

    // 处理进位（例如：3.9995 → 4.0）
    if(decimal_part >= factor) {
        integer_part++;
        decimal_part -= factor;
    }

    // 构建字符串
    uint8_t idx = 0;

    // 负号
    if(is_negative) {
        str_buf[idx++] = '-';
    }

    // 整数部分
    if(integer_part == 0) {
        str_buf[idx++] = '0';
    } else {
        int32_t temp = integer_part;
        uint8_t num_digits = 0;
        char num_buf[10];

        // 分离数字（逆序存储）
        while(temp > 0) {
            num_buf[num_digits++] = (temp % 10) + '0';
            temp /= 10;
        }

        // 正序写入缓冲区
        for(int i=num_digits-1; i>=0; i--) {
            str_buf[idx++] = num_buf[i];
        }
    }

    // 小数部分
    if(decimals > 0) {
        str_buf[idx++] = '.';

        // 分离小数数字（正序存储）
        for(int i=decimals-1; i>=0; i--) {
            str_buf[idx + i] = (decimal_part % 10) + '0';
            decimal_part /= 10;
        }
        idx += decimals;
    }

    str_buf[idx] = '\0';

    // 调试输出（可选）
    // printf("Formatted: %s\n", str_buf);

    OLED_ShowString(x, y, str_buf, size);
}
void OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t color) {
    if (x >= 128 || y >= 64) return; // 边界检查，防止越界访问

    uint8_t page = y / 8; // 计算页号（每页 8 行）
    uint8_t bit = y % 8;  // 计算该点在页内的具体位

    oled_buffer[page * 128 + x] |= (1 << bit); // 点亮

}

void OLED_DrawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t color) {
    if (x0 >= 128 || y0 >= 64 || x1 >= 128 || y1 >= 64) return; // 边界检查

    if (x0 == x1) { // 垂直线
        int16_t sy = (y0 < y1) ? 1 : -1;
        for (int y = y0; y != y1; y += sy) OLED_DrawPoint(x0, y, color);
        OLED_DrawPoint(x1, y1, color);
        return;
    }

    if (y0 == y1) { // 水平线
        int16_t sx = (x0 < x1) ? 1 : -1;
        for (int x = x0; x != x1; x += sx) OLED_DrawPoint(x, y0, color);
        OLED_DrawPoint(x1, y1, color);
        return;
    }

    int16_t dx = abs(x1 - x0), dy = abs(y1 - y0);
    int16_t sx = (x0 < x1) ? 1 : -1, sy = (y0 < y1) ? 1 : -1;
    int16_t err = dx - dy;

    while (1) {
        OLED_DrawPoint(x0, y0, color);

        if (x0 == x1 && y0 == y1) break;

        int16_t e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx) { err += dx; y0 += sy; }
    }
}


void OLED_Update_Screen(void) {
    for (uint8_t page = 0; page < 8; page++) {
        OLED_Set_Pos(0, page); // 设置页地址
        for (uint8_t col = 0; col < 128; col++) {
            OLED_Write_Dat(oled_buffer[page * 128 + col]); // 逐列写入数据
        }
    }
}


//局部更新波形
void OLED_Update_wave(void) {
    for (uint8_t page = 0; page < 8; page++) {
        OLED_Set_Pos(0, page); // 设置页地址
        for (uint8_t col = 0; col < 92; col++) {
            OLED_Write_Dat(oled_buffer[page * 128 + col]); // 逐列写入数据
        }
    }
}



void OLED_Clear_wave(void) {
    for (uint8_t page = 0; page < 8; page++) {
        OLED_Set_Pos(0, page); // 设置页地址
        for (uint8_t col = 0; col < 92; col++) {
        	oled_buffer[page * 128 + col]=0;
        }
    }
}



void OLED_Update_num(void) {
    for (uint8_t page = 0; page < 8; page++) {
        OLED_Set_Pos(93, page);
        for (uint8_t col = 93; col < 128; col++) {
            OLED_Write_Dat(oled_buffer[page * 128 + col]);
        }
    }
}
void OLED_Clear_num_1(void) {
	    uint8_t page=1;
        OLED_Set_Pos(93, page);
        for (uint8_t col = 93; col < 128; col++)
        {
        	oled_buffer[page * 128 + col]=0;
        }
}
void OLED_Clear_num_2(void) {
	    uint8_t page=2;
        OLED_Set_Pos(93, page);
        for (uint8_t col = 93; col < 128; col++)
        {
        	oled_buffer[page * 128 + col]=0;
        }
}
void OLED_Clear_num_3(void) {
	    uint8_t page=6;
        OLED_Set_Pos(93, page);
        for (uint8_t col = 93; col < 128; col++)
        {
        	oled_buffer[page * 128 + col]=0;
        }
}

void OLED_Clear_num_4(void) {
	    uint8_t page=7;
        OLED_Set_Pos(93, page);
        for (uint8_t col = 93; col < 128; col++)
        {
        	oled_buffer[page * 128 + col]=0;
        }
}


void OLED_Init(void) {
    HAL_Delay(500);

    OLED_Write_Cmd(0xAE); // display off
    OLED_Write_Cmd(0x00); // set low column address
    OLED_Write_Cmd(0x10); // set high column address
    OLED_Write_Cmd(0x40); // set start line address
    OLED_Write_Cmd(0x81); // contract control
    OLED_Write_Cmd(0xFF); // contrast level
    OLED_Write_Cmd(0xA1); // set segment remap
    OLED_Write_Cmd(0xC8); // Com scan direction
    OLED_Write_Cmd(0xA6); // normal / reverse
    OLED_Write_Cmd(0xA8); // set multiplex ratio
    OLED_Write_Cmd(0x3F); // duty = 1/64
    OLED_Write_Cmd(0xD3); // set display offset
    OLED_Write_Cmd(0x00);
    OLED_Write_Cmd(0xD5); // set osc division
    OLED_Write_Cmd(0x80);
    OLED_Write_Cmd(0xD9); // Set Pre-Charge Period
    OLED_Write_Cmd(0xF1);
    OLED_Write_Cmd(0xDA); // set com pin configuartion
    OLED_Write_Cmd(0x12);
    OLED_Write_Cmd(0xDB); // set Vcomh
    OLED_Write_Cmd(0x40);
    OLED_Write_Cmd(0x20);
    OLED_Write_Cmd(0x02);
    OLED_Write_Cmd(0x8D); // set charge pump enable
    OLED_Write_Cmd(0x14);
    OLED_Write_Cmd(0xA4);
    OLED_Write_Cmd(0xA6);
    OLED_Write_Cmd(0xAF); // turn on oled panel

    OLED_Clear();
    OLED_Set_Pos(0, 0);
}

void OLED_ShowCHinese(uint8_t x, uint8_t y, uint8_t no)
{
    uint8_t t;
    // 汉字占两页（16x16像素）
    for (t = 0; t < 16; t++) {
        if (x + t < 128) {
            // 写入上半部分（页y）
            oled_buffer[y * 128 + x + t] = Hzk[2 * no][t];
            // 写入下半部分（页y+1）
            if (y + 1 < 8) {
                oled_buffer[(y + 1) * 128 + x + t] = Hzk[2 * no + 1][t];
            }
        }
    }
}

void OLED_Load()
{

  OLED_ShowCHinese(0, 0, 3);
  OLED_ShowCHinese(17, 0, 4);
  OLED_ShowCHinese(33, 0, 5);
  OLED_ShowCHinese(49, 0, 6);


  OLED_ShowCHinese(0, 2, 0);
  OLED_ShowCHinese(17, 2, 1);
  OLED_ShowCHinese(33, 2, 2);

  OLED_ShowString(0, 4, "23211165", 16);

  OLED_ShowCHinese(0, 6, 7);
  OLED_ShowCHinese(17, 6, 8);
  OLED_ShowCHinese(33, 6, 8);
  OLED_ShowCHinese(49, 6, 9);
  OLED_ShowString(65, 6, "2302", 16);



}

void OLED_grid()
{
	OLED_DrawLine(0, 26, 93, 26, 1);

	OLED_DrawLine(93, 0, 93, 3, 1);
	OLED_DrawLine(93, 11, 93, 14, 1);
	OLED_DrawLine(93, 24, 93, 27, 1);
	OLED_DrawLine(93, 37, 93, 40, 1);
	OLED_DrawLine(93, 49, 93, 52, 1);

	OLED_DrawLine(90, 0, 93, 0, 1);
	OLED_DrawLine(90, 52, 93, 52, 1);

	OLED_DrawLine(0, 11, 0, 14, 1);
	OLED_DrawLine(0, 24, 0, 27, 1);
	OLED_DrawLine(0, 37, 0, 40, 1);
	OLED_DrawLine(0, 49, 0, 52, 1);
	OLED_DrawLine(3, 0, 0, 0, 1);
	OLED_DrawLine(3, 52,0, 52, 1);



	OLED_DrawLine(34, 0, 34, 3, 1);
	OLED_DrawLine(34, 11, 34, 14, 1);
	OLED_DrawLine(34, 24, 34, 27, 1);
	OLED_DrawLine(34, 37, 34, 40, 1);
	OLED_DrawLine(34, 49, 34, 52, 1);


	OLED_DrawLine(65, 0, 65, 3, 1);
	OLED_DrawLine(65, 11, 65, 14, 1);
	OLED_DrawLine(65, 24, 65, 27, 1);
	OLED_DrawLine(65, 37, 65, 40, 1);
	OLED_DrawLine(65, 49, 65, 52, 1);
}
