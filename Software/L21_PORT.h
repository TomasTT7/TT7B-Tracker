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


#ifndef L21_PORT_H
#define L21_PORT_H


#include "stdint.h"


// Functions
void PORT_switch_enable(uint8_t swtch);
void PORT_switch_disable(uint8_t swtch);


#endif // L21_PORT_H_