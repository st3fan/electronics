// Blink2313 - Blink a LED on PD5 of a ATTiny 2313

#include <avr/io.h>
#include <util/delay.h>

//

static const char RED_PIN = PB0;
static const char GREEN_PIN = PB1;
static const char BLUE_PIN = PB2;

inline void red_on() {
    PORTB |= (1 << RED_PIN);
}

inline void red_off() {
    PORTB ^= (1 << RED_PIN);    
}

inline void green_on() {
    PORTB |= (1 << GREEN_PIN);
}

inline void green_off() {
    PORTB ^= (1 << GREEN_PIN);    
}

inline void blue_on() {
    PORTB |= (1 << BLUE_PIN);
}

inline void blue_off() {
    PORTB ^= (1 << BLUE_PIN);
}

//

void delay()
{
    for (int i = 0; i < 10; i++) {
        _delay_ms(30);
    }
}

//

int main(void)
{
    DDRB = (1 << PB0 | 1 << PB1 | 1 << PB2);
    
    while (1)
    {
        //red_on();
        //delay();
        //red_off();
        //delay();

        green_on();
        delay();
        green_off();
        delay();

        //blue_on();
        //delay();
        //blue_off();
        //delay();
    }
    
    return 0;
}
