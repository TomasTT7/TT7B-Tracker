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


/*
	Read operations are performed by addressing the NVM main address space or the RWWEE address space directly.
*/
void NVM_flash_read(uint8_t * buffer, uint16_t * address, uint32_t num)
{
	uint16_t * _add = address;
	
	for(uint32_t i = 0; i < num; i+=2)
	{
		uint16_t _temp = *_add;
		
		buffer[i] = (_temp >> 8);
		if(num-1 >= i+1) buffer[i+1] = _temp & 0xFF;
		
		_add++;
	}
}


/*
	Row erases must be performed by issuing commands through the NVM Controller.
	Row Erase:	6ms
	
	ADDRESS
		Any address within the ROW can be used.
*/
void NVM_flash_erase_row(uint32_t address)
{
	NVMCTRL->ADDR.bit.ADDR = address / 2;						// ADDR drives the hardware (16-bit) address to the NVM when a command is executed.
	
	NVMCTRL->CTRLA.reg = (0xA5 << 8) | 0x02;					// CMDEX, CMD - Erase Row (ER) command
	while(!(NVMCTRL->INTFLAG.bit.READY));						// READY bit in INTFLAG register will be low while programming is in progress.
}


/*
	Manual page writes must be performed by issuing commands through the NVM Controller.
	The NVM Controller requires that an erase must be done before programming.
	Page Write:	2.5ms
	
	The page buffer is at address zero, is write only, and must only be written using 16 or 32-bit accesses.
*/
void NVM_flash_write(uint8_t * buffer, uint32_t address, uint8_t num)
{
	uint16_t * page_buffer = 0;									// Page Buffer at address 0x00000000
	
	NVMCTRL->CTRLB.bit.CACHEDIS = 1;							// The cache is disabled.
	NVMCTRL->CTRLB.bit.MANW = 1;								// Write commands must be issued through the CTRLA.CMD register.
	
	for(uint8_t i = 0; i < num; i+=2)
	{
		if(i >= 64) break;										// maximum 64 byte write
		
		if(num-1 < i+1)	*page_buffer = (buffer[i] << 8);		// 16-bit write in case of odd number of bytes
		else *page_buffer = (buffer[i] << 8) | buffer[i+1];		// 16-bit write
		
		page_buffer++;
	}
	
	NVMCTRL->ADDR.bit.ADDR = address / 2;						// page aligned 8-bit address modified to reflect 16-bit addressing
	
	NVMCTRL->CTRLA.reg = (0xA5 << 8) | 0x04;					// CMDEX, CMD - Write Page (WP) command
	while(!(NVMCTRL->INTFLAG.bit.READY));						// READY bit in INTFLAG register will be low while programming is in progress.
	
	NVMCTRL->CTRLB.bit.CACHEDIS = 0;							// The cache is enabled.
}


/*
	 If a partial page has been written and it is desired to clear the contents of the page buffer, the Page Buffer Clear command can be used.
*/
void NVM_clear_page_buffer(void)
{
	NVMCTRL->CTRLA.reg = (0xA5 << 8) | 0x44;					// CMDEX, CMD - Page Buffer Clear (PBC) command
	while(!(NVMCTRL->INTFLAG.bit.READY));						// READY bit in INTFLAG register will be low while programming is in progress.
}