/*
	ZOE-M8B is ultra-small, highly integrated GNSS SiP (System in Package) module.
	A load switch on PA22 has to be enabled first to supply power to the module.
	
	Firmware version	SPG 3.51 (Standard Precision GNSS)
	Protocol version	23.01
	Receiver Channels	72
	
	UART INTERFACE (Asynchronous)
		baud	9600	(4800, 9600, 19200, 38400, 57600, 115200, 230400, 460800)
		mode	8N1		(Bits, Parity, Stop Bits)
	
	Default output: GGA, GLL, GSA, GSV, RMC, VTG and TXT NMEA messages. Input automatically accepts UBX, NMEA and RTCM protocols.
	
	Operational Limits				Horizontal Position Accuracy (GPS)		Typical Consumption (GPS)
		Altitude	50,000m				Continuous				2.5m			Acquisition				34.5mA		Super-E (Power Save)	6.3mA
		Velocity	500m/s				Super-E (performance)	3.0m			Tracking (Continuous)	32.5mA		Software Backup			20uA
		Dynamics	<4g					Super-E (power save)	3.5m			Super-E (Performance)	7.3mA		Hardware Backup			15uA
	
	Defaults to Super-E mode on power-up.
		-On startup, the module uses the acquisition engine until sufficient number of satellites is acquired.
		The acquisition engine is active at least for 3 minutes after the receiver startup.
		Then uses duty-cycled, according to the signal strength, tracking engine to track the satellites.
		-1Hz (default), 2Hz, or 4Hz operation. A slower operation rate with an interval of 2–10s can be selected.
		-Two settings: PERFORMANCE (default) setting provides the best balance for power vs. performance.
		POWER SAVE setting provides up to an additional 15-20% power savings at the cost of position accuracy.
		-In Super-E mode, the receiver needs to be able to track at least 6 - 8 satellites constantly.
		If too many of the currently known satellites are obscured, the receiver must restart the acquisition engine
		and stop power optimized tracking to read ephemeris data for the new satellites.
		-Navigation performance improves if ephemeris of many more satellites is known beforehand, because the receiver
		can then use new satellites even if several of the previously used satellites are out of view.
		The five-minute (default) initial acquisition period on receiver startup helps to read the ephemeris of many satellites.
		
	Modes
		Continuous - best GNSS reception. It uses the acquisition engine until all visible satellites are acquired. Tracking engine is not duty-cycled.
		Super-E - optimized power consumption (default mode).
			Performance
			Power Save
		Backup - essential data for quick re-starting of navigation can be saved.
	
	Providing a V_BCKP supply maintains the time (RTC) and the GNSS orbit data in backup RAM. This ensures that any subsequent
	re-starts after a VCC power failure will benefit from the stored data, providing a faster Time To First Fix (TTFF).
	
	The difference between Hardware and Software Backup modes is that the receiver enters HW backup mode automatically when main
	power supply is no longer powered. It enters SW backup mode when the host directs the receiver to go to backup mode with a UBX message.
	
	Two GNSS signals (GPS L1C/A, GLONASS L1OF, Galileo E1B/C and BeiDou B1) can be received and processed concurrently. In continuous mode,
	this concurrent operation is extended to three GNSS when GPS and Galileo are used in addition to GLONASS or BeiDou.
	
	By default the M8 receivers are configured for concurrent GPS and GLONASS, including SBAS and QZSS reception. If power consumption
	is a key factor, then the receiver should be configured for a single GNSS operation using GPS, GLONASS or BeiDou and disabling SBAS.
	
	AssistNow Autonomous provides aiding information based on previous broadcast satellite ephemeris data downloaded to and stored
	by the ZOE-M8B receiver. It automatically generates accurate predictions of satellite orbital data that is usable for future GNSS position fixes.
*/


#ifndef L21_ZOE_M8B_H
#define L21_ZOE_M8B_H


#include "stdint.h"


// Functions
void ZOE_M8B_get_version(void);
void ZOE_M8B_get_solution(void);
uint8_t ZOE_M8B_get_NMEA_message(uint8_t message, uint8_t * buffer);
void ZOE_M8B_get_dynamic_model(void);
void ZOE_M8B_get_ANA_status(void);
void ZOE_M8B_get_odometer_distance(void);

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
uint8_t ZOE_M8B_receive_acknowledge(void);
static uint8_t ZOE_M8B_verify_checksum(uint8_t *buffer, uint8_t len);


#endif // L21_ZOE_M8B_H_