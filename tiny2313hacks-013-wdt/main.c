// main.c

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <avr/wdt.h>

volatile uint8_t counter = 0;

ISR(WDT_OVERFLOW_vect)
{
    counter++;
    
    // Disable the watchdog
    WDTCSR |= (1 << WDCE) | (1 << WDE);
    WDTCSR &= ~(1 << WDE);
}

int main(void)
{
    DDRB = (1 << PB0);

    sei();
    
    while (1)
    {
        // Disable the watchdog
        //WDTCSR |= (1 << WDCE) | (1 << WDE);
        //xoWDTCSR &= ~(1 << WDE);

        // Enable the watchdog. It will fire every 4 seconds.
        WDTCSR = (1 << WDIE) | (1 << WDE) | (1 << WDP0) | (1 << WDP1) | (1 << WDP2);
        
        // Sleep in power down mode. We will be woken up by the watchdog interrupt
        set_sleep_mode(SLEEP_MODE_PWR_DOWN);
        sleep_enable();
        sleep_cpu();

        // When we wake up the watchdog interrupt has fired, chck if we have done enough
        // iterations for an hour. If so, pulse PB0 low.

        if (counter >= 5)
        {
            for (uint8_t i = 0; i < 3; i++) {
                PORTB |= (1 << PB0);
                _delay_ms(25);
                PORTB &= ~(1 << PB0);
                _delay_ms(25);
            }

            counter = 0;
        }

    }

    return 0;
}
