/*
	WatchDog is enabled by default. Must be disabled in case it is not desired.
*/


#include "sam.h"
#include "L21_WatchDog.h"


/*
	There are 12 possible WDT time-out (TOWDT) periods, selectable from 8ms to 16s.
	The WDT will continue to operate in any sleep mode where the source clock is active except backup mode.
	One period as a number of 1.024kHz CLK_WDTOSC clock cycles:
	
		PER Value	Name		Description			Period
		0x0			CYC8		8 clock cycles		7.8125ms
		0x1			CYC16		16 clock cycles		15.625ms
		0x2			CYC32		32 clock cycles		31.25ms
		0x3			CYC64		64 clock cycles		62.5ms
		0x4			CYC128		128 clock cycles	125ms
		0x5			CYC256		256 clock cycles	250ms
		0x6			CYC512		512 clock cycles	500ms
		0x7			CYC1024		1024 clock cycles	1s
		0x8			CYC2048		2048 clock cycles	2s
		0x9			CYC4096		4096 clock cycles	4s
		0xA			CYC8192		8192 clock cycles	8s
		0xB			CYC16384	16384 clock cycles	16s
		0xC-0xF		Reserved
*/
void WatchDog_enable(uint8_t PER)
{
	if(PER > 0x0B) WDT->CONFIG.reg = 0x0B;				// configure the timeout period
	else WDT->CONFIG.reg = PER;
	
	while(WDT->SYNCBUSY.bit.ENABLE);					// check if a sync operation is in progress
	WDT->CTRLA.bit.ENABLE = 1;							// enable the timer
}


/*
	Disables the timer in case it is not utilized.
*/
void WatchDog_disable(void)
{
	while(WDT->SYNCBUSY.bit.ENABLE);					// check if a sync operation is in progress
	WDT->CTRLA.reg = 0x00;								// Always-On disabled, Window Mode disabled, WatchDog disabled
}


/*
	Clears the WatchDog timer.
	Writing anything else to this register but 0xA5 issues immediate system reset.
*/
void WatchDog_reset(void)
{
	WDT->CLEAR.reg = 0xA5;
}