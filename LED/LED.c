/*
 * LED.c
 *
 * Created: 2016/07/12 5:19:27
 *  Author: gizmo
 */ 

#define F_CPU	8000000UL

#include <avr/io.h>
#include <util/delay.h>

int main(void)
{
	PORTB = 0xFF;
	DDRB  = 0xFF;
	
    while(1)
    {
        PORTB = 0xAA;
		_delay_ms(500);
		PORTB = 0x55;
		_delay_ms(500);
    }
}