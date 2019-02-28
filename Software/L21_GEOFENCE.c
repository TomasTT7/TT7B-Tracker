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


#include "sam.h"
#include "L21_GEOFENCE.h"


/*
	Arrays of positional coordinates outlining areas on Earth.
*/
static float polyARG[] = {
	-68.0967,	-57.0310,
	-47.0506,	-35.9905,
	-57.4657,	-30.2155,
	-60.1426,	-22.0487,
	-67.8770,	-21.7225,
	-74.2491,	-54.7650,
	-68.0967,	-57.0310
};

static float polyAUS[] = {
	105.6095,	-18.2615,
	105.9172,	-36.5192,
	152.0158,	-49.0600,
	164.5842,	-26.2647,
	147.3576,	-7.4832,
	130.7023,	-7.3960,
	105.6095,	-18.2615
};

static float polyBRA[] = {
	-47.2264,	5.5786,
	-64.1893,	5.7535,
	-75.5272,	-8.4877,
	-62.3126,	-14.4031,
	-57.4657,	-30.2155,
	-47.0506,	-35.9905,
	-30.9302,	-20.0699,
	-27.6708,	-2.5415,
	-47.2264,	5.5786
};

static float polyCHN[] = {
	87.5641,	49.8807,
	65.8377,	45.1225,
	66.1627,	38.4280,
	79.9615,	29.9362,
	97.7518,	25.5758,
	111.5143,	14.7964,
	124.3463,	18.2505,
	125.6065,	36.4592,
	135.9479,	45.4141,
	123.8190,	53.8657,
	105.1422,	44.7626,
	87.5641,	49.8807
};

static float polyJPN[] = {
	124.5683,	21.2113,
	132.4496,	21.2116,
	146.3613,	30.8631,
	159.7969,	45.4214,
	136.3028,	51.1362,
	135.9479,	45.4141,
	125.6065,	36.4592,
	124.5683,	21.2113
};

static float polyNZ[] = {
	155.2130,	-46.8897,
	176.8341,	-54.2904,
	179.9922,	-50.4242,
	179.9922,	-30.4242,
	167.3420,	-23.4424,
	155.2130,	-46.8897
};

static float polyUK[] = {
	-5.7579,	50.0109,
	1.2678,		50.8958,
	1.7842,		52.5493,
	-1.4681,	55.3289,
	-2.1598,	59.4504,
	-8.0485,	58.0136,
	-6.1589,	55.9796,
	-7.9606,	54.3354,
	-4.3132,	53.8457,
	-5.7579,	50.0109
};



/*
	Returns appropriate frequency for coordinates passed to the function.
*/
uint32_t GEOFENCE_frequency(float latitude, float longitude)
{
	if(latitude > 10.0)													// Northern Hemisphere
	{
		if(longitude < -42.0)											// 144.390MHz (North and Central America)
		{
			return 144390000;
		}
		else if(longitude < 62.0)										// Europe, Africa
		{
			if(pointInPolygonF(10, polyUK, latitude, longitude))		// no TX (United Kingdom)
			{
				return 0;
			}
			else														// 144.800MHz (default Europe and Africa)
			{
				return 144800000;
			}
		}
		else															// Asia, Pacific
		{
			if(pointInPolygonF(12, polyCHN, latitude, longitude))		// 144.640MHz (China)
			{
				return 144640000;
			}
			else if(pointInPolygonF(8, polyJPN, latitude, longitude))	// 144.660MHz (Japan)
			{
				return 144660000;
			}
			else if(latitude > 35.0)									// 144.800MHz (Russia)
			{
				return 144800000;
			}
			else														// 144.390MHz (default Thailand)
			{
				return 144390000;
			}
		}
	}
	else																// Southern Hemisphere
	{
		if(longitude < -25.0)											// South America
		{
			if(pointInPolygonF(9, polyBRA, latitude, longitude))		// 145.570MHz (Brazil)
			{
				return 145570000;
			}
			else if(pointInPolygonF(7, polyARG, latitude, longitude))	// 144.930MHz (Argentina, Uruguay)
			{
				return 144930000;
			}
			else														// 144.390MHz (default South America)
			{
				return 144390000;
			}
		}
		else if(longitude < 60.0)										// 144.800MHz (default Africa)
		{
			return 144800000;
		}
		else															// Australia and New Zealand
		{
			if(pointInPolygonF(7, polyAUS, latitude, longitude))		// 145.175MHz (Australia)
			{
				return 145175000;
			}
			else if(pointInPolygonF(6, polyNZ, latitude, longitude))	// 144.575MHz (New Zealand)
			{
				return 144575000;
			}
			else														// 144.390MHz (default Indonesia and Malaysia)
			{
				return 144390000;
			}
		}
	}
}


/*
	Adapted version of pointInPolygon() function from:	http://alienryderflex.com/polygon/
	
	Returns '0' if the point is outside of the polygon and '1' if it's inside.
	
	Uses FLOAT input for better accuracy.
*/
int32_t pointInPolygonF(int32_t polyCorners, float * polygon, float latitude, float longitude)
{
	int32_t i;
	int32_t j = polyCorners * 2 - 2;
	int32_t oddNodes = 0;

	for(i = 0; i < polyCorners * 2; i += 2)
	{
		if((polygon[i + 1] < latitude && polygon[j + 1] >= latitude
		|| polygon[j + 1] < latitude && polygon[i + 1] >= latitude)
		&& (polygon[i] <= longitude || polygon[j] <= longitude))
		{
			oddNodes ^= (polygon[i] + (latitude - polygon[i + 1])
			/ (polygon[j + 1] - polygon[i + 1]) * (polygon[j] - polygon[i]) < longitude);
		}

		j = i;
	}

	return oddNodes;
}