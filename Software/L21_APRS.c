/*
	APRS PACKET SIZE [byte]
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
#include "L21_NVM.h"
#include "L21_TC.h"
#include "math.h"


#define lo8(x) ((x)&0xff)
#define hi8(x) ((x)>>8)


/*
	STRUCTURE:
		
		!/5LD\S*,yON2WYm%=,)ZiLx,f:-D33ZM0!<QU5N^JS%<z!(Z/'T$r7U@#]KRj1F(!9T,wQ7,8Z
		
		!									Data Type Identifier
		/									Symbol Table Identifier
		5LD\		49.49148°				Latitude
		S*,y		18.22311°				Longitude
		O									Symbol Code (Balloon)
		N2			1127m					Altitude (coarse)
		W									Compression Type Identifier
		
		Ym			23.44°C					Temperature MCU
		%=			23.74°C					Temperature THERMISTOR_1
		,)			0.01°C					Temperature THERMISTOR_2
		Zi			25.18°C					Temperature MS5607_1
		Lx			0.00°C					Temperature MS5607_2
		,f:			97395Pa					Pressure MS5607_1
		-D3			102575Pa				Pressure MS5607_2
		3Z			1.512V					Battery Voltage
		M0			22.10lux				Ambient Light
		!<QU		2m, 4, 0.1s, POR		Altitude (offset: 0-99m), Satellites, Active Time, Last Reset
		
		5N^J		49.44184°				Backlog: Latitude
		S%<z		18.01337°				Backlog: Longitude
		!(Z/		619m, 5, NONE			Backlog: Altitude (precise: 0-50,000m), Satellites, Last Reset
		'T$r7		20181103 10:34, 12.3s	Backlog: Year, Month, Day, Hour, Minute, Active Time
		U@			15.26°C					Backlog: Temperature MCU
		#]			35.99°C					Backlog: Temperature THERMISTOR_1
		KR			-62.65°C				Backlog: Temperature THERMISTOR_2
		j1			53.18°C					Backlog: Temperature MS5607_1
		F(			-12.52°C				Backlog: Temperature MS5607_2
		!9T			2235Pa					Backlog: Pressure MS5607_1
		,wQ			98965Pa					Backlog: Pressure MS5607_2
		7,			1.795V					Backlog: Battery Voltage
		8Z			0.5279lux				Backlog: Ambient Light
*/
uint8_t APRS_packet(uint8_t * buffer, uint8_t * callsign, uint8_t ssid, float lat, float lon, uint16_t alt, float temp_mcu,
					uint16_t temp_th1, uint16_t temp_th2, float temp_ms1, float temp_ms2, uint32_t pres_ms1, uint32_t pres_ms2,
					uint16_t battV, float light, uint8_t sats, uint16_t time, uint8_t rcause, uint16_t * backlog_index, uint8_t nogps)
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
	
	if(!nogps)
	{
		/* Information Field */
		buffer[n++] = '!';										// Data Type Identifier
		buffer[n++] = '/';										// Symbol Table Identifier
	
		uint32_t latitude = (90.0 - lat) * 380926.0;
		uint32_t longitude = (180.0 + lon) * 190463.0;
		uint16_t altitude = log10((float)alt * 3.28084) / log10(1.002);
	
		n = Base91_encode_u32(latitude, buffer, n);				// Latitude
		n = Base91_encode_u32(longitude, buffer, n);			// Longitude
		buffer[n++] = 'O';										// Symbol Code (Balloon)
		n = Base91_encode_u16(altitude, buffer, n);				// Altitude (coarse)
		buffer[n++] = 'W';										// Compression Type Identifier (0b00110110)
	}
	else
	{
		/* Information Field */
		buffer[n++] = '!';										// Data Type Identifier
		
		uint8_t empty_coords[] = "0000.00N\\00000.00W.";
		for(uint8_t i = 0; i < 19; i++) buffer[n++] = empty_coords[i];
	}
	
	temp_mcu = temp_mcu * 50.0 + 4000.0;
	temp_ms1 = temp_ms1 * 50.0 + 4000.0;
	temp_ms2 = temp_ms2 * 50.0 + 4000.0;
	if(temp_mcu > 8280.0) temp_mcu = 8280.0;					// allowed range tops out at 85.6°C
	if(temp_ms1 > 8280.0) temp_ms1 = 8280.0;					// allowed range tops out at 85.6°C
	if(temp_ms2 > 8280.0) temp_ms2 = 8280.0;					// allowed range tops out at 85.6°C
	if(pres_ms1 > 753570) pres_ms1 = 753570;					// allowed range tops out at 753,570Pa
	if(pres_ms2 > 753570) pres_ms2 = 753570;					// allowed range tops out at 753,570Pa
	
	uint16_t amb_light = APRS_ambient_light(light);
	
	n = Base91_encode_u16((uint16_t)temp_mcu, buffer, n);		// Temperature MCU
	n = Base91_encode_u16(temp_th1, buffer, n);					// Temperature THERMISTOR_1
	n = Base91_encode_u16(temp_th2, buffer, n);					// Temperature THERMISTOR_2
	n = Base91_encode_u16((uint16_t)temp_ms1, buffer, n);		// Temperature MS5607_1
	n = Base91_encode_u16((uint16_t)temp_ms2, buffer, n);		// Temperature MS5607_2
	n = Base91_encode_u24(pres_ms1, buffer, n);					// Pressure MS5607_1
	n = Base91_encode_u24(pres_ms2, buffer, n);					// Pressure MS5607_2
	n = Base91_encode_u16(battV, buffer, n);					// Battery Voltage
	n = Base91_encode_u16(amb_light, buffer, n);				// Ambient Light
	
	uint8_t reset = APRS_reset_source(rcause);
	uint8_t alt_offset = APRS_altitude_offset(alt);
	uint32_t data1 = APRS_data_block_1(alt_offset, sats, time, reset);
	
	n = Base91_encode_u32(data1, buffer, n);					// Altitude (offset), Satellites, Active Time, Last Reset
	
	/* Backlog */
	uint8_t backlog_buff[37];
	
	*backlog_index = (*backlog_index + 1) % 512;
	uint16_t index = Van_Der_Corput_Sequence(*backlog_index, 9);
	
	APRS_backlog_get(backlog_buff, 37, index);
	
	if(backlog_buff[0] >= 33 && backlog_buff[0] <= 125)			// if not backlog probably not stored yet, skip it this time
	{
		for(uint8_t i = 0; i < 37; i++) buffer[n++] = backlog_buff[i];
	}
	
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
	
		5N^JS%<z!(Z/'T$r7U@#]KRj1F(!9T,wQ7,8Z
	
		5N^J		49.44184°				Backlog: Latitude
		S%<z		18.01337°				Backlog: Longitude
		!(Z/		619m, 5, NONE			Backlog: Altitude (precise: 0-50,000m), Satellites, Last Reset
		'T$r7		20181103 10:34, 12.3s	Backlog: Year, Month, Day, Hour, Minute, Active Time
		U@			15.26°C					Backlog: Temperature MCU
		#]			35.99°C					Backlog: Temperature THERMISTOR_1
		KR			-62.65°C				Backlog: Temperature THERMISTOR_2
		j1			53.18°C					Backlog: Temperature MS5607_1
		F(			-12.52°C				Backlog: Temperature MS5607_2
		!9T			2235Pa					Backlog: Pressure MS5607_1
		,wQ			98965Pa					Backlog: Pressure MS5607_2
		7,			1.795V					Backlog: Battery Voltage
		8Z			0.5279lux				Backlog: Ambient Light
	
	BACKLOG:	37 bytes
*/
void APRS_backlog_encode(uint8_t * buffer, float lat, float lon, uint16_t alt, float temp_mcu, uint16_t temp_th1,
						 uint16_t temp_th2, float temp_ms1, float temp_ms2, uint32_t pres_ms1, uint32_t pres_ms2,
						 uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint16_t battV,
						 float light, uint8_t sats, uint16_t time, uint8_t rcause)
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
	
	temp_mcu = temp_mcu * 50.0 + 4000.0;
	temp_ms1 = temp_ms1 * 50.0 + 4000.0;
	temp_ms2 = temp_ms2 * 50.0 + 4000.0;
	if(temp_mcu > 8280.0) temp_mcu = 8280.0;					// allowed range tops out at 85.6°C
	if(temp_ms1 > 8280.0) temp_ms1 = 8280.0;					// allowed range tops out at 85.6°C
	if(temp_ms2 > 8280.0) temp_ms2 = 8280.0;					// allowed range tops out at 85.6°C
	if(pres_ms1 > 753570) pres_ms1 = 753570;					// allowed range tops out at 753,570Pa
	if(pres_ms2 > 753570) pres_ms2 = 753570;					// allowed range tops out at 753,570Pa
	
	uint16_t amb_light = APRS_ambient_light(light);
	
	n = Base91_encode_u16((uint16_t)temp_mcu, buffer, n);		// Temperature MCU
	n = Base91_encode_u16(temp_th1, buffer, n);					// Temperature THERMISTOR_1
	n = Base91_encode_u16(temp_th2, buffer, n);					// Temperature THERMISTOR_2
	n = Base91_encode_u16((uint16_t)temp_ms1, buffer, n);		// Temperature MS5607_1
	n = Base91_encode_u16((uint16_t)temp_ms2, buffer, n);		// Temperature MS5607_2
	n = Base91_encode_u24(pres_ms1, buffer, n);					// Pressure MS5607_1
	n = Base91_encode_u24(pres_ms2, buffer, n);					// Pressure MS5607_2
	n = Base91_encode_u16(battV, buffer, n);					// Battery Voltage
	n = Base91_encode_u16(amb_light, buffer, n);				// Ambient Light
}


/*
	Backlog Size:			512 pages
	Storing Period:			20 minutes
	Backlogs per day:		72
	Backlog Depth:			7.1 days
	Transmission Period:	1 minute
	Time to transmit all:	8.5 hours
	
	Row Erase instruction erases 4 pages.
	Time to fill row:		60 minutes
	
	One row/page used to store a pointer to the oldest backlog. Written with every new log.
	Cycling Endurance:		25,000 (write & erase)
	Reliability:			347.2 days
	
	SAML21E17 (128kB NVM, 2048 pages, 64 bytes/page)
	Pointer Address:		98,048		(0x00017F00)	page 1532
	Backlog Start Address:	98,304		(0x00018000)	page 1536
	Backlog Last Address:	131,008		(0x0001FFC0)	page 2047
	
	LEN		maximum 64 bytes
*/
void APRS_backlog_store(uint8_t * buffer, uint8_t len)
{
	uint8_t _point[4];
	uint32_t _pointer = 0;
	
	/* Get Pointer */
	NVM_flash_read(_point, (uint16_t *) (98048), 4);			// read pointer to oldest backlog
	
	_pointer = ((uint32_t)_point[0] << 24) | ((uint32_t)_point[1] << 16) | ((uint32_t)_point[2] << 8) | (uint32_t)_point[3];
	
	if(_pointer < 98304 || _pointer > 131008) _pointer = 98304;	// by default start at first backlog address
	
	/* Write Backlog Data */
	if((_pointer % 256) == 0) NVM_flash_erase_row(_pointer);	// erase row if pointing at first page in row	
	NVM_flash_write(buffer, _pointer, len);						// write LEN bytes starting at _POINTER
	
	/* Update Pointer */
	_pointer = _pointer + 64;
	if(_pointer > 131008) _pointer = 98304;
	
	_point[0] = (uint8_t)(_pointer >> 24);
	_point[1] = (uint8_t)(_pointer >> 16);
	_point[2] = (uint8_t)(_pointer >> 8);
	_point[3] = (uint8_t)_pointer;
	
	NVM_flash_erase_row(98048);									// erase row where pointer is stored
	NVM_flash_write(_point, 98048, 4);							// write new pointer
}


/*
	Backlog Size:			512 pages
	
	SAML21E17 (128kB NVM, 2048 pages, 64 bytes/page)
	Pointer Address:		98,048		(0x00017F00)
	Backlog Start Address:	98,304		(0x00018000)
	Backlog Last Address:	131,008		(0x0001FFC0)
	
	N	index of the backlog
*/
void APRS_backlog_get(uint8_t * buffer, uint8_t len, uint16_t n)
{
	NVM_flash_read(buffer, (uint16_t *) (98304 + n * 64), len);
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
	
	TC0_buffer_clear();											// buffer must be zeroed first
	
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
		
	For input ALT between 0 and 50,000, the return value is an integer offset between 1 and 99m.
*/
uint8_t APRS_altitude_offset(uint16_t alt)
{
	uint16_t altitude = log10((float)alt * 3.28084) / log10(1.002);
	
	altitude = (uint16_t)(powf(10.0, (float)altitude * log10(1.002)) * 0.3048);
	
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
	Prepares light sensor reading for two char Base91 encoding.
	
		input min:		0.0072 lux
		input max:		110,083 lux
		
		output min:		0
		output max:		8280
	
	Due to the encoding, resolution of a decoded LUX value decreases with higher values.
	
		VALUE			OUTPUT		RESOLUTION
		1.804 lux		2765		0.0036 lux
		501.89 lux		5582		1.002 lux
		110,083 lux		8280		219.73 lux
*/
uint16_t APRS_ambient_light(float light)
{
	if(light < 0.0072) return 0;
	if(light > 110083.0) return 8280;
	
	uint16_t result = (uint16_t)(log(light * 139.0) / log(1.002));
	
	return result;
}


/*
	Altitude offset		0-99	[m]
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
	BITS	IN					OUT
	2		0,1,2,3				0,2,1,3
	3		0,1..6,7			0,4,2,6,1,5,3,7
	4		0,1..14,15			0,8,4,12,2,10,6,14,1,9,5,13,3,11,7,15
	5		0,1..30,31			0,16,8,24,4,20,12,28,2,18,10,26,6,22,14,30,1,17,9,25,5,21,13,29,3,19,11,27,7,23,15,31
	..
	16		0,1..65534,65535	...
*/
uint16_t Van_Der_Corput_Sequence(uint16_t in, uint8_t bits)
{
	uint16_t out = 0;
	
	for(uint8_t i = bits; i; i--)
	{
	    if(in & (1 << (i-1))) out |= (1 << (bits-i));
	}
	
	return out;
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
	Encodes values (max 753570) to Base91 format for APRS telemetry.
	
	Output THREE chars.
*/
uint8_t Base91_encode_u24(uint32_t number, uint8_t *buffer, uint8_t n)
{
	if(number > 753570)											// maximum acceptable value
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
	Encodes values (max 68574960) to Base91 format for APRS telemetry.
	
	Output FOUR chars.
*/
uint8_t Base91_encode_u32(uint32_t number, uint8_t *buffer, uint8_t n)
{
	if(number > 68574960)										// maximum acceptable value
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
	Encodes values (max 6240321450) to Base91 format for APRS telemetry.
	
	Output FIVE chars.
*/
uint8_t Base91_encode_u40(uint64_t number, uint8_t *buffer, uint8_t n)
{
	if(number > 6240321450)										// maximum acceptable value
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