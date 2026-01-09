# AVR example : Makefile

SRC = 3_avr_timer_ovf

CC = avr-gcc
OBJCOPY = avr-objcopy
SIZE = avr-size

AVR_ISP = avrdude
#PORT = /dev/ttyUSB0
PORT = usb
AVR_MCU = atmega328p
#PROGRAMMER = avrisp2
PROGRAMMER = usbasp
F_CPU = 16000000UL

all:
	$(CC) -Wall -Os -DF_CPU=$(F_CPU) -mmcu=$(AVR_MCU) -o $(SRC).elf $(SRC).c
	$(OBJCOPY) -j .text -j .data -O ihex $(SRC).elf $(SRC).hex
	$(SIZE) --format=avr --mcu=$(MCU) $(SRC).elf

upload:
	sudo $(AVR_ISP) -v -p $(AVR_MCU) -c $(PROGRAMMER) -P $(PORT) -U flash:w:$(SRC).hex:i

clean:
	rm *.elf *.hex

