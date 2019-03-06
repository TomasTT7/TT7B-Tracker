/*
	The Analog-to-Digital Converter (ADC) has up to 12-bit resolution, and is capable of a sampling rate of up to 1MSPS.
	The ADC can be configured for 8-, 10- or 12-bit results. Averaging and oversampling with decimation to support up to 16-bit result.
	
	An integrated temperature sensor is available for use with the ADC.
	
	TT7B TRACKER
		ADC[7]		PA07		Peripheral function B
		ADC[18]		PA10		Peripheral function B
		ADC[19]		PA11		Peripheral function B
*/


#ifndef L21_ADC_H
#define L21_ADC_H


#include "stdint.h"


#define VDDANA_ADC 1800											// [mV] ADC's VDDANA reference voltage


// Functions
void ADC_enable(uint8_t prescaler, uint8_t refsel, uint8_t ressel, uint8_t samplenum, uint8_t adjres, uint8_t samplen);
void ADC_disable(void);
void ADC_reset(void);
uint16_t ADC_sample_channel_x(uint8_t input);
uint32_t ADC_battery_voltage(uint16_t adc_result, uint8_t bit);
float ADC_temperature_thermistor(uint16_t adc_result);
float ADC_temperature_mcu(uint16_t adc_result);


#endif // L21_ADC_H_