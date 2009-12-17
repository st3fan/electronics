// tiny2313-hacks-002-adc/main.c

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

void usart_setup()
{
    UBRRH = 0;
    UBRRL = 25; // 6 for 9600
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

void usart_tx_hex_uint8(uint8_t c)
{
    static char digits[16] = { '0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f' };
    usart_tx(digits[(c >> 4) & 0x0f]);
    usart_tx(digits[(c >> 0) & 0x0f]);
}

void usart_tx_hex_uint16(uint16_t c)
{
    usart_tx_hex_uint8((c >> 8) & 0x00f);
    usart_tx_hex_uint8((c >> 0) & 0x00f);
}

//

void display_write(uint16_t value)
{
    usart_tx((value >> 12) & 0x0f);
    usart_tx((value >>  8) & 0x0f);
    usart_tx((value >>  4) & 0x0f);
    usart_tx((value >>  0) & 0x0f);
}

//

int main(void)
{
    usart_setup();

    DDRB = (1 << PB0);

    _delay_ms(1000);
    
    while (1)
    {
        for (uint16_t i = 0; i <= 0xffff; i++)
        {
            display_write(i);
            
            PORTB |= (1 << PB0);
            _delay_ms(500);
            PORTB &= ~(1 << PB0);
            _delay_ms(500);
        }
    }
}
