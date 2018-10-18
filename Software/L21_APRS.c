/*
	
*/


#include "sam.h"
#include "L21_APRS.h"
#include "L21_TC.h"


/*
	APRS PACKET
		Individual bytes transmitted least significant bit (LSB) first.
		After every series of five '1' bits there is one extra '0' bit stuffed into the bit stream.
		Bit stuffing is not applied when transmitting the 0x7E FLAGS (0b01111110).
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