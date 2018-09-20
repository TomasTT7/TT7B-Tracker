/*
	The Power Manager (PM) controls the sleep modes and the power domain gating of the device.
	
	To help balance between performance and power consumption, the device has two performance levels.
	Each of the performance levels has a maximum operating frequency and a corresponding maximum
	consumption in µA/MHz.
		PL0		Performance Level 0 (PL0) provides the maximum energy efficiency configuration.
		PL2		Performance Level 2 (PL2) provides the maximum operating frequency.
	
	In addition to the supply domains (VDDIO, VDDIN and VDDANA) the device provides these power domains:
		PD0, PD1, PD2
		PDTOP
		PDBACKUP
	The PD0, PD1 and PD2 are "switchable power domains". In standby or backup sleep mode, they can be
	turned off to save leakage consumption according to user configuration.
	
	After a power-on reset, the PM is enabled, the device is in ACTIVE mode, the performance level is PL0
	(the lowest power consumption) and all the power domains are in active state.
*/


#ifndef L21_PM_H
#define L21_PM_H


#include "stdint.h"


// Functions
void PM_set_sleepmode(uint8_t sleepmode, uint8_t linkpd, uint8_t vregsmod, uint8_t dpgpd1, uint8_t dpgpd0, uint8_t pdcfg);
void PM_sleep(void);
void PM_set_performance_level(uint8_t plsel, uint8_t pldis);


#endif // L21_PM_H