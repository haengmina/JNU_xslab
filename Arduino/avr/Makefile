# AVR example : Makefile

SRC = blink
#PORT = /dev/ttyUSB0
PORT = usb
AVR_MCU = atmega328p
#PROGRAMMER = avrisp2
PROGRAMMER = usbasp
F_CPU = 16000000UL

all:
	avr-gcc -Wall -Os -DF_CPU=$(F_CPU) -mmcu=$(AVR_MCU) -o $(SRC).elf $(SRC).c
	avr-objcopy -j .text -j .data -O ihex $(SRC).elf $(SRC).hex
	avr-size --format=avr --mcu=$(MCU) $(SRC).elf

upload:
	sudo avrdude -v -p $(AVR_MCU) -c $(PROGRAMMER) -P $(PORT) -U flash:w:$(SRC).hex:i

clean:
	rm *.elf *.hex

