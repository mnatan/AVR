all: main.c
	avr-gcc -mmcu=atmega8 -Os -c -DF_CPU=1000000UL main.c -o main.o
	avr-gcc -mmcu=atmega8 main.o -o main.elf
	avr-objcopy -O ihex -R .eeprom main.elf main.hex

