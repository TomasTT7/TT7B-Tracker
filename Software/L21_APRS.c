/*
	APRS PACKET SIZE
		Flags			?
		Destination		7
		Source			7
		Path			0-56
		Control			1
		Protocol ID		1
		Information		1-256
		FCS				2
		Flags			?
*/


#include "sam.h"
#include "L21_APRS.h"
#include "L21_TC.h"
#include "math.h"


#define lo8(x) ((x)&0xff)
#define hi8(x) ((x)>>8)


/*
	STRUCTURE:
		
		!/5LD\S*,yON2W?[%)Bxe`7,.>B!1d3_!?K+5MHWS(KY"`8S(pNxj?[%)Bxe`$V.>B!1d2V
		
		!		Data Type Identifier
		/		Symbol Table Identifier
		5LD\	Latitude
		S*,y	Longitude
		O		Symbol Code (Balloon)
		N2		Altitude
		W		Compression Type Identifier
		
		?[		Temperature MCU
		%)		Temperature THERMISTOR_1
		Bx		Temperature THERMISTOR_2
		e`		Temperature MS5607_1
		7,		Temperature MS5607_2
		.>B		Pressure MS5607_1
		!1d		Pressure MS5607_2
		3_		Battery Voltage
		!?K+	Altitude (precise: 0-99m), Satellites, Active Time, Last Reset
		
		5MHW	Backlog: Latitude
		S(KY	Backlog: Longitude
		"`8S	Backlog: Altitude (precise: 0-50,000m)
		(pNxj	Backlog: Year, Month, Day, Hour, Minute, Active Time
		?[		Backlog: Temperature MCU
		%)		Backlog: Temperature THERMISTOR_1
		Bx		Backlog: Temperature THERMISTOR_2
		e`		Backlog: Temperature MS5607_1
		$V		Backlog: Temperature MS5607_2
		.>B		Backlog: Pressure MS5607_1
		!1d		Backlog: Pressure MS5607_2
		2V		Backlog: Battery Voltage
*/
uint8_t APRS_packet(uint8_t * buffer, uint8_t * callsign, uint8_t ssid, float lat, float lon, uint16_t alt, uint16_t temp_mcu,
					uint16_t temp_th1, uint16_t temp_th2, float temp_ms1, float temp_ms2, uint32_t pres_ms1, uint32_t pres_ms2,
					uint16_t battV, uint8_t sats, uint16_t time, uint8_t rcause)
{
	uint8_t n = 0;
	
	/* Flags */
	for(uint8_t i = 0 ; i < 12; i++) buffer[n++] = 0x7E;		// 12 0x7E flags precedes data bytes 
	
	/* Destination Address */
	uint8_t destination[6] = "APRS  ";
	for(uint8_t i = 0; i < 6; i++) buffer[n++] = destination[i];
	
	/* Destination SSID */
	uint8_t dssid = 0;											// SSID: 0-15
	buffer[n++] = 0b01110000 | (dssid & 0x0F);
	
	/* Source Address */
	for(uint8_t i = 0; i < 6; i++) buffer[n++] = callsign[i];
	
	/* Source SSID */
	buffer[n++] = 0b00110000 | (ssid & 0x0F);					// SSID: 0-15
	
	/* Path */
	uint8_t path[6] = "WIDE2 ";
	for(uint8_t i = 0; i < 6; i++) buffer[n++] = path[i];
	
	/* Path SSID */
	buffer[n++] = 0b00110001;									// WIDE2-1
	
	/* Left Shift Address Bytes */
	for(uint8_t i = 12; i < n; i++) buffer[i] <<= 1;
	buffer[n-1] |= 1;											// indicate end of address fields				
	
	/* Control Field */
	buffer[n++] = 0x03;
	
	/* Protocol ID */
	buffer[n++] = 0xF0;
	
	/* Information Field */
	buffer[n++] = '!';											// Data Type Identifier
	buffer[n++] = '/';											// Symbol Table Identifier
	
	uint32_t latitude = (90.0 - lat) * 380926.0;
	uint32_t longitude = (180.0 + lon) * 190463.0;
	uint16_t altitude = log((float)alt * 3.28084) / log(1.002);
	
	n = Base91_encode_u32(latitude, buffer, n);					// Latitude
	n = Base91_encode_u32(longitude, buffer, n);				// Longitude
	buffer[n++] = 'O';											// Symbol Code (Balloon)
	n = Base91_encode_u16(altitude, buffer, n);					// Altitude (coarse)
	buffer[n++] = 'W';											// Compression Type Identifier (0b00110110)
	
	n = Base91_encode_u16(temp_mcu, buffer, n);					// Temperature MCU
	n = Base91_encode_u16(temp_th1, buffer, n);					// Temperature THERMISTOR_1
	n = Base91_encode_u16(temp_th2, buffer, n);					// Temperature THERMISTOR_2
	
	temp_ms1 = temp_ms1 * 50.0 + 4000.0;
	temp_ms2 = temp_ms2 * 50.0 + 4000.0;
	
	n = Base91_encode_u16((uint16_t)temp_ms1, buffer, n);		// Temperature MS5607_1
	n = Base91_encode_u16((uint16_t)temp_ms2, buffer, n);		// Temperature MS5607_2
	n = Base91_encode_u24(pres_ms1, buffer, n);					// Pressure MS5607_1
	n = Base91_encode_u24(pres_ms2, buffer, n);					// Pressure MS5607_2
	n = Base91_encode_u16(battV, buffer, n);					// Battery Voltage
	
	uint8_t reset = APRS_reset_source(rcause);
	uint8_t alt_precise = APRS_altitude_precise(alt);
	uint32_t data1 = APRS_data_block_1(alt_precise, sats, time, reset);
	
	n = Base91_encode_u32(data1, buffer, n);					// Altitude (precise), Satellites, Active Time, Last Reset
	
	/* Frame Check Sequence */
	uint16_t crc = 0xFFFF;
	
	for(uint8_t i = 0; i < (n - 12); i++) crc = crc_ccitt_update(crc, buffer[i+12]);
	
	crc = ~crc;													// FCS is sent with bits flipped low-byte first
	buffer[n++] = crc & 0xFF;
	buffer[n++] = (crc >> 8) & 0xFF;
	
	/* End Flags */
	buffer[n++] = 0x7E;
	buffer[n++] = 0x7E;
	
	return n;
}


/*
	STRUCTURE:
	
		5MHWS(KY"`8S(pNxj?[%)Bxe`$V.>B!1d2V
	
		5MHW	Latitude
		S(KY	Longitude
		"`8S	Altitude (precise: 0-50,000m)
		(pNxj	Year, Month, Day, Hour, Minute, Active Time
		?[		Temperature MCU
		%)		Temperature THERMISTOR_1
		Bx		Temperature THERMISTOR_2
		e`		Temperature MS5607_1
		$V		Temperature MS5607_2
		.>B		Pressure MS5607_1
		!1d		Pressure MS5607_2
		2V		Battery Voltage
*/
void APRS_backlog_encode(uint8_t * buffer, float lat, float lon, uint16_t alt, uint16_t temp_mcu, uint16_t temp_th1,
						 uint16_t temp_th2, float temp_ms1, float temp_ms2, uint32_t pres_ms1, uint32_t pres_ms2,
						 uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint16_t battV,
						 uint8_t sats, uint16_t time, uint8_t rcause)
{
	uint8_t n = 0;
	
	uint32_t latitude = (90.0 - lat) * 380926.0;
	uint32_t longitude = (180.0 + lon) * 190463.0;
	
	uint8_t reset = APRS_reset_source(rcause);
	uint32_t data2 = APRS_data_block_2(alt, sats, reset);
	uint64_t data3 = APRS_data_block_3(year, month, day, hour, minute, time);
	
	n = Base91_encode_u32(latitude, buffer, n);					// Latitude
	n = Base91_encode_u32(longitude, buffer, n);				// Longitude
	n = Base91_encode_u32(data2, buffer, n);					// Data Block 2
	n = Base91_encode_u40(data3, buffer, n);					// Data Block 3
	n = Base91_encode_u16(temp_mcu, buffer, n);					// Temperature MCU
	n = Base91_encode_u16(temp_th1, buffer, n);					// Temperature THERMISTOR_1
	n = Base91_encode_u16(temp_th2, buffer, n);					// Temperature THERMISTOR_2
	
	temp_ms1 = temp_ms1 * 50.0 + 4000.0;
	temp_ms2 = temp_ms2 * 50.0 + 4000.0;
	
	n = Base91_encode_u16((uint16_t)temp_ms1, buffer, n);		// Temperature MS5607_1
	n = Base91_encode_u16((uint16_t)temp_ms2, buffer, n);		// Temperature MS5607_2
	n = Base91_encode_u24(pres_ms1, buffer, n);					// Pressure MS5607_1
	n = Base91_encode_u24(pres_ms2, buffer, n);					// Pressure MS5607_2
	n = Base91_encode_u16(battV, buffer, n);					// Battery Voltage
}


/*
	APRS PACKET
		Individual bytes transmitted least significant bit (LSB) first.
		After every series of five '1' bits there is one extra '0' bit stuffed into the bit stream.
		Bit stuffing is not applied when transmitting the 0x7E FLAGS (0b01111110).
	
	DATA
		Pointer to a buffer containing an APRS packet.
	
	LEN
		Number of data bytes in the buffer.
*/
void APRS_prepare_bitstream(uint8_t * data, uint8_t len)
{
	uint8_t series = 0;											// keeps track of successive '1's
	
	for(uint16_t i = 0; i < len; i++)
	{
		for(uint8_t y = 0; y < 8; y++)
		{
			if(i == 0 && y == 0)
			{
				TC0_buffer_add_bit(data[0] & 0x01, 1);			// reset buffer bit counter and start from buffer beginning
				if(data[0] & 0x01) series++;
			}
			else
			{
				if(data[i] == 0x7E)								// flag - don't stuff '0' bits
				{
					TC0_buffer_add_bit(((data[i] & (1 << y)) >> y), 0);
					series = 0;
				}
				else
				{
					if(series == 5)								// stuff a '0' bit
					{
						TC0_buffer_add_bit(0, 0);
						series = 0;
					}
					
					uint8_t _bit = ((data[i] & (1 << y)) >> y);
					
					if(_bit) series++;
					else series = 0;
					
					TC0_buffer_add_bit(_bit, 0);
				}
			}
		}
	}
}


/*
	BASE91 ENCODING RESOLUTION
		CHAR_1	ALT. [m]	RES. [m]
		35		176.9		0.35
		36		212.2		0.42
		37		254.5		0.51
		38		305.2		0.61
		39		366.1		0.73
		40		439.1		0.88
		41		526.6		1.05
		42		631.6		1.26
		43		757.5		1.52
		44		908.6		1.82
		45		1089.8		2.18
		46		1307.1		2.61
		47		1567.7		3.14
		48		1880.3		3.76
		49		2255.2		4.51
		50		2704.9		5.41
		51		3244.2		6.49
		52		3891.1		7.78
		53		4667.0		9.33
		54		5597.6		11.20
		55		6713.7		13.43
		56		8052.4		16.10
		57		9658.0		19.32
		58		11583.8		23.17
		59		13893.5		27.79
		60		16663.8		33.33
		61		19986.5		39.97
		62		23971.8		47.94
		63		28751.6		57.50
		64		34484.6		68.97
		65		41360.7		82.72
		66		49607.9		99.22
*/
uint8_t APRS_altitude_precise(uint16_t alt)
{
	uint16_t altitude = log((float)alt * 3.28084) / log(1.002);
	
	altitude = (uint16_t)(powf(10.0, (float)altitude * 0.00086772) * 0.3048);
	
	return alt - altitude;
}


/*
	EFFECTS OF DIFFERENT RESET CAUSES
											Power Supply Reset			User Reset												Backup Reset
											POR, BOD33		BOD12		External Reset		WDT Reset, System Reset Request		RTC, EXTWAKE, BBPS
		RTC, OSC32KCTRL, RSTC, CTRLA.IORET	Y				N			N					N									N
		GCLK with WRTLOCK					Y				Y			N					N									Y
		Debug logic							Y				Y			Y					N									Y
		Others								Y				Y			Y					Y									Y
	
	RCAUSE
		Bit 7	Backup Reset			(Refer to BKUPEXIT register to identify the source of the Backup Reset.)
		Bit 6	System Reset Request	(Refer to the Cortex processor documentation for more details.)
		Bit 5	Watchdog Reset			(This bit is set if a Watchdog Timer Reset has occurred.)
		Bit 4	External Reset			(This bit is set if an external Reset has occurred.)
		Bit 2	BOD33 Reset				(This bit is set if a Brown Out 33 Detector Reset has occurred.)
		Bit 1	BOD12 Reset				(This bit is set if a Brown Out 12 Detector Reset has occurred.)
		Bit 0	Power On Reset			(This bit is set if a POR has occurred.)
	
	RETURN
		5	System Reset Request generated by the CPU when asserting the SYSRESETREQ bit.
		4	Watchdog Timer Reset (WDT) has occurred.
		3	External Reset (RESET) has occurred.
		2	Brown Out 33 Detector Reset (BOD33) or Brown Out 12 Detector Reset (BOD12) has occurred.
		1	Power-on-reset (POR) has occurred
		0	none
*/
uint8_t APRS_reset_source(uint8_t rcause)
{
	if(!rcause) return 0;
	if(rcause & 0b00000001) return 1;
	if(rcause & 0b00000010) return 2;
	if(rcause & 0b00000100) return 2;
	if(rcause & 0b00010000) return 3;
	if(rcause & 0b00100000) return 4;
	if(rcause & 0b01000000) return 5;
	
	return 0;
}


/*
	Altitude precise	0-99	[m]
	Satellites			0-16	[n]
	Active time			0-999	[0.1s]
	Last Reset			0-5		[NONE, POR, BOD12, BOD33, WDT, SYS]
	
	DATA max value:		10,199,999
*/
uint32_t APRS_data_block_1(uint8_t altitude, uint8_t satellites, uint16_t time, uint8_t reset)
{
	uint32_t data;
	
	data = (((uint32_t)altitude * 17 + (uint32_t)satellites) * 1000 + (uint32_t)time) * 6 + (uint32_t)reset;
	
	return data;
}


/*
	Backlog Altitude		0-50,000	[m]
	Backlog Satellites		0-16		[n]
	Backlog Reset			0-5			[NONE, POR, BOD12, BOD33, WDT, SYS]
	
	DATA max value:		5,100,101
*/
uint32_t APRS_data_block_2(uint16_t altitude, uint8_t satellites, uint8_t reset)
{
	uint32_t data;
	
	data = ((uint32_t)altitude * 17 + (uint32_t)satellites) * 6 + (uint32_t)reset;
	
	return data;
}


/*
	Backlog Year			2018-2028	[year]
	Backlog Month			1-12		[month]
	Backlog Day				1-31		[day]
	Backlog Hour			0-23		[h]
	Backlog Minute			0-59		[min]
	Backlog Active time		0-999		[0.1s]
	
	DATA max value:		5,892,479,999
*/
uint64_t APRS_data_block_3(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint16_t time)
{
	uint64_t data;
	
	data = ((((uint64_t)((year - 2018) * 12 + (month - 1)) * 31 + (uint64_t)(day - 1)) * 24 + (uint64_t)hour) * 60 + (uint64_t)minute) * 1000 + (uint64_t)time;
	
	return data;
}


/*
	Encodes values (max 8280) to Base91 format for APRS telemetry.
	
	Output TWO chars.
*/
uint8_t Base91_encode_u16(uint16_t number, uint8_t *buffer, uint8_t n)
{
	if(number > 8280)											// maximum acceptable value
	{
		for(uint8_t i = 0; i < 2; i++) buffer[n++] = '!';		// decoded as 0
	}
	else
	{
		buffer[n++] = (number / 91) + '!';
		buffer[n++] = (number % 91) + '!';
	}
	
	return n;
}


/*
	Encodes values (max 753571) to Base91 format for APRS telemetry.
	
	Output THREE chars.
*/
uint8_t Base91_encode_u24(uint32_t number, uint8_t *buffer, uint8_t n)
{
	if(number > 753571)											// maximum acceptable value
	{
		for(uint8_t i = 0; i < 3; i++) buffer[n++] = '!';		// decoded as 0
	}
	else
	{
		buffer[n++] = (number / 8281) + '!';
		buffer[n++] = (number % 8281 / 91) + '!';
		buffer[n++] = (number % 8281 % 91) + '!';
	}
	
	return n;
}


/*
	Encodes values (max 68574961) to Base91 format for APRS telemetry.
	
	Output FOUR chars.
*/
uint8_t Base91_encode_u32(uint32_t number, uint8_t *buffer, uint8_t n)
{
	if(number > 68574961)										// maximum acceptable value
	{
		for(uint8_t i = 0; i < 4; i++) buffer[n++] = '!';		// decoded as 0
	}
	else
	{
		buffer[n++] = (number / 753571) + '!';
		buffer[n++] = (number % 753571 / 8281) + '!';
		buffer[n++] = (number % 753571 % 8281 / 91) + '!';
		buffer[n++] = (number % 753571 % 8281 % 91) + '!';
	}
	
	return n;
}


/*
	Encodes values (max 6240321451) to Base91 format for APRS telemetry.
	
	Output FIVE chars.
*/
uint8_t Base91_encode_u40(uint64_t number, uint8_t *buffer, uint8_t n)
{
	if(number > 6240321451)										// maximum acceptable value
	{
		for(uint8_t i = 0; i < 5; i++) buffer[n++] = '!';		// decoded as 0
	}
	else
	{
		buffer[n++] = (number / 68574961) + '!';
		buffer[n++] = (number % 68574961 / 753571) + '!';
		buffer[n++] = (number % 68574961 % 753571 / 8281) + '!';
		buffer[n++] = (number % 68574961 % 753571 % 8281 / 91) + '!';
		buffer[n++] = (number % 68574961 % 753571 % 8281 % 91) + '!';
	}
	
	return n;
}


/*
	Calculates the Frame Check Sequence (FCS) of the APRS packet. 
*/
uint16_t crc_ccitt_update(uint16_t crc, uint8_t data)
{
	data ^= lo8 (crc);
	data ^= data << 4;
	
	return ((((uint16_t)data << 8) | hi8 (crc)) ^ (uint8_t)(data >> 4) ^ ((uint16_t)data << 3));
}