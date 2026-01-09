/** AVR example : avr_io.c **/

#include <avr/io.h>
#include <util/delay.h>

// I/O Ports
#define DDR_LED		DDRB
#define PORT_LED	PORTB

int main()
{
    unsigned char led_status = 0xFF;

    DDR_LED = 0x20;    	// Arduino 13 pin : ATmega328P
    PORT_LED = 0x00;	// LED Off

    while(1)
    {
        // LED toggle
	led_status = ~led_status;
	PORTB = led_status;

	_delay_ms(1000);
    }

    return 0;
}

    /*
    DDR_LED = 0x20; DDRB 레지스터를 0x20으로 설정. 
    Arduino 13 핀(ATmega328P의 PB5)을 출력 모드로 설정함.
    
    PORTB의 모든 핀을 low로 설정해 LED를 초기에는 끔.
    while 루프 내에서 led_status 변수를 반전시켜 LED를 토글함.
    
    */