/*
	Timer Counter (TC) instances:	3 (SAML21E)
	Selectable configuration:		8-bit, 16-bit or 32-bit operation
	
	TC0 and TC1 share a peripheral clock channel, as do TC2 and TC3. For this reason they
	cannot be set to different clock frequencies.
*/


#ifndef L21_TC_H
#define L21_TC_H


#include "stdint.h"


// Functions
void TC0_enable(uint8_t prescaler, uint8_t compare0, uint8_t per, uint8_t interrupt);
void TC0_disable(void);
void TC0_reset(void);
void TC0_count_value(uint8_t count);
void TC0_compare_value_ch0(uint8_t compare);
void TC0_compare_value_ch1(uint8_t compare);
void TC0_Handler(void);

void TC4_enable(uint8_t prescaler, uint8_t compare0, uint8_t per, uint8_t interrupt);
void TC4_disable(void);
void TC4_reset(void);
void TC4_count_value(uint8_t count);
void TC4_compare_value_ch0(uint8_t compare);
void TC4_compare_value_ch1(uint8_t compare);
void TC4_Handler(void);

#endif // L21_TC_H_