/*
	The Reset Controller (RSTC) manages the reset of the MCU. It issues a microcontroller reset,
	sets the device to its initial state and allows the reset source to be identified by software.
	
	RESET SOURCES
		Power supply reset sources: POR, BOD12, BOD33
		User reset sources: External reset (RESET), Watchdog reset, and System Reset Request
		Backup exit sources: Real-Time Counter (RTC), External Wake-up (EXTWAKE), and Battery Backup Power Switch (BBPS)
*/


#include "sam.h"
#include "L21_RSTC.h"


/*
	EFFECTS OF DIFFERENT RESET CAUSES
											Power Supply Reset			User Reset												Backup Reset
											POR, BOD33		BOD12		External Reset		WDT Reset, System Reset Request		RTC, EXTWAKE, BBPS
		RTC, OSC32KCTRL, RSTC, CTRLA.IORET	Y				N			N					N									N
		GCLK with WRTLOCK					Y				Y			N					N									Y
		Debug logic							Y				Y			Y					N									Y
		Others								Y				Y			Y					Y									Y
	
	RCAUSE
		Bit 7	Backup Reset			(Refer to BKUPEXIT register to identify the source of the Backup Reset.)
		Bit 6	System Reset Request	(Refer to the Cortex processor documentation for more details.)
		Bit 5	Watchdog Reset			(This bit is set if a Watchdog Timer Reset has occurred.)
		Bit 4	External Reset			(This bit is set if an external Reset has occurred.)
		Bit 2	BOD33 Reset				(This bit is set if a Brown Out 33 Detector Reset has occurred.)
		Bit 1	BOD12 Reset				(This bit is set if a Brown Out 12 Detector Reset has occurred.)
		Bit 0	Power On Reset			(This bit is set if a POR has occurred.)
*/
uint8_t RSTC_get_reset_source(void)
{
	uint8_t reg = RSTC->RCAUSE.reg;
	
	return reg;
}


/*
	When a Backup Reset occurs, the bit corresponding to the exit condition is set to '1', the other bits are written to '0'.
	BKUPEXIT
		Bit 2	Battery Backup Power Switch		(Set if the BBPS of SUPC changes back from battery mode to main power mode.)
		Bit 1	Real Timer Counter Interrupt
		Bit 0	External Wake-up
*/
uint8_t RSTC_get_backup_exit_source(void)
{
	uint8_t reg = RSTC->BKUPEXIT.reg;
	
	return reg;
}