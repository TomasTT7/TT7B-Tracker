/*
	
*/


#ifndef L21_APRS_H
#define L21_APRS_H


#include "stdint.h"


// Functions
uint8_t APRS_packet(uint8_t * buffer, uint8_t * callsign, uint8_t ssid, float lat, float lon, uint16_t alt);
void APRS_prepare_bitstream(uint8_t * data, uint8_t len);

uint16_t crc_ccitt_update(uint16_t crc, uint8_t data);


#endif // L21_APRS_H_