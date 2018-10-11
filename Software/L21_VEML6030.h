/*
	VEML6030 is a high accuracy ambient light digital 16-bit resolution sensor.
	A 3.3 boost converter on PA04 has to be enabled first to supply power to the sensors.
	An output on PA06 has to be set HIGH as well to power a 1.8V to 3.3V logic translator.
	
	I2C INTERFACE
		frequency	10-100kHz (400kHz)
		address		0x10 (7-bit), 0x20 (write), 0x21 (read)
*/


#ifndef L21_VEML6030_H
#define L21_VEML6030_H


#include "stdint.h"


#define VEML_ADDRESS 0x10


// Functions
void VEML6030_write_reg(uint8_t cmd, uint16_t data);
uint16_t VEML6030_read_reg(uint8_t cmd);

void VEML6030_enable(uint8_t gain, uint8_t integration);
void VEML6030_disable(void);
void VEML6030_set_power_saving_mode(uint8_t psm, uint8_t psm_en);
uint16_t VEML6030_get_measurement_result(void);
float VEML6030_calculate_lux(uint16_t raw, uint8_t gain, uint8_t integration);


#endif // L21_VEML6030_H_