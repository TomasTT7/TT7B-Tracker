/*
	The Analog-to-Digital Converter (ADC) has up to 12-bit resolution, and is capable of a sampling rate of up to 1MSPS.
	The ADC can be configured for 8-, 10- or 12-bit results. Averaging and oversampling with decimation to support up to 16-bit result.
	
	An integrated temperature sensor is available for use with the ADC.
	
	TT7B TRACKER
		ADC[7]		PA07		Peripheral function B
		ADC[18]		PA10		Peripheral function B
		ADC[19]		PA11		Peripheral function B
*/


#include "sam.h"
#include "L21_ADC.h"
#include "math.h"


/*
	TT7B TRACKER
		ADC[7]		PA07	Battery Voltage				Peripheral function B		0.5 ratio
		ADC[18]		PA10	Thermistor 1 (on-board)		Peripheral function B		Steinhart-Hart equation
		ADC[19]		PA11	Thermistor 2 (external)		Peripheral function B		Steinhart-Hart equation
		
	PRESCALER
		0x0		DIV2		Peripheral clock divided by 2
		0x1		DIV4		Peripheral clock divided by 4
		0x2		DIV8		Peripheral clock divided by 8
		0x3		DIV16		Peripheral clock divided by 16
		0x4		DIV32		Peripheral clock divided by 32
		0x5		DIV64		Peripheral clock divided by 64
		0x6		DIV128		Peripheral clock divided by 128
		0x7		DIV256		Peripheral clock divided by 256
	
	REFSEL
		0x0		INTREF		internal variable reference voltage (configurable in SUPC.VREF - 1.024V, 2.048V, 4.096V)
		0x1		INTVCC0		1/1.6 VDDANA
		0x2		INTVCC1		1/2 VDDANA (only for VDDANA > 2.0V)
		0x3		VREFA		External reference
		0x4		VREFB		External reference
		0x5		INTVCC2		VDDANA
	
	RESSEL
		0x0		12BIT		12-bit result
		0x1		16BIT		For averaging mode output
		0x2		10BIT		10-bit result
		0x3		8BIT		8-bit result
	
	AVERAGING
	Samples		SAMPLENUM	Interm. Precision	Auto Right Shfts	Div Factor		ADJRES		Total Right Shfts		Fin Precis		Auto Div Factor
	1			0x0			12-bit				0					1				0x0									12-bit			0
	2			0x1			13-bit				0					2				0x1			1						12-bit			0
	4			0x2			14-bit				0					4				0x2			2						12-bit			0
	8			0x3			15-bit				0					8				0x3			3						12-bit			0
	16			0x4			16-bit				0					16				0x4			4						12-bit			0
	32			0x5			17-bit				1					16				0x4			5						12-bit			2
	64			0x6			18-bit				2					16				0x4			6						12-bit			4
	128			0x7			19-bit				3					16				0x4			7						12-bit			8
	256			0x8			20-bit				4					16				0x4			8						12-bit			16
	512			0x9			21-bit				5					16				0x4			9						12-bit			32
	1024		0xA			22-bit				6					16				0x4			10						12-bit			64
	
	To perform averaging of two or more samples, Conversion Result Resolution field in Control C register (CTRLC.RESSEL) must be set.
	
	OVERSAMPLING & DECIMATION
	Resolution		Samples		SAMPLENUM	Auto Right Shfts	ADJRES
	13-bits			4^1=4		0x2			0					0x1
	14-bits			4^2=16		0x4			0					0x2
	15-bits			4^3=64		0x6			2					0x1
	16-bits			4^4=256		0x8			4					0x0
	
	SAMPLEN
		0-63	Sampling Time = (SAMPLEN + 1) * CLK_ADC
*/
void ADC_enable(uint8_t prescaler, uint8_t refsel, uint8_t ressel, uint8_t samplenum, uint8_t adjres, uint8_t samplen)
{
	uint32_t * pNVMcalib = (uint32_t *) 0x00806020;				// factory calibration data
	uint32_t biascomp = ((*pNVMcalib & 0x00000038) >> 3);
	uint32_t biasrefbuf = *pNVMcalib & 0x00000007;
	
	GCLK->PCHCTRL[30].bit.GEN = 0;								// source clock for ADC is MCLK
	GCLK->PCHCTRL[30].bit.CHEN = 1;								// enable clock
	while(!(GCLK->PCHCTRL[30].bit.CHEN));						// wait for synchronization
	
	PORT->Group[0].PMUX[3].bit.PMUXO = 0x1;						// PA7 - peripheral function B selected
	PORT->Group[0].PMUX[5].bit.PMUXE = 0x1;						// PA10
	PORT->Group[0].PMUX[5].bit.PMUXO = 0x1;						// PA11
	
	PORT->Group[0].PINCFG[7].bit.PMUXEN = 1;					// PA7 - selected peripheral function controls direction and output drive value.
	PORT->Group[0].PINCFG[10].bit.PMUXEN = 1;					// PA10
	PORT->Group[0].PINCFG[11].bit.PMUXEN = 1;					// PA11
	
	ADC->CTRLA.bit.ENABLE = 0;									// ADC must be disabled for configuration
	while(ADC->SYNCBUSY.bit.ENABLE);							// SYNCBUSY.ENABLE will be cleared when operation is complete.
	
	if(prescaler > 7) ADC->CTRLB.reg = 0;						// This field defines the ADC clock relative to the peripheral clock.
	else ADC->CTRLB.bit.PRESCALER = prescaler;
	
	if(refsel > 5) ADC->REFCTRL.bit.REFSEL = 5;					// Reference Selection
	else ADC->REFCTRL.bit.REFSEL = refsel;
	
	ADC->CALIB.reg = (biasrefbuf << 8) | biascomp;				// Bias Reference Buffer Scaling and Bias Comparator Scaling calibration
	
	ADC->AVGCTRL.reg = (adjres << 4) | samplenum;				// Result Averaging configuration
	while(ADC->SYNCBUSY.bit.AVGCTRL);							// Bit is cleared when synchronization of AVGCTRL reg between clock domains is complete.
	
	if(samplenum > 0) ADC->CTRLC.bit.RESSEL = 1;				// 16-bit resolution for averaging mode output
	else ADC->CTRLC.bit.RESSEL = ressel;						// ADC resolution
	while(ADC->SYNCBUSY.bit.CTRLC);								// Bit is cleared when synchronization of CTRLC reg between clock domains is complete.
	
	ADC->SAMPCTRL.bit.SAMPLEN = samplen;						// 0-63, Sampling Time = (SAMPLEN + 1) * CLK_ADC
	while(ADC->SYNCBUSY.bit.SAMPCTRL);							// Bit is cleared when synchronization of SAMPCTRL reg between clock domains is complete.
	
	ADC->CTRLA.bit.ENABLE = 1;									// enable ADC
	while(ADC->SYNCBUSY.bit.ENABLE);							// SYNCBUSY.ENABLE will be cleared when the	operation is complete.
}


/*
	Disable ADC peripheral, its clock, and restore original pin configuration.
*/
void ADC_disable(void)
{
	ADC->CTRLA.bit.ENABLE = 0;									// disable ADC
	while(ADC->SYNCBUSY.bit.ENABLE);							// SYNCBUSY.ENABLE will be cleared when operation is complete.
	
	GCLK->PCHCTRL[30].bit.CHEN = 0;								// disable ADC clock
	while(GCLK->PCHCTRL[30].bit.CHEN);							// wait for synchronization
	
	PORT->Group[0].PINCFG[7].bit.PMUXEN = 0;					// PA7 - PORT registers control the direction and output drive value.
	PORT->Group[0].PINCFG[10].bit.PMUXEN = 0;					// PA10
	PORT->Group[0].PINCFG[11].bit.PMUXEN = 0;					// PA11
}


/*
	All registers in the ADC, except DBGCTRL, will be reset to their initial state, and the ADC will be disabled.
*/
void ADC_reset(void)
{
	ADC->CTRLA.bit.SWRST = 1;									// reset ADC
	while(ADC->SYNCBUSY.bit.SWRST);								// SYNCBUSY.SWRST will be cleared when reset is complete.
}


/*
	TT7B TRACKER
		AIN[7]		0x07		ADC[7]		PA07	Battery Voltage				Peripheral function B		0.5 ratio
		AIN[18]		0x12		ADC[18]		PA10	Thermistor 1 (on-board)		Peripheral function B		Steinhart-Hart equation
		AIN[19]		0x13		ADC[19]		PA11	Thermistor 2 (external)		Peripheral function B		Steinhart-Hart equation
		TEMP		0x18		Temperature Sensor
	
	For the integrated temperature sensor measurement, the sensor must be first enabled in SUPC.
	And the required value of the voltage reference must be selected in SUPC as well.
	The Sampling Time Length bit group in the Sampling Control register must be written with a corresponding value as well.
	
	MUXNEG
		0x00	AIN0	ADC AIN0 pin
		0x01	AIN1	ADC AIN1 pin
		0x02	AIN2	ADC AIN2 pin
		0x03	AIN3	ADC AIN3 pin
		0x04	AIN4	ADC AIN4 pin
		0x05	AIN5	ADC AIN5 pin
		0x18	GND		Internal ground
	
	MUXPOS
		0x00	AIN0			ADC AIN0 pin
		0x01	AIN1			ADC AIN1 pin
		0x02	AIN2			ADC AIN2 pin
		0x03	AIN3			ADC AIN3 pin
		0x04	AIN4			ADC AIN4 pin
		0x05	AIN5			ADC AIN5 pin
		0x06	AIN6			ADC AIN6 pin
		0x07	AIN7			ADC AIN7 pin
		0x08	AIN8			ADC AIN8 pin
		0x09	AIN9			ADC AIN9 pin
		0x0A	AIN10			ADC AIN10 pin
		0x0B	AIN11			ADC AIN11 pin
		0x0C	AIN12			ADC AIN12 pin
		0x0D	AIN13			ADC AIN13 pin
		0x0E	AIN14			ADC AIN14 pin
		0x0F	AIN15			ADC AIN15 pin
		0x10	AIN16			ADC AIN16 pin
		0x11	AIN17			ADC AIN17 pin
		0x12	AIN18			ADC AIN18 pin
		0x13	AIN19			ADC AIN19 pin
		0x18	TEMP			Temperature Sensor
		0x19	BANDGAP			Bandgap Voltage
		0x1A	SCALEDCOREVCC	1/4 Scaled Core Supply
		0x1B	SCALEDIOVCC		1/4 Scaled I/O Supply
*/
uint16_t ADC_sample_channel_x(uint8_t input)
{
	uint16_t data = 0;
	
	ADC->INPUTCTRL.reg = (0x18 << 8) | input;
	while(ADC->SYNCBUSY.bit.INPUTCTRL);							// Bit is cleared when synchronization of INPUTCTRL reg between clock domains is complete.
	
	while(ADC->SYNCBUSY.bit.SWTRIG);							// Bit is cleared when synchronization of SWTRIG register between domains is complete.
	ADC->SWTRIG.bit.START = 1;									// Writing a '1' to this bit will start a conversion or sequence.
	while(!(ADC->INTFLAG.bit.RESRDY));							// Flag is set when conversion result is available, it is cleared by reading RESULT register.
	
	data = ADC->RESULT.reg;										// Result Conversion Value
	
	return data;
}


/*
	Input						raw ADC result
	Output						mV
	Minimum ADC Resolution		1.8V / 4095 = 0.439mV
	
	BIT
		8		8-bit input
		10		10-bit input
		12		12-bit input
		13		13-bit input (oversampled)
		14		14-bit input (oversampled)
		15		15-bit input (oversampled)
		16		16-bit input (oversampled)
		
	Voltage divider at the input. Two 1M resistors - 0.5 ratio.
*/
uint32_t ADC_battery_voltage(uint16_t adc_result, uint8_t bit)
{
	uint32_t result, divider;
	
	switch(bit)
	{
		case 8:
			divider = 255;
			break;
		case 10:
			divider = 1023;
			break;
		case 12:
			divider = 4095;
			break;
		case 13:
			divider = 8191;
			break;
		case 14:
			divider = 16383;
			break;
		case 15:
			divider = 32767;
			break;
		case 16:
			divider = 65535;
			break;
		default:
			return 0xFFFF;
	}
	
	result = (uint32_t)adc_result * VDDANA_ADC / divider * 2;
	
	return result;
}


/*
	VOLTAGE DIVIDER
		R1		49.9k
		R2		Thermistor min: 92.8R (150°C), max: 3,683M (-80°C)
	
	STEINHART-HART COEFFICIENTS
		a		1.284E-03
		b		2.363E-04
		c		9.275E-08
	
	V_m = ADC_m / 4095 * VDDANA						ADC equation
	R2 = (V_m * R1) / (V_in - V_m)					Voltage Divider equation
	T =	1 / (a + b * ln(R2) + c * (ln(R2))^3)		Steinhart-Hart equation
*/
float ADC_temperature_thermistor(uint16_t adc_result)
{
	float V_in = (float)VDDANA_ADC / 1000.0;
	float a = 0.00128424;
	float b = 0.00023629;
	float c = 0.0000000928;
	
	float V_m = (float)adc_result / 4095.0 * V_in;
	float R2 = (V_m * 49900.0) / (V_in - V_m);
	float T = 1.0 / (a + b * log(R2) + c * powf(log(R2), 3)) - 273.15;
	
	return T;
}


/*
	Temperature sensor slope		2.24mV/°C
	Variation over VDDANA voltage	1.1mV/V
	
	DATASHEET CONFIGURATION FOR TEMPERATURE MEASUREMENTS
		Supply voltages					VDDIN = VDDIO = VDDANA = 3.3V
		ADC Clock conversion rate		100kSa/s
		Offset compensation				ADC.SAMPCTRL.OFFCOMP=0
		F_EXT							3.2MHz
		F_ADC							F_EXT / 2 = 1.6MHz
		Sampling time					(63 + 1) / F_ADC = 40µs
		ADC voltage reference			1.0V internal reference (INTREF, SUPC.VREF.SEL=1V0)
		ADC input						Temperature sensor (ADC.INPUTCTRL.MUXPOS=TEMP)
		
	NVM Temperature Log Row at 0x00806030 contains calibration data determined and written during production test.
	
		[7:0]		Integer part of room temperature in °C
		[11:8]		Decimal part of room temperature
		[19:12]		Integer part of hot temperature in °C
		[23:20]		Decimal part of hot temperature
		[31:24]		2’s complement of internal 1V reference drift at room temp (versus 1.0 centered value)
		[39:32]		2’s complement of internal 1V reference drift at hot temp (versus 1.0 centered value)
		[51:40]		Temperature sensor 12bit ADC conversion at room temperature
		[63:52]		Temperature sensor 12bit ADC conversion at hot temperature
*/
float ADC_temperature_mcu(uint16_t adc_result)
{
	uint64_t * pNVMtemp = (uint64_t *) 0x00806030;
	uint64_t NVMlog = *pNVMtemp;
	
	float temp_r = (float)((NVMlog & 0xFF) * 10 + ((NVMlog >> 8) & 0x0F)) / 10.0;
	float temp_h = (float)(((NVMlog >> 12) & 0xFF) * 10 + ((NVMlog >> 20) & 0x0F)) / 10.0;
	float int1v_r = 1.0 - ((float)((int8_t)(NVMlog >> 24)) * 0.001);
	float int1v_h = 1.0 - ((float)((int8_t)(NVMlog >> 32)) * 0.001);
	float adc_r = (float)((NVMlog >> 40) & 0xFFF);
	float adc_h = (float)((NVMlog >> 52) & 0xFFF);
	float adc_m = (float)adc_result;
	
	float temp_c = temp_r + ((adc_m - adc_r * int1v_r) * (temp_h - temp_r)) / (adc_h * int1v_h - adc_r * int1v_r);
	float int1v_m = int1v_r + ((int1v_h - int1v_r) * (temp_c - temp_r)) / (temp_h - temp_r);
	float temp_f = temp_r + ((adc_m * int1v_m - adc_r * int1v_r) * (temp_h - temp_r)) / (adc_h * int1v_h - adc_r * int1v_r);
	
	return temp_f;
}