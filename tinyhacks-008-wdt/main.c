// Blink2313 - Blink a LED on PD5 of a ATTiny 2313

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/delay.h>

volatile uint8_t count = 0;

ISR(WDT_vect)
{
    // Ping the watchdog LED

    PORTB |= (1 << PB1);
    _delay_ms(5);
    PORTB &= ~(1 << PB1);
    _delay_ms(5);

    // Disable the watchdog
    
    //WDTCR |= (1 << WDCE) | (1 << WDE);
    //WDTCR &= ~(1 << WDE);

    // Increment the timer
    
    count++;
}

int main(void)
{
    DDRB = (1 << PB0) | (1 << PB1);

    sei();
    
    while (1)
    {
        // Enable the watchdog

        WDTCR = (1 << WDIE) | (1 << WDE) | (1 << WDP2) | (1 << WDP1) | (1 << WDP0);

        // Sleep in power down mode. We will be woken up by the watchdog interrupt

        set_sleep_mode(SLEEP_MODE_PWR_DOWN);
        sleep_enable();
        sleep_cpu();

        // When we wake up the watchdog interrupt has fired. Blink the LED.

        for (uint8_t i = 0; i < count; i++)
        {
            PORTB |= (1 << PB0);
            _delay_ms(50);
            PORTB &= ~(1 << PB0);
            _delay_ms(50);
        }
        
        // We blink up to 5 times
        
        if (count == 5) {
            count = 0;
        }
        
    }

    return 0;
}
