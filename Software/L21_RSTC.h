/*
	The Reset Controller (RSTC) manages the reset of the MCU. It issues a microcontroller reset,
	sets the device to its initial state and allows the reset source to be identified by software.
	
	RESET SOURCES
		Power supply reset sources: POR, BOD12, BOD33
		User reset sources: External reset (RESET), Watchdog reset, and System Reset Request
		Backup exit sources: Real-Time Counter (RTC), External Wake-up (EXTWAKE), and Battery Backup Power Switch (BBPS)
*/


#ifndef L21_RSTC_H
#define L21_RSTC_H


#include "stdint.h"


// Functions
uint8_t RSTC_get_reset_source(void);
uint8_t RSTC_get_backup_exit_source(void);
void RSTC_system_reset_request(void);


#endif // L21_RSTC_H_