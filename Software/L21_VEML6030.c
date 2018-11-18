/*
	VEML6030 is a high accuracy ambient light digital 16-bit resolution sensor.
	A 3.3 boost converter on PA04 has to be enabled first to supply power to the sensors.
	An output on PA06 has to be set HIGH as well to power a 1.8V to 3.3V logic translator.
	
	I2C INTERFACE
		frequency	10-100kHz (400kHz)
		address		0x10 (7-bit), 0x20 (write), 0x21 (read)
*/


#include "sam.h"
#include "L21_SERCOM_I2C.h"
#include "L21_VEML6030.h"
#include "math.h"


/*
	COMMAND		REGISTER		BIT		R/W		DESCRIPTION
	00			ALS_CONF 0		15:0	W		ALS gain, integration time, interrupt, and shut down
	01			ALS_WH			15:8	W		ALS high threshold window setting (MSB)
								7:0		W		ALS high threshold window setting (LSB)
	02			ALS_WL			15:8	W		ALS low threshold window setting (MSB)
								7:0		W		ALS low threshold window setting (LSB)
	03			Power saving	15:0			Set (15:3) 0000 0000 0000 0b
	04			ALS				15:8	R		MSB 8 bits data of whole ALS 16 bits
								7:0		R		LSB 8 bits data of whole ALS 16 bits
	05			WHITE			15:8	R		MSB 8 bits data of whole WHITE 16 bits
								7:0		R		LSB 8 bits data of whole WHITE 16 bits
	06			ALS_INT			15:0	R		ALS INT trigger event
	
	ALS_GAIN	PSM		ALS_IT	REFRESH(ms)	IDD(μA)		RESOLUTION(lx/bit)
	01			00		0000	600			8			0.0288
	01			01		0000	1100		5			0.0288
	01			10		0000	2100		3			0.0288
	01			11		0000	4100		2			0.0288
	01			00		0001	700			13			0.0144
	01			01		0001	1200		8			0.0144
	01			10		0001	2200		5			0.0144
	01			11		0001	4200		3			0.0144
	01			00		0010	900			20			0.0072
	01			01		0010	1400		13			0.0072
	01			10		0010	2400		8			0.0072
	01			11		0010	4400		5			0.0072
	01			00		0011	1300		28			0.0036
	01			01		0011	1800		20			0.0036
	01			10		0011	2800		13			0.0036
	01			11		0011	4800		8			0.0036
	
	LUMINANCE			EXAMPLE
	10-5 lx				Light from Sirius, the brightest star in the night sky
	10-4 lx				Total starlight, overcast sky
	0.002 lx			Moonless clear night sky with airglow
	0.01 lx				Quarter moon, 0.27 lx; full moon on a clear night
	1 lx				Full moon overhead at tropical latitudes
	3.4 lx				Dark limit of civil twilight under a clear sky
	50 lx				Family living room
	80 lx				Hallway / bathroom
	100 lx				Very dark overcast day
	320 lx - 500 lx		Office lighting
	400 lx				Sunrise or sunset on a clear day
	1 klx				Overcast day; typical TV studio lighting
	10 klx - 25 klx		Full daylight (not direct sun)
	32 klx - 130 klx	Direct sunlight
*/


/*
	Writes 16-bits of DATA to a CMD (8-bit) register.
*/
void VEML6030_write_reg(uint8_t cmd, uint16_t data)
{
	uint8_t ack;
	
	ack = SERCOM_I2C_transmit_address(VEML_ADDRESS << 1);		// send transmit address
	
	if(ack)
	{
		SERCOM_I2C_transmit_byte(cmd);							// send command
		SERCOM_I2C_transmit_byte(data & 0xFF);					// send data low byte
		SERCOM_I2C_transmit_byte(data >> 8);					// send data high byte
		SERCOM_I2C_end_transmission();
	}
}


/*
	Reads 16-bits of data from a CMD (8-bit) register.
*/
uint16_t VEML6030_read_reg(uint8_t cmd)
{
	uint8_t ack;
	uint16_t data = 0;
	
	ack = SERCOM_I2C_transmit_address(VEML_ADDRESS << 1);		// send transmit address
	
	if(ack)
	{
		SERCOM_I2C_transmit_byte(cmd);							// send command
		
		ack = SERCOM_I2C_transmit_address((VEML_ADDRESS << 1) | 0x01);	// send receive address (repeated start)
		
		if(ack)
		{
			data = SERCOM_I2C_receive_byte(0);					// receive data low byte
			SERCOM_I2C_receive_byte(1);
			data |= (uint16_t)SERCOM_I2C_end_reception() << 8;	// receive data high byte
		}
	}
	
	return data;
}


/*
	When activating the sensor, setting bit 0 of the command register to “0”; a wait time of 4 ms
	should be observed before the first	measurement is picked up, to allow for a correct start
	of the signal processor and oscillator.
	
	ALS_CONF 0 REGISTER (0x00)
		Reserved	15:13	Set 000b
		ALS_GAIN	12:11	Gain selection
							00 = ALS gain x 1
							01 = ALS gain x 2
							10 = ALS gain x (1/8)
							11 = ALS gain x (1/4)
		reserved	10		Set 0b
		ALS_IT		9:6		ALS integration time setting
							1100 = 25 ms
							1000 = 50 ms
							0000 = 100 ms
							0001 = 200 ms
							0010 = 400 ms
							0011 = 800 ms
		ALS_PERS	5:4		ALS persistence protect number setting
							00 = 1
							01 = 2
							10 = 4
							11 = 8
		Reserved	3:2		Set 00b
		ALS_INT_EN	1		ALS interrupt enable setting
							0 = ALS INT disable
							1 = ALS INT enable
		ALS_SD		0		ALS shut down setting
							0 = ALS power on
							1 = ALS shut down
	
	To avoid possible saturation/overflow effects, application software should always start
	with low gain: ALS gain x 1/8 or gain 1/4.
	
	The standard integration time is 100 ms. If a very high resolution is needed, one may increase
	this integration time up to 800 ms. If faster measurement results are needed, it can be
	decreased down to 25 ms.
	
	Start with GAIN 1/8 and INTEGRATION TIME 25ms -> resolution 1.8432 lux, maximum value 120796 lux.
	If result < 25 (46 lux), measure again with GAIN 2 and INTEGRATION TIME 100ms -> 0.0288 lux, 1887 lux.
*/
void VEML6030_enable(uint8_t gain, uint8_t integration)
{
	uint16_t reg_val = ((gain & 0b11) << 11) | ((integration & 0b1111) << 6);
	
	VEML6030_write_reg(0x00, reg_val);							// ALS_CONF 0 command
}


/*
	
*/
void VEML6030_disable(void)
{
	VEML6030_write_reg(0x00, 0x01);								// ALS_CONF 0 command: ALS shut down
}


/*
	POWER SAVING REGISTER (0x03)
		PSM		2:1		Power saving mode; see table “Refresh time”
						00 = mode 1
						01 = mode 2
						10 = mode 3
						11 = mode 4
		PSM_EN	0		Power saving mode enable setting
						0 = disable
						1 = enable
	
	Bits 2 and 1 define the power saving modes. Bits 15:3 are reserved.
*/
void VEML6030_set_power_saving_mode(uint8_t psm, uint8_t psm_en)
{
	uint16_t reg_val = ((psm & 0b11) << 1) | (psm_en & 0b1);
	
	VEML6030_write_reg(0x03, reg_val);							// Power saving command
}


/*
	The VEML6030 can memorize the last ambient data before shutdown and keep this data before
	waking up. When the device is in shutdown mode, the host can freely read this data directly
	via a read command. When the VEML6030 wakes up, the data will be refreshed by new detection.
*/
uint16_t VEML6030_get_measurement_result(void)
{
	return VEML6030_read_reg(0x04);								// ALS command
}


/*
	RESOLUTION AND MAXIMUM DETECTION RANGE
					GAIN 2	GAIN 1	GAIN 1/4	GAIN 1/8		GAIN 2	GAIN 1	GAIN 1/4	GAIN 1/8
		IT (ms)				TYPICAL RESOLUTION						MAXIMUM POSSIBLE ILLUMINATION
		800			0.0036	0.0072	0.0288		0.0576			236		472		1887		3775
		400			0.0072	0.0144	0.0576		0.1152			472		944		3775		7550
		200			0.0144	0.0288	0.1152		0.2304			944		1887	7550		15099
		100			0.0288	0.0576	0.2304		0.4608			1887	3775	15099		30199
		50			0.0576	0.1152	0.4608		0.9216			3775	7550	30199		60398
		25			0.1152	0.2304	0.9216		1.8432			7550	15099	60398		120796
	
	Illumination values higher than 1000 lx show non-linearity. This non-linearity is the same for all
	sensors, so a compensation formula can be applied if this light level is exceeded.
	
	y = 6.0135E-13x^4 - 9.3924E-09x^3 + 8.1488E-05x^2 + 1.0023E+00x
*/
float VEML6030_calculate_lux(uint16_t raw, uint8_t gain, uint8_t integration)
{
	float result, _gain, _integ;
	
	switch(gain)
	{
		case 0b00:
			_gain = 1.0;
			break;
			
		case 0b01:
			_gain = 2.0;
			break;
		
		case 0b10:
			_gain = 0.125;
			break;
		
		case 0b11:
			_gain = 0.25;
			break;
		
		default:
			return 0.0;
	}
	
	switch(integration)
	{
		case 0b0000:
			_integ = 0.0625;
			break;
		
		case 0b0001:
			_integ = 0.125;
			break;
		
		case 0b0010:
			_integ = 0.25;
			break;
		
		case 0b0011:
			_integ = 0.5;
			break;
		
		case 0b1000:
			_integ = 0.03125;
			break;
		
		case 0b1100:
			_integ = 0.015625;
			break;
		
		default:
			return 0.0;
	}
	
	result = (float)raw * (0.0036 / _gain / _integ);
	
	if(result > 1000.0)
	{
		result = 0.00000000000060135 * powf(result, 4) - 0.0000000093924 * powf(result, 3)
				+ 0.000081488 * powf(result, 2) + 1.0023 * result;
	}
	
	return result;
}