#include	<avr/io.h>
#include	<avr/interrupt.h>
#include	<stdio.h>
#include	"usart.h"

volatile unsigned char rx0_buffer[LENGTH_RX_BUFFER], tx0_buffer[LENGTH_TX_BUFFER];
volatile unsigned char rx0_head=0, rx0_tail=0, tx0_head=0, tx0_tail=0;

// 인터럽트 USART 초기화
void USART_init(unsigned int ubrr_baud)
{
	UCSR0B |= 1<<RXEN0 | 1<<TXEN0 | 1<<RXCIE0;
	UBRR0H = 0;
	UBRR0L = ubrr_baud;
}

// 인터럽트에 의한 문자 전송 호출
int USART0_send(char data, FILE *seam)
{
	while( !(UCSR0A & (1 << UDRE0)) ); // 송신 가능 대기
	UDR0 = data;
	//// txbuffer[] full, 한 개라도 빌 때까지 기다림
	//while( (tx0_head+1==tx0_tail) || ((tx0_head==LENGTH_TX_BUFFER-1) && (tx0_tail==0)) );
	//
	//tx0_buffer[tx0_head] = data;
	//tx0_head = (tx0_head==LENGTH_TX_BUFFER-1) ? 0 : tx0_head+1;
	//UCSR0B = UCSR0B | 1<<UDRIE0;	// 보낼 문자가 있으므로 UDRE1 빔 인터럽트 활성화
//
	//return data;
}

void UART0_printf_string(char *str) // 문자열 송신
{
	for(int i = 0; str[i]; i++) // \0 문자를 만날 때까지 반복 (NULL)
	UART0_transmit(str[i]); // 바이트 단위 출력
}

// 인터럽트에 의한 문자 수신 호출
int USART0_receive(FILE *stream)
{	unsigned char data;
	
	while( rx0_head==rx0_tail );	// 수신 문자가 없음

	data = rx0_buffer[rx0_tail];
	rx0_tail = (rx0_tail==LENGTH_RX_BUFFER-1) ? 0 : rx0_tail + 1;
	
	return data;
}

// USART1 UDR empty interrupt service
//ISR(USART0_UDRE_vect)
//{
	//UDR0 = tx0_buffer[tx0_tail];
	//tx0_tail = (tx0_tail==LENGTH_TX_BUFFER-1) ? 0 : tx0_tail+1;
	//
	//if( tx0_tail==tx0_head)		// 보낼 문자가 없으면 UDRE1 빔 인터럽트 비활성화
	//UCSR0B = UCSR0B & ~(1<<UDRIE0);
//}

// USART1 RXC interrupt service
ISR(USART0_RX_vect)
{	
	volatile unsigned char data;
	
	// rx_buffer[] full, 마지막 수신 문자 버림
	if( (rx0_head+1==rx0_tail) || ((rx0_head==LENGTH_RX_BUFFER-1) && (rx0_tail==0)) ){
		data = UDR0;
		}else{
		rx0_buffer[rx0_head] = UDR0;
		rx0_head = (rx0_head==LENGTH_RX_BUFFER-1) ? 0 : rx0_head+1;
	}
}

// USART1 receive char check
int	USART0_rx_check(void)
{
	return (rx0_head != rx0_tail) ? 1 : 0;
}
