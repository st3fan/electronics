// Blink2313 - Blink a LED on PD5 of a ATTiny 2313

#include <avr/io.h>
#include <avr/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

volatile uint8_t count = 0;

ISR(INT0_vect)
{
    GIMSK &= ~(1 << INT0);
    count++;
}

int main(void)
{
    DDRB |= (1 << PB0);
    MCUCR &= ~(1 << ISC00);
    MCUCR &= ~(1 << ISC01);
    PRR = (1 << PRTIM1) | (1 << PRTIM0) | (1 << PRUSI) | (1 << PRADC);
    
    sei();

    while (1)
    {
        // Sleep

        set_sleep_mode(SLEEP_MODE_PWR_DOWN);
        sleep_enable();
        GIMSK |= (1 << INT0);
        sleep_cpu();
        
        // If we have been woken up 4 times then it is time to do 'work'
        
        if (count == 4)
        {
            // Flash the led three times
            
            for (uint8_t i = 0; i < 3; i++) {
                PORTB |= (1 << PB0);
                _delay_ms(50);
                PORTB &= ~(1 << PB0);
                _delay_ms(50);
            }
        
            // Reset the counter
    
            count = 0;
        }

        // Wait while the line goes up again
        
        while ((PINB & (1 << PB2)) == 0) {
            // Wait
        }
    }

    return 0;
}
