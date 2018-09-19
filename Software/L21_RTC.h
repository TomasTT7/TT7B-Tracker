/*
	The Real-Time Counter (RTC) is a 32-bit counter with a 10-bit programmable prescaler.
*/


#ifndef L21_RTC_H
#define L21_RTC_H


#include "stdint.h"


// Functions
void RTC_mode0_enable(uint8_t prescaler, uint32_t comp);
void RTC_mode0_disable(void);
void RTC_mode0_reset(void);
void RTC_mode0_update_compare(uint32_t comp);
void RTC_Handler(void);


#endif // L21_RTC_H_