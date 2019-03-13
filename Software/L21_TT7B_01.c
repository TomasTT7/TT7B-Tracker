/*
	TT7B Superpressure Balloon Tracker firmware.
	
		More info on the hardware:
			http://tt7hab.blogspot.com/2018/09/the-tt7b-tracker.html
		
		by TT7 on 2018/09/16.
*/


#include "sam.h"
#include "L21_ADC.h"
#include "L21_APRS.h"
#include "L21_DAC.h"
#include "L21_GCLK.h"
#include "L21_GEOFENCE.h"
#include "L21_MS5607.h"
#include "L21_NVM.h"
#include "L21_OSC.h"
#include "L21_PM.h"
#include "L21_PORT.h"
#include "L21_RSTC.h"
#include "L21_RTC.h"
#include "L21_SERCOM_I2C.h"
#include "L21_SERCOM_SPI.h"
#include "L21_SERCOM_USART.h"
#include "L21_SI5351B.h"
#include "L21_SUPC.h"
#include "L21_SysTick.h"
#include "L21_TC.h"
#include "L21_VEML6030.h"
#include "L21_WatchDog.h"
#include "L21_ZOE_M8B.h"


uint32_t SysTick_CLK = 4000000;										// default MCLK after MCU reset


// Reset_Handler ---------------------------------------------------------------------------------
/*
	This is the code that gets called on processor reset.
	To initialize the device, and call the main() routine.
	
	Definition: startup_saml21.c
*/


// Main Function ---------------------------------------------------------------------------------
int main(void)
{
    /* Initialization */
	SystemInit();													// 4MHz MCLK - OSC16M
	WatchDog_enable(0x0B);											// 16s period
	uint8_t last_reset = RSTC_get_reset_source();
    SysTick_delay_init();
	
	/* Initialize BOD33 */
	SUPC_BOD33_disable();
	SUPC_BOD33_set_level(1);										// decrease trigger voltage to ~1.55V
	SUPC_BOD33_enable();
	
	/* Initialize RTC */
	uint32_t RTC_current = 0;
	
	OSC_enable_XOSC32K();											// enable 32.768kHz TCXO input signal
	RTC_mode0_enable(0x0B, 2764800);								// 31.25ms resolution, 24 hours initial compare match
	RTC_mode0_set_count(0);											// reset RTC to count from 0 in case this isn't POR
	
	/* XOSC32K */
	GCLK_x_enable(0, 5, 1, 0, 0);									// 32.768kHz MCLK - XOSC32K
	SysTick_CLK = 32768;
	
	/* Minimum Battery Voltage */
	uint16_t batt_V_raw = 0;
	
	while(batt_V_raw < 841)											// 0.75V minimum Vbatt to continue
	{
		ADC_enable(0, 5, 1, 6, 4, 0);								// 16.384kHz ADC clock, VDDANA reference, 64x averaging
		batt_V_raw = ADC_sample_channel_x(0x07);					// battery voltage raw
		ADC_disable();
		
		SysTick_delay_s(1);
		
		/* WatchDog Reset */
		WatchDog_reset();											// WatchDog Reset or let it reset every 16s
	}
	
	/* OSC16M */
	GCLK_x_enable(0, 6, 0, 0, 0);									// 4MHz MCLK - OSC16M
	SysTick_CLK = 4000000;
	
	/* Initialize Backlogging */
	uint16_t backlog_index = 0;
	
	/* Initialize Counter */
	uint32_t count = 0;
	
	/* WatchDog Reset */
	WatchDog_reset();
	
	/* Initialize Sensors */
	PORT_switch_enable(2);											// POWER to SENSORS on
	
	/* MS5607 Calibrations and Measurements */
	SERCOM_SPI_enable();											// 2MHz F_SPI (MCK / 2)
	
	MS5607_cmd_Reset(1, 0);											// after power-on, to make sure that calibration PROM gets loaded
	MS5607_cmd_Reset(2, 0);											// after power-on, to make sure that calibration PROM gets loaded
	SysTick_delay_ms(5);
	
	MS5607_cmd_PROMread(1);											// reads and stores 16 calibration bytes for MS5607_1
	MS5607_cmd_PROMread(2);											// reads and stores 16 calibration bytes for MS5607_2
	
	MS5607_cmd_Convert(1, 0x48);									// Oversampling Ratio = 4096, resolution = 0.024mbar, conversion time = 9.04ms
	SysTick_delay_ms(10);
	uint32_t MS1_pres_raw = MS5607_cmd_ADCread(1);
	
	MS5607_cmd_Convert(1, 0x58);									// Oversampling Ratio = 4096, resolution = 0.002°C, conversion time = 9.04ms
	SysTick_delay_ms(10);
	uint32_t MS1_temp_raw = MS5607_cmd_ADCread(1);
	
	MS5607_cmd_Convert(2, 0x48);									// Oversampling Ratio = 4096, resolution = 0.024mbar, conversion time = 9.04ms
	SysTick_delay_ms(10);
	uint32_t MS2_pres_raw = MS5607_cmd_ADCread(2);
	
	MS5607_cmd_Convert(2, 0x58);									// Oversampling Ratio = 4096, resolution = 0.002°C, conversion time = 9.04ms
	SysTick_delay_ms(10);
	uint32_t MS2_temp_raw = MS5607_cmd_ADCread(2);
	
	SERCOM_SPI_disable();
	
	/* ADC Measurements */
	ADC_enable(0, 5, 1, 6, 4, 0);									// 2MHz ADC clock, VDDANA reference, 64x averaging
	batt_V_raw = ADC_sample_channel_x(0x07);						// battery voltage raw
	uint16_t therm1_raw = ADC_sample_channel_x(0x12);				// thermistor 1 voltage
	uint16_t therm2_raw = ADC_sample_channel_x(0x13);				// thermistor 2 voltage
	ADC_disable();
	
	/* Sensors - Power-Down */
	PORT_switch_disable(2);											// POWER to SENSORS off
	
	/* Initialize VEML6030 */
	PORT_switch_enable(3);											// POWER to 3.3V on
	PORT_switch_enable(4);											// POWER to TRANSLATOR on
	SERCOM_I2C_enable(4000000, 90000);								// 4MHz MCK, 90kHz I2C
	
	VEML6030_disable();
	SysTick_delay_ms(50);											// give 3.3V circuit short on time
	
	SERCOM_I2C_disable();
	PORT_switch_disable(4);											// POWER to TRANSLATOR off
	PORT_switch_disable(3);											// POWER to 3.3V off
	
	/* WatchDog Reset */
	WatchDog_reset();
	
	/* Initialize GPS */
	uint16_t year		= 0;
	uint8_t month		= 0;
	uint8_t day			= 0;
	uint8_t hour		= 0;
	uint8_t min			= 0;
	uint8_t sec			= 0;
	uint8_t valid		= 0;
	uint8_t fixType		= 0;
	uint8_t gnssFixOK	= 0;
	uint8_t psmState	= 0;
	uint8_t numSV		= 0;
	float lon			= 0.0;
	float lat			= 0.0;
	int32_t hMSL		= 0;
	uint32_t hAcc		= 0;
	uint32_t vAcc		= 0;
	uint16_t pDOP		= 0;
	uint8_t noGPS		= 0;
	uint8_t valid_time	= 0;
	
	uint8_t GPS_buffer[110];
	
	uint8_t ack			= 0;
	uint8_t checksum	= 0;
	uint8_t reinit		= 0;
	
	PORT_switch_enable(1);											// POWER to GPS on
	SERCOM_USART_enable(4000000, 9600);								// initial baud rate 9600
	
	ZOE_M8B_send_message((uint8_t*) 0xFF, 1);						// wakes up receiver if it is in Inactive state, otherwise ignored
	SysTick_delay_ms(500);											// if interval too short, receiver may not be ready to process configuration messages
	
	ZOE_M8B_set_port(3);											// 115200 baud, only UBX protocol
	SysTick_delay_ms(1500);											// must be >1s, because in case of baud mismatch u-blox disables UART for 1s
	
	SERCOM_USART_disable();
	SERCOM_USART_enable(4000000, 115200);							// baud rate 115200
	
	clear_buffer(GPS_buffer, 110);
	ack = ZOE_M8B_get_port(GPS_buffer, 230000);						// request confirmation of higher baud rate
	
	uint32_t baud_val = (GPS_buffer[16] << 16) | (GPS_buffer[15] << 8) | GPS_buffer[14];
	
	if(ack != 1 || baud_val != 115200)								// if it fails continue with default baud rate
	{
		baud_val = 9600;
		SERCOM_USART_disable();
		SERCOM_USART_enable(4000000, baud_val);						// baud rate 9600
	}
	
	/* WatchDog Reset */
	WatchDog_reset();
	
	ack = ZOE_M8B_power_saving(0);									// Continuous mode
	ack = ZOE_M8B_set_GNSS_system(1);								// GPS and QZSS only
	ack = ZOE_M8B_set_update_rate(3);								// 100ms between measurements, 1 measurement per solution
	ack = ZOE_M8B_set_dynamic_model(6);								// Airborne <1g (50,000m ceiling)
	ack = ZOE_M8B_save_current_configuration();
	
	while(1)
	{
		/* WatchDog Reset */
		WatchDog_reset();
		
		clear_buffer(GPS_buffer, 110);
		checksum = ZOE_M8B_get_solution(GPS_buffer, 230000);		// 1.5s timeout with 4MHz MCLK
		
		if(checksum == 1)
		{
			ZOE_M8B_parse_solution(GPS_buffer, &year, &month, &day, &hour, &min, &sec, &valid, &fixType, &gnssFixOK,
									&psmState, &numSV, &lon, &lat, &hMSL, &hAcc, &vAcc, &pDOP);
			
			if((valid & 0x04) == 0x04 || (valid & 0x03) == 0x03) valid_time = 1;
			else valid_time = 0;
			
			if(gnssFixOK && valid_time && numSV >= 7) break;		// valid fix, time and date and minimum number of satellites condition
		}
		
		RTC_current = RTC_get_current_count();		
		if(RTC_current > 9600) break;								// 5min timeout
	}
	
	SERCOM_USART_disable();
	PORT_switch_disable(1);											// POWER to GPS off
	
	/* Main Loop */
	while(1)
	{
		/* Counter Update */
		count++;
		
		/* WatchDog Reset */
		WatchDog_reset();
		
		/* GPS - Wake Up */
		PORT_switch_enable(1);										// POWER to GPS on
		SERCOM_USART_enable(4000000, baud_val);						// operating baud rate (9600 or 115200)
		
		ZOE_M8B_send_message((uint8_t*) 0xFF, 1);					// wakes up receiver if it is in Inactive state, otherwise ignored
		SERCOM_USART_disable();
		
		/* Sensors - Power-Up */
		PORT_switch_enable(2);										// POWER to SENSORS on
		
		/* MS5607 */
		SERCOM_SPI_enable();										// 2MHz F_SPI (MCK / 2)
		MS5607_cmd_Reset(1, 0);										// after power-on, to make sure that calibration PROM gets loaded
		MS5607_cmd_Reset(2, 0);										// after power-on, to make sure that calibration PROM gets loaded
		SysTick_delay_ms(5);

		MS5607_cmd_Convert(1, 0x48);								// Oversampling Ratio = 4096, resolution = 0.024mbar, conversion time = 9.04ms
		SysTick_delay_ms(10);
		MS1_pres_raw = MS5607_cmd_ADCread(1);
		
		MS5607_cmd_Convert(1, 0x58);								// Oversampling Ratio = 4096, resolution = 0.002°C, conversion time = 9.04ms
		SysTick_delay_ms(10);
		MS1_temp_raw = MS5607_cmd_ADCread(1);
		
		MS5607_cmd_Convert(2, 0x48);								// Oversampling Ratio = 4096, resolution = 0.024mbar, conversion time = 9.04ms
		SysTick_delay_ms(10);
		MS2_pres_raw = MS5607_cmd_ADCread(2);
		
		MS5607_cmd_Convert(2, 0x58);								// Oversampling Ratio = 4096, resolution = 0.002°C, conversion time = 9.04ms
		SysTick_delay_ms(10);
		MS2_temp_raw = MS5607_cmd_ADCread(2);
		
		SERCOM_SPI_disable();
		
		/* ADC Averaged */
		ADC_enable(0, 5, 1, 6, 4, 0);								// 2MHz ADC clock, VDDANA reference, 64x averaging
		batt_V_raw = ADC_sample_channel_x(0x07);					// battery voltage raw
		therm1_raw = ADC_sample_channel_x(0x12);					// thermistor 1 voltage
		therm2_raw = ADC_sample_channel_x(0x13);					// thermistor 2 voltage
		ADC_disable();
		
		/* Sensors - Power-Down */
		PORT_switch_disable(2);										// POWER to SENSORS off
		
		/* MS5607 - Results */
		float MS1_pres, MS1_temp, MS2_pres, MS2_temp;
		MS5607_calculate_results(1, MS1_pres_raw, MS1_temp_raw, &MS1_pres, &MS1_temp);
		MS5607_calculate_results(2, MS2_pres_raw, MS2_temp_raw, &MS2_pres, &MS2_temp);
		
		/* WatchDog Reset */
		WatchDog_reset();
		
		/* MCU Temperature Sensor */
		uint32_t average_n = 64;									// number of samples to average
		uint32_t raw_collected = 0;									// maximum 1,048,832 samples
		
		NVM_wait_states(3);											// NVM Read Wait States: 0-15, default 0, 3 for 48MHz PL2 operation
		PM_set_performance_level(2, 0);
		OSC_enable_DFLL48M_open();
		GCLK_x_enable(0, 7, 15, 0, 0);								// 3.2MHz MCLK - DFLL48M / 15
		SysTick_CLK = 3200000;
		
		SUPC_temperature_sensor(1);
		ADC_enable(0, 0, 0, 0, 0, 63);								// 1.6MHz ADC clock, INTREF reference, 12-bit resolution
		
		for(uint32_t i = 0; i < average_n; i++)
		{
			raw_collected += ADC_sample_channel_x(0x18);			// temperature sensor raw, one measurement 13 clock cycles
		}
		uint16_t temp_sensor_raw = raw_collected / average_n;		// average raw ADC value
		
		ADC_disable();
		SUPC_temperature_sensor(0);
		
		float MCU_temp = ADC_temperature_mcu(temp_sensor_raw);		// [°C]
		
		/* OSC16M */
		GCLK_x_enable(0, 6, 0, 0, 0);								// 4MHz MCLK - OSC16M
		SysTick_CLK = 4000000;
		OSC_disable_DFLL48M();
		PM_set_performance_level(0, 0);
		NVM_wait_states(0);											// NVM Read Wait States: 0-15, default 0, 3 for 48MHz PL2 operation
		
		/* WatchDog Reset */
		WatchDog_reset();
		
		/* VEML6030 */
		PORT_switch_enable(3);										// POWER to 3.3V on
		PORT_switch_enable(4);										// POWER to TRANSLATOR on
		SERCOM_I2C_enable(4000000, 90000);							// 4MHz MCK, 90kHz I2C
		
		VEML6030_enable(0b10, 0b1100);								// Gain 1/8, Integration Time 25ms, maximum 120klux, resolution 1.8432lux
		SysTick_delay_ms(55);										// for consistent readings (2x integration time + 5ms)
		uint16_t light_raw = VEML6030_get_measurement_result();
		float ambient_light = VEML6030_calculate_lux(light_raw, 0b10, 0b1100);
		
		if(light_raw < 20)											// if measurement < 37 lux
		{
			VEML6030_enable(0b01, 0b0000);							// Gain 2, Integration Time 100ms, maximum 1887lux, resolution 0.0288lux
			SysTick_delay_ms(205);									// for consistent readings (2x integration time + 5ms)
			
			light_raw = VEML6030_get_measurement_result();
			ambient_light = VEML6030_calculate_lux(light_raw, 0b01, 0b0000);
		}
		
		VEML6030_disable();
		SERCOM_I2C_disable();
		PORT_switch_disable(4);										// POWER to TRANSLATOR off
		PORT_switch_disable(3);										// POWER to 3.3V off
		
		/* WatchDog Reset */
		WatchDog_reset();
		
		/* GPS - Get Data */
		noGPS = 0;
		uint8_t tout = 15;
		uint16_t attempts = 0;
		
		SERCOM_USART_enable(4000000, baud_val);						// operating baud rate (9600 or 115200)
		
		while(1)													// delay until GPS module responds properly
		{
			clear_buffer(GPS_buffer, 110);
			checksum = ZOE_M8B_get_dynamic_model(GPS_buffer, 16000);
			
			tout--;
			
			if(!tout || checksum == 1) break;
		}
		
		if(tout)													// previous request didn't time out, communication works
		{
			/* Dynamic Model Check */
			uint8_t dynamic_model = 0;
			
			ZOE_M8B_parse_dynamic_model(GPS_buffer, &dynamic_model);
			
			if(dynamic_model != 6)
			{
				clear_buffer(GPS_buffer, 110);
				ack = ZOE_M8B_set_dynamic_model(6);					// Airborne <1g (50,000m ceiling)
			}
			
			/* Data Acquisition */
			while(1)
			{
				/* WatchDog Reset */
				WatchDog_reset();
				
				attempts++;
				
				clear_buffer(GPS_buffer, 110);
				checksum = ZOE_M8B_get_solution(GPS_buffer, 230000);		// 1.5s timeout with 4MHz MCLK
				
				if(checksum == 1)
				{
					ZOE_M8B_parse_solution(GPS_buffer, &year, &month, &day, &hour, &min, &sec, &valid, &fixType, &gnssFixOK,
											&psmState, &numSV, &lon, &lat, &hMSL, &hAcc, &vAcc, &pDOP);
					
					if((valid & 0x04) == 0x04 || (valid & 0x03) == 0x03) valid_time = 1;
					else valid_time = 0;
					
					if(gnssFixOK && valid_time && numSV >= 5) break;		// valid fix, time and date and minimum number of satellites condition
				}
				
				if(attempts > 600)									// limit acquisition to 60s (600 attempts at 10Hz)
				{
					noGPS = 1;										// transmit packet without position
					reinit++;
					break;
				}
			}
		
			ack = ZOE_M8B_save_current_configuration();
		}
		else
		{
			noGPS = 1;												// transmit packet without position
			reinit++;
		}
		
		if(!noGPS) reinit = 0;										// zero counter of failed fix acquisitions in row
		
		SERCOM_USART_disable();
		PORT_switch_disable(1);										// POWER to GPS off
		
		/* Active Time */
		uint16_t active_time;
		
		RTC_current = RTC_get_current_count();
		active_time = (uint16_t)((float)RTC_current * 0.3125);		// [0.1s], 100ms resolution (rounded down)
		
		if(active_time > 999) active_time = 999;					// set upper limit - 99.9s
		
		/* Backlog */
		if((count % 10) == 0 && !noGPS)								// store backlog every X cycles, but only with valid GPS data
		{
			uint8_t bck_buffer[37];
			
			APRS_backlog_encode(bck_buffer, lat, lon, (uint16_t)hMSL, MCU_temp, therm1_raw, therm2_raw, MS1_temp, MS2_temp,
								(uint32_t)MS1_pres, (uint32_t)MS2_pres, year, month, day, hour, min, batt_V_raw, ambient_light,
								numSV, active_time, last_reset);
			APRS_backlog_store(bck_buffer, 37);
		}
		
		/* APRS Packet */
		uint8_t n;
		uint8_t aprs_buffer[150];
		
		n = APRS_packet(aprs_buffer, (uint8_t *)"OK7DMT", 1, lat, lon, (uint16_t)hMSL, MCU_temp, therm1_raw, therm2_raw, MS1_temp, MS2_temp,
						(uint32_t)MS1_pres, (uint32_t)MS2_pres, batt_V_raw, ambient_light, numSV, active_time, last_reset, &backlog_index, noGPS);
		APRS_prepare_bitstream(aprs_buffer, n);
		
		/* WatchDog Reset */
		WatchDog_reset();
		
		/* GeoFenced Frequency */
		uint32_t tx_frequency = GEOFENCE_frequency(lat, lon);		// decide on frequency based on GPS position
		
		/* APRS Transmission */
		if(tx_frequency > 144000000 && tx_frequency < 146000000)	// no transmission if tx_frequency equals 0 
		{
			/* FDPLL96M */
			NVM_wait_states(3);										// NVM Read Wait States: 0-15, default 0, 3 for 48MHz PL2 operation
			PM_set_performance_level(2, 0);
			OSC_enable_FDPLL96M(0, 0, 1545, 14);					// 50,688,000Hz FDPLL96M
			GCLK_x_enable(0, 8, 2, 0, 0);							// 25,344,000Hz MCLK - FDPLL96M / 2
			SysTick_CLK = 25344000;
			
			/* SI5351B & DAC */
			PORT_switch_enable(3);									// POWER to 3.3V on
			PORT_switch_enable(4);									// POWER to TRANSLATOR on
			SERCOM_I2C_enable(25344000, 90000);
			
			SI5351B_init();
			SI5351B_frequency(tx_frequency, 38);
			GCLK_x_enable(2, 8, 12, 0, 0);							// 4,224,000Hz GCLK[2]
			DAC_enable(1, 1);										// VDDANA, 1.2MHz-6MHz
			
			SI5351B_enable_output();
			TC0_enable(0x00, 660, 1);								// 25,344,000Hz, 660/65535 -> 38,400Hz
			TC4_enable(0x00, 360, 0);								// 25,344,000Hz, 360/65535 -> 70,400Hz
			TC0_transmission();
			TC4_disable();
			TC0_disable();
			SI5351B_disable_output();
			DAC_disable();
			SI5351B_deinit();
			
			SERCOM_I2C_disable();
			PORT_switch_disable(4);									// POWER to TRANSLATOR off
			PORT_switch_disable(3);									// POWER to 3.3V off
			
			/* XOSC32K */
			GCLK_x_enable(0, 5, 1, 0, 0);							// 32.768kHz MCLK - XOSC32K
			SysTick_CLK = 32768;
			OSC_disable_FDPLL96M();
			PM_set_performance_level(0, 0);
			NVM_wait_states(0);										// NVM Read Wait States: 0-15, default 0, 3 for 48MHz PL2 operation
		}
		else
		{
			/* XOSC32K */
			GCLK_x_enable(0, 5, 1, 0, 0);							// 32.768kHz MCLK - XOSC32K
			SysTick_CLK = 32768;
		}
		
		/* WatchDog Reset */
		WatchDog_reset();
		
		/* Sleep */
		for(uint8_t i = 0; i < 12; i++)								// 120s = 12 * 10s
		{
			RTC_mode0_update_compare(320);							// 10s sleep
		
			PM_set_sleepmode(4, 0, 0, 0, 0, 0);						// Standby, domains not linked, automatic mode, no sleepwalking, handled by HW
			PM_sleep();												// enter Standby
			
			/* WatchDog Reset */
			WatchDog_reset();										// WatchDog period 16s
		}
		
		/* Wake-Up */
		GCLK_x_enable(0, 6, 0, 0, 0);								// 4MHz MCLK - OSC16M
		SysTick_CLK = 4000000;
		
		/* Force Reset Check */
		if(reinit >= 5) RSTC_system_reset_request();				// reset MCU if too many positional fix failures in row
		
		RTC_mode0_set_count(0);										// reset RTC to count from 0 again
	}
}