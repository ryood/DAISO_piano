/*
* PWM.c
*
* PD3: PWM out
*
* Created: 2016/07/12 20:01:13
*  Author: gizmo
*/

#define F_CPU    8000000ul

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdint.h>

#define SAMPLE_CLOCK    (16000)
#define POW_2_16        (65536ul)

#define NOTE_A3	440.00f
#define NOTE_B3	493.88f
#define NOTE_C4	523.25f
#define NOTE_D4	587.32f
#define NOTE_E4	659.25f
#define NOTE_F4	698.45f
#define NOTE_G4	783.99f
#define NOTE_A4	880.00f
#define NOTE_B4 987.76f
#define NOTE_C5	1046.50f
#define NOTE_D5	1174.65f
#define NOTE_E5	1318.51f
#define NOTE_F5	1396.91f
#define NOTE_G5	1567.98f

// Sine wave table
const PROGMEM uint8_t sineTable[] = {
	32,
	28,
	25,
	22,
	19,
	16,
	14,
	11,
	9,
	7,
	5,
	3,
	2,
	1,
	0,
	0,
	0,
	0,
	0,
	1,
	2,
	3,
	5,
	7,
	9,
	11,
	14,
	16,
	19,
	22,
	25,
	28,
	32,
	35,
	38,
	41,
	44,
	47,
	49,
	52,
	54,
	56,
	58,
	60,
	61,
	62,
	63,
	63,
	64,
	63,
	63,
	62,
	61,
	60,
	58,
	56,
	54,
	52,
	49,
	47,
	44,
	41,
	38,
	35,
};

volatile uint16_t phaseAccumlator;
volatile uint16_t tuningWord;

//=============================================================================
// 波形生成
//
// ----------------------------------------------------------------------------
// setDDSParameter()
// parameter: frequency: 生成する周波数
//
void setDDSParameter(float frequency)
{
	tuningWord = (uint16_t)(frequency * POW_2_16 / SAMPLE_CLOCK);
}

// ----------------------------------------------------------------------------
// generateWave()
// return: 出力値(0..255)
//
uint8_t generateSawWave()
{
	uint8_t index;
	
	phaseAccumlator += tuningWord;
	
	// 右へシフト: 16bit -> 6bit(64個)
	index = phaseAccumlator >> 10;
	return pgm_read_byte(&sineTable[index]);
}

//=============================================================================
// 波形生成
//
// ----------------------------------------------------------------------------
// setPWMDuty()
// parameter: value: 設定するDuty(0 .. OCR0A)
//
void setPWMDuty(uint8_t value)
{
	// PWMのデューティー比を設定
	OCR0B = value;
}

//=============================================================================
// Timer1割り込みハンドラ
//
// ----------------------------------------------------------------------------
//
ISR(TIMER1_COMPA_vect)
{
	uint8_t v;	// PWM出力値
	
	// Debug用: PD6
	PORTD |= (1 << PORTD6);
	
	v = generateSawWave();
	setPWMDuty(v);
	
	// Debug用: PD6
	PORTD &= ~(1 << PORTD6);
}

//=============================================================================
// メイン・ルーチン
//
int main()
{
	//-------------------------------------------------------------------------
	// PORT設定
	//-------------------------------------------------------------------------
	// DDRD = 0;
	DDRD |= (1 << DDD5);    // PD5(OC0B): PWM out
	// Debug用
	//
	DDRD |= (1 << DDB6);    // PD6: output
	
	//-------------------------------------------------------------------------
	// PWM設定
	//-------------------------------------------------------------------------
	// 波形生成モード: WGM0: 1:1:1
	// 高速PWM(モード7)
	TCCR0A |= (1 << WGM01) | (1 << WGM00);
	TCCR0B |= (1 << WGM02);
	//-------------------------------------------------------------------------
	// コンペア・アウトプットB: COM0B: 1:0
	// コンペア・マッチでOC0Bクリア、TOPでOC0Bセット
	TCCR0A |= (1 << COM0B1) | (0 << COM0B0);
	//-------------------------------------------------------------------------
	// クロック設定: CS0: 0:0:1
	// 分周なし
	TCCR0B |= (0 << CS02) | (0 << CS01) | (1 << CS00);
	
	// TCCR0A = 0b00100011;
	// TCCR0B = 0B00001001;
	
	//-------------------------------------------------------------------------
	// 分解能 6bit(0 .. 63)
	OCR0A = 64;
	
	//-------------------------------------------------------------------------
	// Timer1設定
	//-------------------------------------------------------------------------
	// 波形生成モード: WGM0: 1:1:1:1
	// 高速PWM(モード15)
	TCCR1B |= (1 << WGM13) | (1 << WGM12);
	TCCR1A |= (1 << WGM11) | (1 << WGM10);
	//-------------------------------------------------------------------------
	// クロック設定: CS1: 0:1:0
	// 1/8分周
	TCCR1B |= (1 << CS11);
	//-------------------------------------------------------------------------
	// Compare 1A match interrupt enable
	TIMSK1 = (1 << OCIE1A);
	//-------------------------------------------------------------------------
	// Compare resister 1A
	// サンプリングレートの設定
	// (F_CPU / prescaler) / SAMPLE_RATE
	OCR1A = (F_CPU / 8) / SAMPLE_CLOCK;
	
	setDDSParameter(NOTE_A3);	// 440Hz
	
	sei();

	for (;;) {
		setDDSParameter(NOTE_C4);
		_delay_ms(500);
		setDDSParameter(NOTE_D4);
		_delay_ms(500);
		setDDSParameter(NOTE_E4);
		_delay_ms(500);
		setDDSParameter(NOTE_F4);
		_delay_ms(500);
		setDDSParameter(NOTE_G4);
		_delay_ms(500);
		setDDSParameter(NOTE_A4);
		_delay_ms(500);
		setDDSParameter(NOTE_B4);
		_delay_ms(500);
		setDDSParameter(NOTE_C5);
		_delay_ms(500);
	}
}
// EOF