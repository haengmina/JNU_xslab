/** AVR example : avr_io_input.c **/

#include <avr/io.h>
#include <util/delay.h>

// ATmega328P (PB5) pin = Arduino (13) pin
#define DDR_LED		DDRB
#define PORT_LED	PORTB

// ATmega328P (PD5) pin = Arduino (5) pin
#define DDR_KEY		DDRD
#define PIN_KEY		PIND

int main()
{
    DDR_LED = 0x20;    	// LED Port init -> Output
    PORT_LED = 0x00;	// LED Off

    DDR_KEY = 0x00;	// KEY Port init -> Input 으로 설정.

    while(1)
    {
        // KEY input value -> LED output
	PORT_LED = PIN_KEY;
    }

    return 1;
}
