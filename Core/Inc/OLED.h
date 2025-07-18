#ifndef __OLED_H
#define __OLED_H

#include "i2c.h"

/* OLED初始化 */
void OLED_Init(void);

/* OLED控制用函数 */
void OLED_Set_Pos(uint8_t x, uint8_t y);
void OLED_Display_On(void);
void OLED_Display_Off(void);
void OLED_Clear(void);
void OLED_On(void);

/* OLED功能函数 */
void OLED_ShowChar(uint8_t x,uint8_t y,uint8_t chr,uint8_t size);
void OLED_ShowNum(uint8_t x,uint8_t y,uint32_t num,uint8_t len,uint8_t size);
void OLED_ShowString(uint8_t x,uint8_t y, char *chr,uint8_t size);
void OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t color);
void OLED_DrawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t color);
void OLED_ShowFloat(uint8_t x, uint8_t y, float num, uint8_t decimals, uint8_t size);
void OLED_ShowCHinese(uint8_t x, uint8_t y, uint8_t no);

/*OLED刷新函数*/
void OLED_Update_Screen(void);


void OLED_Update_wave(void);
void OLED_Clear_wave(void);


void OLED_Update_num(void);
void OLED_Clear_num_1(void);
void OLED_Clear_num_2(void);
void OLED_Clear_num_3(void);
void OLED_Clear_num_4(void);



void OLED_Load();
void OLED_grid();


#endif



