/*
	The Si5351 is an I2C configurable clock generator with an internal VCXO. Based on
	a PLL/VCXO + high resolution MultiSynth fractional divider architecture, the Si5351
	can generate any frequency up to 200 MHz on each of its outputs with 0 ppm error.
	
	A 3.3 boost converter on PA04 has to be enabled first to supply power to the sensors.
	An output on PA06 has to be set HIGH as well to power a 1.8V to 3.3V logic translator.
	
	I2C INTERFACE
		frequency	100kHz (400kHz)
		address		0x60 (7-bit), 0xC0 (write), 0xC1 (read)
*/


#ifndef L21_SI5351B_H
#define L21_SI5351B_H


#include "stdint.h"


#define SI5351B_ADDRESS		0x60								// I2C slave address
#define TCXO_FREQ			26000000							// [Hz] frequency of TCXO reference
#define FREQ_OFFSET			3900								// [Hz] fixed offset to center AFSK modulation on desired frequency
#define FREQ_OFFSET_WSPR	142									// [Hz] fixed offset to center WSPR modulation on desired frequency


// Functions
void SI5351B_write_reg(uint8_t reg, uint8_t data);
uint8_t SI5351B_read_reg(uint8_t reg);

void SI5351B_init(void);
void SI5351B_deinit(void);
void SI5351B_frequency(uint32_t freq_Hz, uint8_t APR);
void SI5351B_frequency_WSPR(uint32_t freq_Hz, uint8_t APR);
void SI5351B_enable_output(void);
void SI5351B_disable_output(void);
void SI5351B_drive_strength(uint8_t drvStr);


#endif // L21_SI5351B_H_