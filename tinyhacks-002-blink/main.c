// Blink2313 - Blink a LED on PD5 of a ATTiny 2313

#include <avr/io.h>
#include <avr/delay.h>

void mydelay(uint8_t delay)
{
    int n = 0;
    for (int i = 0; i < delay; i++) {
        for (int j = 0; j < 255; j++) {
            n++;
        }
    }
}

int main(void)
{
    DDRB = (1 << PB0);

    while (1)
    {
        for (uint8_t delay = 10; delay <= 100; delay += 10) {
            for (uint8_t i = 0; i < 4; i++) {
                mydelay(delay);
                PORTB ^= (1 << PB0);
            }
        }
        
        for (uint8_t delay = 100; delay >= 10; delay -= 10) {
            for (uint8_t i = 0; i < 4; i++) {
                mydelay(delay);
                PORTB ^= (1 << PB0);
            }
        }
    }
    
    return 0;
}
