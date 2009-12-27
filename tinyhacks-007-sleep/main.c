// Blink2313 - Blink a LED on PD5 of a ATTiny 2313

#include <avr/io.h>
#include <avr/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

void mydelay(uint8_t delay)
{
    int n = 0;
    for (int i = 0; i < delay; i++) {
        for (int j = 0; j < 255; j++) {
            n++;
        }
    }
}

volatile uint8_t count;

ISR(INT0_vect)
{
    mydelay(50);

    cli();

    count++;
    
    if (count == 4)
    {
        PORTB |= (1 << PB3);

        // Flash the led three times

        for (uint8_t i = 0; i < 3; i++) {
            PORTB |= (1 << PB0);
            mydelay(50);
            PORTB &= ~(1 << PB0);
            mydelay(50);
        }
        
        PORTB &= ~(1 << PB3);

        count = 0;
    }
        
    // Sleep
        
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable(); 

    MCUCR &= ~(1 << ISC00);
    MCUCR &= ~(1 << ISC01);
    GIMSK = (1 << INT0);
    sei();

    sleep_cpu();
}

int main(void)
{
    count = 0;

    DDRB |= (1 << PB0) | (1 << PB3);

    MCUCR &= ~(1 << ISC00);
    MCUCR &= ~(1 << ISC01);
    GIMSK = (1 << INT0);
    sei();

    while (1) {}

    return 0;
}
