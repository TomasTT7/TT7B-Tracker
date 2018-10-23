/*
	
*/


#ifndef L21_APRS_H
#define L21_APRS_H


#include "stdint.h"


// Functions
uint8_t APRS_packet(uint8_t * buffer, uint8_t * callsign, uint8_t ssid, float lat, float lon, uint16_t alt, uint16_t temp_mcu,
					uint16_t temp_th1, uint16_t temp_th2, float temp_ms1, float temp_ms2, uint32_t pres_ms1, uint32_t pres_ms2,
					uint16_t battV, uint8_t sats, uint16_t time, uint8_t rcause);
void APRS_prepare_bitstream(uint8_t * data, uint8_t len);
uint8_t APRS_altitude_precise(uint16_t alt);
uint8_t APRS_reset_source(uint8_t rcause);
uint32_t APRS_data_block_1(uint8_t altitude, uint8_t satellites, uint16_t time, uint8_t reset);
uint32_t APRS_data_block_2(uint16_t altitude, uint8_t satellites, uint8_t reset);
uint64_t APRS_data_block_3(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint16_t time);

uint8_t Base91_encode_u16(uint16_t number, uint8_t *buffer, uint8_t n);
uint8_t Base91_encode_u24(uint32_t number, uint8_t *buffer, uint8_t n);
uint8_t Base91_encode_u32(uint32_t number, uint8_t *buffer, uint8_t n);
uint8_t Base91_encode_u40(uint64_t number, uint8_t *buffer, uint8_t n);
uint16_t crc_ccitt_update(uint16_t crc, uint8_t data);


#endif // L21_APRS_H_