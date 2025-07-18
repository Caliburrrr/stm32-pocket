/*
 * oscilloscope.h
 *
 *  Created on: Feb 9, 2025
 *      Author: asus
 */
#include "main.h"

#ifndef INC_OSCILLOSCOPE_H_
#define INC_OSCILLOSCOPE_H_

void get_max_min(uint16_t *max, uint16_t *min);
float get_amplitude();


void fftdata(void);
void getfrequency(float* freq);


void adcvalue_change_waveform(uint8_t xpoint,uint8_t y);
uint8_t get_triggervoltage_pos(uint8_t tgvalue);
void oledshow_waveform(uint8_t tgpos);


#endif /* INC_OSCILLOSCOPE_H_ */
