/*
	The Universal Synchronous and Asynchronous Receiver and Transmitter (USART) is one of the available modes in the Serial Communication Interface (SERCOM).
	
	TT7B TRACKER
		PA23	RX		SERCOM 3/5		Pad[1]		Peripheral function C/D
		PA24	TX		SERCOM 3/5		Pad[2]		Peripheral function C/D
*/


#include "sam.h"
#include "L21_SERCOM_USART.h"


/*
	MODE
		0x0		USART with external clock
		0x1		USART with internal clock
		0x2		SPI in slave operation
		0x3		SPI in master operation
		0x4		I2C slave operation
		0x5		I2C master operation
*/
void SERCOM_USART_enable(uint32_t mclk, uint32_t baud)
{
	GCLK->PCHCTRL[21].bit.GEN = 0;								// source clock for SERCOM3 is MCLK
	GCLK->PCHCTRL[21].bit.CHEN = 1;								// enable GCLK_SERCOM3_CORE clock
	while(!(GCLK->PCHCTRL[21].bit.CHEN));						// wait for synchronization
	
	PORT->Group[0].PMUX[11].bit.PMUXO = 0x2;					// PA23 - peripheral function C selected
	PORT->Group[0].PMUX[12].bit.PMUXE = 0x2;					// PA24
	
	PORT->Group[0].PINCFG[23].bit.PMUXEN = 1;					// PA23 - selected peripheral function controls direction and output drive value.
	PORT->Group[0].PINCFG[24].bit.PMUXEN = 1;					// PA24
	
	SERCOM3->USART.CTRLA.bit.MODE = 0x1;						// USART with internal clock.
	SERCOM3->USART.CTRLA.bit.CMODE = 0;							// Asynchronous communication.
	SERCOM3->USART.CTRLA.bit.RXPO = 0x1;						// PA23 - RX - SERCOM PAD[1] is used for data reception
	SERCOM3->USART.CTRLA.bit.TXPO = 0x1;						// PA24 - TX - SERCOM PAD[2] is used for data transmission
	SERCOM3->USART.CTRLB.bit.CHSIZE = 0;						// Character size 8 bits
	SERCOM3->USART.CTRLA.bit.DORD = 1;							// LSB is transmitted first.
	SERCOM3->USART.CTRLB.bit.SBMODE = 0;						// One stop bit.
	
	uint16_t reg_val = (uint16_t)(65536.0 * (1.0 - (float)baud * 16.0 / (float)mclk));
	
	SERCOM3->USART.BAUD.reg = reg_val;							// BAUD = 65536 * (1 - F_b * S / F_ref) -> 63019 = 65536 * (1 - 9600 * 16 / 4000000)
	
	SERCOM3->USART.CTRLB.reg |= (1 << 17) | (1 << 16);			// RXEN receiver is enabled, TXEN transmitter is enabled
	
	SERCOM3->USART.CTRLA.bit.ENABLE = 1;						// enable SERCOM3
	while(SERCOM3->USART.SYNCBUSY.bit.ENABLE);					// SYNCBUSY.ENABLE is cleared when the operation is complete.
}


/*
	
*/
void SERCOM_USART_disable(void)
{
	SERCOM3->USART.CTRLA.bit.ENABLE = 0;						// disable SERCOM3
	while(SERCOM3->USART.SYNCBUSY.bit.ENABLE);					// SYNCBUSY.ENABLE is cleared when the operation is complete.
	
	GCLK->PCHCTRL[21].bit.CHEN = 0;								// disable GCLK_SERCOM3_CORE clock
	while(GCLK->PCHCTRL[21].bit.CHEN);							// wait for synchronization
	
	PORT->Group[0].PINCFG[23].bit.PMUXEN = 0;					// PA23 - PORT registers control the direction and output drive value.
	PORT->Group[0].PINCFG[24].bit.PMUXEN = 0;					// PA24
}


/*
	
*/
void SERCOM_USART_reset(void)
{
	SERCOM3->USART.CTRLA.bit.SWRST = 1;							// Resets all registers in SERCOM3 to their initial state, and SERCOM3 will be disabled.
	while(SERCOM3->USART.SYNCBUSY.bit.SWRST);					// SYNCBUSY.SWRST will be cleared when reset is complete.
}


/*
	
*/
void SERCOM_USART_write_byte(uint8_t data)
{
	while(!(SERCOM3->USART.INTFLAG.bit.DRE));					// This flag is set when DATA is empty and ready to be written.
	SERCOM3->USART.DATA.reg = data;								// Writing to DATA will write the Transmit Data register.
}


/*
	
*/
uint8_t SERCOM_USART_read_byte(uint32_t * timeout)
{
	uint8_t data;
	uint32_t _timeout = *timeout;
	
	while(!(SERCOM3->USART.INTFLAG.bit.RXC) && _timeout) _timeout--;	// This flag is set when there are unread data in DATA.
	
	if(!_timeout)												// byte reception timed out
	{
		*timeout = 0;											// inform about timing out
		return 0;
	}
	
	data = SERCOM3->USART.DATA.reg;								// Reading DATA will return the contents of the Receive Data register.
	
	return data;												// return received byte
}


/*
	Disabling the receiver will flush the receive buffer and clear the FERR, PERR and BUFOVF bits in the STATUS register.
*/
void SERCOM_USART_flush_rxbuffer(void)
{
	SERCOM3->USART.CTRLB.bit.RXEN = 0;							// Writing '0' to this bit will disable the USART receiver.
	SERCOM3->USART.CTRLB.bit.RXEN = 1;							// When USART is enabled, SYNCBUSY.CTRLB will remain set until receiver is enabled.
	while(SERCOM3->USART.SYNCBUSY.bit.CTRLB);					// SYNCBUSY.CTRLB, which will remain set until the receiver is enabled.
}