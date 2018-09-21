/*
	The OSC32KCTRL gathers controls for all 32KHz oscillators and provides clock sources to the Generic
	Clock Controller (GCLK), Real-Time Counter (RTC), and Watchdog Timer (WDT).
	
	XOSC32K		32.768kHz external crystal oscillator
	OSC32K		32.768kHz high accuracy internal oscillator
	OSCULP32K	32.768kHz ultra low power internal oscillator (always on)
	
	The OSCCTRL gathers controls for all device oscillators and provides clock sources to the Generic Clock
	Controller (GCLK). The available clock sources are: XOSC, OSC16M, DFLL48M and FDPLL96M.
	
	XOSC		0.4-32MHz external crystal oscillator
	OSC16M		4/8/12/16MHz internal oscillator (default 4MHz clock source enabled after reset)
	DFLL48M		Digital Frequency Locked Loop, 48MHz output frequency
	FDPLL96M	Fractional Digital Phase Locked Loop, 48MHz to 96MHz output frequency, 32kHz to 2MHz reference
*/


#include "sam.h"
#include "L21_OSC.h"


/*
	Enables a 32kHz external clock signal (SiT1552 TCXO).
*/
void OSC_enable_XOSC32K(void)
{
	OSC32KCTRL->XOSC32K.bit.ONDEMAND = 0;						// oscillator always runs in standby
	OSC32KCTRL->XOSC32K.bit.RUNSTDBY = 1;
	OSC32KCTRL->XOSC32K.bit.EN1K = 1;							// 1KHz output is enabled
	OSC32KCTRL->XOSC32K.bit.EN32K = 1;							// 32KHz output is enabled
	OSC32KCTRL->XOSC32K.bit.XTALEN = 0;							// external clock connected on XIN32
	OSC32KCTRL->XOSC32K.bit.ENABLE = 1;							// enable oscillator
	while(!(OSC32KCTRL->STATUS.bit.XOSC32KRDY));				// wait for XOSC32K to be stable and ready
}


/*
	
*/
void OSC_disable_XOSC32K(void)
{
	OSC32KCTRL->XOSC32K.reg = 0x00;								// disable oscillator
}


/*
	Enables internal 32kHz oscillator.
*/
void OSC_enable_OSC32K(void)
{
	uint32_t * pNVMcalib = (uint32_t *) 0x00806020;				// factory calibration data
	uint32_t OSC32Kcalib = ((*pNVMcalib & 0x00001FC0) >> 6);
	
	OSC32KCTRL->OSC32K.bit.CALIB = OSC32Kcalib;					// OSC32K factory calibration
	OSC32KCTRL->OSC32K.bit.STARTUP = 3;							// 0.305ms
	OSC32KCTRL->OSC32K.bit.ONDEMAND = 0;						// oscillator always runs in standby
	OSC32KCTRL->OSC32K.bit.RUNSTDBY = 1; 
	OSC32KCTRL->OSC32K.bit.EN1K = 1;							// 1KHz output is enabled
	OSC32KCTRL->OSC32K.bit.EN32K = 1;							// 32KHz output is enabled
	OSC32KCTRL->OSC32K.bit.ENABLE = 1;							// enable oscillator
	while(!(OSC32KCTRL->STATUS.bit.OSC32KRDY));					// wait for OSC32K to be stable and ready
}


/*
	
*/
void OSC_disable_OSC32K(void)
{
	OSC32KCTRL->OSC32K.reg = 0x00;								// disable oscillator
}


/*
	Enables internal selectable frequency oscillator.

	FSEL	Frequency
	0		4MHz
	1		8MHz
	2		12MHz
	3		16MHz
*/
void OSC_enable_OSC16M(uint8_t fsel)
{
	OSCCTRL->OSC16MCTRL.bit.ONDEMAND = 0;						// in Standby runs if requested by peripheral
	OSCCTRL->OSC16MCTRL.bit.RUNSTDBY = 0;
	
	if(fsel > 3) OSCCTRL->OSC16MCTRL.bit.FSEL = 0;				// oscillator frequency selection
	else OSCCTRL->OSC16MCTRL.bit.FSEL = fsel;
	
	OSCCTRL->OSC16MCTRL.bit.ENABLE = 1;							// enable oscillator
	while(!(OSCCTRL->STATUS.bit.OSC16MRDY));					// wait for OSC16M to be stable and ready
}


/*
	
*/
void OSC_disable_OSC16M(void)
{
	OSCCTRL->OSC16MCTRL.reg = 0x00;								// disable oscillator
}


/*
	Enable DFLL48M in closed loop mode - 48MHz output frequency from a 32.768kHz reference frequency.
	Example power consumption - 362uA at 3.3V.
	
	Fclkdfll48m = DFLLMUL.MUL * Fclkdfll48m_ref
	
	GCLK[1] must be set to XOSC32K and enabled first.
	The device has to be in Performance Level 2 (PL2).
*/
void OSC_enable_DFLL48M_closed(void)
{
	GCLK->PCHCTRL[0].reg = (0x01 << 6) | 0x01;					// enable peripheral channel GCLK_DFLL48M_REF, source GCLK[1]
	while (!(GCLK->PCHCTRL[0].reg & (0x01 << 6)));				// wait for synchronization
	
	uint32_t * pNVMcalib = (uint32_t *) 0x00806020;				// factory calibration data
	uint32_t DFLL48Mcalib = ((*pNVMcalib & 0xFC000000) >> 26);
	
	while(!(OSCCTRL->STATUS.bit.DFLLRDY));						// wait for synchronization
	OSCCTRL->DFLLCTRL.reg = (1 << 10) | (1 << 8);				// bypass coarse lock, chill cycle disable, disable ONDEMAND
	
	OSCCTRL->DFLLSYNC.bit.READREQ = 1;							// to be able to read current value of DFLLVAL in closed-loop mode
	while(!(OSCCTRL->STATUS.bit.DFLLRDY));						// wait for synchronization
	OSCCTRL->DFLLVAL.reg = (DFLL48Mcalib << 10) | 512;			// write coarse and fine calibration values
	
	while(!(OSCCTRL->STATUS.bit.DFLLRDY));						// wait for synchronization
	OSCCTRL->DFLLMUL.reg = (0x01 << 26) | (10 << 16) | 1464;	// 47,972,352Hz = 32,768Hz * 1464 (datasheet max.: 47.981MHz)
	
	while(!(OSCCTRL->STATUS.bit.DFLLRDY));						// wait for synchronization
	OSCCTRL->DFLLCTRL.bit.MODE = 1;								// enter closed-loop mode
	
	while(!(OSCCTRL->STATUS.bit.DFLLRDY));						// wait for synchronization
	OSCCTRL->DFLLCTRL.bit.ENABLE = 1;							// enable DFLL
	while(!(OSCCTRL->STATUS.bit.DFLLLCKF));						// wait for fine lock
}


/*
	Enable DFLL48M in closed loop mode - 48MHz output frequency.
	Example power consumption - 286uA at 3.3V.
	
	The device has to be in Performance Level 2 (PL2).
*/
void OSC_enable_DFLL48M_open(void)
{
	uint32_t * pNVMcalib = (uint32_t *) 0x00806020;				// factory calibration data
	uint32_t DFLL48Mcalib = ((*pNVMcalib & 0xFC000000) >> 26);
	
	while(!(OSCCTRL->STATUS.bit.DFLLRDY));
	OSCCTRL->DFLLCTRL.reg = (0x1 << 1);							// enable DFLL
	while(!(OSCCTRL->STATUS.bit.DFLLRDY));
	OSCCTRL->DFLLVAL.reg = (OSCCTRL_DFLLVAL_COARSE(DFLL48Mcalib) | OSCCTRL_DFLLVAL_FINE(512));
	while(!(OSCCTRL->STATUS.bit.DFLLRDY));
}


/*
	
*/
void OSC_disable_DFLL48M(void)
{
	while(!(OSCCTRL->STATUS.bit.DFLLRDY));						// wait for synchronization
	OSCCTRL->DFLLCTRL.reg = 0;									// disable DFLL
}


/*
	Enable FDPLL96M - 48MHz-96MHz output frequency from a 32.768kHz reference frequency.
	Example power consumption - 454uA in PL0, 934uA in PL2.
	
	Three sources of reference clocks: XOSC32K, XOSC, GCLK.
	
	REFCLK	SOURCE CLOCK
	0		XOSC32K
	1		XOSC
	2		GCLK
	
	Fck = Fckr * (LDR + 1 + LDRFRAC / 16) * (1 / 2^PRESC)
	
	PRESC	Division Factor
	0		DPLL output is divided by 1
	1		DPLL output is divided by 2
	2		DPLL output is divided by 4
*/
void OSC_enable_FDPLL96M(uint8_t refclk, uint8_t presc)
{
	if(refclk > 2) OSCCTRL->DPLLCTRLB.bit.REFCLK = 0;			// XOSC32K clock reference
	else OSCCTRL->DPLLCTRLB.bit.REFCLK = refclk;
	
	if(presc > 2) OSCCTRL->DPLLPRESC.reg = 0;					// DPLL output is divided by 1
	else OSCCTRL->DPLLPRESC.reg = presc;
	while(OSCCTRL->DPLLSYNCBUSY.bit.DPLLPRESC);					// wait while synchronization is in progress
	
	OSCCTRL->DPLLRATIO.bit.LDR = 1463;							// 48,001,024Hz = 32768Hz * (1463 + 1 + 14/16) * (1 / 1^0)
	OSCCTRL->DPLLRATIO.bit.LDRFRAC = 14;
	while(OSCCTRL->DPLLSYNCBUSY.bit.DPLLRATIO);					// wait while synchronization is in progress
	
	OSCCTRL->DPLLCTRLA.bit.ONDEMAND = 0;						// in Standby runs if requested by peripheral
	OSCCTRL->DPLLCTRLA.bit.RUNSTDBY = 0;
	OSCCTRL->DPLLCTRLA.bit.ENABLE = 1;							// enable PLL
	while(OSCCTRL->DPLLSYNCBUSY.bit.ENABLE);					// wait while synchronization is in progress
}


/*
	
*/
void OSC_disable_FDPLL96M(void)
{
	OSCCTRL->DPLLCTRLA.reg = 0;									// disable PLL
	while(OSCCTRL->DPLLSYNCBUSY.bit.ENABLE);					// wait while synchronization is in progress
}