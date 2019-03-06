/*
	SysTick
		24-bit system timer that counts down from the reload value to zero.
		CLK source is either MCK or MCK/8.
*/


#ifndef L21_SYSTICK_H
#define L21_SYSTICK_H


#include "stdint.h"


extern uint32_t SysTick_CLK;					// [Hz] current MCK (e.g. 4000000 for 4MHz MCK)


// Functions
void SysTick_delay_init(void);
void SysTick_delay_s(uint32_t s);
void SysTick_delay_ms(uint32_t ms);
void SysTick_delay_us(uint32_t us);
void SysTick_delay_disable(void);


#endif // L21_SYSTICK_H_