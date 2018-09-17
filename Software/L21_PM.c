/*
	
*/


#include "sam.h"
#include "L21_PM.h"


/*
	
*/
void PM_set_sleepmode(void)
{
	PM->SLEEPCFG.reg = 0x04;
	while(PM->SLEEPCFG.reg != 0x04);
	
	PM->STDBYCFG.reg = 0x00;
}


/*
	DSB (Data Synchronization Barrier) instruction ensures all ongoing memory accesses have completed.
	WFI (Wait For Interrupt) instruction places the device into the specified sleep mode until woken by an interrupt.
*/
void PM_sleep(void)
{
	__DSB();
	__WFI();
}