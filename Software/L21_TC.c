/*
	Timer Counter (TC) instances:	3 (SAML21E)
	Selectable configuration:		8-bit, 16-bit or 32-bit operation
	
	TC0 and TC1 share a peripheral clock channel, as do TC2 and TC3. For this reason they
	cannot be set to different clock frequencies.
*/


#include "sam.h"
#include "L21_TC.h"
#include "L21_DAC.h"


/*
	OSCILLATOR: FDPLL96, REFCLK = XOSC32K
		Fck = Fckr * (LDR + 1 + LDRFRAC / 16) * (1 / 2^PRESC)
		65,894,400Hz = 32,768Hz * (2009 + 1 + 15/16) * (1 / 2^0)
	
	MCLK: GCLK[0], DIV = 4, SRC = FDPLL96
		16,473,600Hz = 65,894,400Hz / 4
	
	DAC: GCLK[2], DIV = 16, SRC = FDPLL96
		4,118,400Hz = 65,894,400Hz / 16
	
	TC0: MCLK, COMPARE = 858
		19,200Hz = 16,473,600Hz / 858
	
	TC4: MCLK, COMPARE = 468
		35,200Hz = 16,473,600Hz / 468
	
	1200Hz TONE: TABLE = 16 byte
		1200Hz = 19,200Hz / 16
	
	2200Hz TONE: TABLE = 16 byte
		2200hz = 35,200Hz / 16
	
	SINE WAVE
		Necessary to adjust voltage levels in the 1200Hz tone due to PRE-EMPHASIS.
		One table for each tone to avoid lengthy computation inside interrupt handler.
		SINE_2200HZ: RANGE = 0V-1.8V
		SINE_1200HZ: RANGE = 0.41V-1.4V
	
	SI5351B
		OFFSET = Hz
*/
volatile static uint8_t tone_switch = 1;						// controls switching between 1200Hz (1) and 2200Hz (0) tones
volatile static uint8_t _n = 0;									// iterates through sine wave tables
volatile static uint16_t count_bit = 0;							// times the start of the next bit

volatile static uint8_t transmitting = 0;
volatile static uint16_t bit_tx = 0;							// index of bit for transmission
static uint16_t buffer_bits;									// holds number of bits inside BUFFER
static uint8_t buffer[150];										// contains bits for TC0_HANDLER to walk through

//static uint16_t sine_1200hz[16] = {2048, 2479, 2844, 3088, 3174, 3088, 2844, 2479, 2048, 1617, 1252, 1008, 922, 1008, 1252, 1617};
//static uint16_t sine_2200hz[16] = {2048, 2831, 3495, 3939, 4095, 3939, 3495, 2831, 2048, 1264, 600, 156, 0, 156, 600, 1264};
static uint16_t sine_2200hz[32] = {2048, 2447, 2831, 3185, 3495, 3750, 3939, 4056, 4095, 4056, 3939, 3750, 3495, 3185, 2831, 2447,
								   2048, 1648, 1264, 910, 600, 345, 156, 39, 0, 39, 156, 345, 600, 910, 1264, 1648};
static uint16_t sine_1200hz[32] = {2048, 2268, 2479, 2674, 2844, 2984, 3088, 3152, 3174, 3152, 3088, 2984, 2844, 2674, 2479, 2268,
								   2048, 1828, 1617, 1422, 1252, 1112, 1008, 944, 922, 944, 1008, 1112, 1252, 1422, 1617, 1828};


/*
	Set up as an 16-bit timer in compare operation.
	
	PRESCALER
		0x0		DIV1		GCLK_TC
		0x1		DIV2		GCLK_TC/2
		0x2		DIV4		GCLK_TC/4
		0x3		DIV8		GCLK_TC/8
		0x4		DIV16		GCLK_TC/16
		0x5		DIV64		GCLK_TC/64
		0x6		DIV256		GCLK_TC/256
		0x7		DIV1024		GCLK_TC/1024
	
	COMPARE0
		compare/capture value in 16-bit TC mode - channel 0
	
	INTERRUPT
		0	TC0's Match or Capture Channel x Interrupt is disabled
		1	TC0's Match or Capture Channel x Interrupt is enabled
*/
void TC0_enable(uint8_t prescaler, uint16_t compare0, uint8_t interrupt)
{
	GCLK->PCHCTRL[27].bit.GEN = 0;								// source clock for TC0 and TC1 is MCLK
	GCLK->PCHCTRL[27].bit.CHEN = 1;								// enable clock
	while(!(GCLK->PCHCTRL[27].bit.CHEN));						// wait for synchronization
	
	TC0->COUNT16.CTRLA.bit.MODE = 0;							// counter in 16-bit mode
	TC0->COUNT16.CTRLA.bit.CAPTEN0 = 0;							// selects whether channel x is capture (1) or compare (0) channel
	TC0->COUNT16.CTRLA.bit.PRESCALER = prescaler;				// These bits select the counter prescaler factor.
	
	if(prescaler) TC0->COUNT16.CTRLA.bit.PRESCSYNC = 1;			// Reload or reset the counter on next prescaler clock.
	else TC0->COUNT16.CTRLA.bit.PRESCSYNC = 0;					// Reload or reset the counter on next generic clock (GCLK_TCx).
	
	TC0->COUNT16.CTRLBCLR.bit.DIR = 1;							// Writing '1' to this bit will clear the bit and make the counter count up.
	while(TC0->COUNT16.SYNCBUSY.bit.CTRLB);						// bit is cleared when synchronization of CTRLB between clock domains is complete
	
	TC0->COUNT16.CC[0].reg = compare0;							// These bits contain the compare/capture value in 16-bit TC mode.
	while(TC0->COUNT16.SYNCBUSY.bit.CC0);						// bit is set when synchronization of CCx between clock domains is started
	
	if(interrupt) TC0->COUNT16.INTENSET.bit.MC0 = 1;			// sets corresponding Match or Capture Channel x Interrupt Enable bit
	NVIC_EnableIRQ(TC0_IRQn);									// interrupts must be globally enabled for interrupt requests to be generated
	
	TC0->COUNT16.CTRLA.bit.ENABLE = 1;							// enable TC0
	while(TC0->COUNT16.SYNCBUSY.bit.ENABLE);					// SYNCBUSY.ENABLE will be cleared when the operation is complete.
}


/*
	
*/
void TC0_disable(void)
{
	TC0->COUNT16.CTRLA.bit.ENABLE = 0;							// disable TC0
	while(TC0->COUNT16.SYNCBUSY.bit.ENABLE);					// SYNCBUSY.ENABLE will be cleared when the operation is complete.
	
	TC0->COUNT16.INTENCLR.bit.MC0 = 1;							// clears corresponding Match or Capture Channel x Interrupt Enable bit
	
	GCLK->PCHCTRL[27].bit.CHEN = 0;								// disable TC0 & TC1 clock
	while((GCLK->PCHCTRL[27].bit.CHEN));						// wait for synchronization
}


/*
	The TC should be disabled before the TC is reset in order to avoid undefined behavior.
*/
void TC0_reset(void)
{
	TC0->COUNT16.CTRLA.bit.SWRST = 1;							// resets all registers in TC, except DBGCTRL, and TC will be disabled
	while(TC0->COUNT16.SYNCBUSY.bit.SWRST);						// SYNCBUSY.SWRST will be cleared when the operation is complete.
}


/*
	It is possible to change the counter value (by writing directly in the COUNT register) even when the
	counter is running.
*/
void TC0_count_value(uint16_t count)
{
	TC0->COUNT16.COUNT.reg = count;								// These bits contain the current counter value.
	while(TC0->COUNT16.SYNCBUSY.bit.COUNT);						// bit is cleared when synchronization of COUNT between clock domains is complete
}


/*
	When using the TC and the Compare/Capture Value registers (CCx) for compare operations, the counter
	value is continuously compared to the values in the CCx registers.
*/
void TC0_compare_value_ch0(uint16_t compare)
{
	TC0->COUNT16.CC[0].reg = compare;							// These bits contain the compare/capture value in 16-bit TC mode.
	while(TC0->COUNT16.SYNCBUSY.bit.CC0);						// bit is set when synchronization of CCx between clock domains is started
}


/*
	When using the TC and the Compare/Capture Value registers (CCx) for compare operations, the counter
	value is continuously compared to the values in the CCx registers.
*/
void TC0_compare_value_ch1(uint16_t compare)
{
	TC0->COUNT16.CC[1].reg = compare;							// These bits contain the compare/capture value in 16-bit TC mode.
	while(TC0->COUNT16.SYNCBUSY.bit.CC1);						// bit is set when synchronization of CCx between clock domains is started
}


/*
	BIT
		0	adds '0' in queue
		1	adds '1' in queue
	
	RESET
		0	adds BIT in queue, increments BUFFER_BITS
		1	adds BIT at the beginning of BUFFER, resets BUFFER_BITS to 1
*/
void TC0_buffer_add_bit(uint8_t bit, uint8_t reset)
{
	if(reset) buffer_bits = 1;
	else buffer_bits++;
	
	if(buffer_bits > (150 * 8))
	{
		buffer_bits--;
		return;
	}
	
	uint8_t _byte = (buffer_bits - 1) / 8;
	uint8_t _bit = (buffer_bits - 1) % 8;
	
	buffer[_byte] |= (bit << _bit);
}


/*
	Blocks code execution while TC0_HANDLER handles transmission of individual bits in BUFFER.
*/
void TC0_transmission(void)
{
	_n = 0;														// initialize sine wave table index
	bit_tx = 0;													// initialize to first bit in BUFFER
	count_bit = 0;												// initialize interrupt counter
	tone_switch = 1;											// transmissions always start with 1200Hz tone
	
	transmitting = 1;											// set flag
	
	while(transmitting);
}


/*
	TC0 Match or Capture Channel x Interrupt handler - 19,200Hz.
*/
void TC0_Handler(void)
{
	TC0->COUNT16.INTFLAG.reg = TC_INTFLAG_MC0;					// clears corresponding Match or Capture Channel x interrupt flag
	
	uint16_t _current = TC0->COUNT16.CC[0].reg;
	TC0->COUNT16.CC[0].reg = _current + 660;					// These bits contain the compare/capture value in 16-bit TC mode.
	
	count_bit++;
	
	if(count_bit >= 32)											// next bit - 1200bps (19,200 / 16 = 1200)
	{
		bit_tx++;
		
		if(bit_tx < buffer_bits)
		{
			uint8_t _current_byte = bit_tx / 8;
			uint8_t _curent_bit = bit_tx % 8;
			
			if(!(buffer[_current_byte] & (1 << _curent_bit)))	// '0' change tone
			{
				if(tone_switch)									// change to 2200Hz
				{
					tone_switch = 0;
					
					TC4->COUNT16.COUNT.reg = 0;					// These bits contain the current counter value.
					TC4->COUNT16.CC[0].reg = 0;					// These bits contain the compare/capture value in 16-bit TC mode.
					TC4->COUNT16.INTENSET.bit.MC0 = 1;			// sets corresponding Match or Capture Channel x Interrupt Enable bit
				}
				else											// change to 1200Hz
				{
					tone_switch = 1;
					
					TC4->COUNT16.INTENCLR.bit.MC0 = 1;			// clears corresponding Match or Capture Channel x Interrupt Enable bit
					TC4->COUNT16.INTFLAG.reg = TC_INTFLAG_MC0;	// clears corresponding Match or Capture Channel x interrupt flag
				}
			}
			// else '1' keep tone						
		}
		else
		{
			transmitting = 0;									// clear flag - end transmission
		}
		
		count_bit = 0;
	}
	
	if(tone_switch)
	{	
		if(_n >= 32) _n = 0;
		
		DAC->DATA[1].reg = sine_1200hz[_n++];					// contains 12-bit value that is converted to voltage by DAC1
	}
}


/*
	Set up as an 16-bit timer in compare operation.
	
	PRESCALER
		0x0		DIV1		GCLK_TC
		0x1		DIV2		GCLK_TC/2
		0x2		DIV4		GCLK_TC/4
		0x3		DIV8		GCLK_TC/8
		0x4		DIV16		GCLK_TC/16
		0x5		DIV64		GCLK_TC/64
		0x6		DIV256		GCLK_TC/256
		0x7		DIV1024		GCLK_TC/1024
	
	COMPARE0
		compare/capture value in 16-bit TC mode - channel 0
	
	INTERRUPT
		0	TC4's Match or Capture Channel x Interrupt is disabled
		1	TC4's Match or Capture Channel x Interrupt is enabled
*/
void TC4_enable(uint8_t prescaler, uint16_t compare0, uint8_t interrupt)
{
	GCLK->PCHCTRL[29].bit.GEN = 0;								// source clock for TC4 is MCLK
	GCLK->PCHCTRL[29].bit.CHEN = 1;								// enable clock
	while(!(GCLK->PCHCTRL[29].bit.CHEN));						// wait for synchronization
	
	TC4->COUNT16.CTRLA.bit.MODE = 0;							// counter in 16-bit mode
	TC4->COUNT16.CTRLA.bit.CAPTEN0 = 0;							// selects whether channel x is capture (1) or compare (0) channel
	TC4->COUNT16.CTRLA.bit.PRESCALER = prescaler;				// These bits select the counter prescaler factor.
	
	if(prescaler) TC4->COUNT16.CTRLA.bit.PRESCSYNC = 1;			// Reload or reset the counter on next prescaler clock.
	else TC4->COUNT16.CTRLA.bit.PRESCSYNC = 0;					// Reload or reset the counter on next generic clock (GCLK_TCx).
	
	TC4->COUNT16.CTRLBCLR.bit.DIR = 1;							// Writing '1' to this bit will clear the bit and make the counter count up.
	while(TC4->COUNT16.SYNCBUSY.bit.CTRLB);						// bit is cleared when synchronization of CTRLB between clock domains is complete
	
	TC4->COUNT16.CC[0].reg = compare0;							// These bits contain the compare/capture value in 16-bit TC mode.
	while(TC4->COUNT16.SYNCBUSY.bit.CC0);						// bit is set when synchronization of CCx between clock domains is started
	
	if(interrupt) TC4->COUNT16.INTENSET.bit.MC0 = 1;			// sets corresponding Match or Capture Channel x Interrupt Enable bit
	NVIC_EnableIRQ(TC4_IRQn);									// interrupts must be globally enabled for interrupt requests to be generated
	
	TC4->COUNT16.CTRLA.bit.ENABLE = 1;							// enable TC4
	while(TC4->COUNT16.SYNCBUSY.bit.ENABLE);					// SYNCBUSY.ENABLE will be cleared when the operation is complete.
}


/*
	
*/
void TC4_disable(void)
{
	TC4->COUNT16.CTRLA.bit.ENABLE = 0;							// disable TC4
	while(TC4->COUNT16.SYNCBUSY.bit.ENABLE);					// SYNCBUSY.ENABLE will be cleared when the operation is complete.
	
	TC4->COUNT16.INTENCLR.bit.MC0 = 1;							// clears corresponding Match or Capture Channel x Interrupt Enable bit
	
	GCLK->PCHCTRL[29].bit.CHEN = 0;								// disable TC4 clock
	while((GCLK->PCHCTRL[29].bit.CHEN));						// wait for synchronization
}


/*
	The TC should be disabled before the TC is reset in order to avoid undefined behavior.
*/
void TC4_reset(void)
{
	TC4->COUNT16.CTRLA.bit.SWRST = 1;							// resets all registers in TC, except DBGCTRL, and TC will be disabled
	while(TC4->COUNT16.SYNCBUSY.bit.SWRST);						// SYNCBUSY.SWRST will be cleared when the operation is complete.
}


/*
	It is possible to change the counter value (by writing directly in the COUNT register) even when the
	counter is running.
*/
void TC4_count_value(uint16_t count)
{
	TC4->COUNT16.COUNT.reg = count;								// These bits contain the current counter value.
	while(TC4->COUNT16.SYNCBUSY.bit.COUNT);						// bit is cleared when synchronization of COUNT between clock domains is complete
}


/*
	When using the TC and the Compare/Capture Value registers (CCx) for compare operations, the counter
	value is continuously compared to the values in the CCx registers.
*/
void TC4_compare_value_ch0(uint16_t compare)
{
	TC4->COUNT16.CC[0].reg = compare;							// These bits contain the compare/capture value in 16-bit TC mode.
	while(TC4->COUNT16.SYNCBUSY.bit.CC0);						// bit is set when synchronization of CCx between clock domains is started
}


/*
	When using the TC and the Compare/Capture Value registers (CCx) for compare operations, the counter
	value is continuously compared to the values in the CCx registers.
*/
void TC4_compare_value_ch1(uint16_t compare)
{
	TC4->COUNT16.CC[1].reg = compare;							// These bits contain the compare/capture value in 16-bit TC mode.
	while(TC4->COUNT16.SYNCBUSY.bit.CC1);						// bit is set when synchronization of CCx between clock domains is started
}


/*
	TC4 Match or Capture Channel x Interrupt handler - 35,200Hz.
*/
void TC4_Handler(void)
{
	TC4->COUNT16.INTFLAG.reg = TC_INTFLAG_MC0;					// clears corresponding Match or Capture Channel x interrupt flag
	
	uint16_t _current = TC4->COUNT16.CC[0].reg;
	TC4->COUNT16.CC[0].reg = _current + 360;					// These bits contain the compare/capture value in 16-bit TC mode.
	
	if(_n >= 32) _n = 0;
		
	DAC->DATA[1].reg = sine_2200hz[_n++];						// contains 12-bit value that is converted to voltage by DAC1
}