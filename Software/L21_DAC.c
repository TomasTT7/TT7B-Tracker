/*
	The Digital-to-Analog Converter (DAC) converts a digital value to a voltage.
	The DAC is 12-bit resolution and it is capable of converting up to 1,000,000 samples per second (MSPS).
	
	TT7B TRACKER
		DAC[1]		PA05		Peripheral function B
*/


#include "sam.h"
#include "L21_DAC.h"


/*
	REFSEL
		0x0		VREFAU		Unbuffered external voltage reference (not buffered in DAC, direct connection)
		0x1		VDDANA		Voltage supply
		0x2		VREFAB		Buffered external voltage reference (buffered in DAC)
		0x3		INTREF		Internal bandgap reference
	
	VOUTx is at tri-state level if DACx is not enabled.
	
	The DAC can only maintain its output within one LSB of the desired value for approximately 100µs. When
	a DAC is used to generate a static voltage or at a rate less than 20kSPS, the conversion must be
	refreshed periodically.
	
	T_refresh = REFRESH * T_osculp32k
	
	With DACCTRLx.REFRESH=1, if no new conversion is started before the refresh period is completed, DACx
	will convert the DATAx value again.
	
	CCTRL
		0x0		CC100K		GCLK_DAC <= 1.2MHz				(100kSPS)
		0x1		CC1M		1.2MHz < GCLK_DAC <= 6MHz		(500kSPS)
		0x2		CC12M		6MHz < GCLK_DAC <= 12MHz		(1MSPS)
*/
void DAC_enable(uint8_t refsel, uint8_t cctrl)
{
	GCLK->PCHCTRL[32].bit.GEN = 0;								// source clock for DAC is MCLK
	GCLK->PCHCTRL[32].bit.CHEN = 1;								// enable clock
	while(!(GCLK->PCHCTRL[32].bit.CHEN));						// wait for synchronization
	
	PORT->Group[0].PMUX[2].bit.PMUXO = 0x1;						// PA05 - peripheral function B selected
	PORT->Group[0].PINCFG[5].bit.PMUXEN = 1;					// PA05 - selected peripheral function controls direction and output drive value.
	
	DAC->CTRLB.bit.REFSEL = refsel;								// selects the Reference Voltage for both DACs
	
	DAC->DACCTRL[1].bit.REFRESH = 2;							// refresh every 61.04us
	DAC->DACCTRL[1].bit.CCTRL = cctrl;							// defines the current in output buffer
	DAC->DACCTRL[1].bit.ENABLE = 1;								// This bit enables DAC1 when DAC Controller is enabled (CTRLA.ENABLE).
	
	DAC->CTRLA.bit.ENABLE = 1;									// enable DAC
	while(DAC->SYNCBUSY.bit.ENABLE);							// SYNCBUSY.ENABLE will be cleared when the	operation is complete.
	
	while(!(DAC->STATUS.bit.READY1));							// Startup time has elapsed, DAC1 is ready for conversion.
}


/*
	Disable DAC peripheral, its clock, and restore original pin configuration.
*/
void DAC_disable(void)
{
	DAC->CTRLA.bit.ENABLE = 0;									// disable DAC
	while(DAC->SYNCBUSY.bit.ENABLE);							// SYNCBUSY.ENABLE will be cleared when operation is complete.
	DAC->DACCTRL[1].bit.ENABLE = 0;								// This bit enables DAC1 when DAC Controller is enabled (CTRLA.ENABLE).
	
	GCLK->PCHCTRL[32].bit.CHEN = 0;								// disable DAC clock
	while(GCLK->PCHCTRL[32].bit.CHEN);							// wait for synchronization
	
	PORT->Group[0].PINCFG[5].bit.PMUXEN = 0;					// PA05 - PORT registers control the direction and output drive value.
}


/*
	Resets all registers in the DAC to their initial state, and the DAC will be disabled.
*/
void DAC_reset(void)
{
	DAC->CTRLA.bit.SWRST = 1;									// reset DAC
	while(DAC->SYNCBUSY.bit.SWRST);								// SYNCBUSY.SWRST will be cleared when reset is complete.
}


/*
	A conversion is started when new data is loaded to the Data register. The resulting voltage
	is available on the DAC output after the conversion time.
	
	V_out = VAL / 4095 * VREF
	
	A new conversion starts as soon as a new value is loaded into DATAx. DATAx can either be loaded
	via the APB bus during a CPU write operation, using DMA, or from the DATABUFx register when
	a STARTx event occurs
	
	T_conv = 12 * 2 * T_gclk
	
	The conversion time is given by the period TGCLK of the generic clock GCLK_DAC and the number of bits.
	
	A new conversion can be started after only 12 GCLK_DAC periods. Therefore if DATAx is written while
	a conversion is ongoing, start of conversion is postponed until DACx is ready to start next conversion.
*/
void DAC_write_value(uint16_t val)
{
	while(DAC->SYNCBUSY.bit.DATA1);								// This bit is cleared when DATA1 synchronization is completed.
	DAC->DATA[1].reg = val;										// contains 12-bit value that is converted to voltage by DAC1
}