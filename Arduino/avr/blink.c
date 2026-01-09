/** AVR example : blink.c **/

#include <avr/io.h>
#include <util/delay.h>

int main()
{
    DDRB = 0x20;    // Arduino 13 pin : ATmega328P

    while(1)
    {
        PORTB = 0x00;       // LED Off
	_delay_ms(200);    // 1 second delay

        PORTB = 0x20;       // LED On
	_delay_ms(200);    // 1 second delay
    }

    return 0;
}
