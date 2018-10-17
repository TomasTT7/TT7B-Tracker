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


#ifndef L21_OSC_H
#define L21_OSC_H


#include "stdint.h"


// Functions
void OSC_enable_XOSC32K(void);
void OSC_disable_XOSC32K(void);
void OSC_enable_OSC32K(void);
void OSC_disable_OSC32K(void);
void OSC_enable_OSC16M(uint8_t fsel);
void OSC_disable_OSC16M(void);
void OSC_enable_DFLL48M_closed(void);
void OSC_enable_DFLL48M_open(void);
void OSC_disable_DFLL48M(void);
void OSC_enable_FDPLL96M(uint8_t refclk, uint8_t presc, uint16_t ldr, uint8_t ldrfrac);
void OSC_disable_FDPLL96M(void);


#endif // L21_OSC_H_