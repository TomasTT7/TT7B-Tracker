/*
	Example WSPR message:	OK7DMT JN99 30
		CALLSIGN:			OK7DMT
		LOCATOR:			JN99
		POWER:				30			(0,3,7,10,13,17,20,23,27,30,33,37,40,43,47,50,53,57,60) -> 0-999m, 1000-1999m, 2000-2999m... >18000m
		50 bits -> 28 bits callsign, 15 bits locator, 7 bits power (rough altitude)
	
	Extended WSPR message:	<OK7DMT> JN99bl 30
		HASHED CALLSIGN:	<OK7DMT>
		LOCATOR:			JN99bl
		POWER:				30			(0,3,7,10,13,17,20,23,27,30,33,37,40,43,47,50,53,57,60) -> 0-52m, 53-105m, 106-158m... 954-999m
		50 bits -> 15 bits hashed callsign, 28 bits 6-character locator, 7 bits power (refined altitude)
		
	Frequencies
		Band		Dial frequency (MHz)		TX frequency (MHz)
		40m			7.038600					7.040000 - 7.040200
		30m			10.138700					10.140100 - 10.140300
		20m			14.095600					14.097000 - 14.097200
		
	Transmission
		synchronized with GPS time
		begins at the start of an even minute
		should start within +-1s
		162 symbols, one symbol 0.682687s (1.4648 baud)
		4 tones spaced 1.4648Hz apart
		6Hz bandwidth, 110.6s message duration
		allowed drift ~1Hz
		minimum S/N for reception –28 dB in 2500 Hz reference bandwidth
*/


#include "sam.h"
#include "L21_WSPR.h"
#include "L21_DAC.h"
#include "L21_TC.h"
#include "L21_WatchDog.h"
#include "nhash.h"


/*
	
*/
static uint8_t field[]			= {'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R'};
static uint8_t square[]			= {'0','1','2','3','4','5','6','7','8','9'};
static uint8_t subsquare[]		= {'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X'};
static uint8_t powerVals[]		= {0, 3, 7, 10, 13, 17, 20, 23, 27, 30, 33, 37, 40, 43, 47, 50, 53, 57, 60};

static uint8_t WSPRsync[162]	= {1,1,0,0,0,0,0,0,1,0,0,0,1,1,1,0,0,0,1,0,0,1,0,1,1,1,1,0,0,0,0,0,
								   0,0,1,0,0,1,0,1,0,0,0,0,0,0,1,0,1,1,0,0,1,1,0,1,0,0,0,1,1,0,1,0,
								   0,0,0,1,1,0,1,0,1,0,1,0,1,0,0,1,0,0,1,0,1,1,0,0,0,1,1,0,1,0,1,0,
								   0,0,1,0,0,0,0,0,1,0,0,1,0,0,1,1,1,0,1,1,0,0,1,1,0,1,0,0,0,1,1,1,
								   0,0,0,0,0,1,0,1,0,0,1,1,0,0,0,0,0,0,0,1,1,0,1,0,1,1,0,0,0,1,1,0,
								   0,0};


/*
	Converts decimal degree GPS coordinates to Maidenhead symbols.
*/
void WSPR_locator(uint8_t * locator, float lat, float lon)
{
	uint32_t result;

	lon += 180;
	lat += 90;

	uint32_t lon32 = lon * 10000;
	uint32_t lat32 = lat * 10000;

	result = lon32 / 200000;
	locator[0] = field[result];
	result = lat32 / 100000;
	locator[1] = field[result];

	result = (lon32 % 200000) / 20000;
	locator[2] = square[result];
	result = (lat32 % 100000) / 10000;
	locator[3] = square[result];

	result = ((lon32 % 200000) % 20000) / 834;
	locator[4] = subsquare[result];
	result = ((lat32 % 100000) % 10000) / 417;
	locator[5] = subsquare[result];
}


/*
	CALLSIGN may be up to 6 characters (A-Z, 0-9 and ' ').
	Values allocated to the characters are 0-9 for '0'-'9', 10-35 for 'A'-'Z' and 36 for ' '.
	Encoded into 28 bits.
*/
uint32_t WSPR_encode_callsign(uint8_t * callsign)
{
	uint8_t i = 0;
	uint32_t result = 0;

	// first character can be any of the 37 values
	if(callsign[0] > '9') result = callsign[0] - 55;
	else result = callsign[0] - 48;

	// second character can be only one of 36 values (can't be ' ')
	if(callsign[1] > '9') result = result * 36 + (callsign[1] - 55);
	else result = result * 36 + (callsign[1] - 48);

	// third character must be a number
	result = result * 10 + (callsign[2] - 48);

	// the remaining characters can't be numbers so only one of 27 values
	for(i = 3; i < 6; i++)
	{
		result = result * 27 + (callsign[i] - 55 - 10);
	}
  
	return result;
}


/*
	Encodes the 6-digit LOCATOR which was already byte shifted in place of the CALLSIGN. 
	JN99bl -> N99blJ
*/
uint32_t WSPR_encode_callsign_extended(uint8_t * callsign)
{
	uint32_t result = 0;
	
	result = callsign[0] - 55;
	result = result * 36 + (callsign[1] - 48);
	result = result * 10 + (callsign[2] - 48);
	result = result * 27 + (callsign[3] - 55) - 10;
	result = result * 27 + (callsign[4] - 55) - 10;
	result = result * 27 + (callsign[5] - 55) - 10;

	return result;
}


/*
	LOCATOR is 4 characters. First two A-Z, last two 0-9.
	Values allocated to the first pair are 0-17 for A-Z and 0-9 for the second pair.
	Encoded into 15 bits.
*/
uint32_t WSPR_encode_locator(uint8_t * locator)
{
	uint32_t result = 0;
	
	result = (179 - 10 * (locator[0] - 65) - (locator[2] - 48)) * 180 + 10 * (locator[1] - 65) + (locator[3] - 48);
	
	return result;
}


/*
	The power field in the WSPR message represents the tracker's rough altitude.
	
	[N]		[altitude range]
	0		0-1000m
	3		1000-1999m
	7		2000-2999m
	10		3000-3999m
	13		4000-4999m
	17		5000-5999m
	20		6000-6999m
	23		7000-7999m
	27		8000-8999m
	30		9000-9999m
	33		10000-10999m
	37		11000-11999m
	40		12000-12999m
	43		13000-13999m
	47		14000-14999m
	50		15000-15999m
	53		16000-16999m
	57		17000-17999m
	60		>18000m
*/
uint8_t WSPR_encode_power(uint32_t altitude)
{
	uint8_t n = altitude / 1000;
	
	if(n >= 18) return powerVals[18];
	else return powerVals[n];
}


/*
	The rough altitude is refined via the power field of the extended message.
	Add this amount of meters to the km estimate from the basic message.
	
	OK7DMT JN99 37		->	11000-11999m
	+
	<OK7DMT> JN99bl 10	->	11159-11211m
	
	[N]		[altitude range]
	0		0-52m
	3		53-105m
	7		106-158m
	10		159-211m
	13		212-264m
	17		265-317m
	20		318-370m
	23		371-423m
	27		424-476m
	30		477-529m
	33		530-582m
	37		583-635m
	40		636-688m
	43		689-741m
	47		742-794m
	50		795-847m
	53		848-900m
	57		901-953m
	60		954-999m
		
	Altitude higher than 18999 always sent as 60 and 60 -> 18999m.
*/
uint8_t WSPR_encode_power_extended(uint32_t altitude)
{
	uint8_t n = (altitude % 1000) / 53;
	
	if(n >= 18) return powerVals[18];
	else return powerVals[n];
}


/*
	Encodes the standard WSPR message: OK7DMT JN99 30
*/
void WSPR_packet(uint8_t * buffer, uint8_t * callsign, float lat, float lon, uint16_t alt)
{
	/* Encoding */
	uint8_t locator[6];
	
	WSPR_locator(locator, lat, lon);
	
	uint32_t N = WSPR_encode_callsign(callsign);
	uint32_t M = WSPR_encode_locator(locator) * 128 + WSPR_encode_power(alt) + 64;
	
	/* Data Symbols */
	uint8_t bits[11];
	
	bits[0] = (N & 0x0FF00000) >> 20;
	bits[1] = (N & 0x000FF000) >> 12;
	bits[2] = (N & 0x00000FF0) >> 4;
	bits[3] = ((N & 0x0000000F) << 4) | ((M & 0x003C0000) >> 18);
	bits[4] = (M & 0x0003FC00) >> 10;
	bits[5] = (M & 0x000003FC) >> 2;
	bits[6] = (M & 0x00000003) << 6;
	bits[7] = 0;
	bits[8] = 0;
	bits[9] = 0;
	bits[10] = 0;
	
	/* FEC */
	uint32_t FECreg = 0, FECparity0 = 0, FECparity1 = 0;
	uint8_t i = 0, x = 0, temp = 0, parBit0 = 0, parBit1 = 0;

	for(i = 0; i < 81; i++)
	{
		temp = bits[i/8] >> (7 - (i % 8));
		FECreg = (FECreg << 1) | (temp & 0x01);

		FECparity0 = FECreg & 0xF2D05351;
		FECparity1 = FECreg & 0xE4613C47;
		
		parBit0 = 0;
		parBit1 = 0;

		while(FECparity0 != 0)
		{
			parBit0++;
			FECparity0 = FECparity0 & (FECparity0 - 1);
		}
		
		while(FECparity1 != 0)
		{
			parBit1++;
			FECparity1 = FECparity1 & (FECparity1 - 1);
		}
		
		buffer[x++] = parBit0 & 0x01;
		buffer[x++] = parBit1 & 0x01;
	}
}


/*
	Encodes the extended WSPR message: <OK7DMT> JN99bl 30
*/
void WSPR_packet_extended(uint8_t * buffer, uint8_t * callsign, float lat, float lon, uint16_t alt)
{
	/* Encoding */
	uint8_t locator[6];
	
	WSPR_locator(locator, lat, lon);

	uint32_t ihash = nhash(callsign, 6, 146);									// computes the HASH of the callsign
	
	uint8_t _temp = locator[0];
	
	for(uint8_t i = 0; i < 5 ; i++) {locator[i] = locator[i+1];}				// LOCATOR is shifted by one byte
	
	locator[5] = _temp;
	
	uint32_t N = WSPR_encode_callsign_extended(locator);						// 6-digit LOCATOR is encoded as CALLSIGN
	uint32_t M = ihash * 128 - (WSPR_encode_power_extended(alt) + 1) + 64;		// CALLSIGN HASH is encoded with POWER
	
	/* Data Symbols */
	uint8_t bits[11];
	
	bits[0] = (N & 0x0FF00000) >> 20;
	bits[1] = (N & 0x000FF000) >> 12;
	bits[2] = (N & 0x00000FF0) >> 4;
	bits[3] = ((N & 0x0000000F) << 4) | ((M & 0x003C0000) >> 18);
	bits[4] = (M & 0x0003FC00) >> 10;
	bits[5] = (M & 0x000003FC) >> 2;
	bits[6] = (M & 0x00000003) << 6;
	bits[7] = 0;
	bits[8] = 0;
	bits[9] = 0;
	bits[10] = 0;
	
	/* FEC */
	uint32_t FECreg = 0, FECparity0 = 0, FECparity1 = 0;
	uint8_t i = 0, x = 0, temp = 0, parBit0 = 0, parBit1 = 0;

	for(i = 0; i < 81; i++)
	{
		temp = bits[i/8] >> (7 - (i % 8));
		FECreg = (FECreg << 1) | (temp & 0x01);

		FECparity0 = FECreg & 0xF2D05351;
		FECparity1 = FECreg & 0xE4613C47;
    
		parBit0 = 0;
		parBit1 = 0;

		while(FECparity0 != 0)
		{
			parBit0++;
			FECparity0 = FECparity0 & (FECparity0 - 1);
		}
    
		while(FECparity1 != 0)
		{
			parBit1++;
			FECparity1 = FECparity1 & (FECparity1 - 1);
		}
    
		buffer[x++] = parBit0 & 0x01;
		buffer[x++] = parBit1 & 0x01;
	}
}


/*
	Interleaves and merges data symbols with sync symbols.
	FEC tackles random errors better then a burst of errors. Hence the algorithm moves away adjacent bits.
*/
void WSPR_create_tones(uint8_t * buffer, uint8_t * data)
{
	uint8_t P = 0, I = 0, J = 0, h = 0;
	uint16_t g = 0;
	
	for(g = 0; g < 256; g++)
	{
		I = g;
		J = 0;
		
		for(h = 0; h < 8; h++)
		{
			J <<= 1;
			if(I & 1) J = J | 1;
			I >>= 1;
		}

		if(J < 162)
		{
			buffer[J] = WSPRsync[J] + 2 * data[P];
			P++;
		}
	}
}


/*
	Transmits the 162 tones corresponding to the encoded WSPR message.
*/
void WSPR_transmit(uint8_t * data)
{
  uint8_t Tone = 0;
  
  for(Tone = 0; Tone < 162; Tone++)
  {
    /* WatchDog Reset */
    WatchDog_reset();
	
	switch(data[Tone])
    {
      case 0:
		DAC->DATA[1].reg = 3143;												// 0.001 Hz, Tone 0
        break;
	
      case 1:
		DAC->DATA[1].reg = 3153;												// 0.001 Hz, Tone 1
        break;
	
      case 2:
		DAC->DATA[1].reg = 3163;												// 0.001 Hz, Tone 2
        break;
	
      case 3:
		DAC->DATA[1].reg = 3173;												// 0.001 Hz, Tone 3
		break;
	
      default:
		break;
    }
	
	TC0_compare_match_delay();													// 1.4648 baud rate
  }
}