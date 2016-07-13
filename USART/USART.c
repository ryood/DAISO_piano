/*
 * USART.c
 *
 * Created: 2016/07/12 7:26:36
 *  Author: gizmo
 */ 



#include <avr/io.h>

const char init_msg[] = "\n\r Pat1:1 Pat2:2 Pat3:3 Enter Pat# : ";

void USART_Tx(char tx_data)
{
	// 送信バッファが空になるのを待つ
	while (!(UCSR0A & (1 << UDRE0)))
	;
	UDR0 = tx_data;
}

unsigned char USART_Rx()
{
	// 受信バッファにデータが入るのを待つ
	while(!(UCSR0A & (1 << RXC0)))
	;
	return UDR0;
}

void USART_write(const char *msg_string)
{
	unsigned char i = 0;
	while (msg_string[i] != '\0') {
		USART_Tx(msg_string[i]);
		i++;
	}
}


int main(void)
{
	unsigned char command;
	
	// PORT init
	PORTB = 0xFF;
	DDRB  = 0xFF;
	
	// USART init
	UCSR0B = 0x00;	// Disable while setting baud rate
	// UCSR0A = (0 << U2X0) | (0 << MPCM0);	// 2倍速OFF マルチプロセッサモードOFF
	UCSR0A = 0x00;
	// UCSR0C = (0 << UCSZ02) | (1 << UCSZ01) | (1 << UCSZ00);	// 非同期モード 8ビットデータ パリティなし
	UCSR0C = 0x06;
	// UBRR0  = 51;	// 9600bps@8MHz
	UBRR0 = 51;
	// UCSR0B = (1 << RXEN0) | (1 << RXEN0);
	UCSR0B = 0x18;
	
	while(1)
	{
		USART_write(init_msg);
		command = USART_Rx();
		switch(command) {
			case '1':
			USART_Tx(command);
			PORTB = 0xF0;
			break;
			case '2':
			USART_Tx(command);
			PORTB = 0xCC;
			break;
			case '3':
			USART_Tx(command);
			PORTB = 0x55;
			break;
			default:
			USART_Tx('E');
			PORTB = 0xFF;
		}
	}
}
