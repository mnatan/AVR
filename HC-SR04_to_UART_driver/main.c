#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

// BEGIN UART DRIVER
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
// END UART DRIVER

// Globals
volatile uint8_t  ready     = 0;
volatile uint16_t tics      = 0;
volatile uint16_t end_pulse = 0;

ISR(TIMER0_OVF_vect)          // ISR called every 32us due to Timer0 overflow
{
	++tics;
}

ISR(INT0_vect)                // ISR called by the HC-SR04
{
	if (MCUCR & _BV(ISC00))   // If we were waiting for device to start working (raising edge)
	{
		tics = 0;             // Reset the timer
		MCUCR &= ~_BV(ISC00); // Make mcu call this function when the HC-SR04 finishes measurement
	}
	else                      // Device finished measurement
	{
		end_pulse = tics;     // save the measurement
		ready = 1;            // Set the flag, that indicates the measurement is finished
		MCUCR |= _BV(ISC00);  // Przerwanie przy zboczu malejącym na INT0
	}
}

void trigger_sensor()
{
	ready = 0;				// Make sure main loop waits for HC-SR04 to finish
	PORTB |= _BV(PB1);		// 
	_delay_us(10);			//  Give the HC-SR04 signal to start measurement
	PORTB &= ~_BV(PB1);		// 
}

char buffer [100];

int initialize(){
	DDRD &= ~(_BV(PD2));                //
	PORTD |= _BV(PD2);                  // not sure why it is here - no time to check :)

	DDRB |= _BV(PB1);                   // PB1 triggers the sensor

	// Enable UART
	UBRRL = 25;                         // set oscilator frequency to 9600bps
	UCSRA |= _BV(U2X);
	UCSRB = (1 << RXEN) | ( 1 << TXEN);

	// Timer 0
	TCCR0 |= _BV(CS00);
	TIMSK |= _BV(TOIE0);

	// Interrupts
	GICR |= _BV(INT0);                  // Interrupts on INT0
	MCUCR |= _BV(ISC01) | _BV(ISC00);   // Rising edge on INT0 interrupts
	sei();                              // Interrupts on

	_delay_ms(500);                     // Wait for HC-SR04 to initialize (this was so frustrating to find)
	serial_println("Initialized.");

	trigger_sensor();                   // start first measurement
}

int main()
{
	initialize();
	while (1)
	{
		_delay_ms(250);
		if (ready)                                // If there measurement is finished
		{
			trigger_sensor();                     // start the next one
			if (end_pulse > 0)                    // if it is correct
			{
				end_pulse *= 32;
				end_pulse /= 58;                  // 58 us means 1 cm for us.

				itoa(end_pulse, buffer, 10);

				serial_puts("The disstance is ");
				serial_puts(buffer);
				serial_println("cm.");
			}
		}
	}
}
