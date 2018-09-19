//ver1.0
#define F_CPU	16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include "usart.h"
#include "I2C_LCD.h"

static FILE usart0_stdio = FDEV_SETUP_STREAM(USART0_send, USART0_receive, _FDEV_SETUP_RW);

volatile uint8_t distance;
volatile uint8_t Hundred = 0, Ten = 0, One = 0;
volatile uint8_t numbers[] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x27, 0x7f, 0x67};
//0		1		2	3		4	5		6	7		8	9
//0011 1111, 0000 0110, 0101 1011, 0100 1111, 0110 0110, 0110 1101, 0111 1101, 0010 0111, 0111 1111, 0110 0111

#define PRESCALER 1024	//분주비
void Timer_init(void)
{
	TCCR0 |= (1<<CS02) | (1<<CS01) | (1<<CS00);	//분주비 1024
}

uint8_t measure_distance(void)
{
	//트리거 핀으로 펄스 출력
	PORTG &= ~(0x10);	//L
	_delay_us(1);
	PORTG |= (0x10);	//H
	_delay_us(10);
	PORTG &= ~(0x10);	//L
	
	//에코 핀 H 되기까지 대기
	TCNT0 = 0;
	while(!(PING & 0x08))
	if(TCNT0 > 65000) return 0;	//장애물 없는 경우
	
	//에코 핀 L 될때까지 시간 측정
	TCNT0 = 0;
	while(PING & 0x08)
	{
		if (TCNT0 > 65000)
		{
			TCNT0 = 0;
			break;
		}
	}
	
	//에코 핀의 펄스폭을 us 단위로 계산
	double pulse_width = 1000000.0 * TCNT0 * PRESCALER / F_CPU;
	return pulse_width / 58;	//cm 단위 거리 반환
}
ISR(TIMER0_OVF_vect)
{
	//세그먼트
	Hundred = distance / 100;
	Ten = distance % 100 / 10;
	One = distance % 100 % 10;

	PORTF = 0xD0;
	PORTC = numbers[Hundred];
	_delay_ms(4);
	PORTF = 0xB0;
	PORTC = numbers[Ten];
	_delay_ms(4);
	PORTF = 0x70;
	PORTC = numbers[One];
}
int main(void)
{
	char lcd_buff0[20], lcd_buff1[20];
	
	//세그먼트
	DDRC = 0xff;	//세그먼트 LED 제어 8핀
	DDRF = 0xf0;	//자릿수 선택핀 4개

	stdin = stdout = stderr = &usart0_stdio;
	USART_init(BR9600);
	
	I2C_LCD_init();
	sei();
	
	DDRG |= 0x10;		//트리거 출력
	DDRG &= ~(0x08);	//에코 입력
	
	Timer_init();
	
	printf("System Start\r\n");
	I2C_LCD_write_string_XY(1, 0, "System Start");
	
	
	
	TIMSK |= (1<<TOIE0);
	
	while(1)
	{
		distance = measure_distance();	//거리측정
		printf("Distance : %03d cm\r\n", distance);
		
		if (distance <= 10)
		{
			sprintf(lcd_buff0, "Danger");
			I2C_LCD_write_string_XY(0, 0, lcd_buff0);
		}
		else if ((distance > 10) && (distance <= 50))
		{
			sprintf(lcd_buff0, "Watch ");
			I2C_LCD_write_string_XY(0, 0, lcd_buff0);
		}
		else if ((distance > 50) && (distance <= 120))
		{
			sprintf(lcd_buff0, "Normal");
			I2C_LCD_write_string_XY(0, 0, lcd_buff0);
		}
		else if (distance > 120)
		{
			sprintf(lcd_buff0, "Safety");
			I2C_LCD_write_string_XY(0, 0, lcd_buff0);
		}
		
		sprintf(lcd_buff1, "Distance : %03dcm", distance);
		I2C_LCD_write_string_XY(1, 0, lcd_buff1);
		_delay_ms(400);
	}
	return 0;
}