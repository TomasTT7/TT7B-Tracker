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


#include "sam.h"
#include "L21_SERCOM_I2C.h"
#include "L21_SI5351B.h"
#include "math.h"


/*
	APRS FREQUENCIES
		AREA	[MHz]		VCO [MHz]
		US		144.390		866.340
		NZ		144.575		867.450
		SK		144.620		867.720
		CHN		144.640		867.840
		JAP		144.660		867.960
		EU		144.800		868.800
		ARG		144.930		869.580
		VEN		145.010		870.060
		AU		145.175		871.050
		TH		145.525		873.150
		BR		145.570		873.420
*/


/*
	Writes one byte to an 8-bit address.
*/
void SI5351B_write_reg(uint8_t reg, uint8_t data)
{
	uint8_t ack;
	
	ack = SERCOM_I2C_transmit_address(SI5351B_ADDRESS << 1);	// send transmit address
	
	if(ack)
	{
		SERCOM_I2C_transmit_byte(reg);							// send register address
		SERCOM_I2C_transmit_byte(data);							// send data byte
		SERCOM_I2C_end_transmission();
	}
}


/*
	Reads and returns a byte from an 8-bit address.
*/
uint8_t SI5351B_read_reg(uint8_t reg)
{
	uint8_t ack;
	uint8_t data = 0;
	
	ack = SERCOM_I2C_transmit_address(SI5351B_ADDRESS << 1);	// send transmit address
	
	if(ack)
	{
		SERCOM_I2C_transmit_byte(reg);							// send desired register address
		SERCOM_I2C_end_transmission();
		
		ack = SERCOM_I2C_transmit_address((SI5351B_ADDRESS << 1) | 0x01);	// send receive address
		
		if(ack)
		{
			SERCOM_I2C_receive_byte(1);
			data = SERCOM_I2C_end_reception();					// receive data byte
		}
		else
		{
			return 0xFF;
		}
	}
	else
	{
		return 0xFF;
	}
	
	return data;
}


/*
	PLLB must be used as the source for any VCXO output clock. Feedback B Multisynth divider
	ratio must be set such that the denominator (c) in the fractional divider a+b/c is fixed to 10^6.
*/
void SI5351B_init(void)
{
	while((SI5351B_read_reg(0) >> 7) == 1);						// wait while device is in system initialization mode
	
	SI5351B_write_reg(3, 0b11111111);							// disable CLKx output where x = 0, 1, 2, 3, 4, 5, 6, 7
	SI5351B_write_reg(183, 0b00010010);							// Crystal Load Capacitance 0pF
	SI5351B_write_reg(9, 0b11111111);							// OEB pin does not control enable/disable state of CLKx output.
	SI5351B_write_reg(24, 0b10101010);							// CLK0-3 is set to a HIGH IMPEDANCE state when disabled.
	SI5351B_write_reg(25, 0b10101010);							// CLK4-7 is set to a HIGH IMPEDANCE state when disabled.
	SI5351B_write_reg(15, 0b00000000);							// CLKIN_DIV by 1, PLLB_SRC is XTAL, PLLA_SRC is XTAL
	
	SI5351B_write_reg(16, 0b10001100);							// CLK0 is powered down, PLLA, Drive Strength: 2mA
	SI5351B_write_reg(17, 0b10001100);							// CLK1 is powered down, PLLA, Drive Strength: 2mA
	SI5351B_write_reg(18, 0b10001100);							// CLK2 is powered down, PLLA, Drive Strength: 2mA
	SI5351B_write_reg(19, 0b10001100);							// CLK3 is powered down, PLLA, Drive Strength: 2mA
	SI5351B_write_reg(20, 0b10001100);							// CLK4 is powered down, PLLA, Drive Strength: 2mA
	SI5351B_write_reg(21, 0b01101111);							// CLK5 is powered up, PLLB, Drive Strength: 8mA, integer mode
	SI5351B_write_reg(22, 0b10001100);							// CLK6 is powered down, PLLA, Drive Strength: 2mA
	SI5351B_write_reg(23, 0b10001100);							// CLK7 is powered down, PLLA, Drive Strength: 2mA
	
	SI5351B_write_reg(177, 0b10100000);							// PLLB and PLLA reset
}


/*
	
*/
void SI5351B_deinit(void)
{
	SI5351B_write_reg(3, 0b11111111);							// disable CLKx output where x = 0, 1, 2, 3, 4, 5, 6, 7
	SI5351B_write_reg(21, 0b11101100);							// CLK5 is powered down, PLLB, Drive Strength: 2mA, integer mode
	SI5351B_write_reg(177, 0b10100000);							// PLLB and PLLA reset
}


/*
	VCO
		The device consists of two PLLs (PLLA, PLLB). Each PLL consists of a Feedback Multisynth
		used to generate an intermediate VCO frequency in the range of 600 to 900 MHz.
		
		Fvco = Fxtal * (a + b / c)
		
		where (a+b/c) is in the range of 15+0/1,048,575 to 90+0/1,048,575.
		
		MSNx_P1 - integer part of PLLx Feedback Multisynth divider
		MSNx_P2 - numerator for fractional part of PLLx Feedback Multisynth divider
		MSNx_P3 - denominator for fractional part of PLLx Feedback Multisynth divider
		
		When using the VCXO function, set the MSNB divide ratio a+b/c such that c=10^6.
	
	MULTISYNTH
		Either of the two VCO frequencies (PLLA, PLLB) can be divided down by the individual
		output Multisynth dividers to generate a Multisynth frequency between 500 kHz and 200 MHz.
		
		Multisynth = a + b / c
		Fout = Fvco / (Multisynth * R)
		
		where valid Multisynth divider ratios are 4, 6, 8, and any fractional value
		between 8+1/1,048,575 and 900+0/1.
		
		MSx_DIVBY4 - MSx Divide by 4 Enable 0b00/0b11 (for 150MHz < Fout <= 200MHz)
		Rx_DIV - Rx Output Divider: 1/2/4/8/16/32/64/128 (for Fout < 500kHz)
		MSx_P1 - integer part of the MultiSynth divider
		MSx_P2 - numerator for the fractional part of the MultiSynth Divider
		MSx_P3 - denominator for the fractional part of the MultiSynth Divider
		
		If a+b/c is an even integer, integer mode may be enabled for PLLA or PLLB by setting
		parameter FBA_INT or FBB_INT respectively.
		
	VCXO
		For a desired pull-range of +/– 30ppm, the value APR in the equation below is 30,
		for +/– 60ppm APR¨is 60, and so on.
		
		VCXO_Param[21:0] = 1.03 * (128 * a + b / 10^6) * APR
		
		VCXO
			VCXO Control Voltage Range			0V			3.3V
			VCXO Gain (configurable)			18ppm/V		150ppm/V
			VCXO Control Voltage Linearity		-5%			+5%
			VCXO Pull Range (configurable)		+-30ppm		+-240ppm
			VCXO Modulation Bandwidth			10kHz
		
		APR
			30 to 240 ppm
*/
void SI5351B_frequency(uint32_t freq_Hz, uint8_t APR)
{
	/* PLLB VCO FREQUENCY */
	uint32_t VCOfreq = (freq_Hz + FREQ_OFFSET) * 6;
	
	uint32_t a = VCOfreq / TCXO_FREQ;
	uint32_t b = (uint64_t)(VCOfreq % TCXO_FREQ) * 1000000 / (uint64_t)TCXO_FREQ;
	uint32_t c = 1000000;
	
	uint32_t p1 = 128 * a + floor(128 * b / c) - 512;
	uint32_t p2 = 128 * b - c * floor(128 * b / c);
	
	SI5351B_write_reg(34, (c & 0x0000FF00) >> 8);				// MSNB_P3[15:8]
	SI5351B_write_reg(35, (c & 0x000000FF));					// MSNB_P3[7:0]
	SI5351B_write_reg(36, (p1 & 0x00030000) >> 16);				// MSNB_P1[17:16]
	SI5351B_write_reg(37, (p1 & 0x0000FF00) >> 8);				// MSNB_P1[15:8]
	SI5351B_write_reg(38, (p1 & 0x000000FF));					// MSNB_P1[7:0]
	SI5351B_write_reg(39, ((c & 0x000F0000) >> 12) | ((p2 & 0x000F0000) >> 16));	// MSNB_P3[19:16], MSNB_P2[19:16]
	SI5351B_write_reg(40, (p2 & 0x0000FF00) >> 8);				// MSNB_P2[15:8]
	SI5351B_write_reg(41, (p2 & 0x000000FF));					// MSNB_P2[7:0]
	
	/* VCXO */
	uint32_t VCXO_Param = (uint32_t)(1.03 * (128.0 * (float)a + (float)b / 1000000.0) * (float)APR);
	
	SI5351B_write_reg(162, VCXO_Param & 0xFF);					// VCXO_Param[7:0]
	SI5351B_write_reg(163, (VCXO_Param >> 8) & 0xFF);			// VCXO_Param[15:8]
	SI5351B_write_reg(164, (VCXO_Param >> 16) & 0x3F);			// VCXO_Param[21:16]
	
	/* MULTISYNTH 5 FREQUENCY */
	a = 6;
	b = 0;
	c = 1;
	
	p1 = 128 * a + floor(128 * b / c) - 512;
	p2 = 128 * b - c * floor(128 * b / c);
	
	SI5351B_write_reg(82, (c & 0x0000FF00) >> 8);				// MS5_P3[15:8]
	SI5351B_write_reg(83, (c & 0x000000FF));					// MS5_P3[7:0]
	SI5351B_write_reg(84, (p1 & 0x00030000) >> 16);				// R5_DIV[2:0], MS5_DIVBY4[1:0], MS5_P1[17:16]
	SI5351B_write_reg(85, (p1 & 0x0000FF00) >> 8);				// MS5_P1[15:8]
	SI5351B_write_reg(86, (p1 & 0x000000FF));					// MS5_P1[7:0]
	SI5351B_write_reg(87, ((c & 0x000F0000) >> 12) | ((p2 & 0x000F0000) >> 16));	// MS5_P3[19:16], MS5_P2[19:16]
	SI5351B_write_reg(88, (p2 & 0x0000FF00) >> 8);				// MS5_P2[15:8]
	SI5351B_write_reg(89, (p2 & 0x000000FF));					// MS5_P2[7:0]
}


/*
	Enables output on CLK5 and disables all other output.
*/
void SI5351B_enable_output(void)
{
	SI5351B_write_reg(3, 0b11011111);							// enable CLK5 output
}


/*
	Disables all outputs.
*/
void SI5351B_disable_output(void)
{
	SI5351B_write_reg(3, 0b11111111);							// disable CLKx output where x = 0, 1, 2, 3, 4, 5, 6, 7
}


/*
	CLK5_IDRV (CLK5)
	00	2 mA
	01	4 mA
	10	6 mA
	11	8 mA
*/
void SI5351B_drive_strength(uint8_t drvStr)
{
	if(drvStr > 3) return;
	
	SI5351B_write_reg(21, 0b01101100 | drvStr);					// CLK5 is powered up, PLLB, integer mode
}