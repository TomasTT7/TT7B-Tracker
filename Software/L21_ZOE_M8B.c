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
*/


#include "sam.h"
#include "L21_ZOE_M8B.h"
#include "L21_SERCOM_USART.h"
#include "L21_SysTick.h"


/*
	NMEA TALKER ID
		GP		GPS, SBAS, QZSS
		GL		GLONASS
		GA		Galileo
		GB		BeiDou
		GN		Any combination of GNSS
	
	EXAMPLE NMEA MESSAGES
		$GPDTM,999,,0.08,N,0.07,E,-47.7,W84*1C
		$EIGBQ,RMC*28
		$GPGBS,235458.00,1.4,1.3,3.1,03,,-21.4,3.8,1,0*5B
		$GPGGA,092725.00,4717.11399,N,00833.91590,E,1,08,1.01,499.6,M,48.0,M,,*5B
		$GPGLL,4717.11364,N,00833.91565,E,092321.00,A,A*60
		$EIGLQ,RMC*3A
		$EIGNQ,RMC*3A
		$GPGNS,091547.00,5114.50897,N,00012.28663,W,AA,10,0.83,111.1,45.6,,,V*71
		$EIGPQ,RMC*3A
		$GPGRS,082632.00,1,0.54,0.83,1.00,1.02,-2.12,2.64,-0.71,-1.18,0.25,,,1,0*70
		$GPGSA,A,3,23,29,07,08,09,18,26,28,,,,,1.94,1.18,1.54,1*0D
		$GPGST,082356.00,1.8,,,,1.7,1.3,2.2*7E
		$GPGSV,3,1,10,23,38,230,44,29,71,156,47,07,29,116,41,08,09,081,36,0*7F
		$GPRMC,083559.00,A,4717.11437,N,00833.91522,E,0.004,77.52,091202,,,A,V*57
		$GPTXT,01,01,02,u-blox ag - www.u-blox.com*50
		$GPVLW,,N,,N,15.8,N,1.2,N*06
		$GPVTG,77.52,T,,M,0.004,N,0.008,K,A*06
		$GPZDA,082710.00,16,09,2002,00,00*64
	
	EXAMPLE PROPRIETARY MESSAGES
		$PUBX,41,1,0007,0003,19200,0*25
		$PUBX,00,081350.00,4717.113210,N,00833.915187,E,546.589,G3,2.1,2.0,0.007,77.52,0.007,,0.92,1.19,0.77,9,0,0*5F
		$PUBX,40,GLL,1,0,0,0,0,0*5D
		$PUBX,03,11,23,-,,,45,010,29,-,,,46,013,07,-,,,42,015,08,U,067,31,42,025,10,U,195,33,46,026,18,U,326,08,39,026,
		17,-,,,32,015,26,U,306,66,48,025,27,U,073,10,36,026,28,U,089,61,46,024,15,-,,,39,014*0D
		$PUBX,04,073731.00,091202,113851.00,1196,15D,1930035,-2660.664,43,*3C
	
	UBX PROTOCOL
		When messages from the class CFG are sent to the receiver, the receiver will send an "acknowledge"
		(ACK-ACK) or a "not acknowledge" (ACK-NAK) message back. Some messages from other classes (LOG)
		also use the same mechanism.
		
		All messages that are output by the receiver in a periodic manner (i.e. messages in classes MON,
		NAV and RXM) can also be polled. The messages can be polled by sending the message required to
		the receiver but without a payload. The receiver then responds with the same message with
		the payload populated.
		
		gnssId	GNSS
		0		GPS
		1		SBAS
		2		Galileo
		3		BeiDou
		4		IMES
		5		QZSS
		6		GLONASS
*/


/*
	Output:
		Zero-terminated Software Version String		buffer[6:35]		Zero-terminated Hardware Version String		buffer[36:45]	
	UBX-MON-VER (48 + 30 * N bytes)
*/
uint8_t ZOE_M8B_get_version(uint8_t * buffer)
{
	static uint8_t requestUBX_MON_VER[8] = {0xB5, 0x62, 0x0A, 0x04, 0x00, 0x00, 0x0E, 0x34};
	ZOE_M8B_send_message(requestUBX_MON_VER, 8);
	
	for(uint8_t i = 0; i < 46; i++)
	{
		buffer[i] = SERCOM_USART_read_byte();
	}
	
	// unpredictable response length -> don't verify checksum
	
	if(buffer[2] == 0x0A && buffer[3] == 0x04) return 1;
	else return 0;
}


/*
	UBX-NAV-PVT (100 bytes)
*/
uint8_t ZOE_M8B_get_solution(uint8_t * buffer)
{
	static uint8_t requestUBX_NAV_PVT[8] = {0xB5, 0x62, 0x01, 0x07, 0x00, 0x00, 0x08, 0x19};
	ZOE_M8B_send_message(requestUBX_NAV_PVT, 8);
	
	for(uint8_t i = 0; i < 100; i++)
	{
		buffer[i] = SERCOM_USART_read_byte();
	}
	
	uint8_t checksum = ZOE_M8B_verify_checksum(buffer, 100);
	
	if(checksum) return 1;
	else return 0;
}


/*
	Requests an NMEA message. NMEA output must be enabled.
	Only GPS NMEA Talker (e.g. GPGGA).
	
	MESSAGE
		0	GPGGA
		1	GPRMC
		2	GPZDA
		3	GNGGA
		4	GNRMC
		5	GNZDA
	
	The default 1s periodic NMEA output rate must be changed to zero.
*/
uint8_t ZOE_M8B_get_NMEA_message(uint8_t message, uint8_t * buffer)
{
	switch(message)
	{
		case 0:;
			static uint8_t requestGPGGA[15] = "$EIGPQ,GGA*27\r\n";
			ZOE_M8B_send_message(requestGPGGA, 15);
			break;
		
		case 1:;
			static uint8_t requestGPRMC[15] = "$EIGPQ,RMC*3A\r\n";
			ZOE_M8B_send_message(requestGPRMC, 15);
			break;
		
		case 2:;
			static uint8_t requestGPZDA[15] = "$EIGPQ,ZDA*39\r\n";
			ZOE_M8B_send_message(requestGPZDA, 15);
			break;
			
		case 3:;
			static uint8_t requestGNGGA[15] = "$EIGNQ,GGA*39\r\n";
			ZOE_M8B_send_message(requestGNGGA, 15);
			break;
			
		case 4:;
			static uint8_t requestGNRMC[15] = "$EIGNQ,RMC*24\r\n";
			ZOE_M8B_send_message(requestGNRMC, 15);
			break;
			
		case 5:;
			static uint8_t requestGNZDA[15] = "$EIGNQ,ZDA*27\r\n";
			ZOE_M8B_send_message(requestGNZDA, 15);
			break;
			
		default:
			break;
	}
	
	uint8_t i = 0;
	
	while(1)
	{
		buffer[i++] = SERCOM_USART_read_byte();
		if(buffer[i-1] == 0x0D) return i;
	}
}


/*
	Navigation Engine Settings
	
	UBX-CFG-NAV5 (44 bytes)
*/
uint8_t ZOE_M8B_get_dynamic_model(uint8_t * buffer)
{
	static uint8_t requestUBX_CFG_NAV5[8] = {0xB5, 0x62, 0x06, 0x24, 0x00, 0x00, 0x2A, 0x84};
	ZOE_M8B_send_message(requestUBX_CFG_NAV5, 8);
	
	for(uint8_t i = 0; i < 44; i++)
	{
		buffer[i] = SERCOM_USART_read_byte();
	}
	
	uint8_t checksum = ZOE_M8B_verify_checksum(buffer, 44);
	
	if(checksum) return 1;
	else return 0;
}


/*
	The status indicates whether the AssistNow Autonomous subsystem is currently idle (or not enabled)
	or busy generating data or orbits. Hosts should monitor this information and only power-off
	the receiver when the subsystem is idle (that is, when the status field shows a steady zero).
	
	The calculations require energy and users may therefore occasionally see increased power consumption
	during short periods (several seconds, rarely more than 60 seconds) when such calculations are running.
	Ongoing calculations will automatically prevent the power save mode from entering the power-off state.
	The power-down will be delayed until all calculations are done.
	
	UBX-NAV-AOPSTATUS (24 bytes)
*/
uint8_t ZOE_M8B_get_ANA_status(uint8_t * buffer)
{
	static uint8_t requestUBX_NAV_AOPSTATUS[8] = {0xB5, 0x62, 0x01, 0x60, 0x00, 0x00, 0x61, 0x24};
	ZOE_M8B_send_message(requestUBX_NAV_AOPSTATUS, 8);
	
	for(uint8_t i = 0; i < 24; i++)
	{
		buffer[i] = SERCOM_USART_read_byte();
	}
	
	uint8_t checksum = ZOE_M8B_verify_checksum(buffer, 24);
	
	if(checksum) return 1;
	else return 0;
}


/*
	ODOMETER OUTPUT
		Ground distance since last reset		distance
		Ground distance accuracy				distanceStd
		Total cumulative ground distance		totalDistance
	
	UBX-NAV-ODO (28 bytes)
*/
uint8_t ZOE_M8B_get_odometer_distance(uint8_t * buffer)
{
	static uint8_t requestUBX_NAV_ODO[8] = {0xB5, 0x62, 0x01, 0x09, 0x00, 0x00, 0x0A, 0x1F};
	ZOE_M8B_send_message(requestUBX_NAV_ODO, 8);
	
	for(uint8_t i = 0; i < 28; i++)
	{
		buffer[i] = SERCOM_USART_read_byte();
	}
	
	uint8_t checksum = ZOE_M8B_verify_checksum(buffer, 28);
	
	if(checksum) return 1;
	else return 0;
}


/*
	Dilution of precision. All DOP values are scaled by a factor of 100. If the unit transmits a value
	of e.g. 156, the DOP value is 1.56.
	
	Geometric DOP
	Position DOP
	Time DOP
	Vertical DOP
	Horizontal DOP
	Northing DOP
	Easting DOP
	
	UBX-NAV-DOP (26 bytes)
*/
uint8_t ZOE_M8B_get_dilution_of_precision(uint8_t * buffer)
{
	static uint8_t requestUBX_NAV_DOP[8] = {0xB5, 0x62, 0x01, 0x04, 0x00, 0x00, 0x05, 0x10};
	ZOE_M8B_send_message(requestUBX_NAV_DOP, 8);
	
	for(uint8_t i = 0; i < 26; i++)
	{
		buffer[i] = SERCOM_USART_read_byte();
	}
	
	uint8_t checksum = ZOE_M8B_verify_checksum(buffer, 26);
	
	if(checksum) return 1;
	else return 0;
}


/*
	Specifies which GNSS signals should be processed along with limits on how many tracking channels
	should be allocated to each GNSS. To avoid cross-correlation issues, it is recommended that GPS
	and QZSS are always both enabled or both disabled.
	
	U-blox recommends that receivers are cold started after any change that disables an active GNSS.
	
	gnssId	GNSS
		0		GPS
		1		SBAS
		2		Galileo
		3		BeiDou
		4		IMES
		5		QZSS
		6		GLONASS
	
	MODE
	0	GPS, QZSS and GLONASS (default)
	1	GPS and QZSS only
	
	UBX-CFG-GNSS
*/
uint8_t ZOE_M8B_set_GNSS_system(uint8_t mode)
{
	switch(mode)
	{
		case 0:;
			static uint8_t gnss_default[68] = {0xB5, 0x62, 0x06, 0x3E, 0x3C, 0x00, 0x00, 0x00, 0x20, 0x07, 0x00, 0x08, 0x10, 0x00, 0x01, 0x00,
											   0x00, 0x01, 0x01, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x04, 0x08, 0x00, 0x00, 0x00,
											   0x00, 0x01, 0x03, 0x08, 0x10, 0x00, 0x00, 0x00, 0x00, 0x01, 0x04, 0x00, 0x08, 0x00, 0x00, 0x00,
											   0x00, 0x01, 0x05, 0x00, 0x03, 0x00, 0x01, 0x00, 0x00, 0x01, 0x06, 0x08, 0x0E, 0x00, 0x01, 0x00,
											   0x00, 0x01, 0x27, 0xA7};
			ZOE_M8B_send_message(gnss_default, 68);
			break;
			
		case 1:;
			static uint8_t gnss_gps[68] = {0xB5, 0x62, 0x06, 0x3E, 0x3C, 0x00, 0x00, 0x00, 0x20, 0x07, 0x00, 0x08, 0x10, 0x00, 0x01, 0x00,
										   0x00, 0x01, 0x01, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x04, 0x08, 0x00, 0x00, 0x00,
										   0x00, 0x01, 0x03, 0x08, 0x10, 0x00, 0x00, 0x00, 0x00, 0x01, 0x04, 0x00, 0x08, 0x00, 0x00, 0x00,
										   0x00, 0x01, 0x05, 0x00, 0x03, 0x00, 0x01, 0x00, 0x00, 0x01, 0x06, 0x08, 0x0E, 0x00, 0x00, 0x00,
										   0x00, 0x01, 0x26, 0xA3};
			ZOE_M8B_send_message(gnss_gps, 68);
			break;
			
		default:
			return 0;
	}
	
	uint8_t acknack = ZOE_M8B_receive_acknowledge();
	
	return acknack;
}


/*
	MODEL	PLATFORM		MAX ALT[m]	MAX H VEL.[m/s]	MAX V VEL.[m/s]	SANITY CHECK			MAX DEVIATION
	0		Portable		12000		310				50				Altitude and Velocity	Medium
	2		Stationary		9000		10				6				Altitude and Velocity	Small
	3		Pedestrian		9000		30				20				Altitude and Velocity	Small
	4		Automotive		6000		100				15				Altitude and Velocity	Medium
	5		At sea			500			25				5				Altitude and Velocity	Medium
	6		Airborne <1g	50000		100				100				Altitude				Large
	7		Airborne <2g	50000		250				100				Altitude				Large
	8		Airborne <4g	50000		500				100				Altitude				Large
	9		Wrist			9000		30				20				Altitude and Velocity	Medium
	
	UBX-CFG-NAV5
*/
uint8_t ZOE_M8B_set_dynamic_model(uint8_t model)
{
	switch(model)
	{
		case 0:;
			static uint8_t Portable[44] = {0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27,
										   0x00, 0x00, 0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00,
										   0x10, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x47, 0x0F};
			ZOE_M8B_send_message(Portable, 44);
			break;
		
		case 2:
			return 0;
		
		case 3:
			return 0;
		
		case 4:
			return 0;
		
		case 5:
			return 0;
		
		case 6:;
			static uint8_t Airborne1g[44] = {0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27,
											 0x00, 0x00, 0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00,
											 0x10, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4D, 0xDB};
			ZOE_M8B_send_message(Airborne1g, 44);
			break;
		
		case 7:
			return 0;
		
		case 8:
			return 0;
		
		case 9:
			return 0;
		
		default:
			return 0;
	}
	
	uint8_t acknack = ZOE_M8B_receive_acknowledge();
	
	return acknack;
}


/*
	This message allows the user to alter the rate at which navigation solutions (and the measurements
	that they depend on) are generated by the receiver. Each measurement triggers the measurements generation
	and raw data output. The navRate value defines that every nth measurement triggers a navigation epoch.
	
	The update rate has a direct influence on the power consumption. The more fixes that are required,
	the more CPU power and communication resources are required.

	When using Power Save Mode, measurement and navigation rate can differ from the values configured here.
	
	RATE
	0	1Hz Measurement frequency, 1Hz Navigation frequency
	1	2Hz Measurement frequency, 2Hz Navigation frequency
	2	4Hz Measurement frequency, 4Hz Navigation frequency
	
	UBX-CFG-RATE
*/
uint8_t ZOE_M8B_set_update_rate(uint8_t rate)
{
	switch(rate)
	{
		case 0:;
			static uint8_t rate1Hz[14] = {0xB5, 0x62, 0x06, 0x08, 0x06, 0x00, 0xE8, 0x03, 0x01, 0x00, 0x01, 0x00, 0x01, 0x39};
			ZOE_M8B_send_message(rate1Hz, 14);
			break;
			
		case 1:;
			static uint8_t rate2Hz[14] = {0xB5, 0x62, 0x06, 0x08, 0x06, 0x00, 0xF4, 0x01, 0x01, 0x00, 0x01, 0x00, 0x0B, 0x77};
			ZOE_M8B_send_message(rate2Hz, 14);
			break;
			
		case 2:;
			static uint8_t rate4Hz[14] = {0xB5, 0x62, 0x06, 0x08, 0x06, 0x00, 0xFA, 0x00, 0x01, 0x00, 0x01, 0x00, 0x10, 0x96};
			ZOE_M8B_send_message(rate4Hz, 14);
			break;
		
		default:
			return 0;
	}
	
	uint8_t acknack = ZOE_M8B_receive_acknowledge();
	
	return acknack;
}


/*
	UART BAUD RATE		UBX-NAV-PVT (100 byte)
		0	4800		208ms
		1	9600		104ms
		2	19200		52ms
		3	38400		26ms
		4	57600		17ms
		5	115200		8.7ms
		6	230400		4.3ms
		7	460800		2.2ms
	
	MODE	BAUD RATE	PROTOCOL OUT	PROTOCOL IN
	0		9600		UBX, NMEA		UBX, NMEA, RTCM
	1		9600		UBX				UBX, NMEA, RTCM
	2		115200		UBX, NMEA		UBX, NMEA, RTCM
	3		115200		UBX				UBX, NMEA, RTCM
	
	UBX-CFG-PRT
*/
void ZOE_M8B_set_port(uint8_t mode)
{
	switch(mode)
	{
		case 0:;
			static uint8_t slowNMEA[28] = {0xB5, 0x62, 0x06, 0x00, 0x14, 0x00, 0x01, 0x00, 0x00, 0x00, 0xD0, 0x08, 0x00, 0x00, 0x80, 0x25,
										   0x00, 0x00, 0x07, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA2, 0xB5};
			ZOE_M8B_send_message(slowNMEA, 28);
			break;
		
		case 1:;
			static uint8_t slowUBX[28] = {0xB5, 0x62, 0x06, 0x00, 0x14, 0x00, 0x01, 0x00, 0x00, 0x00, 0xD0, 0x08, 0x00, 0x00, 0x80, 0x25,
										  0x00, 0x00, 0x07, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA0, 0xA9};
			ZOE_M8B_send_message(slowUBX, 28);
			break;
			
		case 2:;
			static uint8_t fastNMEA[28] = {0xB5, 0x62, 0x06, 0x00, 0x14, 0x00, 0x01, 0x00, 0x00, 0x00, 0xD0, 0x08, 0x00, 0x00, 0x00, 0xC2,
										   0x01, 0x00, 0x07, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x7E};
			ZOE_M8B_send_message(fastNMEA, 28);
			break;
			
		case 3:;
			static uint8_t fastUBX[28] = {0xB5, 0x62, 0x06, 0x00, 0x14, 0x00, 0x01, 0x00, 0x00, 0x00, 0xD0, 0x08, 0x00, 0x00, 0x00, 0xC2,
										  0x01, 0x00, 0x07, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBE, 0x72};
			ZOE_M8B_send_message(fastUBX, 28);
			break;
		
		default:
			break;
	}
	
	// Don't wait for ACK/NACK. The baud rate may be different.
}


/*
	If the rate of a navigation message is set to 2, the message is sent every second navigation solution.
	
	MESSAGE
	0	GGA rate 0
	1	GLL rate 0
	2	GSA rate 0
	3	GSV rate 0
	4	RMC rate 0
	5	VTG rate 0
	
	UBX-CFG-MSG
*/
uint8_t ZOE_M8B_set_message_rate(uint8_t message)
{
	switch(message)
	{
		case 0:;
			static uint8_t GGArate0[16] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x23};
			ZOE_M8B_send_message(GGArate0, 16);
			break;
		
		case 1:;
			static uint8_t GLLrate0[16] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2A};
			ZOE_M8B_send_message(GLLrate0, 16);
			break;
			
		case 2:;
			static uint8_t GSArate0[16] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x31};
			ZOE_M8B_send_message(GSArate0, 16);
			break;
			
		case 3:;
			static uint8_t GSVrate0[16] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x38};
			ZOE_M8B_send_message(GSVrate0, 16);
			break;
			
		case 4:;
			static uint8_t RMCrate0[16] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x3F};
			ZOE_M8B_send_message(RMCrate0, 16);
			break;
			
		case 5:;
			static uint8_t VTGrate0[16] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x46};
			ZOE_M8B_send_message(VTGrate0, 16);
			break;
		
		default:
			return 0;
	}
	
	uint8_t acknack = ZOE_M8B_receive_acknowledge();
	
	return acknack;
}


/*
	Selecting one of the available setups is the equivalent of using a combination of the configuration
	messages with appropriate parameters that impact the power consumption of the receiver.
	
	MODE	SETUP NAME			DESCRIPTION
	0		Full Power			No compromises on power saves
	1		Balanced			Power savings without performance degradation
	2		Interval			ON OFF mode setup
	3		Aggressive 1Hz		Best power saving setup (1Hz rate) (corresponds to Super-E mode PERFORMANCE)
	4		Aggressive 2Hz		Excellent power saving setup (2Hz rate)
	5		Aggressive 4Hz		Good power saving setup (4Hz rate)
	
	In 4Hz mode, when running a flash firmware, it is recommended to run with a subset of GNSS
	systems, to avoid system overload.
	
	Using UBX-CFG-PMS to set Super-E mode 1, 2, 4Hz navigation rates sets 180s minAcqTime instead
	the default 300s in protocol version 23.01. 300s is recommended for the best performance.
	
	Sending UBX-CFG-PMS message resets the UBX-CFG-PM2 settings. Always first send UBX-CFG-PMS message
	followed by UBX-CFG-PM2 if further configuration is needed.
	
	UBX-CFG-PMS
*/
uint8_t ZOE_M8B_set_power_mode(uint8_t mode)
{
	switch(mode)
	{
		case 0:;
			static uint8_t FullPower[16] = {0xB5, 0x62, 0x06, 0x86, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x94, 0x5A};
			ZOE_M8B_send_message(FullPower, 16);
			break;
		
		case 1:;
			static uint8_t Balanced[16] = {0xB5, 0x62, 0x06, 0x86, 0x08, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x95, 0x61};
			ZOE_M8B_send_message(Balanced, 16);
			break;
		
		case 2:;
			static uint8_t Interval[16] = {0xB5, 0x62, 0x06, 0x86, 0x08, 0x00, 0x00, 0x02, 0x0A, 0x00, 0x01, 0x00, 0x00, 0x00, 0xA1, 0xA8};
			ZOE_M8B_send_message(Interval, 16);
			break;
		
		case 3:;
			static uint8_t Aggressive1Hz[16] = {0xB5, 0x62, 0x06, 0x86, 0x08, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x97, 0x6F};
			ZOE_M8B_send_message(Aggressive1Hz, 16);
			break;
		
		case 4:;
			static uint8_t Aggressive2Hz[16] = {0xB5, 0x62, 0x06, 0x86, 0x08, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x98, 0x76};
			ZOE_M8B_send_message(Aggressive2Hz, 16);
			break;
		
		case 5:;
			static uint8_t Aggressive4Hz[16] = {0xB5, 0x62, 0x06, 0x86, 0x08, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x99, 0x7D};
			ZOE_M8B_send_message(Aggressive4Hz, 16);
			break;
			
		default:
			return 0;
	}
	
	uint8_t acknack = ZOE_M8B_receive_acknowledge();
	
	return acknack;
}


/*
	For update rates from 1 Hz to 4 Hz, always use UBX-CFG-PMS message to set the update rate. If further
	configuration with UBX-CFG-PM2 is needed, the field updatePeriod in UBX-CFG-PM2 message must exactly
	match the update rate set with UBX-CFG-PMS message. For example, if 2 Hz update rate is selected
	with UBX-CFG-PMS, the field updatePeriod in UBX-CFG-PM2 must be 500 ms.
	
	For update periods longer than 1 s (up to 10 s), first select 1 Hz update rate with UBX-CFG-PMS message,
	followed by UBX-CFG-PM2 message with the desired value for updatePeriod between 1–10 s.
	
	MODE
	0	performance (default)
	1	power save
	
	UBX-CFG-PM2
*/
uint8_t ZOE_M8B_set_power_management(uint8_t mode)
{
	switch(mode)
	{
		case 0:;
			/*static uint8_t performance[56] = {0xB5, 0x62, 0x06, 0x3B, 0x30, 0x00, 0x02, 0x06, 0x00, 0x00, 0x00, 0x80, 0x43, 0x01, 0xE8, 0x03,
											  0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2C, 0x01, 0x2C, 0x01,
											  0x00, 0x00, 0x4F, 0xC1, 0x03, 0x00, 0x87, 0x02, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x64, 0x40,
											  0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF9, 0xA9};*/ // u-center (18.00 protocol version)
			static uint8_t performance[56] = {0xB5, 0x62, 0x06, 0x3B, 0x30, 0x00, 0x02, 0x06, 0x00, 0x00, 0x00, 0x00, 0x43, 0x01, 0xE8, 0x03,
											  0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2C, 0x01, 0x2C, 0x01,
											  0x00, 0x00, 0xCF, 0x41, 0x00, 0x00, 0x88, 0x6A, 0xA4, 0x46, 0xFE, 0x00, 0x00, 0x00, 0x40, 0x00,
											  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x63, 0x2F}; // ZOE-M8B System Integration Manual
			ZOE_M8B_send_message(performance, 56);
			break;
		
		case 1:;
			/*static uint8_t powersave[56] = {0xB5, 0x62, 0x06, 0x3B, 0x30, 0x00, 0x02, 0x06, 0x00, 0x00, 0x02, 0x80, 0x43, 0x01, 0xE8, 0x03,
											0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2C, 0x01, 0x2C, 0x01,
											0x00, 0x00, 0x4F, 0xC1, 0x03, 0x00, 0x87, 0x02, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x64, 0x40,
											0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFB, 0x01};*/ //  u-center (18.00 protocol version)
			static uint8_t powersave[56] = {0xB5, 0x62, 0x06, 0x3B, 0x30, 0x00, 0x02, 0x06, 0x00, 0x00, 0x02, 0x00, 0x43, 0x01, 0xE8, 0x03,
											0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2C, 0x01, 0x2C, 0x01,
											0x00, 0x00, 0xCF, 0x40, 0x00, 0x00, 0x87, 0x5A, 0xA4, 0x46, 0xFE, 0x00, 0x00, 0x00, 0x20, 0x00,
											0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x33, 0x74}; // ZOE-M8B System Integration Manual
			ZOE_M8B_send_message(powersave, 56);
			break;
		
		default:
			return 0;
	}
	
	uint8_t acknack = ZOE_M8B_receive_acknowledge();
	
	return acknack;
}


/*
	The power save mode (PSM) configured in UBX-CFG-PMS and UBX-CFG-PM2 is turned on/off with this command.
	
	If balanced operation (UBX-CFG-PMS) is selected for the continuous mode, then some GNSS RF operations
	are optimized. This reduces the power consumption slightly for the tracking phase.
	
	ENABLE
	0	Continuous mode
	1	Power Save mode
	
	UBX-CFG-RXM
*/
uint8_t ZOE_M8B_power_saving(uint8_t enable)
{
	if(enable)
	{
		static uint8_t powersave[10] = {0xB5, 0x62, 0x06, 0x11, 0x02, 0x00, 0x08, 0x01, 0x22, 0x92};
		ZOE_M8B_send_message(powersave, 10);
	}
	else
	{
		static uint8_t continuous[10] = {0xB5, 0x62, 0x06, 0x11, 0x02, 0x00, 0x08, 0x00, 0x21, 0x91};
		ZOE_M8B_send_message(continuous, 10);
	}
	
	uint8_t acknack = ZOE_M8B_receive_acknowledge();
	
	return acknack;
}


/*
	The receiver is forced to enter Inactive state (SOFTWARE BACKUP).
	Wakeup the receiver if there is an edge on the UART RX pin.
	
	UBX-RXM-PMREQ
*/
void ZOE_M8B_backup_mode(void)
{
	static uint8_t backup[24] = {0xB5, 0x62, 0x02, 0x41, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00,
								 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x5D, 0x4B};
	ZOE_M8B_send_message(backup, 24);
}


/*
	RESET TYPE
	0x00	Hardware reset (Watchdog) immediately
	0x01	Controlled Software reset
	0x02	Controlled Software reset (GNSS only)
	0x04	Hardware reset (Watchdog) after shutdown
	0x08	Controlled GNSS stop
	0x09	Controlled GNSS start
	
	BRB MASK
	0x0000	Hot start
	0x0001	Warm start
	0xFFFF	Cold start
	
	MODE
		0	Hot start (Hardware reset (Watchdog) immediately)
		1	Warm start (Hardware reset (Watchdog) immediately)
		2	Cold start (Hardware reset (Watchdog) immediately)
	
	UBX-CFG-RST
*/
void ZOE_M8B_reset(uint8_t mode)
{
	switch(mode)
	{
		case 0:;
			static uint8_t hot[12] = {0xB5, 0x62, 0x06, 0x04, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0E, 0x64};
			ZOE_M8B_send_message(hot, 12);
			break;
			
		case 1:;
			static uint8_t warm[12] = {0xB5, 0x62, 0x06, 0x04, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0F, 0x68};
			ZOE_M8B_send_message(warm, 12);
			break;
			
		case 2:;
			static uint8_t cold[12] = {0xB5, 0x62, 0x06, 0x04, 0x04, 0x00, 0xFF, 0xB9, 0x00, 0x00, 0xC6, 0x8B};
			ZOE_M8B_send_message(cold, 12);
			break;
			
		default:
			break;
	}
	
	// Don't expect this message to be acknowledged by the receiver.
}


/*
	Send a dummy sequence of 0xFF to the receiver's UART interface. This will wake up the receiver
	if it is in Inactive state. If the receiver is not in Inactive state, the sequence will be ignored.
	
	Send following messages about half a second after the dummy sequence. If the interval between
	the dummy sequence and the next message is too short, the receiver may not yet be ready. It is
	therefore important to check for a UBX-ACK-ACK reply from the receiver to confirm that
	the configuration message was received.
*/
void ZOE_M8B_wakeup_sequence(uint32_t delay_ms)
{
	static uint8_t dummy[1] = {0xFF};
	ZOE_M8B_send_message(dummy, 1);
	
	if(delay_ms > 0) SysTick_delay_ms(delay_ms);
}


/*
	The Current Configuration is stored in the volatile RAM of the u-blox receiver.
	It can be made permanent by storring it in the on-chip BBR (battery backed RAM).
	
	UBX-CFG-CFG
*/
uint8_t ZOE_M8B_save_current_configuration(void)
{
	static uint8_t saveConfiguration[21] = {0xB5, 0x62, 0x06, 0x09, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
											0x00, 0x00, 0x01, 0x1B, 0xA9};
	ZOE_M8B_send_message(saveConfiguration, 21);
	
	uint8_t acknack = ZOE_M8B_receive_acknowledge();
	
	return acknack;
}


/*
	AssistNow Autonomous is disabled by default. Once enabled, the receiver will automatically produce
	data for newly received broadcast ephemerides and, if that data is available, automatically provide
	the navigation subsystem with orbits when necessary and adequate.
	
	Default orbit data validity of approximately three days (for GPS satellites observed once) and up to
	six days (for GPS and GLONASS satellites observed multiple times over a period of at least half a day).
	
	Enabling the AssistNow Autonomous feature will lead to increased power consumption while prediction
	is calculated. Therefore for each application special care must be taken to judge whether AssistNow
	Autonomous is beneficial to the overall power consumption or not.
	
	UBX-CFG-NAVX5
*/
uint8_t ZOE_M8B_AssistNow_Autonomous(uint8_t enable)
{
	if(enable)
	{
		static uint8_t ANAenable[52] = {0xB5, 0x62, 0x06, 0x23, 0x2C, 0x00, 0x03, 0x00, 0x4C, 0x66, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00,
										0x03, 0x20, 0x06, 0x00, 0x01, 0x00, 0x00, 0x00, 0x90, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
										0x00, 0x01, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
										0x00, 0x00, 0xF0, 0x03};
		ZOE_M8B_send_message(ANAenable, 52);
	}
	else
	{
		static uint8_t ANAdisable[52] = {0xB5, 0x62, 0x06, 0x23, 0x2C, 0x00, 0x03, 0x00, 0x4C, 0x66, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00,
										 0x03, 0x20, 0x06, 0x00, 0x01, 0x00, 0x00, 0x00, 0x90, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
										 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
										 0x00, 0x00, 0xEF, 0xF2};
		ZOE_M8B_send_message(ANAdisable, 52);
	}
	
	uint8_t acknack = ZOE_M8B_receive_acknowledge();
	
	return acknack;
}


/*
	ODOMETER OUTPUT
		Ground distance since last reset		distance
		Ground distance accuracy				distanceStd
		Total cumulative ground distance		totalDistance
		
	In ZOE-M8B the odometer feature is enabled by default
	
	UBX-CFG-ODO
*/
uint8_t ZOE_M8B_odometer(uint8_t enable)
{
	if(enable)
	{
		static uint8_t ODOenable[28] = {0xB5, 0x62, 0x06, 0x1E, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00,
										0x00, 0x00, 0x0A, 0x32, 0x00, 0x00, 0x99, 0x4C, 0x00, 0x00, 0x68, 0xE0};
		ZOE_M8B_send_message(ODOenable, 28);
	}
	else
	{
		static uint8_t ODOdisable[28] = {0xB5, 0x62, 0x06, 0x1E, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00,
										 0x00, 0x00, 0x0A, 0x32, 0x00, 0x00, 0x99, 0x4C, 0x00, 0x00, 0x67, 0xD0};
		ZOE_M8B_send_message(ODOdisable, 28);
	}
	
	uint8_t acknack = ZOE_M8B_receive_acknowledge();
	
	return acknack;
}


/*
	UBX-NAV-RESETODO
*/
uint8_t ZOE_M8B_odometer_reset(void)
{
	static uint8_t resetodo[8] = {0xB5, 0x62, 0x01, 0x10, 0x00, 0x00, 0x11, 0x34};
	ZOE_M8B_send_message(resetodo, 8);
	
	uint8_t acknack = ZOE_M8B_receive_acknowledge();
	
	return acknack;
}


/*
	Sends LENGTH bytes of MESSAGE to the GPS module.
*/
void ZOE_M8B_send_message(uint8_t * message, uint8_t length)
{
	for(uint8_t i = 0; i < length; i++)
	{
		SERCOM_USART_write_byte(*message++);
	}
}


/*
	Waits for an Acknowledged or Not-Acknowledged message from the module.
	
		UBX-ACK-ACK		0xB5 0x62 0x05 0x01 0xXX 0xXX 0xXX 0xXX
		UBX-ACK-NCK		0xB5 0x62 0x05 0x00 0xXX 0xXX 0xXX 0xXX
	
	Ack/Nak Messages: i.e. Acknowledge or Reject messages to CFG input messages.
*/
uint8_t ZOE_M8B_receive_acknowledge(void)
{
	uint8_t buffer[10];
	
	for(uint8_t i = 0; i < 10; i++)
	{
		buffer[i] = SERCOM_USART_read_byte();
	}
	
	if(buffer[2] == 0x05 && buffer[3] == 0x01) return 1;
	else return 0;
}


/*
	Verifies the checksum of received UBX messages.
*/
uint8_t ZOE_M8B_verify_checksum(uint8_t *buffer, uint8_t len)
{
	uint8_t CK_A_comp = 0, CK_B_comp = 0;
	
	for(uint8_t i = 2; i < len-2; i++)
	{
		CK_A_comp = CK_A_comp + buffer[i];
		CK_B_comp = CK_A_comp + CK_B_comp;
	}
	
	if(buffer[len-2] == CK_A_comp && buffer[len-1] == CK_B_comp) return 1;
	else return 0;
}


/*
	UBX-NAV-PVT in BUFFER.
	
	YEAR		Year (UTC)
	MONTH		Month, range 1..12 (UTC)
	DAY			Day of month, range 1..31 (UTC)
	HOUR		Hour of day, range 0..23 (UTC)
	MIN			Minute of hour, range 0..59 (UTC)
	SEC			Seconds of minute, range 0..60 (UTC)	VALID		Bit[0] validDate		1 = valid UTC Date				Bit[1] validTime		1 = valid UTC Time of Day				Bit[2] fullyResolved	1 = UTC Time of Day has been fully resolved (no seconds uncertainty)				Bit[3] validMag			1 = valid Magnetic declination	FIXTYPE		0: no fix
				1: dead reckoning only
				2: 2D-fix
				3: 3D-fix
				4: GNSS + dead reckoning combined
				5: time only fix	GNSSFIXOK	1 = valid fix (i.e within DOP & accuracy masks)	PSMSTATE	0: PSM is not active
				1: Enabled (an intermediate state before Acquisition state
				2: Acquisition
				3: Tracking
				4: Power Optimized Tracking
				5: Inactive
	NUMSV		Number of satellites used in Navigation Solution
	LON			Longitude
	LAT			Latitude
	HMSL		Height above mean sea level
*/
void ZOE_M8B_parse_solution(uint8_t * buffer, uint16_t * year, uint8_t * month, uint8_t * day, uint8_t * hour,
							uint8_t * min, uint8_t * sec, uint8_t * valid, uint8_t * fixType, uint8_t * gnssFixOK,
							uint8_t * psmState, uint8_t * numSV, float * lon, float * lat, int32_t * hMLS)
{
	*year = ((uint16_t)buffer[11] << 8) | (uint16_t)buffer[10];
	*month = buffer[12];
	*day = buffer[13];
	*hour = buffer[14];
	*min = buffer[15];
	*sec = buffer[16];
	*valid = buffer[17];
	*fixType = buffer[26];
	*gnssFixOK = buffer[27] & 0x01;
	*psmState = (buffer[27] >> 2) & 0x07;
	*numSV = buffer[29];
	int32_t _lon = ((int32_t)buffer[33] << 24) | ((int32_t)buffer[32] << 16) | ((int32_t)buffer[31] << 8) | (int32_t)buffer[30];
	*lon = (float)_lon / 10000000.0;
	int32_t _lat = ((int32_t)buffer[37] << 24) | ((int32_t)buffer[36] << 16) | ((int32_t)buffer[35] << 8) | (int32_t)buffer[34];
	*lat = (float)_lat / 10000000.0;
	*hMLS = (((int32_t)buffer[45] << 24) | ((int32_t)buffer[44] << 16) | ((int32_t)buffer[43] << 8) | (int32_t)buffer[42]) / 1000;
}


/*
	UBX-CFG-NAV5 in BUFFER. MODEL points to target variable.
	
	MODEL
		0	portable
		2	stationary
		3	pedestrian
		4	automotive
		5	sea
		6	airborne with <1g acceleration
		7	airborne with <2g acceleration
		8	airborne with <4g acceleration
		9	wrist worn watch
*/
void ZOE_M8B_parse_dynamic_model(uint8_t * buffer, uint8_t * model)
{
	*model = buffer[8];
}


/*
	UBX-NAV-AOPSTATUS in BUFFER. STATUS points to target variable.
	
	STATUS
		0		AssistNow Autonomous subsystem is idle
		not 0	AssistNow Autonomous subsystem is running
*/
void ZOE_M8B_parse_ANA_status(uint8_t * buffer, uint8_t * status)
{
	*status = buffer[11];
}


/*
	UBX-NAV-ODO in BUFFER. DISTANCE and TOTALDISTANCE point to target variables.
	
	DISTANCE		[m] ground distance since last reset
	TOTALDISTANCE	[m] total cumulative ground distance
*/
void ZOE_M8B_parse_odometer_distance(uint8_t * buffer, uint32_t * distance, uint32_t * totalDistance)
{
	*distance = ((uint32_t)buffer[17] << 24) | ((uint32_t)buffer[16] << 16) | ((uint32_t)buffer[15] << 8) | (uint32_t)buffer[14];
	*totalDistance = ((uint32_t)buffer[21] << 24) | ((uint32_t)buffer[20] << 16) | ((uint32_t)buffer[19] << 8) | (uint32_t)buffer[18];
}


/*
	UBX-NAV-DOP in BUFFER.
	
	Geometric DOP
	Position DOP
	Time DOP
	Vertical DOP
	Horizontal DOP
	Northing DOP
	Easting DOP
	
	All DOP values are scaled by a factor of 100.
*/
void ZOE_M8B_parse_dilution_of_precision(uint8_t * buffer, uint16_t * gDOP, uint16_t * pDOP, uint16_t * tDOP,
										 uint16_t * vDOP, uint16_t * hDOP, uint16_t * nDOP, uint16_t * eDOP)
{
	*gDOP = ((uint16_t)buffer[11] << 8) | (uint16_t)buffer[10];
	*pDOP = ((uint16_t)buffer[13] << 8) | (uint16_t)buffer[12];
	*tDOP = ((uint16_t)buffer[15] << 8) | (uint16_t)buffer[14];
	*vDOP = ((uint16_t)buffer[17] << 8) | (uint16_t)buffer[16];
	*hDOP = ((uint16_t)buffer[19] << 8) | (uint16_t)buffer[18];
	*nDOP = ((uint16_t)buffer[21] << 8) | (uint16_t)buffer[20];
	*eDOP = ((uint16_t)buffer[23] << 8) | (uint16_t)buffer[22];
}