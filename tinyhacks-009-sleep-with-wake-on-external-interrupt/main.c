// tinyhacks-007-sleep/main.c

#include <avr/io.h>
#include <avr/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

ISR(INT0_vect)
{
    // Disable the INT0 interrupt to prevent it from firing before we are ready to handle it again
    GIMSK &= ~(1 << INT0);
}

int main(void)
{
    // PB0 is connected to a LED
    DDRB |= (1 << PB0);

    // Disable all internal systems during power down mode
    PRR = (1 << PRTIM1) | (1 << PRTIM0) | (1 << PRUSI) | (1 << PRADC);
    
    sei();

    while (1)
    {
        GIMSK |= (1 << INT0); // This needs to be before the sleep instruction

        // Sleep in power down mode. Before we sleep we enable the INT0 interrupt.

        set_sleep_mode(SLEEP_MODE_PWR_DOWN);
        sleep_enable();
        sleep_cpu();
        
        // If we have been woken up 4 times then it is time to do 'work'
        
        // Flash the led three times
        for (uint8_t i = 0; i < 3; i++) {
            PORTB |= (1 << PB0);
            _delay_ms(50);
            PORTB &= ~(1 << PB0);
            _delay_ms(50);
        }
        
        // Wait while the line goes up again
        
        while ((PINB & (1 << PB2)) == 0) {
            // Wait
        }
    }

    return 0;
}
