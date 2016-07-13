/*
 * I2C_LCD.c
 *
 * Created: 2016/07/12 18:59:54
 *  Author: gizmo
 *
 * PORTB: SWx8
 *
 */ 

#define F_CPU	8000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdint.h>
#include "SPLC792-I2C.h"

#define TITLE_STR0	"DAISO piano"
#define TITLE_STR1	"2016.07.12"

int main(void)
{
	// Switches port init. input / pullup
	PORTB = 0xFF;
	DDRB  = 0x00;
	
	// I2C LCD init	
	I2C_LCD_init();
	I2C_LCD_puts(TITLE_STR0);
	I2C_LCD_setpos(0, 1);
	I2C_LCD_puts(TITLE_STR1);
	_delay_ms(2000);
	I2C_LCD_clear();
	
	while(1)
    {
		uint8_t switches = PINB;
		
		char lcdBuff[17];
		sprintf(lcdBuff, "SW: %x", switches);
		
		I2C_LCD_home();
		I2C_LCD_puts(TITLE_STR0);
		I2C_LCD_setpos(0, 1);
		I2C_LCD_puts(lcdBuff);		
    }
}
