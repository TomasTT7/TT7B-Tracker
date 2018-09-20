/*
	The Power Manager (PM) controls the sleep modes and the power domain gating of the device.
	
	To help balance between performance and power consumption, the device has two performance levels.
	Each of the performance levels has a maximum operating frequency and a corresponding maximum
	consumption in µA/MHz.		PL0		Performance Level 0 (PL0) provides the maximum energy efficiency configuration.
		PL2		Performance Level 2 (PL2) provides the maximum operating frequency.
	
	In addition to the supply domains (VDDIO, VDDIN and VDDANA) the device provides these power domains:
		PD0, PD1, PD2
		PDTOP
		PDBACKUP
	The PD0, PD1 and PD2 are "switchable power domains". In standby or backup sleep mode, they can be
	turned off to save leakage consumption according to user configuration.
	
	After a power-on reset, the PM is enabled, the device is in ACTIVE mode, the performance level is PL0
	(the lowest power consumption) and all the power domains are in active state.
*/


#include "sam.h"
#include "L21_PM.h"


/*
	CONSUMPTION (DATASHEET)
		IDLE		17-30uA/MHz
		STANDBY		1.3-3.1uA		(PD0, PD1, PD2 in retention state)
					1.5-3.2uA		(PD0, PD1, PD2 in retention state with RTC running on OSCULP32K)
					1.6-3.6uA		(PD1, PD2 in retention state and PD0 in active state)
					2.3-5.6uA		(PD2 in retention state and PD0, PD1 in active state)
					3.3-7.9uA		(PD0, PD1, PD2 in active state)
		BACKUP		0.48-0.83uA		(powered by VDDIN, VDDIN+VDDANA+VDDIO consumption)
		OFF			0.16-0.29uA
	
	MODE OVERVIEW
		Mode	Main clock	CPU		AHBx and APBx clock	GCLK clocks	OSC	ONDEMAND=0		OSC ONDEMAND=1		Regulator					NVM
		Active	Run			Run		Run					Run(3)		Run					Run if requested	MAINVREG					active
		IDLE	Run			Stop	Stop(1)				Run(3)		Run					Run if requested	MAINVREG					active
		STANDBY	Stop(1)		Stop	Stop(1)				Stop(1)		Run if requested	Run if requested	MAINVREG in low power mode	Ultra Low power
		BACKUP	Stop		Stop	Stop				Stop		Stop				Stop				Backup regulator (ULPVREG)	OFF
		OFF		Stop		Stop	Stop				OFF			OFF					OFF					OFF							OFF
	
		1. Running if requested by peripheral during SleepWalking.
		2. Running during SleepWalking.
		3. Following On-Demand Clock Request principle.
	
	The IDLE mode allows power optimization with the fastest wake-up time.
	The STANDBY mode is the lowest power configuration while keeping the state of the logic and the content of the RAM.
		The regulator operates in low-power mode by default and switches automatically to the normal mode in case of a sleepwalking task
		requiring more power. It returns automatically to low power mode when the sleepwalking task is completed.
	The BACKUP mode allows achieving the lowest power consumption aside from OFF. The device is entirely powered off except for the backup domain.
	In OFF mode, the device is entirely powered-off.
	
	By default, in standby sleep mode and backup sleep mode, the RAMs, NVM, and regulators
	are automatically set in low-power mode in order to reduce power consumption.
	
		SLEEP MODE		SWITCHABLE POWER DOMAINS			RAMs MODE(1)			NVM			REGULATORS
						PD0			PD1			PD2			LP SRAM		SRAM					VDDCORE			VDDBU
																								MAIN	ULP
		ACTIVE			active		active		active		normal		normal		normal		on		on		on
		IDLE			active		active		active		normal		auto(2)		on			on		on		on
		STANDBY 1		active		active		active		normal		normal		auto(2)		auto(3)	on		on
		STANDBY 2		active		active		retention	normal		low power	low power	auto(3)	on		on
		STANDBY 3		active		retention	retention	low power	low power	low power	auto(3)	on		on
		STANDBY 4		retention	retention	retention	low power	low power	low power	off		on		on
		BACKUP			off			off			off			off			off			off			off		off		on
		OFF				off			off			off			off			off			off			off		off		off
		
		1. RAMs mode by default: STDBYCFG.BBIAS bits are set to their default value.
		2. auto: by default, NVM is in low-power mode if not accessed.
		3. auto: by default, the main voltage regulator is on if GCLK, APBx, or AHBx clock is running during SleepWalking.
	
	SLEEPMODE
		0x2		IDLE		CPU, AHBx, and APBx clocks are OFF
		0x4		STANDBY		ALL clocks are OFF, unless requested by sleepwalking peripheral
		0x5		BACKUP		Only Backup domain is powered ON
		0x6		OFF			All power domains are powered OFF
	
	LINKPD
		0x0		DEFAULT		Power domains PD0/PD1/PD2 are not linked.
		0x1		PD01		Power domains PD0 and PD1 are linked. If PD0 is active, then PD1 is active even if there is no activity in PD1.
		0x2		PD12		Power domains PD1 and PD2 are linked. If PD1 is active, then PD2 is active even if there is no activity in PD2.
		0x3		PD012		All Power domains are linked. If PD0 is active, then PD1 and PD2 are active even if there is no activity in PD1 or PD2.
	
	VREGSMOD
		0x0		AUTO		Automatic Mode
		0x1		PERFORMANCE	Performance oriented
		0x2		LP			Low Power consumption oriented
	
	DPGPD1
		0		Dynamic SleepWalking for power domain 1 is disabled.
		1		Dynamic SleepWalking for power domain 1 is enabled.
		
	DPGPD0
		0		Dynamic SleepWalking for power domain 0 is disabled.
		1		Dynamic SleepWalking for power domain 0 is enabled.
		
	PDCFG
		0x0		DEFAULT		In standby mode, all power domain switching are handled by hardware.
		0x1		PD0			In standby mode, power domain 0 (PD0) is forced ACTIVE. Other power domain switching is handled by hardware.
		0x2		PD01		In standby mode, power domains PD0 and PD1 are forced ACTIVE. Power domain 2 switching is handled by hardware.
		0x3		PD012		In standby mode, all power domains are forced ACTIVE.
*/
void PM_set_sleepmode(uint8_t sleepmode, uint8_t linkpd, uint8_t vregsmod, uint8_t dpgpd1, uint8_t dpgpd0, uint8_t pdcfg)
{
	if(!(sleepmode == 2 || sleepmode == 4 || sleepmode == 5 || sleepmode == 6)) return;
	
	PM->SLEEPCFG.bit.SLEEPMODE = sleepmode;						// set sleep mode
	while(PM->SLEEPCFG.bit.SLEEPMODE != sleepmode);				// Software has to make sure the SLEEPCFG register reads the wanted value.
	
	PM->STDBYCFG.bit.LINKPD = ((linkpd & 0x03) << 8);			// configure power domain linking
	PM->STDBYCFG.bit.VREGSMOD = ((vregsmod & 0x03) << 6);		// configure voltage regulator (main/low power)
	PM->STDBYCFG.bit.DPGPD1 = dpgpd1;							// configure dynamic power gating for PD1
	PM->STDBYCFG.bit.DPGPD0 = dpgpd0;							// configure dynamic power gating for PD0 
	PM->STDBYCFG.bit.PDCFG = (pdcfg & 0x03);					// configure power domains
}


/*
	DSB (Data Synchronization Barrier) instruction ensures all ongoing memory accesses have completed.
	WFI (Wait For Interrupt) instruction places the device into the specified sleep mode until woken by an interrupt.
	
	WAKE-UP TIME
		IDLE		1.2us		(PL2 or PL0)
		STANDBY		5.1us		(PL0 PM.PLSEL.PLDIS=1)
					16us		(PL2 Voltage scaling at fastest setting: SUPC->VREG.VSVSTEP=15 SUPC->VREG.VSPER=0)
					76us		(PL2 Voltage scaling at default values: SUPC->VREG.VSVSTEP=0 SUPC->VREG.VSPER=0)
		BACKUP		90us
		OFF			2.2ms
*/
void PM_sleep(void)
{
	__DSB();
	__WFI();
}


/*
	The application can change the performance level on the fly.
	After a reset, the device starts in the lowest PL (lowest power consumption and lowest max frequency).
	
	List of peripherals/clock sources not available in PL0:
		USB (limited by logic frequency)
		DFLL48M
	List of peripherals/clock sources with limited capabilities in PL0:
		All AHB/APB peripherals are limited by CPU frequency
		DPLL96M: may be able to generate 48MHz internally, but the output cannot be used by logic
		GCLK: the maximum frequency is by factor 4 compared to PL2
		SW interface: the maximum frequency is by factor 4 compared to PL2
		TC: the maximum frequency is by factor 4 compared to PL2
		TCC: the maximum frequency is by factor 4 compared to PL2
		SERCOM: the maximum frequency is by factor 4 compared to PL2	
	List of peripherals/clock sources with full capabilities in PL0:
		AC, ADC, DAC, EIC, OPAMP, OSC16M, PTC, All 32KHz clock sources and peripherals
	
	MAXIMUM CLOCK FREQUENCIES
								PL0			PL2
		F_GCLK					24MHz		96MHz
		F_CPU					12MHz		48MHz
		F_AHB					12MHz		48MHz
		F_APBA					6MHz		6MHz		(Bus clock domain = BACKUP)
		F_APBx					12MHz		48MHz
		F_GCLK_DFLL48M_REF		N/A			33kHz		(DFLL48M Reference clock frequency)
		F_GCLK_DPLL				2MHz		2MHz		(FDPLL96M Reference clock frequency)
		F_GCLK_DPLL_32K			32kHz		100kHz		(FDPLL96M 32k Reference clock frequency)
		F_GCLK_EIC				12MHz		48MHz
		F_GCLK_USB				N/A			60MHz
		F_GCLK_EVSYS_CHANNEL_x	12MHz		48MHz
		F_GCLK_SERCOMx_SLOW		1MHz		5MHz
		F_GCLK_SERCOMx_CORE		12MHz		48MHz
		F_GCLK_TCC0, GCLK_TCC1	24MHz		96MHz
		F_GCLK_TCC2, GCLK_TC0	12MHz		48MHz
		F_GCLK_TC1, GCLK_TC2	12MHz		48MHz
		F_GCLK_TC3, GCLK_TC4	12MHz		48MHz
		F_GCLK_ADC				12MHz		48MHz
		F_GCLK_AC				12MHz		48MHz
		F_GCLK_DAC				12MHz		48MHz
		F_GCLK_PTC				12MHz		48MHz
		F_GCLK_CCL				12MHz		48MHz
		
	PLSEL
		0x0		PL0		Performance Level 0
		0x2		PL2		Performance Level 2
	
	PLDIS
		0		The Performance Level mechanism is enabled.
		1		The Performance Level mechanism is disabled.
	
	Disabling the automatic PL selection forces the device to run in PL0, reducing the power consumption
	and the wake-up time from standby sleep mode.
*/
void PM_set_performance_level(uint8_t plsel, uint8_t pldis)
{
	if(!(plsel == 0 || plsel == 2)) return;
	
	PM->PLCFG.reg = (pldis << 7) | plsel;						// set performance level and enable/disable automatic PL selection
	while(!(PM->INTFLAG.bit.PLRDY));							// this flag is set when performance level is ready
}