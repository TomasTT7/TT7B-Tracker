/*
	The Supply Controller (SUPC) manages the voltage reference, power supply, and supply monitoring of the device.
	The SUPC controls the voltage regulators for the core (VDDCORE) and backup (VDDBU) domains.
	The SUPC embeds two Brown-Out Detectors. BOD33 monitors the voltage applied to the device (VDD or VBAT)
	and BOD12 monitors the internal voltage to the core (VDDCORE).
	The SUPC generates also a selectable reference voltage and a voltage dependent on the temperature
	which can be used by analog modules like the ADC or DAC.
	
	VOLTAGE REGULATOR SYSTEM
		Main voltage regulator: LDO or Buck Converter in active mode (MAINVREG)
		Low Power voltage regulator in standby mode (LPVREG)
		Backup voltage regulator for backup domains
		Controlled VDDCORE voltage slope when changing VDDCORE
	
	In active mode, the type of the main voltage regulator supplying VDDCORE can be switched on the fly.
	The two alternatives are a LDO regulator and a Buck converter (not supported on TT7B).
*/


#ifndef L21_SUPC_H
#define L21_SUPC_H


#include "stdint.h"


// Functions
void SUPC_temperature_sensor(uint8_t tsen);
void SUPC_BOD33_enable(void);
void SUPC_BOD33_disable(void);
void SUPC_BOD33_set_level(uint8_t level);


#endif // L21_SUPC_H_