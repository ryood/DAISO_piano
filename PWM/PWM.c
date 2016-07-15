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

#define NOTE_C2	130.81f
#define NOTE_D2	146.83f
#define NOTE_E2	164.81f
#define NOTE_F2	174.61f
#define NOTE_G2	195.99f
#define NOTE_A2	220.00f
#define NOTE_B2	246.94f
#define NOTE_C3	261.62f
#define NOTE_D3	293.66f
#define NOTE_E3	329.62f
#define NOTE_F3	349.22f
#define NOTE_G3	391.99f
#define NOTE_A3	440.00f
#define NOTE_B3	493.88f
#define NOTE_C4	523.25f

#define WAVE_SIN	0
#define WAVE_SAW	1
#define WAVE_SQR	2
#define WAVE_N		3

// wave table
const PROGMEM uint8_t sinTable[] = {
	31,	28,	25,	22,	19,	16,	14,	11,	9,	7,	5,	3,	2,	1,	0,	0,
	0,	0,	0,	1,	2,	3,	5,	7,	9,	11,	14,	16,	19,	22,	25,	28,
	31,	35,	38,	41,	44,	47,	49,	52,	54,	56,	58,	60,	61,	62,	63,	63,
	63,	63,	63,	62,	61,	60,	58,	56,	54,	52,	49,	47,	44,	41,	38,	35,
};
const PROGMEM uint8_t sawTable[] = {
	0,	1,	2,	3,	4,	5,	6,	7,	8,	9,	10,	11,	12,	13,	14,	15,
	16,	17,	18,	19,	20,	21,	22,	23,	24,	25,	26,	27,	28,	29,	30,	31,
	32,	33,	34,	35,	36,	37,	38,	39,	40,	41,	42,	43,	44,	45,	46,	47,
	48,	49,	50,	51,	52,	53,	54,	55,	56,	57,	58,	59,	60,	61,	62,	63,
};
const PROGMEM uint8_t sqrTable[] = {
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
	63,	63,	63,	63,	63,	63,	63,	63,	63,	63,	63,	63,	63,	63,	63,	63,
	63,	63,	63,	63,	63,	63,	63,	63,	63,	63,	63,	63,	63,	63,	63,	63,
};

volatile uint16_t phaseAccumlator;
volatile uint16_t tuningWord;

uint8_t const *waveForms[WAVE_N];
volatile uint8_t  waveForm;

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
	return pgm_read_byte(waveForms[waveForm] + index);
}

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
	
	#if DEBUG
	PORTD |= (1 << PORTD6);
	#endif
	
	v = generateSawWave();
	setPWMDuty(v);
	
	#if DEBUG
	PORTD &= ~(1 << PORTD6);
	#endif
}

//=============================================================================
// メイン・ルーチン
//
int main()
{
	waveForms[0] = sinTable;
	waveForms[1] = sawTable;
	waveForms[2] = sqrTable;
	
	//-------------------------------------------------------------------------
	// PORT設定
	//-------------------------------------------------------------------------
	DDRD |= (1 << DDD5);    // PD5(OC0B): PWM out
	
	#if DEBUG
	DDRD |= (1 << DDB6);    // PD6: output
	#endif
	
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
	
	//setDDSParameter(1000);
	
	sei();

	for (;;) {
		// 波形切替
		waveForm++;
		if (waveForm == WAVE_N) {
			waveForm = 0;
		}
		setDDSParameter(NOTE_C2);
		_delay_ms(500);
		setDDSParameter(NOTE_D2);
		_delay_ms(500);
		setDDSParameter(NOTE_E2);
		_delay_ms(500);
		setDDSParameter(NOTE_F2);
		_delay_ms(500);
		setDDSParameter(NOTE_G2);
		_delay_ms(500);
		setDDSParameter(NOTE_A2);
		_delay_ms(500);
		setDDSParameter(NOTE_B2);
		_delay_ms(500);
		setDDSParameter(NOTE_C3);
		_delay_ms(500);
		setDDSParameter(NOTE_D3);
		_delay_ms(500);
		setDDSParameter(NOTE_E3);
		_delay_ms(500);
		setDDSParameter(NOTE_F3);
		_delay_ms(500);
		setDDSParameter(NOTE_G3);
		_delay_ms(500);
		setDDSParameter(NOTE_A3);
		_delay_ms(500);
		setDDSParameter(NOTE_B3);
		_delay_ms(500);
		setDDSParameter(NOTE_C4);
		_delay_ms(500);
	}
}
// EOF