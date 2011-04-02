
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <avr/sleep.h>
#include <util/delay.h>

// The current values of the RGB LED is stored in three separate
// variables. These represent the amount of time that the color
// is 'on'.

uint8_t red = 0;
uint8_t green = 0;
uint8_t blue = 0;

// We store the current program (effect) in the EEPROM. Although
// this looks like a normal global variable, the GCC specific
// annotations put this in the EEPROM. Note that you cannot read
// this variable directly. You can only reference it and then use
// it in the eeprom_read/write functions.

uint8_t ee_program __attribute__((section(".eeprom"))) = 0;

// This empty interrupt handler must exist for the Pin Change
// interrupt to work. We use it to wake up from sleep.

EMPTY_INTERRUPT(PCINT0_vect);

// The pulse() functions takes care of two things. If checks for button
// presses and it 'pulses' the LED.

void pulse()
{
	// Check if the button is pressed down. If it is then 'debounce'
	// it by waiting 50 milliseconds and checking if the button has
	// been released.

    if ((PINB & (1 << PB3)) == 0)
    {
        _delay_ms(50);

        if ((PINB & (1 << PB3)) == 0)
        {
			// Increment the active program number that we have stored
			// in the EEPROM. We read it, increment it, make sure that
			// it goes back to 0 when we have reached the end and then
			// write it back to the EEPROM.

			uint8_t program = eeprom_read_byte(&ee_program);
			program++;
			if (program == 4) {
				program = 0;
			}
			eeprom_write_byte(&ee_program, program);

			// The AVR has no reset instruction. The common way to
			// cause a chip reset is to start the Watchdog Timer and
			// then loop. The WDT will trigger and reset the device.

			wdt_enable(WDTO_15MS); 
			while (1) {
				// Loop until the WDT fails and does a reset
			}
        }
    }

	// Turn on all colors (if enabled)

    if (red) {
        PORTB |= (1 << PB0);
    }
        
    if (green) {
        PORTB |= (1 << PB1);
    }

    if (blue) {
        PORTB |= (1 << PB2);
    }

	// One duty cycle is 100 steps.

    for (uint8_t i = 0; i < 100; i++)
    {
        if (i == red) {
            PORTB &= ~(1 << PB0);
        }
        
        if (i == green) {
            PORTB &= ~(1 << PB1);
        }
        
        if (i == blue) {
            PORTB &= ~(1 << PB2);
        }

		// Delay 5 micro seconds
        
        _delay_us(5);
    }
}

void sleep()
{
	GIMSK |= (1 << PCIE);
	PCMSK |= (1 << PCINT3);

	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	sleep_enable();
	sleep_cpu();
}

// Program 0 fades between the primary colors. Because we pull the
// RGB values up and down simultaniously, this also results in nice
// fading and many other colors.

void program0()
{
    uint8_t n = 8;

    // To red

    for (uint8_t i = 0; i < n*8; i++) {
        red++;
        for (int j = 0; j < n; j++) {
            pulse();
        }
    }

    for (uint8_t i = 0; i < n * 8; i++) {
        pulse();
    }

    for (int n = 0; n < 255; n++)
    {
        // red to blue

        for (uint8_t i = 0; i < n*8; i++) {
            red--;
            blue++;
            for (int j = 0; j < n; j++) {
                pulse();
            }
        }

        for (uint8_t i = 0; i < n*8; i++) {
            pulse();
        }

        // blue to green

        for (uint8_t i = 0; i < n*8; i++) {
            blue--;
            green++;
            for (int j = 0; j < n; j++) {
                pulse();
            }
        }

        for (uint8_t i = 0; i < n*8; i++) {
            pulse();
        }

        // green to red

        for (uint8_t i = 0; i < n*8; i++) {
            green--;
            red++;
            for (int j = 0; j < n; j++) {
                pulse();
            }
        }

        for (uint8_t i = 0; i < n*8; i++) {
            pulse();
        }
    }
}

// Program 1 loops through red, green and blue. No fading.

void program1()
{
    for (int n = 0; n < 255; n++)
	{
        for (uint8_t i = 0; i < 255; i++) {
            red = 99; green = 0; blue = 0;
            pulse();
        }
        for (uint8_t i = 0; i < 255; i++) {
            red = 0; green = 99; blue = 0;
            pulse();
        }
        for (uint8_t i = 0; i < 255; i++) {
            red = 0; green = 0; blue = 99;
            pulse();
        }
    }
}

// Program 2 continuously fades the red LED in and out.

void program2()
{
    red = green = blue = 0;

	for (int n = 0; n < 255; n++)
	{
        for (uint8_t i = 0; i < 100; i++) {
            red++;
            pulse();
        }
        for (uint8_t i = 0; i < 100; i++) {
            red--;
            pulse();
        }
	}
}

// Program 3 simply cycles through some known nice colors.

void program3()
{
	for (int n = 0; n < 255; n++)
	{
        for (uint8_t i = 0; i < 255; i++) {
            red = 99; green = 0; blue = 0;
            pulse();
        }
        for (uint8_t i = 0; i < 255; i++) {
            red = 99; green = 99; blue = 0;
            pulse();
        }
        for (uint8_t i = 0; i < 255; i++) {
            red = 0; green = 99; blue = 0;
            pulse();
        }
        for (uint8_t i = 0; i < 255; i++) {
            red = 0; green = 99; blue = 99;
            pulse();
        }
        for (uint8_t i = 0; i < 255; i++) {
            red = 0; green = 0; blue = 99;
            pulse();
        }
        for (uint8_t i = 0; i < 255; i++) {
            red = 99; green = 0; blue = 99;
            pulse();
        }
    }
}

// The main program that is executed when the chip is powered on.

int main(void)
{
    sei();

	// Configure pins PB0, PB1 and PB2 as output pins. These are the
	// pins that are driving the RGB LED.

    DDRB = (1 << PB0) | (1 << PB1) | (1 << PB2);

	// We have a switch connected to PB3. Setting the PB3 bit in the
	// PORTB registers turns on the Internal Pull Up Resistor.

    PORTB |= (1 << PB3);

#if 0
        for (uint8_t i = 0; i < 5; i++) {
			PORTB |= (1 << PB1);
			_delay_ms(250);
			PORTB &= ~(1 << PB1);
			_delay_ms(250);
		}
#endif

	// Clear the CPU status register and disable the Watchdog Timer.
	// We do this because we might arrive here because the user
	// pressed the button, which causes a reset by the Watchdog Timer.

    MCUSR = 0;
    wdt_disable();

	// Load the program number from the EEPROM. We check if the number
	// is in range of the allowed values. It might not be if the EEPROM
	// is not properly initialized. In that case we reset the program
	// to 0 and write it back to the EEPROM.
    
	uint8_t program = eeprom_read_byte(&ee_program);
	if (program > 3) {
		program = 0;
		eeprom_write_byte(&ee_program, program);
	}

	// Run a loop where we start the program and then go to sleep.
	// When we wake up we delay a bit to debounce the switch and
	// continue with the same program.

	while (1)
	{
		switch (program) {
			case 0:
				program0();
				break;
			case 1:
				program1();
				break;
			case 2:
				program2();
				break;
			case 3:
				program3();
				break;
		}

		sleep();
		_delay_ms(250);
	}
    
    return 0;
}
