/*
	ZOE-M8B is ultra-small, highly integrated GNSS SiP (System in Package) module.
	A load switch on PA22 has to be enabled first to supply power to the module.
	
	Firmware version	SPG 3.51 (Standard Precision GNSS)
	Protocol version	23.01
	Receiver Channels	72
	
	UART INTERFACE (Asynchronous)
		baud	9600	(4800, 9600, 19200, 38400, 57600, 115200, 230400, 460800)
		mode	8N1		(Bits, Parity, Stop Bits)
	
	Default output: GGA, GLL, GSA, GSV, RMC, VTG and TXT NMEA messages.
	Default input automatically accepts UBX, NMEA and RTCM protocols.
	
	Operational Limits				Horizontal Position Accuracy (GPS)
		Altitude	50,000m				Continuous				2.5m
		Velocity	500m/s				Super-E (performance)	3.0m
		Dynamics	<4g					Super-E (power save)	3.5m
	
	Typical Consumption (GPS)
		Acquisition				34.5mA		Super-E (Power Save)	6.3mA
		Tracking (Continuous)	32.5mA		Software Backup			20uA
		Super-E (Performance)	7.3mA		Hardware Backup			15uA
	
	Defaults to Super-E mode on power-up.
		-On startup, the module uses the acquisition engine until sufficient number of satellites
		is acquired. The acquisition engine is active at least for 3 minutes after the receiver startup.
		Then uses duty-cycled, according to the signal strength, tracking engine to track the satellites.
		-1Hz (default), 2Hz, or 4Hz operation. A slower rate with an interval of 2–10s can be selected.
		-Two settings: PERFORMANCE (default) setting provides the best balance for power vs. performance.
		POWER SAVE setting provides up to an additional 15-20% power savings at the cost of accuracy.
		-In Super-E mode, the receiver needs to be able to track at least 6 - 8 satellites constantly.
		If too many of the known satellites are obscured, the receiver must restart the acquisition
		engine and stop power optimized tracking to read ephemeris data for the new satellites.
		-Navigation performance improves if ephemeris of many more satellites is known beforehand,
		because the receiver can then use new satellites even if several of the previously used
		satellites are out of view. The five-minute initial acquisition period on receiver startup
		helps to read the ephemeris of many satellites.
		
	Modes
		Continuous - best GNSS reception. Acquisition engine until all visible satellites acquired.
					 Tracking engine not duty-cycled.
		Super-E - optimized power consumption (default mode).
			Performance
			Power Save
		Backup - essential data for quick re-starting of navigation can be saved.
	
	Providing a V_BCKP supply maintains the time (RTC) and the GNSS orbit data in backup RAM. This ensures that
	any subsequent re-starts after a VCC power failure will benefit from the stored data, providing a faster
	Time To First Fix (TTFF).
	
	The difference between Hardware and Software Backup modes is that the receiver enters HW backup mode
	automatically when main	power supply is no longer powered. It enters SW backup mode when the host directs
	the receiver to go to backup mode with a UBX message.
	
	Two GNSS signals (GPS L1C/A, GLONASS L1OF, Galileo E1B/C and BeiDou B1) can be received and processed
	concurrently. In continuous mode, this concurrent operation is extended to three GNSS when GPS and Galileo
	are used in addition to GLONASS or BeiDou.
	
	By default the M8 receivers are configured for concurrent GPS and GLONASS, including SBAS and QZSS reception.
	If power consumption is a key factor, then the receiver should be configured for a single GNSS operation
	using GPS, GLONASS or BeiDou and disabling SBAS.
	
	AssistNow Autonomous provides aiding information based on previous broadcast satellite ephemeris data
	downloaded to and stored by the ZOE-M8B receiver. It automatically generates accurate predictions
	of satellite orbital data that is usable for future GNSS position fixes.
	
	BAUD RATE		8 bytes		50 bytes	100 bytes
		4800		16.7ms		104ms		208ms
		9600		8.3ms		52ms		104ms
		19200		4.2ms		26ms		52ms
		38400		2.1ms		13ms		26ms
		57600		1.39ms		8.7ms		17.4ms
		115200		0.69ms		4.3ms		8.7ms
		230400		0.35ms		2.2ms		4.3ms
		460800		0.17ms		1.1ms		2.2ms
	
	RESPONSE TIMES (at 9600 baud)
		ZOE_M8B_set_port					65.1ms
		ZOE_M8B_power_saving				38.0ms
		ZOE_M8B_set_dynamic_model			65.2ms
		ZOE_M8B_set_GNSS_system				92.5ms
		ZOE_M8B_get_dynamic_model			73.8ms
		ZOE_M8B_save_current_configuration	38.2ms
		ZOE_M8B_get_solution				periodic
*/


#ifndef L21_ZOE_M8B_H
#define L21_ZOE_M8B_H


#include "stdint.h"


#define ACK_TIMEOUT 80000										// ~26 clock cycles per 1 timeout, ~0.52s with 4MHz MCLK


// Functions
uint8_t ZOE_M8B_get_version(uint8_t * buffer, uint32_t timeout);
uint8_t ZOE_M8B_get_solution(uint8_t * buffer, uint32_t timeout);
uint8_t ZOE_M8B_get_NMEA_message(uint8_t message, uint8_t * buffer, uint32_t timeout);
uint8_t ZOE_M8B_get_dynamic_model(uint8_t * buffer, uint32_t timeout);
uint8_t ZOE_M8B_get_ANA_status(uint8_t * buffer, uint32_t timeout);
uint8_t ZOE_M8B_get_odometer_distance(uint8_t * buffer, uint32_t timeout);
uint8_t ZOE_M8B_get_dilution_of_precision(uint8_t * buffer, uint32_t timeout);
uint8_t ZOE_M8B_get_GPS_time_solution(uint8_t * buffer, uint32_t timeout);
uint8_t ZOE_M8B_get_navigation_status(uint8_t * buffer, uint32_t timeout);
uint8_t ZOE_M8B_get_GNSS_system(uint8_t * buffer, uint32_t timeout);
uint8_t ZOE_M8B_get_update_rate(uint8_t * buffer, uint32_t timeout);
uint8_t ZOE_M8B_get_port(uint8_t * buffer, uint32_t timeout);
uint8_t ZOE_M8B_get_power_mode(uint8_t * buffer, uint32_t timeout);
uint8_t ZOE_M8B_get_power_management(uint8_t * buffer, uint32_t timeout);
uint8_t ZOE_M8B_get_power_saving(uint8_t * buffer, uint32_t timeout);
uint8_t ZOE_M8B_get_AssistNow_Autonomous(uint8_t * buffer, uint32_t timeout);
uint8_t ZOE_M8B_get_odometer(uint8_t * buffer, uint32_t timeout);
uint8_t ZOE_M8B_get_SBAS(uint8_t * buffer, uint32_t timeout);
uint8_t ZOE_M8B_get_satellites(uint8_t * buffer, uint32_t timeout, uint8_t * length);

uint8_t ZOE_M8B_set_GNSS_system(uint8_t mode);
uint8_t ZOE_M8B_set_dynamic_model(uint8_t model);
uint8_t ZOE_M8B_set_update_rate(uint8_t rate);
void ZOE_M8B_set_port(uint8_t mode);
uint8_t ZOE_M8B_set_message_rate(uint8_t message);
uint8_t ZOE_M8B_set_power_mode(uint8_t mode);
uint8_t ZOE_M8B_set_power_management(uint8_t mode);

uint8_t ZOE_M8B_power_saving(uint8_t enable);
void ZOE_M8B_backup_mode(void);
void ZOE_M8B_reset(uint8_t mode);
void ZOE_M8B_wakeup_sequence(uint32_t delay_ms);
uint8_t ZOE_M8B_save_current_configuration(void);

uint8_t ZOE_M8B_AssistNow_Autonomous(uint8_t enable);
uint8_t ZOE_M8B_odometer(uint8_t enable);
uint8_t ZOE_M8B_odometer_reset(void);

void ZOE_M8B_send_message(uint8_t * message, uint8_t length);
uint8_t ZOE_M8B_receive_acknowledge(uint32_t timeout);
uint8_t ZOE_M8B_verify_checksum(uint8_t *buffer, uint8_t len);

void ZOE_M8B_parse_solution(uint8_t * buffer, uint16_t * year, uint8_t * month, uint8_t * day, uint8_t * hour,
							uint8_t * min, uint8_t * sec, uint8_t * valid, uint8_t * fixType, uint8_t * gnssFixOK,
							uint8_t * psmState, uint8_t * numSV, float * lon, float * lat, int32_t * hMLS,
							uint32_t * hAcc, uint32_t * vAcc, uint16_t * pDOP);
void ZOE_M8B_parse_dynamic_model(uint8_t * buffer, uint8_t * model);
void ZOE_M8B_parse_ANA_status(uint8_t * buffer, uint8_t * status);
void ZOE_M8B_parse_odometer_distance(uint8_t * buffer, uint32_t * distance, uint32_t * totalDistance);
void ZOE_M8B_parse_dilution_of_precision(uint8_t * buffer, uint16_t * gDOP, uint16_t * pDOP, uint16_t * tDOP,
										 uint16_t * vDOP, uint16_t * hDOP, uint16_t * nDOP, uint16_t * eDOP);
void ZOE_M8B_parse_satellites(uint8_t * buffer, uint8_t len, uint8_t * sats, uint8_t * sig1_10, uint8_t * sig11_20,
							  uint8_t * sig21_30, uint8_t * sig31_40, uint8_t * sig41);

void clear_buffer(uint8_t * buffer, uint8_t len);


#endif // L21_ZOE_M8B_H_