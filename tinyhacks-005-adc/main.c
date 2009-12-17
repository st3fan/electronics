#include <avr/io.h>
#include <util/delay.h>

void mydelay(uint8_t delay)
{
    int n = 0;
    for (int i = 0; i < delay; i++) {
        for (int j = 0; j < 255; j++) {
            n++;
        }
    }
}

void adc_setup()
{
    ADMUX = (1 << MUX0) | (1 << MUX1) | (1 << ADLAR);
}

uint8_t adc_read()
{
    ADCSRA |= (1 << ADSC) | (1 << ADEN);
    
    while (ADCSRA & (1 << ADSC)) {
        // Nothing
    }

    return ADCH;
}

int main(void)
{
    adc_setup();

    DDRB = (1 << PB0);

    while (1) {
        mydelay(adc_read());
        PORTB ^= (1 << PB0);
    }

    return 0;
}
