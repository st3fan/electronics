// Blink2313 - Blink a LED on PD5 of a ATTiny 2313

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

volatile uint8_t count;

ISR(INT0_vect)
{
    count++;
    if (count == 3) {
        PORTB ^= (1 << PB3);
        count = 0;
    }
}

int main(void)
{
    count = 0;

    DDRB = (1 << PB3);

    // Listen for INT0 interrups

    MCUCR = (1 << ISC01);
    GIMSK = (1 << INT0);
    sei();
        
    while (1) {
        // Nothing
    }

    return 0;
}
