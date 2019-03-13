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


#include "sam.h"
#include "L21_SUPC.h"


/*
	TSEN
		0	Temperature Sensor is disabled.
		1	Temperature Sensor is enabled and routed to an ADC input channel.
		
	When VREF.ONDEMAND=0, it is not recommended to enable both voltage reference output and temperature
	sensor at the same time - only the voltage reference output will be present at both ADC inputs.
*/
void SUPC_temperature_sensor(uint8_t tsen)
{
	if(tsen)
	{
		SUPC->VREF.bit.ONDEMAND = 1;						// Voltage reference is enabled when a peripheral is requesting it.
		SUPC->VREF.bit.SEL = 0;								// Select required voltage for internal voltage reference INTREF - 1.024V.
		SUPC->VREF.bit.VREFOE = 1;							// Enable routing INTREF to ADC.
	}
	else
	{
		SUPC->VREF.bit.ONDEMAND = 0;						// Voltage reference is always on, if enabled.
		SUPC->VREF.bit.SEL = 0;								// Restore internal voltage reference INTREF to default.
		SUPC->VREF.bit.VREFOE = 0;							// Disable routing INTREF to ADC.
	}
	
	SUPC->VREF.bit.TSEN = tsen & 0x01;						// enable/disable temperature sensor
}


/*
	
*/
void SUPC_BOD33_enable(void)
{
	SUPC->BOD33.bit.ENABLE = 1;								// BOD33 is enabled.
	while(!SUPC->STATUS.bit.B33SRDY);						// BOD33 synchronization is complete.
}


/*
	
*/
void SUPC_BOD33_disable(void)
{
	SUPC->BOD33.bit.ENABLE = 0;								// BOD33 is disabled.
	while(!SUPC->STATUS.bit.B33SRDY);						// BOD33 synchronization is complete.
}


/*
	The BOD33 register is Enable-Protected, meaning that they can only be written when the BOD is
	disabled (BOD33.ENABLE=0 and SYNCBUSY.BOD33EN=0).
	
	LEVEL	VBOD+	VBOD-
	48		3.20V	3.08V
	39		2.87V	2.77V
	7		1.75V	1.673V
	6		1.72V	1.65V
*/
void SUPC_BOD33_set_level(uint8_t level)
{
	if(level > 48) return;
	
	SUPC->BOD33.bit.LEVEL = level;							// These bits set the triggering voltage threshold for the BOD33.
}