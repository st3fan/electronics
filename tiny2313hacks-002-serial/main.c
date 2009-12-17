// Blink2313 - Blink a LED on PD5 of a ATTiny 2313

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

void usart_setup()
{
    UBRRH = 0;
    UBRRL = 6;
    UCSRA = 0;
    UCSRB = (1 << TXEN);
    UCSRC = (1 << UCSZ1) | (1 << UCSZ0);
}

void usart_tx(unsigned char c)
{
    while ((UCSRA & (1 << UDRE)) == 0x00) {
        // Do nothing
    }
    UDR = c;
}

void usart_tx_string(char* c)
{
    while (*c) {
        usart_tx(*c++);
    }
}

int main(void)
{
    usart_setup();

    sei();
    
    DDRB = (1 << PB0);
    while (1) {
        _delay_ms(500);
        PORTB ^= (1 << PB0);
        usart_tx(0x80);
        usart_tx_string("Hello, world!\n");
    }
    
    return 0;
}
