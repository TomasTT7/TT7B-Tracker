/*
	The serial peripheral interface (SPI) is one of the available modes in the Serial Communication Interface (SERCOM).
	
	TT7B TRACKER
		PA14	CS1		GPIO			-			-
		PA15	CS2		GPIO			-			-
		PA17	MISO	SERCOM 1/3		Pad[1]		Peripheral function C/D
		PA18	MOSI	SERCOM 1/3		Pad[2]		Peripheral function C/D
		PA19	SCLK	SERCOM 1/3		Pad[3]		Peripheral function C/D
*/


#ifndef L21_SERCOM_SPI_H
#define L21_SERCOM_SPI_H


#include "stdint.h"


// Functions
void SERCOM_SPI_enable(void);
void SERCOM_SPI_disable(void);
void SERCOM_SPI_reset(void);
uint8_t SERCOM_SPI_transmission(uint8_t data);


#endif // L21_SERCOM_SPI_H_