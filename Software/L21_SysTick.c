/*
	
*/


#include "sam.h"
#include "L21_SysTick.h"


/*
	Sets clock source to MCK and enables the timer.
*/
void SysTick_delay_init(void)
{
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
}


/*
	SysTick_CLK global variable should be updated after every MCK change.
*/
void SysTick_delay_ms(uint32_t ms)
{
	uint32_t n = SysTick_CLK / 1000;
	
	if(n > 0)
	{
		for(uint32_t i = 0; i < ms; i++)
		{
			SysTick->LOAD = n;
			SysTick->VAL = 0;
		
			while(!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk));
		}
	}
}


/*
	Requires MCK to be in the MHZ range. Ideally, MCK divisible without remainder.
	SysTick_CLK global variable should be updated after every MCK change.
*/
void SysTick_delay_us(uint32_t us)
{
	uint32_t n = SysTick_CLK / 1000000;
	
	if(n > 0)
	{
		for(uint32_t i = 0; i < us; i++)
		{
			SysTick->LOAD = n;
			SysTick->VAL = 0;
		
			while(!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk));
		}
	}
}