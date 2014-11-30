/*
 * df4iah_commands.c
 *
 *  Created on: 01.11.2014
 *      Author: espero
 */

#include <avr/boot.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#include "df4iah_memory.h"

#include "df4iah_serial.h"
#include "df4iah_usb.h"
#include "main.h"


#ifdef RELEASE
__attribute__((section(".df4iah_memory"), aligned(2)))
#endif
void eraseFlash(void)
{
	// erase only main section (bootloader protection)
	uint32_t addr = 0;
	while (APP_END >= addr) {
		boot_page_erase(addr);						// perform page erase
		boot_spm_busy_wait();						// wait until the memory is erased.
		addr += SPM_PAGESIZE;
	}
	boot_rww_enable();
}

#ifdef RELEASE
__attribute__((section(".df4iah_memory"), aligned(2)))
#endif
void readFlashPage(uint8_t target[], pagebuf_t size, uint32_t baddr)
{
	uint16_t data;
	uint8_t idx = 0;

	while (size) {
#ifndef READ_PROTECT_BOOTLOADER
# warning "Bootloader not read-protected"
		if (true) {
#else
		// don't read bootloader
		if (baddr <= APP_END) {
#if defined(RAMPZ)
			data = pgm_read_word_far(baddr);
#else
			data = pgm_read_word_near(baddr);
#endif
		}
		else {
			data = 0xFFFF; 							// fake empty
		}
#endif
		target[idx++] = data & 0xff;				// store LSB
		if (--size) {
			target[idx++] = data >> 8;				// store MSB
			baddr += 2;								// select next word in memory
			--size;									// subtract two bytes from number of bytes to read
		}
	}												// repeat until block has been read
}

#ifdef RELEASE
__attribute__((section(".df4iah_memory"), aligned(2)))
#endif
void readEEpromPage(uint8_t target[], pagebuf_t size, uint16_t baddr)
{
	uint8_t idx = 0;

	while (size) {
		target[idx++] = eeprom_read_byte((uint8_t*) baddr++);
		--size;										// decrease number of bytes to read, repeat until block has been read
	}
}

#ifdef RELEASE
__attribute__((section(".df4iah_memory"), aligned(2)))
#endif
void writeFlashPage(uint8_t source[], pagebuf_t size, uint32_t baddr)
{
	uint32_t pagestart = baddr - (baddr % SPM_PAGESIZE);
	uint16_t data;
	uint8_t idx = 0;

	if ((size == SPM_PAGESIZE) && (pagestart == baddr)) {	// clear flash page only if data block is complete and starting at the bottom of the page
		boot_page_erase(pagestart);					// perform page erase
		boot_spm_busy_wait();						// wait until the memory is erased.
	}

	while (size--) {
		data = source[idx++];
		if (size) {
			data |= source[idx++] << 8;
			size--;									// reduce number of bytes to write
		}
		boot_page_fill(baddr, data);				// call asm routine
		baddr += 2;									// select next word in memory
	}												// loop until all bytes are written

	boot_page_write(pagestart);
	boot_spm_busy_wait();
	boot_rww_enable();								// re-enable the RWW section
}

#ifdef RELEASE
__attribute__((section(".df4iah_memory"), aligned(2)))
#endif
void writeEEpromPage(uint8_t source[], pagebuf_t size, uint16_t baddr)
{
	uint8_t idx = 0;

	while (size--) {								// decrease number of bytes to write
		eeprom_write_byte((uint8_t*) baddr, source[idx++]);
		baddr++;									// select next byte
	}												// loop until all bytes written

	// eeprom_busy_wait();
}
