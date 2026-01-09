/** AVR example : avr_ext_interrupt.c **/

#include <avr/io.h>
#include <avr/interrupt.h>

#define CPU_CLOCK	16000000
#define TICKS_PER_SEC	1000

volatile unsigned int tic_time;
volatile char int_flag;		// interrupt flag

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

// INT0 (PD2) external interrupt
ISR(INT0_vect)
{
    int_flag = 1;
}

int main()
{
    DDRB = 0x20;    // Arduino 13 pin : ATmega328P
    PORTB = 0x00;	// LED Off

    // time0 init
    TCCR0B = (0 << CS02) | (1 << CS01) | (1 << CS00);	// clk/64
    TCNT0 = 256 - (CPU_CLOCK / TICKS_PER_SEC / 64);	// timer0 count init
    TIMSK0 = (1 << TOIE0);	// timer0 overflow interrupt enable

    // INT0 : falling edge
    EICRA = (1 << ISC01) | (0 << ISC00);
    EIMSK = (1 << INT0);

    // global interrupt enable
    sei();

    while(1)
    {
        // check if external interrupt (INT0 rising edge)
	if (int_flag)
	{
	    int_flag = 0; 	// initiallize int_flag

	    PORTB = 0x20;

	    delay_ms(1000);

	    PORTB = 0x00;
	}

    }

    return 1;
}
