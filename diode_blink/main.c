#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

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
	TCCR2 |= _BV(CS20);         				// No prescaling

	serial_println("Initialized.");

	OCR2 = 0;
	_delay_ms(500);
	while (1)
	{
		serial_println("Rozjaśniam diodę");
		for (OCR2 = 0; OCR2 < 255; OCR2++)
		{
			_delay_ms(10);
		}
		serial_println("Przyciemniam diodę");
		for (OCR2 = 255; OCR2 > 0; OCR2--)
		{
			_delay_ms(10);
		}
	}

}
