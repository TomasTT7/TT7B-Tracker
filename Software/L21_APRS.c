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
	
		!/5MHXS(SVOHNW		/YYYYXXXXOcsT
		
		!		Data Type Identifier
		/		Symbol Table Identifier
		5MHX	Latitude
		S(SV	Longitude
		O		Symbol Code (Balloon)
		HN		Altitude
		W		Compression Type Identifier
*/
uint8_t APRS_packet(uint8_t * buffer, uint8_t * callsign, uint8_t ssid, float lat, float lon, uint16_t alt)
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
	
	lat = (90.0f - lat) * 380926.0f;
	lon = (180.0f + lon) * 190463.0f;
	alt = log((float)alt * 3.28f) / log(1.002f);
	alt = (uint16_t)alt % 8281;
	
	
	buffer[n++] = 'O';											// Symbol Code (Balloon)
	
	
	/* Frame Check Sequence */
	uint16_t crc = 0xFFFF;
	
	for(uint8_t i = 0; i < (n - 12); i++) crc = crc_ccitt_update(crc, buffer[i+12]);
	
	crc = ~crc;
	buffer[n++] = crc & 0xFF;									// FCS is sent low-byte first
	buffer[n++] = (crc >> 8) & 0xFF;							// and with bits flipped
	
	/* End Flags */
	buffer[n++] = 0x7E;
	buffer[n++] = 0x7E;
	
	return n;
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
	Function to encode values (max 8280) to Base91 format for APRS telemetry.
*/
void Base91_encode(uint16_t number, uint8_t *buffer)
{
	if(number > 8280)											// maximum acceptable value
	{
		 buffer[num++] = '!';									// decoded as 0
		 buffer[num++] = '!';									// decoded as 0
	}else{
		buffer[num++] = (number / 91) + '!';
		buffer[num++] = (number % 91) + '!';
	}
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