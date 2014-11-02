/*
 * df4iah_serial.c
 *
 *  Created on: 01.11.2014
 *      Author: espero
 */

#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include "df4iah_serial.h"

#include "main.h"


extern uint8_t gBuffer[SPM_PAGESIZE];


__attribute__((section(".df4iah_serial"), aligned(2)))
void init_serial()
{
	// set baud rate
	UART_BAUD_HIGH = ((UART_CALC_BAUDRATE(BAUDRATE)>>8) & 0xFF);
	UART_BAUD_LOW  = ( UART_CALC_BAUDRATE(BAUDRATE)     & 0xFF);

#ifdef UART_DOUBLESPEED
	UART_STATUS = (1<<UART_DOUBLE);
#endif

	UART_CTRL  = UART_CTRL_DATA;
	UART_CTRL2 = UART_CTRL2_DATA;
}

__attribute__((section(".df4iah_serial"), aligned(2)))
void sendchar(uint8_t data)
{
	while (!(UART_STATUS & (1<<UART_TXREADY)));
	UART_DATA = data;
}

__attribute__((section(".df4iah_serial"), aligned(2)))
uint8_t recvchar(void)
{
	while (!(UART_STATUS & (1<<UART_RXREADY)));
	return UART_DATA;
}

__attribute__((section(".df4iah_serial"), aligned(2)))
void recvBuffer(pagebuf_t size)
{
	pagebuf_t cnt;
	uint8_t *tmp = gBuffer;

	for (cnt = 0; cnt < sizeof(gBuffer); cnt++) {
		*tmp++ = (cnt < size) ? recvchar() : 0xFF;
	}
}

// --

__attribute__((section(".df4iah_serial"), aligned(2)))
void send_boot(void)
{
	/*
	sendchar('A');
	sendchar('V');
	sendchar('R');
	sendchar('B');
	sendchar('O');
	sendchar('O');
	sendchar('T');
	*/

	sendchar('F');
	sendchar('D');
	sendchar('L');
	sendchar(' ');
	sendchar('v');
	sendchar(VERSION_HIGH);
	sendchar(VERSION_LOW);
}

__attribute__((section(".df4iah_serial"), aligned(2)))
void ser_error_msg()
{
	sendchar('*');
	sendchar('E');
	sendchar('R');
	sendchar('R');
	sendchar('-');
	sendchar('9');
	sendchar('9');
	sendchar('*');

	cli();
	sleep_cpu();
}
