/*
	Non-Volatile Memory (NVM) is a reprogrammable Flash memory that retains program and data storage
	even with power off. The NVM Controller (NVMCTRL) connects to the AHB and APB bus interfaces for system
	access to the NVM block. The AHB interface is used for reads and writes to the NVM block, while the APB
	interface is used for commands and configuration.
	
	The NVM embeds a main array and a separate smaller array intended for EEPROM emulation (RWWEE)
	that can be programmed while reading the main array.
	
	The NVM is organized into rows, where each ROW contains 4 PAGES.
		A single row erase will erase all four pages in the row.
		Four write operations are used to write the complete row.
	
	MEMORY					START ADDRESS	SAML21x18	SAML21x17	SAML21x16	SAML21E15
	Embedded Flash			0x00000000		256kB		128kB		64kB		32kB
	Embedded RWW section	0x00400000		8kB			4kB			2kB			1kB
	
	FLASH MEMORY
		Device			Flash size		Number of pages		Page size		Last Row Start		Last Page Start		Last Byte
		SAML21x18		256kB			4096 				64B				0x0003FF00			0x0003FFC0			0x0003FFFF
		SAML21x17		128kB			2048 				64B				0x0001FF00			0x0001FFC0			0x0001FFFF
		SAML21x16		64kB			1024 				64B				0x0000FF00			0x0000FFC0			0x0000FFFF
		SAML21E15		32kB			512 				64B				0x00007F00			0x00007FC0			0x00007FFF
	
	RWW SECTION
		Device			Flash size		Number of pages		Page size
		SAML21x18		8kB				128					64B
		SAML21x17		4kB				64					64B
		SAML21x16		2kB				32					64B
		SAML21E15		1kB				16					64B
*/


#ifndef L21_NVM_H
#define L21_NVM_H


#include "stdint.h"


// Functions
void NVM_wait_states(uint8_t rws);
void NVM_flash_read(uint8_t * buffer, uint16_t * address, uint32_t num);
void NVM_flash_erase_row(uint32_t address);
void NVM_flash_write(uint8_t * buffer, uint32_t address, uint8_t num);
void NVM_clear_page_buffer(void);


#endif // L21_NVM_H_