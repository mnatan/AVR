// FIXME The diode acts silly
// TODO  Nice comments 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

uint16_t mapRange(uint16_t s, uint16_t a1, uint16_t a2, uint16_t b1, uint16_t b2)
{
	return b1 + (s - a1) * (b2 - b1) / (a2 - a1);
}

void my_delay_us(uint16_t n)
{
	while (n--)
		_delay_us(20);
}

void serial_putc(uint8_t byte)
{
	while (!(UCSRA & _BV(UDRE))) {}
	UDR = byte;
}
void serial_puts(const char *str)
{
	while (*str != '\0')
		serial_putc(*(str++));
}
void serial_println(const char *str)
{
	serial_puts(str);
	serial_puts("\n\r");
}

char buffer [100];
uint16_t przyspieszenie;

int main()
{
	//INIT

	// Config output
	DDRB |= _BV(PB3);  // output diody na PB3 (bo OC2)

	// Enable UART
	UBRRL = 12; // set oscilator frequency to 9600bps
	UCSRA |= _BV(U2X); // same ^^
	UCSRB = (1 << RXEN) | ( 1 << TXEN); //reciver Enable, transmiter Enable

	// Timer 2
	TCCR2 |= _BV(WGM21) | _BV(WGM20);           // PWM
	TCCR2 |= _BV(COM21);                        // set on CM reset on BOT
	TCCR2 |= _BV(CS20);                         // No prescaling

	// Konfiguracja ADC
	ADMUX |= _BV(REFS1) | _BV(REFS0);           // Reference to 2.56V
	ADMUX |= 0b0101;                            // Channel - ADC5 (PC5)
	ADCSRA |= _BV(ADEN);                        // Enable ADC
	ADCSRA |= _BV(ADPS2);                       // Clock prescaling to 16

	przyspieszenie = 1000;
	serial_println("Initialized.");

	OCR2 = 0;
	_delay_ms(1500);
	while (1)
	{
		ADCSRA |= _BV(ADSC);                    // Start convertion from ADC5
		while (ADCSRA & _BV(ADSC));             // Wait for converter
		przyspieszenie = mapRange(ADC, 0, 1023, 5, 50);
		przyspieszenie *= 100;
		serial_puts("Odczyt napięcia: ");
		itoa(ADC, buffer, 10);
		serial_puts(buffer);
		serial_puts(", Temperatura: ");
		itoa( ADC / 4, buffer, 10);
		serial_puts(buffer);
		serial_puts(" C");
		serial_puts("          \r");

		for (OCR2 = 0; OCR2 < 150; OCR2++)
		{
			my_delay_us(przyspieszenie);
		}
		for (OCR2 = 150; OCR2 > 0; OCR2--)
		{
			my_delay_us(przyspieszenie);
		}
	}
}
