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


#include "sam.h"
#include "L21_MS5607.h"
#include "L21_SERCOM_SPI.h"
#include "L21_SysTick.h"


static uint16_t C1[8];											// array for MS5607_1 coefficients
static uint16_t C2[8];											// array for MS5607_2 coefficients


/*
	COMMANDS
		Bit number				0		1		2		3		4		5		6		7
		Bit name				PRM		COV		-		Typ		Ad2/Os2	Ad1/Os1	Ad0/Os0	Stop
		Reset					0		0		0		1		1		1		1		0		0x1E
		Convert D1 (OSR=256)	0		1		0		0		0		0		0		0		0x40
		Convert D1 (OSR=512)	0		1		0		0		0		0		1		0		0x42
		Convert D1 (OSR=1024)	0		1		0		0		0		1		0		0		0x44
		Convert D1 (OSR=2048)	0		1		0		0		0		1		1		0		0x46
		Convert D1 (OSR=4096)	0		1		0		0		1		0		0		0		0x48
		Convert D2 (OSR=256)	0		1		0		1		0		0		0		0		0x50
		Convert D2 (OSR=512)	0		1		0		1		0		0		1		0		0x52
		Convert D2 (OSR=1024)	0		1		0		1		0		1		0		0		0x54
		Convert D2 (OSR=2048)	0		1		0		1		0		1		1		0		0x56
		Convert D2 (OSR=4096)	0		1		0		1		1		0		0		0		0x58
		ADC Read				0		0		0		0		0		0		0		0		0x00
		PROM Read				1		0		1		0		Ad2		Ad1		Ad0		0		0xA0 to 0xAE
		
	The best noise performance from the module is obtained when the SPI bus is idle and without communication to other devices during the ADC conversion.
*/


/*
	The Reset sequence shall be sent once after power-on to make sure that the calibration PROM gets loaded into the internal register.
	Datasheet shows a 2.8ms reload delay between the end of Reset command and CS de-selection.
	
	SENSOR
		1	MS5607 on-board
		2	MS5607 external
*/
void MS5607_cmd_Reset(uint8_t sensor, uint8_t delay_ms)
{
	switch(sensor)
	{
		case 1:
			PORT->Group[0].OUTCLR.reg = (1 << 14);				// chip select 1 LOW
			break;
		case 2:
			PORT->Group[0].OUTCLR.reg = (1 << 15);				// chip select 2 LOW
			break;
		default:
			return;
	}
	
	SERCOM_SPI_transmission(0x1E);								// RESET command
	
	if(delay_ms > 0) SysTick_delay_ms(delay_ms);
	
	switch(sensor)
	{
		case 1:
			PORT->Group[0].OUTSET.reg = (1 << 14);				// chip select 1 HIGH
			break;
		case 2:
			PORT->Group[0].OUTSET.reg = (1 << 15);				// chip select 2 HIGH
			break;
		default:
			return;
	}
}


/*
	The conversion command is used to initiate uncompensated pressure (D1) or uncompensated temperature (D2) conversion.
	The chip select can be disabled during this time to communicate with other devices.
	
	SENSOR
		1	MS5607 on-board
		2	MS5607 external
	
	MODE
		0x40	Convert D1 (OSR=256)		uncompensated pressure			0.60ms		0.130mbar
		0x42	Convert D1 (OSR=512)		uncompensated pressure			1.17ms		0.084mbar
		0x44	Convert D1 (OSR=1024)		uncompensated pressure			2.28ms		0.054mbar
		0x46	Convert D1 (OSR=2048)		uncompensated pressure			4.54ms		0.036mbar
		0x48	Convert D1 (OSR=4096)		uncompensated pressure			9.04ms		0.024mbar
		0x50	Convert D2 (OSR=256)		uncompensated temperature		0.60ms		0.012°C
		0x52	Convert D2 (OSR=512)		uncompensated temperature		1.17ms		0.008°C
		0x54	Convert D2 (OSR=1024)		uncompensated temperature		2.28ms		0.005°C
		0x56	Convert D2 (OSR=2048)		uncompensated temperature		4.54ms		0.003°C
		0x58	Convert D2 (OSR=4096)		uncompensated temperature		9.04ms		0.002°C
*/
void MS5607_cmd_Convert(uint8_t sensor, uint8_t mode)
{
	switch(sensor)
	{
		case 1:
			PORT->Group[0].OUTCLR.reg = (1 << 14);				// chip select 1 LOW
			break;
		case 2:
			PORT->Group[0].OUTCLR.reg = (1 << 15);				// chip select 2 LOW
			break;
		default:
			return;
	}
	
	SERCOM_SPI_transmission(mode);								// CONVERT command
	
	switch(sensor)
	{
		case 1:
			PORT->Group[0].OUTSET.reg = (1 << 14);				// chip select 1 HIGH
			break;
		case 2:
			PORT->Group[0].OUTSET.reg = (1 << 15);				// chip select 2 HIGH
			break;
		default:
			return;
	}
}


/*
	If the conversion is not executed before the ADC read command, or the ADC read command is repeated, it will give 0 as the output result.
	If the ADC read command is sent during conversion the result will be 0, the conversion will not stop and the final result will be wrong.
	Conversion sequence sent during the already started conversion process will yield incorrect result as well.
	
	After ADC read command the device will return a 24 bit result clocked out MSB first.
*/
uint32_t MS5607_cmd_ADCread(uint8_t sensor)
{
	uint32_t result = 0;
	
	switch(sensor)
	{
		case 1:
			PORT->Group[0].OUTCLR.reg = (1 << 14);				// chip select 1 LOW
			break;
		case 2:
			PORT->Group[0].OUTCLR.reg = (1 << 15);				// chip select 2 LOW
			break;
		default:
			return 0;
	}
	
	SERCOM_SPI_transmission(0x00);								// ADC READ command
	
	result = SERCOM_SPI_transmission(0x00);						// result [23:16]
	result = (result << 8);
	result |= SERCOM_SPI_transmission(0x00);					// result [15:8]
	result = (result << 8);
	result |= SERCOM_SPI_transmission(0x00);					// result [7:0]
	
	switch(sensor)
	{
		case 1:
			PORT->Group[0].OUTSET.reg = (1 << 14);				// chip select 1 HIGH
			break;
		case 2:
			PORT->Group[0].OUTSET.reg = (1 << 15);				// chip select 2 HIGH
			break;
		default:
			return 0;
	}
	
	return result;
}


/*
	The read command for PROM shall be executed once after reset by the user to read the content of the calibration
	PROM and to calculate the calibration coefficients. There are in total 8 addresses resulting in a total memory of 128 bit.
	
	C0					Factory data and setup
	C1		SENST1		Pressure sensitivity
	C2		OFFT1		Pressure offset
	C3		TCS			Temperature coefficient of pressure sensitivity
	C4		TCO			Temperature coefficient of pressure offset
	C5		T REF		Reference temperature
	C6		TEMPSENS	Temperature coefficient of the temperature
	C7					Serial code and CRC
	
	After PROM read command the device will return a 16 bit result clocked out MSB first.
*/
void MS5607_cmd_PROMread(uint8_t sensor)
{
	for(uint8_t i = 0; i < 8; i++)
	{
		uint16_t result = 0;
		
		switch(sensor)
		{
			case 1:
				PORT->Group[0].OUTCLR.reg = (1 << 14);			// chip select 1 LOW
				break;
			case 2:
				PORT->Group[0].OUTCLR.reg = (1 << 15);			// chip select 2 LOW
				break;
			default:
				return;
		}
		
		SERCOM_SPI_transmission(0xA0+2*i);						// PROM READ command (0xA0 to 0xAE)
		
		result = SERCOM_SPI_transmission(0x00);					// High byte [16:8]
		result = (result << 8);
		result |= SERCOM_SPI_transmission(0x00);				// Low byte [7:0]
		
		switch(sensor)
		{
			case 1:
				PORT->Group[0].OUTSET.reg = (1 << 14);			// chip select 1 HIGH
				break;
			case 2:
				PORT->Group[0].OUTSET.reg = (1 << 15);			// chip select 2 HIGH
				break;
			default:
				return;
		}
		
		if(sensor == 1) C1[i] = result;
		else if(sensor == 2) C2[i] = result;
	}
}


/*
	Stores actual pressure value in Pa where *pressure points, and actual temperature in °C where *temperature points. 
	
	SENSOR
		1	MS5607 on-board
		2	MS5607 external
		
	EQUATIONS
		dT = D2 - C5 * 2^8
		OFF = C2 * 2^17 + (C4 * dT) / 2^6
		SENS = C1 * 2^16 + (C3 * dT) / 2^7
		TEMP = 2000 + dT * C6 / 2^23
		P = (D1 * SENS / 2^21 - OFF) / 2^15
	
	in case TEMP < 20°C
		T2 = dT^2 / 2^31
		OFF2 = 61 * (TEMP - 2000)^2 / 2^4
		SENS2 = 2 * (TEMP - 2000)^2
		
	additional correction in case resulting TEMP < -15°C
		OFF2 = OFF2 + 15 * (TEMP + 1500)^2
		SENS2 = SENS2 + 8 * (TEMP + 1500)^2
	
	corrected values T2, SENS and OFF are then used to recalculate P and TEMP
		OFF = OFF - OFF2
		SENS = SENS - SENS2
		TEMP = TEMP - T2
		P = (D1 * SENS / 2^21 - OFF) / 2^15
*/
void MS5607_calculate_results(uint8_t sensor, uint32_t raw_pres, uint32_t raw_temp, float * pressure, float * temperature)
{
	uint16_t _C[8];
	
	for(uint8_t i = 0; i < 8; i++)
	{
		if(sensor == 1) _C[i] = C1[i];
		else if(sensor == 2) _C[i] = C2[i];
	}
	
	int64_t dT, OFF, SENS, TEMP, P;
	
	dT = (int64_t)raw_temp - (int64_t)_C[5] * 256;
	OFF = (int64_t)_C[2] * 131072 + ((int64_t)_C[4] * dT) / 64;
	SENS = (int64_t)_C[1] * 65536 + ((int64_t)_C[3] * dT) / 128;
	TEMP = 2000 + dT * (int64_t)_C[6] / 8388608;
	TEMP = TEMP + (int64_t)(TEMP_ERROR_VOLT * 100.0);			// add fixed offset due to supply voltage
	
	if(TEMP >= 2000)											// for cases where: temperature > 20.00°C
	{
		P = ((int64_t)raw_pres * SENS / 2097152 - OFF) / 32768;
		
		*pressure = (float)P + PRES_ERROR_VOLT;					// [Pa]
		*temperature = (float)TEMP / 100.0;						// [°C]
	}
	else														// for cases where: 20.00°C >= temperature > -15.00°C
	{
		int64_t T2, OFF2, SENS2;
		
		T2 = dT * dT / 2147483648;
		OFF2 = 61 * (TEMP - 2000) * (TEMP - 2000) / 16;
		SENS2 = 2 * (TEMP - 2000) * (TEMP - 2000);
		
		if(TEMP <= -1500)										// for cases where: -15.00°C >= temperature
		{
			OFF2 = OFF2 + 15 * (TEMP + 1500) * (TEMP + 1500);
			SENS2 = SENS2 + 8 * (TEMP + 1500) * (TEMP + 1500);
		}
		
		TEMP = TEMP - T2;
		OFF = OFF - OFF2;
		SENS = SENS - SENS2;
		P = ((int64_t)raw_pres * SENS / 2097152 - OFF) / 32768;
		
		*pressure = (float)P + PRES_ERROR_VOLT;					// [Pa]
		*temperature = (float)TEMP / 100.0;						// [°C]
	}
}