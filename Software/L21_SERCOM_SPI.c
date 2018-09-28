/*
	The serial peripheral interface (SPI) is one of the available modes in the Serial Communication Interface (SERCOM).
	
	TT7B TRACKER
		PA14	CS1		GPIO			-			-
		PA15	CS2		GPIO			-			-
		PA17	MISO	SERCOM 1/3		Pad[1]		Peripheral function C/D
		PA18	MOSI	SERCOM 1/3		Pad[2]		Peripheral function C/D
		PA19	SCLK	SERCOM 1/3		Pad[3]		Peripheral function C/D
*/


#include "sam.h"
#include "L21_SERCOM_SPI.h"


/*
	MODE
		0x0		USART with external clock
		0x1		USART with internal clock
		0x2		SPI in slave operation
		0x3		SPI in master operation
		0x4		I2C slave operation
		0x5		I2C master operation
*/
void SERCOM_SPI_enable(void)
{
	GCLK->PCHCTRL[19].bit.GEN = 0;								// source clock for SERCOM1 is MCLK
	GCLK->PCHCTRL[19].bit.CHEN = 1;								// enable GCLK_SERCOM1_CORE clock
	while(!(GCLK->PCHCTRL[19].bit.CHEN));						// wait for synchronization
	
	PORT->Group[0].DIRSET.reg = (1 << 14) | (1 << 15);			// PA14 and PA15 CS pins set as OUTPUT
	PORT->Group[0].OUTSET.reg = (1 << 14) | (1 << 15);			// PA14 and PA15 CS pins set HIGH
	
	PORT->Group[0].PMUX[8].bit.PMUXO = 0x2;						// PA17 - peripheral function C selected
	PORT->Group[0].PMUX[9].bit.PMUXE = 0x2;						// PA18
	PORT->Group[0].PMUX[9].bit.PMUXO = 0x2;						// PA19
	
	PORT->Group[0].PINCFG[17].bit.PMUXEN = 1;					// PA17 - selected peripheral function controls direction and output drive value.
	PORT->Group[0].PINCFG[18].bit.PMUXEN = 1;					// PA18
	PORT->Group[0].PINCFG[19].bit.PMUXEN = 1;					// PA19
	
	SERCOM1->SPI.CTRLA.bit.MODE = 0x3;							// SPI master operation.
	SERCOM1->SPI.CTRLA.bit.CPOL = 0;							// SCK is low when idle.
	SERCOM1->SPI.CTRLA.bit.CPHA = 0;							// Mode 0. Rising, sample. Falling, change.
	SERCOM1->SPI.CTRLA.bit.DIPO = 0x1;							// PA17 - MISO - SERCOM PAD[1] is used as data input
	SERCOM1->SPI.CTRLA.bit.DOPO = 0x1;							// PA18 - MOSI - SERCOM PAD[2], PA19 - SCLK - SERCOM PAD[3]
	SERCOM1->SPI.CTRLB.bit.CHSIZE = 0;							// Character size 8 bits.
	SERCOM1->SPI.CTRLA.bit.DORD = 0;							// MSB is transferred first.
	SERCOM1->SPI.BAUD.reg = 0;									// F_BAUD = F_REF / (2 * (BAUD + 1)) -> GCLK_SERCOM1_CORE / 2
	SERCOM1->SPI.CTRLB.bit.MSSEN = 0;							// Hardware SS control is disabled.
	SERCOM1->SPI.CTRLB.bit.RXEN = 1;							// Receiver is enabled or it will be enabled when SPI is enabled.
	
	SERCOM1->SPI.CTRLA.bit.ENABLE = 1;							// enable SERCOM1
	while(SERCOM1->SPI.SYNCBUSY.bit.ENABLE);					// SYNCBUSY.ENABLE is cleared when the operation is complete.
}


/*
	
*/
void SERCOM_SPI_disable(void)
{
	SERCOM1->SPI.CTRLA.bit.ENABLE = 0;							// disable SERCOM1
	while(SERCOM1->SPI.SYNCBUSY.bit.ENABLE);					// SYNCBUSY.ENABLE is cleared when the operation is complete.
	
	GCLK->PCHCTRL[19].bit.CHEN = 0;								// disable GCLK_SERCOM1_CORE clock
	while(GCLK->PCHCTRL[19].bit.CHEN);							// wait for synchronization
	
	PORT->Group[0].PINCFG[17].bit.PMUXEN = 0;					// PA17 - PORT registers control the direction and output drive value.
	PORT->Group[0].PINCFG[18].bit.PMUXEN = 0;					// PA18
	PORT->Group[0].PINCFG[19].bit.PMUXEN = 0;					// PA19
	
	PORT->Group[0].OUTCLR.reg = (1 << 14) | (1 << 15);			// PA14 and PA15 CS pins set LOW
	PORT->Group[0].DIRCLR.reg = (1 << 14) | (1 << 15);			// PA14 and PA15 CS pins set as INPUT
}


/*
	
*/
void SERCOM_SPI_reset(void)
{
	SERCOM1->SPI.CTRLA.bit.SWRST = 1;							// Resets all registers in SERCOM1 to their initial state, and SERCOM1 will be disabled.
	while(SERCOM1->SPI.SYNCBUSY.bit.SWRST);						// SYNCBUSY.SWRST will be cleared when reset is complete.
}


/*
	
*/
uint8_t SERCOM_SPI_transmission(uint8_t data)
{
	uint8_t rx_data;
	
	SERCOM1->SPI.DATA.reg = data;								// Writing DATA register will write the transmit data buffer.
	while(!(SERCOM1->SPI.INTFLAG.bit.RXC));						// Flag is cleared by reading DATA register or by disabling receiver.
	
	rx_data = SERCOM1->SPI.DATA.reg;							// Reading DATA register will return the contents of the receive data buffer. 
	
	return rx_data;
}