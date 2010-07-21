// blink a LED that is connected to PB4 of a Tiny45

#include <avr/io.h>
#include <util/delay.h>

int main(void)
{
    DDRB = (1 << PB4);

    while (1)
    {
        for (uint8_t delay = 10; delay <= 100; delay += 10) {
            for (uint8_t i = 0; i < 4; i++) {
                _delay_ms(delay);
                PORTB ^= (1 << PB4);
            }
        }
        
        for (uint8_t delay = 100; delay >= 10; delay -= 10) {
            for (uint8_t i = 0; i < 4; i++) {
                _delay_ms(delay);
                PORTB ^= (1 << PB4);
            }
        }
    }
    
    return 0;
}
