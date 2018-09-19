/*
	The Real-Time Counter (RTC) is a 32-bit counter with a 10-bit programmable prescaler.
*/


#include "sam.h"
#include "L21_RTC.h"


/*
	MODE
		0	32-bit counter
		1	16-bit counter
		2	Clock/Calendar
	
	PRESCALER																	60s		90s		120s
		0x0		OFF			CLK_RTC_CNT = GCLK_RTC/1		30.51us		36.4h	
		0x1		DIV1		CLK_RTC_CNT = GCLK_RTC/1		30.51us		36.4h	
		0x2		DIV2		CLK_RTC_CNT = GCLK_RTC/2		61.04us		72.8h	
		0x3		DIV4		CLK_RTC_CNT = GCLK_RTC/4		0.122ms		6.1d	
		0x4		DIV8		CLK_RTC_CNT = GCLK_RTC/8		0.244ms		12.1d	
		0x5		DIV16		CLK_RTC_CNT = GCLK_RTC/16		0.488ms		24.3d	
		0x6		DIV32		CLK_RTC_CNT = GCLK_RTC/32		0.977ms		48.5d	
		0x7		DIV64		CLK_RTC_CNT = GCLK_RTC/64		1.953ms		97.1d	
		0x8		DIV128		CLK_RTC_CNT = GCLK_RTC/128		3.906ms		194d	
		0x9		DIV256		CLK_RTC_CNT = GCLK_RTC/256		7.813ms		1.06y	7680	11520	15360
		0xA		DIV512		CLK_RTC_CNT = GCLK_RTC/512		15.63ms		2.13y	3840	5760	7680
		0xB		DIV1024		CLK_RTC_CNT = GCLK_RTC/1024		31.25ms		4.26y	1920	2880	3840
		
	Expects XOSC32K clock source already running.
	
	RTCSEL
		0x0		ULP1K		1.024kHz from 32KHz internal ULP oscillator
		0x1		ULP32K		32.768kHz from 32KHz internal ULP oscillator
		0x2		OSC1K		1.024kHz from 32KHz internal oscillator
		0x3		OSC32K		32.768kHz from 32KHz internal oscillator
		0x4		XOSC1K		1.024kHz from 32KHz external oscillator
		0x5		XOSC32K		32.768kHz from 32KHz external crystal oscillator	
*/
void RTC_mode0_enable(uint8_t prescaler, uint32_t comp)
{
	OSC32KCTRL->RTCCTRL.bit.RTCSEL = 0x05;
	
	if(prescaler > 0xB) RTC->MODE0.CTRLA.bit.PRESCALER = 0;
	else RTC->MODE0.CTRLA.bit.PRESCALER = prescaler;
	
	RTC->MODE0.CTRLA.bit.COUNTSYNC = 1;							// COUNT read synchronization is enabled
	RTC->MODE0.CTRLA.bit.MATCHCLR = 0;							// counter is not cleared on a Compare/Alarm 0 match
	RTC->MODE0.CTRLA.bit.MODE = 0;								// 32-bit counter
	
	RTC->MODE0.INTENSET.bit.CMP0 = 1;							// Compare 0 interrupt is enabled
	NVIC_EnableIRQ(RTC_IRQn);
	
	RTC->MODE0.COMP[0].bit.COMP = comp;							// set first compare match
	while(RTC->MODE0.SYNCBUSY.bit.COMP0);						// wait while write synchronization for COMP0 register is ongoing
	
	RTC->MODE0.CTRLA.bit.ENABLE = 1;							// enable RTC
	while(RTC->MODE0.SYNCBUSY.bit.ENABLE);						// SYNCBUSY.ENABLE will be cleared when synchronization is complete
}


/*
	
*/
void RTC_mode0_disable(void)
{
	RTC->MODE0.CTRLA.bit.ENABLE = 0;							// disable RTC
	while(RTC->MODE0.SYNCBUSY.bit.ENABLE);						// SYNCBUSY.ENABLE will be cleared when synchronization is complete
}


/*
	All registers in the RTC, will be reset to their initial state, and the RTC will be disabled.
	The	RTC must be disabled before resetting it.
*/
void RTC_mode0_reset(void)
{
	RTC->MODE0.CTRLA.bit.SWRST = 1;								// reset RTC
	while(RTC->MODE0.CTRLA.bit.SWRST);							// CTRLA.SWRST will be cleared when reset is complete
}



/*
	Update the compare match register in a continually running RTC setup.
*/
void RTC_mode0_update_compare(uint32_t comp)
{
	uint32_t count = RTC->MODE0.COUNT.reg;						// get the current RTC COUNT value
	
	RTC->MODE0.COMP[0].bit.COMP = count + comp;					// update next compare match
	while(RTC->MODE0.SYNCBUSY.bit.COMP0);						// wait while write synchronization for COMP0 register is ongoing
}


/*
	RTC Compare 0 interrupt handler.
*/
void RTC_Handler(void)
{
	RTC->MODE0.INTFLAG.bit.CMP0 = 1;							// writing a '1' to this bit clears the Compare 0 interrupt flag
}