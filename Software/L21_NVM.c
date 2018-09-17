/*
	
*/


#include "sam.h"
#include "L21_NVM.h"


/*
	NVM Max Speed Characteristics (CPU Fmax - MHz)
								0WS	1WS	2WS	3WS
		PL0		VDDIN > 1.6V	6	12	12	12
				VDDIN > 2.7V	7.5	12	12	12
		PL2		VDDIN > 1.6V	14	28	42	48
				VDDIN > 2.7V	24	45	48	48
	
	Default 0 read wait states. Maximum 15 read wait states.
*/
void NVM_wait_states(uint8_t rws)
{
	if(rws > 15) NVMCTRL->CTRLB.bit.RWS = 0;
	else NVMCTRL->CTRLB.bit.RWS = rws;
}