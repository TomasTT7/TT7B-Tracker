/*
	The Universal Synchronous and Asynchronous Receiver and Transmitter (USART) is one of the available modes in the Serial Communication Interface (SERCOM).
	
	TT7B TRACKER
		PA23	RX		SERCOM 3/5		Pad[1]		Peripheral function C/D
		PA24	TX		SERCOM 3/5		Pad[2]		Peripheral function C/D
*/


#ifndef L21_SERCOM_USART_H
#define L21_SERCOM_USART_H


#include "stdint.h"


// Functions
void SERCOM_USART_enable(uint32_t mclk, uint32_t baud);
void SERCOM_USART_disable(void);
void SERCOM_USART_reset(void);
void SERCOM_USART_write_byte(uint8_t data);
uint8_t SERCOM_USART_read_byte(void);


#endif // L21_SERCOM_USART_H_