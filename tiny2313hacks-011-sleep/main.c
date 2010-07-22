// Blink2313 - Blink a LED on PD5 of a ATTiny 2313

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

int main(void)
{
    DDRB = (1 << PB0);
    
    while (1)
    {
        if ((PIND & (1 << PD6)) == 0) {
            set_sleep_mode(SLEEP_MODE_IDLE);
            sleep_enable();
            sei();
            sleep_cpu();
        }
        
        _delay_ms(5);
        
        PORTB ^= (1 << PB0);
    }
    
    return 0;
}
