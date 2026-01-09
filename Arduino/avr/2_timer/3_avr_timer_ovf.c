/** AVR example : avr_timer.c **/

#include <avr/io.h>
//#include <util/delay.h>
#include <avr/interrupt.h>

#define CPU_CLOCK	16000000
#define TICKS_PER_SEC	1000

volatile unsigned int tic_time;

// timer0
ISR(TIMER0_OVF_vect)
{
    tic_time++;

    TCNT0 = 256 - (CPU_CLOCK / TICKS_PER_SEC / 64);
}

// msec delay
void delay_ms(unsigned int msec)
{
    tic_time = 0;

    while (msec > tic_time);
}

int main()
{
    unsigned char led_status = 0xFF;

    DDRB = 0x20;    // Arduino 13 pin : ATmega328P
    PORTB = 0x00;	// LED Off

    // time0 init
    TCCR0B = (0 << CS02) | (1 << CS01) | (1 << CS00);	// clk/64
    TCNT0 = 256 - (CPU_CLOCK / TICKS_PER_SEC / 64);	// timer0 count init
    TIMSK0 = (1 << TOIE0);	// timer0 overflow interrupt enable

    // global interrupt enable
    sei();

    while(1)
    {
        // LED toggle
	led_status = ~led_status;
	PORTB = led_status;

	// delay
	//_delay_ms(500);
	delay_ms(1000);
    }

    return 1;
}

/*
 * 설정 : 1ms 마다 타이머 overflow interrupt가 발생하도록 타이머 설정
 * Interrupt : interrupt가 발생할 때마다 tic_time 변수를 1++해서 1ms가 지났음을 기록
 * 메인 루프 : LED를 켜거나 끈 뒤, tic_time 변수가 1000(1초)가 될 때까지 기다림
 * 반복 : 3번 과정을 계속 반복해 LED가 1초마다 깜빡임
 */

