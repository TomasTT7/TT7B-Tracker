/*
	The inter-integrated circuit (I2C) interface is one of the available modes in the Serial Communication Interface (SERCOM).
	
	TT7B TRACKER
		PA08	SDA		SERCOM 0/2		Pad[0]		Peripheral function C/D
		PA09	SCL		SERCOM 0/2		Pad[1]		Peripheral function C/D
*/


#include "sam.h"
#include "L21_SERCOM_I2C.h"


/*
	MODE
		0x0		USART with external clock
		0x1		USART with internal clock
		0x2		SPI in slave operation
		0x3		SPI in master operation
		0x4		I2C slave operation
		0x5		I2C master operation
*/
void SERCOM_I2C_enable(uint32_t mclk, uint32_t fscl)
{
	GCLK->PCHCTRL[18].bit.GEN = 0;								// source clock for SERCOM0 is MCLK
	GCLK->PCHCTRL[18].bit.CHEN = 1;								// enable GCLK_SERCOM0_CORE clock
	while(!(GCLK->PCHCTRL[18].bit.CHEN));						// wait for synchronization
	
	PORT->Group[0].PMUX[4].bit.PMUXE = 0x2;						// PA08 - peripheral function C selected
	PORT->Group[0].PMUX[4].bit.PMUXO = 0x2;						// PA09
	
	PORT->Group[0].PINCFG[8].bit.PMUXEN = 1;					// PA08 - selected peripheral function controls direction and output drive value.
	PORT->Group[0].PINCFG[9].bit.PMUXEN = 1;					// PA09
	
	SERCOM0->I2CM.CTRLA.bit.MODE = 0x05;						// I2C master operation
	SERCOM0->I2CM.CTRLA.bit.SPEED = 0;							// Standard-mode (Sm) up to 100 kHz and Fast-mode (Fm) up to 400 kHz
	SERCOM0->I2CM.CTRLA.bit.PINOUT = 0;							// 4-wire operation disabled
	SERCOM0->I2CM.CTRLA.bit.RUNSTDBY = 0;						// GCLK_SERCOMx_CORE is disabled and I2C master will not operate in standby sleepmode.
	
	SERCOM0->I2CM.CTRLB.bit.ACKACT = 0;							// Send ACK after a data byte is received from I2C slave.
	SERCOM0->I2CM.CTRLB.bit.SMEN = 1;							// Smart mode is enabled (acknowledge action is sent when DATA.DATA is read).
	
	uint8_t reg_val = (uint8_t)((float)mclk / (float)fscl / 2.0 - 5.0 - (float)mclk * 0.000001 / 2.0);
	
	SERCOM0->I2CM.BAUD.bit.BAUD = reg_val;						// F_SCL = F_GCLK / (10 + 2 * BAUD + F_GCLK * T_RISE)
	
	SERCOM0->I2CM.CTRLA.bit.ENABLE = 1;							// enable SERCOM0
	while(SERCOM0->I2CM.SYNCBUSY.bit.ENABLE);					// SYNCBUSY.ENABLE is cleared when the operation is complete.
}


/*
	
*/
void SERCOM_I2C_disable(void)
{
	SERCOM0->I2CM.CTRLA.bit.ENABLE = 0;							// disable SERCOM0
	while(SERCOM0->I2CM.SYNCBUSY.bit.ENABLE);					// SYNCBUSY.ENABLE is cleared when the operation is complete.
	
	GCLK->PCHCTRL[18].bit.CHEN = 0;								// disable GCLK_SERCOM0_CORE clock
	while(GCLK->PCHCTRL[18].bit.CHEN);							// wait for synchronization
	
	PORT->Group[0].PINCFG[8].bit.PMUXEN = 0;					// PA08 - PORT registers control the direction and output drive value.
	PORT->Group[0].PINCFG[9].bit.PMUXEN = 0;					// PA09
}


/*
	
*/
void SERCOM_I2C_reset(void)
{
	SERCOM0->I2CM.CTRLA.bit.SWRST = 1;							// Resets all registers in SERCOM0 to their initial state, and SERCOM0 will be disabled.
	while(SERCOM0->I2CM.SYNCBUSY.bit.SWRST);					// SYNCBUSY.SWRST will be cleared when reset is complete.
}


/*
	
*/
uint8_t SERCOM_I2C_transmit_address(uint8_t address)
{	
	if(SERCOM0->I2CM.STATUS.bit.BUSSTATE == 0x00)				// The bus state is unknown to the I2C master.
	{
		SERCOM0->I2CM.STATUS.bit.BUSSTATE = 0x01;				// When in UNKNOWN state, writing 0x1 to BUSSTATE forces bus into IDLE state.
		while(SERCOM0->I2CM.SYNCBUSY.bit.SYSOP);				// Writing CTRLB.CMD, STATUS.BUSSTATE, ADDR, or DATA when SERCOM enabled requires sync.
	}
	
	SERCOM0->I2CM.ADDR.bit.ADDR = address;						// The I2C master will issue a start condition followed by address written in ADDR.
	
	while(!(SERCOM0->I2CM.INTFLAG.bit.MB) && !(SERCOM0->I2CM.INTFLAG.bit.SB));
	
	if(SERCOM0->I2CM.STATUS.bit.RXNACK) return 0;				// This bit indicates whether the last address or data packet sent was acknowledged or not.
	else return 1;
}


/*
	
*/
uint8_t SERCOM_I2C_transmit_byte(uint8_t data, uint8_t stop, uint8_t repeat)
{
	SERCOM0->I2CM.DATA.reg = data;								// provides access to master transmit and receive data buffers
	
	while(!(SERCOM0->I2CM.INTFLAG.bit.MB));						// This flag is set when a byte is transmitted in master write mode.
	
	if(stop)
	{
		SERCOM0->I2CM.CTRLB.bit.CMD = 0x3;						// Execute acknowledge action succeeded by issuing a stop condition.
		while(SERCOM0->I2CM.SYNCBUSY.bit.SYSOP);				// Issuing command will set System Operation bit in Synchronization Busy register.
	}
	
	if(repeat)
	{
		SERCOM0->I2CM.CTRLB.bit.CMD = 0x1;						// Execute acknowledge action succeeded by repeated Start
		while(SERCOM0->I2CM.SYNCBUSY.bit.SYSOP);				// Issuing command will set System Operation bit in Synchronization Busy register.
	}
	
	if(SERCOM0->I2CM.STATUS.bit.RXNACK) return 0;				// This bit indicates whether the last address or data packet sent was acknowledged or not.
	else return 1;
}


/*
	
*/
uint8_t SERCOM_I2C_receive_byte(uint8_t stop)
{
	uint8_t data = 0;
	
	while(!(SERCOM0->I2CM.INTFLAG.bit.SB));						// Slave on Bus flag is set when a byte is successfully received in master read mode.
	
	if(stop)
	{
		SERCOM0->I2CM.CTRLB.bit.CMD = 0x3;						// Execute acknowledge action succeeded by issuing a stop condition.
		while(SERCOM0->I2CM.SYNCBUSY.bit.SYSOP);				// Issuing command will set System Operation bit in Synchronization Busy register.
	}
	
	data = SERCOM0->I2CM.DATA.reg;								// provides access to master transmit and receive data buffers
	
	return data;
}