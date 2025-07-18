/*
 * generate.c
 *
 *  Created on: Feb 11, 2025
 *      Author: asus
 */

#include "main.h"

#define WAVE_LENGTH 256  // 波形数据长度
extern uint16_t sin_wavedata[WAVE_LENGTH];
extern uint16_t sq_wavedata[WAVE_LENGTH];
extern uint16_t tang_wavedata[WAVE_LENGTH];
extern uint16_t dc_wavedata[WAVE_LENGTH];


// 生成正弦波数据表
void Generate_Sine_Wave(float amplitude)
{
    for (int i=0;i<WAVE_LENGTH;i++)
    {
        float x = 2 * 3.14 * i / WAVE_LENGTH;
        sin_wavedata[i] = ((amplitude/2)*sin(x)+1.65)*255/3.3;
    }
}

// 生成方波数据表
void Generate_Square_Wave(float amplitude)
{
   uint16_t high=(3.3/2+amplitude/2)*255.0/3.3;
   uint16_t low=(3.3/2-amplitude/2)*255.0/3.3;
    for(int i=0;i<WAVE_LENGTH/2;i++)
    {
    	sq_wavedata[i]=high;
    }
    for(int i=WAVE_LENGTH/2;i<WAVE_LENGTH;i++)
    {
    	sq_wavedata[i]=low;
    }
}

// 生成三角波数据表
void Generate_Triangle_Wave(float amplitude)
{
    float mid=amplitude/2;
    for(int i=0;i<WAVE_LENGTH/2;i++)
    {
    	tang_wavedata[i]=((amplitude/127.0)*i+1.65-mid)*255.0/3.3;
    }
    for(int i=WAVE_LENGTH/2;i<WAVE_LENGTH;i++)
    {
    	tang_wavedata[i]=((-amplitude/127.0)*(i-128)+1.65-mid+amplitude)*255.0/3.3;
    }
}

// 生成直流数据表
void Generate_DC_Wave(float amplitude)
{
    uint16_t dc_value = (amplitude*255.0)/3.3 ;
    for (int i = 0; i < WAVE_LENGTH; i++)
    {
        dc_wavedata[i] = dc_value;
    }
}


//初始化
void Generate_init(float amplitude)
{
	   Generate_Sine_Wave(amplitude);
	   Generate_Square_Wave(amplitude);
	   Generate_Triangle_Wave(amplitude);
	   Generate_DC_Wave(amplitude);
}
