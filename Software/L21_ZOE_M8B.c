/*
	
*/


#include "sam.h"
#include "L21_ZOE_M8B.h"
#include "L21_SERCOM_USART.h"
#include "L21_SysTick.h"


/*
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
*/


/*
	
*/
void ZOE_M8B_get_version(void)
{
	// UBX-MON-VER
	/* VERSION								response 48 + 30 * N bytes */
	static uint8_t request0A04[8] = {0xB5, 0x62, 0x0A, 0x04, 0x00, 0x00, 0x0E, 0x34};
}


/*
	
*/
void ZOE_M8B_get_solution(void)
{
	// UBX-NAV-PVT
	// validDate, validTime, confirmedDate, confirmedTime flags
	/* EVERYTHING							response 100 bytes (UBLOX 8), 92 bytes (UBLOX 7) */
	static uint8_t request0107[8] = {0xB5, 0x62, 0x01, 0x07, 0x00, 0x00, 0x08, 0x19};
}


/*
	
*/
void ZOE_M8B_get_NMEA_message(uint8_t message)
{
	switch(message)
	{
		case 0:
			
			break;
		
		default:
			break;
	}
}


/*
	The status indicates whether the AssistNow Autonomous subsystem is currently idle (or not enabled)
	or busy generating data or orbits. Hosts should monitor this information and only power-off
	the receiver when the subsystem is idle (that is, when the status field shows a steady zero).
	
	The calculations require energy and users may therefore occasionally see increased power consumption during
	short periods (several seconds, rarely more than 60 seconds) when such calculations are running.
	Ongoing calculations will automatically prevent the power save mode from entering the power-off state.
	The power-down will be delayed until all calculations are done.
*/
void ZOE_M8B_get_ANA_status(void)
{
	// UBX-NAV-AOPSTATUS
}


// UBX-CFG-PMS - power mode
// UBX-CFG-PM2 - update periods longer than 1s
// UBX-CFG-PM2 - optTarget bit. Super-E mode settings (performance/power save)
// UBX-NAV-PVT - psmState bit. state of the receiver info
// try 2/4Hz navigation rate to shorten time to first lock after wake up (?)
// power save with 1Hz:				B5 62 06 3B 30 00 02 06 00 00 00 00 43 01 E8 03 00 00 10 27 00 00 00 00 00 00 00 00 2C 01 2C 01 00 00 CF 41 00 00 88 6A A4 46 FE 00 00 00 40 00 00 00 00 00 00 00 63 2F
// performance (default) with 1Hz:	B5 62 06 3B 30 00 02 06 00 00 02 00 43 01 E8 03 00 00 10 27 00 00 00 00 00 00 00 00 2C 01 2C 01 00 00 CF 40 00 00 87 5A A4 46 FE 00 00 00 20 00 00 00 00 00 00 00 33 74
// Sending UBX-CFG-PMS message resets the UBX-CFG-PM2 settings. Always first send UBX-CFG-PMS message followed by UBX-CFG-PM2 if further configuration is needed.
// For update rates from 1 Hz to 4 Hz, always use UBX-CFG-PMS message to set the update rate. If further configuration with UBX-CFG-PM2 is needed, the field updatePeriod in UBX-CFG-PM2 message must exactly match the update rate set with UBX-CFG-PMS message. For example, if 2 Hz update rate is selected with UBX-CFG-PMS, the field updatePeriod in UBX-CFG-PM2 must be 500 ms.
// Using Multi-GNSS Assistance data on receiver start-up can improve the start-up performance. Multi-GNSS Assistance data ensures minimal power consumption, since A-GNSS enables the chip to maximize its poweroptimized period.
// leave only GGA or RMC for start
// Current Configuration - changed by UBX-CFG-xxx messages
// UBX-CFG-CFG - saves the current configuration to non-volatile memory
// UBX-CFG-GNSS - allows to specify which GNSS signals should be processed along with limits on how many tracking channels should be allocated to each GNSS
// UBX-CFG-NAV5 - dynamic platform models
// valid fix - check gnssFixOK flag in the UBX-NAV-PVT
// UBX-CFG-NAVX5 - AssistNow Autonomous enable
// UBX-RXM-PMREQ - the receiver can be forced to enter Inactive state


/*
	The Current Configuration is stored in the volatile RAM of the u-blox receiver.
	It can be made permanent by storring it in the on-chip BBR (battery backed RAM).
*/
void ZOE_M8B_save_current_configuration(void)
{
	// UBX-CFG-CFG
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
*/
void ZOE_M8B_set_dynamic_model(uint8_t model)
{
	// UBX-CFG-NAV5
}


/*
	
*/
void ZOE_M8B_set_update_rate(uint8_t rate)
{
	// UBX-CFG-RATE
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
*/
void ZOE_M8B_set_baud_rate(uint8_t baud)
{
	// UBX-CFG-PRT
}


/*
	MODE	SETUP NAME			DESCRIPTION
	0		Full Power			No compromises on power saves
	1		Balanced			Power savings without performance degradation
	2		Interval			ON OFF mode setup
	3		Aggressive 1Hz		Best power saving setup (1Hz rate). This corresponds to Super-E mode performance setting.
	4		Aggressive 2Hz		Excellent power saving setup (2Hz rate)
	5		Aggressive 4Hz		Good power saving setup (4Hz rate)
	
	In 4Hz mode, when running a flash firmware, it is recommended to run with a subset of GNSS
	systems, to avoid system overload.
	
	Using UBX-CFG-PMS to set Super-E mode 1, 2, 4Hz navigation rates sets 180s minAcqTime instead
	the default 300s in protocol version 23.01. 300s is recommended for the best performance.
*/
void ZOE_M8B_set_power_mode(uint8_t mode)
{
	// UBX-CFG-PMS
}


/*
	AssistNow Autonomous is disabled by default. Once enabled, the receiver will automatically produce
	data for newly received broadcast ephemerides and, if that data is available, automatically provide
	the navigation subsystem with orbits when necessary and adequate.
	
	Default orbit data validity of approximately three days (for GPS satellites observed once)
	and up to six days (for GPS and GLONASS satellites observed multiple times over a period of at least half a day).
	
	Enabling the AssistNow Autonomous feature will lead to increased power consumption while prediction is calculated.
	Therefore for each application special care must be taken to judge whether AssistNow Autonomous is beneficial
	to the overall power consumption or not.
*/
void ZOE_M8B_AssistNow_Autonomous(uint8_t enable)
{
	// UBX-CFG-NAVX5
}


/*
	Send a dummy sequence of 0xFF to the receiver's UART interface. This will wake up the receiver if it is in Inactive state.
	If the receiver is not in Inactive state, the sequence will be ignored.
	
	Send following messages about half a second after the dummy sequence. If the interval between the dummy sequence
	and the next message is too short, the receiver may not yet be ready. 
*/
void ZOE_M8B_wakeup_sequence(uint32_t delay_ms)
{
	// send dummy byte 0xFF
	if(delay_ms > 0) SysTick_delay_ms(delay_ms);
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
*/
void ZOE_M8B_reset(uint8_t type, uint8_t mask)
{
	// UBX-CFG-RST
}