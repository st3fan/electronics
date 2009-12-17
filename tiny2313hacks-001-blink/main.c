// Blink2313 - Blink a LED on PD5 of a ATTiny 2313

#include <avr/io.h>
#include <util/delay.h>

int main(void)
{
    DDRB = (1 << PB0);
    
    while (1) {
        _delay_ms(500);
        PORTB ^= (1 << PB0);
    }
    
    return 0;
}
