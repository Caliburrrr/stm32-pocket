/*
 * oscilloscope.c
 *
 *  Created on: Feb 9, 2025
 *      Author: asus
 */

#include "oscilloscope.h"
#include "math.h"
#include "OLED.h"

#include "arm_math.h"
#include "arm_const_structs.h"



#define  adcvaluelength 3000
#define  waveformlength 150

extern uint16_t adcvalue[adcvaluelength];
extern TIM_HandleTypeDef htim2;


uint16_t waveform[waveformlength];

float32_t fft_input[2048];
float32_t fft_output[1024];

//减去直流
void fftdata(void)
{
    for(int i=0; i<2048; i++)
    {
        fft_input[i] = (float32_t)adcvalue[i] - 2047.5;
    }
}

void getfrequency(float32_t* freq)
{
    arm_rfft_fast_instance_f32 fft_inst;
    float32_t max_value;
    uint32_t max_pos;

    arm_rfft_fast_init_f32(&fft_inst, 1024);

    arm_rfft_fast_f32(&fft_inst, fft_input, fft_output, 0);

    arm_cmplx_mag_f32(fft_output, fft_output, 512);

    arm_max_f32(fft_output+1, 511, &max_value, &max_pos);

    max_pos++;  // 跳过直流分量

    // 计算实际频率
    float32_t sample=htim2.Instance->PSC;
    *freq = (max_pos * 1000000.0) / 1024.0f/sample;
}

void get_max_min(uint16_t *max, uint16_t *min)
{
    *max = 0;
    *min = 5000;
    for (uint16_t i = 0; i < adcvaluelength; i++)
    {
        if (adcvalue[i] > *max)
            *max = adcvalue[i];
        if (adcvalue[i] < *min)
            *min = adcvalue[i];
    }
}

float get_amplitude()
{
    uint16_t maxvoltage, minvoltage;
    get_max_min(&maxvoltage, &minvoltage);
    float amp = (maxvoltage - minvoltage) / 4095.0 * 3.3;
    return amp;
}



//数据适配oled
void adcvalue_change_waveform(uint8_t xpoint,uint8_t y)
{
	uint8_t ypoint=4095/26;
	for(uint16_t i=0, j=0;i<adcvaluelength&&j<waveformlength;i=i+xpoint)
	{
	    float off = (adcvalue[i]-2047) / ypoint;
	    float yf=y/10.0;
	    float temp=26+off*yf;
		if (temp > 52) temp = 52;
		waveform[j++] = temp;
	}
}


//触发电压的位置
uint8_t get_triggervoltage_pos(uint8_t tgvalue)
{
	for(uint8_t i=0;i<waveformlength;i++)
	{
		if(tgvalue==0&&waveform[i]==tgvalue&&waveform[i+3]>0)
		{
			return i;
		}
		else if(tgvalue!=0&&waveform[i]==tgvalue&&waveform[i-3]<=tgvalue)
		{
			return i;
		}
	}
	return 0;
}


//画波形
void oledshow_waveform(uint8_t tgpos)
{
	for(uint8_t i=0;i<92;i++)
	{
		OLED_DrawLine(i-1, 52-waveform[i-1+tgpos], i, 52-waveform[i+tgpos], 1);
	}
}

