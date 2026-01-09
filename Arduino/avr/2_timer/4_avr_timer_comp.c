/** AVR example : 4_avr_timer_comp.c **/

#include <avr/io.h>
//#include <util/delay.h>
#include <avr/interrupt.h>

// ATmega328P (PB5) pin = Arduino (13) pin
#define DDR_LED		DDRB
#define PORT_LED	PORTB

// 8-bit Timer/Counter0
#define CPU_CLOCK	16000000
#define TICKS_PER_SEC	1000

volatile unsigned int tic_time;

// Compare Output mode
ISR(TIMER0_COMPA_vect)
{
    tic_time++;
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

    DDR_LED = 0x20;     // LED Port init -> Output
    PORT_LED = 0x00;	// LED Off

    // Timer init
    TCCR0A =                (1 << WGM01) | (1 << WGM00);		// CTC mode
    TCCR0B = (0 << WGM02) | (0 << CS02) | (1 << CS01) | (1 << CS00);	// clk/64
    TIMSK0 = (1 << OCIE0A);	// Compare Output interrupt enable
    TCNT0 = 0;
    OCR0A = (CPU_CLOCK / TICKS_PER_SEC / 64) - 1;	// "n - 1" : Register start begin with "0"

    // global interrupt enable
    sei();

    while(1)
    {
        // LED toggle
	led_status = ~led_status;
	PORTB = led_status;

	//_delay_ms(500);
	delay_ms(1000);
    }

    return 1;
}
