/*
	MS5607-02BA03 is a high resolution barometric pressure sensor.
	A load switch on PA16 has to be enabled first to supply power to the sensors.
	
	TT7B TRACKER
		MS5607 on-board		chip select 1 (CS1)
		MS5607 external		chip select 2 (CS2)
		
	SPI INTERFACE
		F_max	20MHz
		mode	0/3
*/


#ifndef L21_MS5607_H
#define L21_MS5607_H


#include "stdint.h"


static uint16_t C1[8];											// array for MS5607_1 coefficients
static uint16_t C2[8];											// array for MS5607_2 coefficients


// Functions
void MS5607_cmd_Reset(uint8_t sensor, uint8_t delay_ms);
void MS5607_cmd_Convert(uint8_t sensor, uint8_t mode);
uint32_t MS5607_cmd_ADCread(uint8_t sensor);
void MS5607_cmd_PROMread(uint8_t sensor);
void MS5607_calculate_results(uint8_t sensor, uint32_t raw_pres, uint32_t raw_temp, float * pressure, float * temperature);


#endif // L21_MS5607_H_