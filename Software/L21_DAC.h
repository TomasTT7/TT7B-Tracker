/*
	The Digital-to-Analog Converter (DAC) converts a digital value to a voltage.
	The DAC is 12-bit resolution and it is capable of converting up to 1,000,000 samples per second (MSPS).
	
	TT7B TRACKER
		DAC[1]		PA05		Peripheral function B
*/


#ifndef L21_DAC_H
#define L21_DAC_H


#include "stdint.h"


#define VDDANA_DAC 1800											// [mV] DAC's VDDANA reference voltage


// Functions
void DAC_enable(uint8_t refsel, uint8_t cctrl);
void DAC_disable(void);
void DAC_reset(void);
void DAC_write_value(uint16_t val);


#endif // L21_DAC_H_