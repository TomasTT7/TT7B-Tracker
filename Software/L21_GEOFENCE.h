/*
	APRS IGATE FREQUENCIES:
		Canada				144.390
		Chile				144.390
		Colombia			144.390
		Costa Rica			144.390
		Indonesia			144.390
		Malaysia			144.390
		Mexico				144.390
		Panama				144.390
		Thailand			144.390
		USA					144.390
		New Zealand			144.575
		South Korea			144.620
		China				144.640
		Japan				144.660
		Europe				144.800
		Russia				144.800
		South Africa		144.800
		Argentina			144.930
		Uruguay				144.930
		Australia			145.175
		Brazil				145.570
		India (New Delhi)	145.825 (ISS)
*/


#ifndef L21_GEOFENCE_H
#define L21_GEOFENCE_H


#include "stdint.h"


// Functions
uint32_t GEOFENCE_frequency(float latitude, float longitude);

int32_t pointInPolygonF(int32_t polyCorners, float * polygon, float latitude, float longitude);


#endif // L21_GEOFENCE_H_