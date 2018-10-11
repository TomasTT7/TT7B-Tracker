/*
	The inter-integrated circuit (I2C) interface is one of the available modes in the Serial Communication Interface (SERCOM).
	
	TT7B TRACKER
		PA08	SDA		SERCOM 0/2		Pad[0]		Peripheral function C/D
		PA09	SCL		SERCOM 0/2		Pad[1]		Peripheral function C/D
*/


#ifndef L21_SERCOM_I2C_H
#define L21_SERCOM_I2C_H


#include "stdint.h"


// Functions
void SERCOM_I2C_enable(uint32_t mclk, uint32_t fscl);
void SERCOM_I2C_disable(void);
void SERCOM_I2C_reset(void);
uint8_t SERCOM_I2C_transmit_address(uint8_t address);
uint8_t SERCOM_I2C_transmit_byte(uint8_t data);
uint8_t SERCOM_I2C_receive_byte(uint8_t last);
void SERCOM_I2C_end_transmission(void);
uint8_t SERCOM_I2C_end_reception(void);


#endif // L21_SERCOM_I2C_H_