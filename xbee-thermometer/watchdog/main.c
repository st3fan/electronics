// Blink2313 - Blink a LED on PD5 of a ATTiny 2313

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>

volatile uint8_t counter = 0;

ISR(WDT_vect)
{
    counter++;
}

int main(void)
{
    DDRB = (1 << PB0);

    // Watchdog signal will be active low
    PORTB |= (1 << PB0);

    // Setup the power reduction register
    PRR = (1 << PRTIM1) | (1 << PRTIM0) | (1 << PRUSI) | (1 << PRADC);

    sei();
    
    while (1)
    {
        // Disable the watchdog
    
        WDTCR |= (1 << WDCE) | (1 << WDE);
        WDTCR &= ~(1 << WDE);
        
        // Enable the watchdog. It will fire every 4 seconds.
        
        WDTCR = (1 << WDIE) | (1 << WDE) | (1 << WDP3);

        // Sleep in power down mode. We will be woken up by the watchdog interrupt

        set_sleep_mode(SLEEP_MODE_PWR_DOWN);
        sleep_enable();
        sleep_cpu();

        // When we wake up the watchdog interrupt has fired, chck if we have done enough
        // iterations for an hour. If so, pulse PB0 low.

        if (counter >= (60 / 4))
        {
            counter = 0;

            PINB |= (1 << PB0);
            _delay_ms(10);
            PINB |= (1 << PB0);
        }
    }

    return 0;
}
