/*
	The IO Pin Controller (PORT) controls the I/O pins of the device.
	
	After reset, all standard function device I/O pads are connected to the PORT with outputs
	tri-stated and input buffers disabled, even if there is no clock running.
	
	PIN CONFIGURATIONS
		DIR		INEN	PULLEN	OUT		Configuration
		0		0		0		X		Reset or analog I/O: all digital disabled
		0		0		1		0		Pull-down; input disabled
		0		0		1		1		Pull-up; input disabled
		0		1		0		X		Input
		0		1		1		0		Input with pull-down
		0		1		1		1		Input with pull-up
		1		0		X		X		Output; input disabled
		1		1		X		X		Output; input enabled
*/


#include "sam.h"
#include "L21_PORT.h"


/*	
	SWTCH
	1		Load Switch - GPS			PA22		(external pull-down)
	2		Load Switch - Sensors		PA16		(external pull-down)
	3		Boost Converter - 3.3V		PA04		(external pull-down)
	4		Voltage Translator			PA06		(serves as 1.8V power supply)
*/
void PORT_switch_enable(uint8_t swtch)
{
	switch(swtch)
	{
		case 1:
			PORT->Group[0].DIRSET.reg = PORT_PA22;				// I/O pin in the PORT group is configured as an OUTPUT.
			PORT->Group[0].OUTSET.reg = PORT_PA22;				// Pin output is driven HIGH, or input is connected to an internal PULL-UP.
			break;
			
		case 2:
			PORT->Group[0].DIRSET.reg = PORT_PA16;				// I/O pin in the PORT group is configured as an OUTPUT.
			PORT->Group[0].OUTSET.reg = PORT_PA16;				// Pin output is driven HIGH, or input is connected to an internal PULL-UP.
			break;
			
		case 3:
			PORT->Group[0].DIRSET.reg = PORT_PA04;				// I/O pin in the PORT group is configured as an OUTPUT.
			PORT->Group[0].OUTSET.reg = PORT_PA04;				// Pin output is driven HIGH, or input is connected to an internal PULL-UP.
			break;
			
		case 4:
			PORT->Group[0].PINCFG[6].reg = (0x1 << 6);			// DRVSTR - Pin drive strength is set to stronger drive strength.
			PORT->Group[0].DIRSET.reg = PORT_PA06;				// I/O pin in the PORT group is configured as an OUTPUT.
			PORT->Group[0].OUTSET.reg = PORT_PA06;				// Pin output is driven HIGH, or input is connected to an internal PULL-UP.
			break;
			
		default:
			break;
	}
}


/*
	SWTCH
	1		Load Switch - GPS			PA22		(external pull-down)
	2		Load Switch - Sensors		PA16		(external pull-down)
	3		Boost Converter - 3.3V		PA04		(external pull-down)
	4		Voltage Translator			PA06		(serves as 1.8V power supply)
*/
void PORT_switch_disable(uint8_t swtch)
{
	switch(swtch)
	{
		case 1:
			PORT->Group[0].OUTCLR.reg = PORT_PA22;				// Pin output is driven LOW, or input is connected to an internal PULL-DOWN.
			PORT->Group[0].DIRCLR.reg = PORT_PA22;				// I/O pin in the PORT group is configured as an INPUT.
			break;
			
		case 2:
			PORT->Group[0].OUTCLR.reg = PORT_PA16;				// Pin output is driven LOW, or input is connected to an internal PULL-DOWN.
			PORT->Group[0].DIRCLR.reg = PORT_PA16;				// I/O pin in the PORT group is configured as an INPUT.
			break;
			
		case 3:
			PORT->Group[0].OUTCLR.reg = PORT_PA04;				// Pin output is driven LOW, or input is connected to an internal PULL-DOWN.
			PORT->Group[0].DIRCLR.reg = PORT_PA04;				// I/O pin in the PORT group is configured as an INPUT.
			break;
			
		case 4:
			PORT->Group[0].OUTCLR.reg = PORT_PA06;				// Pin output is driven LOW, or input is connected to an internal PULL-DOWN.
			PORT->Group[0].DIRCLR.reg = PORT_PA06;				// I/O pin in the PORT group is configured as an INPUT.
			break;
			
		default:
			break;
	}
}