// tiny2313-hacks-002-adc/main.c

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

void usart_tx_hex_uint8(uint8_t c)
{
    static char digits[16] = { '0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f' };
    usart_tx(digits[(c >> 4) & 0x0f]);
    usart_tx(digits[(c >> 0) & 0x0f]);
}

void usart_tx_hex_uint16(uint16_t c)
{
    usart_tx_hex_uint8((c >> 8) & 0x00ff);
    usart_tx_hex_uint8((c >> 0) & 0x00ff);
}

//

#define SPI_PORT    DDRB
#define SPI_CLK_PIN PB7
#define SPI_MISO_PIN  PB6
#define SPI_MOSI_PIN  PB5

#define DELAY 0

void spi_setup()
{
    // Clock and Data Out are configured as output pins
    SPI_PORT |= (1 << SPI_CLK_PIN) | (1 << SPI_MOSI_PIN);
}

void spi_write_bits(uint8_t b, uint8_t n)
{
    while (n--) {
        if (b & (1 << n)) {
            PORTB |= (1 << SPI_MOSI_PIN);
        } else {
            PORTB &= ~(1 << SPI_MOSI_PIN);
        }
        PORTB |= (1 << SPI_CLK_PIN);
        PORTB &= ~(1 << SPI_CLK_PIN);
    }
}

uint16_t spi_read_bits(uint8_t n)
{
    uint16_t v = 0;

    for (int i = (n - 1); i >= 0; i--) {
        if (PINB & (1 << SPI_MISO_PIN)) {
            v |= (1 << i);
        }
        PORTB |= (1 << SPI_CLK_PIN);
        PORTB &= ~(1 << SPI_CLK_PIN);
    }
    
    return v;
}

//

#define DISPLAY_DDR DDRB
#define DISPLAY_OUT PORTB
#define DISPLAY_PIN PB0

inline void display_select()
{
    DISPLAY_OUT &= ~(1 << DISPLAY_PIN);
}

inline void display_deselect()
{
    DISPLAY_OUT |= (1 << DISPLAY_PIN);    
}

void display_setup()
{
    DISPLAY_DDR |= (1 << DISPLAY_PIN);
}

void display_write(uint16_t value)
{
    display_select();
    {
        spi_write_bits((uint8_t) (value >> 12) & 0x000f, 8);
        spi_write_bits((uint8_t) (value >>  8) & 0x000f, 8);
        spi_write_bits((uint8_t) (value >>  4) & 0x000f, 8);
        spi_write_bits((uint8_t) (value >>  0) & 0x000f, 8);
    }
    display_deselect();
}

//

int main(void)
{
    usart_setup();
    spi_setup();
    display_setup();

    while (1) {
        for (uint16_t i = 0; i <= 0xffff; i++) {
            display_write(i);
            _delay_ms(500);
        }
    }

    return 0;
}
