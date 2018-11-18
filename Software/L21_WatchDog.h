/*
	WatchDog is enabled by default. Must be disabled in case it is not desired.
*/


#ifndef L21_WATCHDOG_H
#define L21_WATCHDOG_H


#include "stdint.h"


// Functions
void WatchDog_enable(uint8_t PER);
void WatchDog_disable(void);
void WatchDog_reset(void);


#endif // L21_WATCHDOG_H_