/* serial.c */

#include <avr/io.h>
#include <util/delay.h>

// Baudrate of 9600 bits per second means 9600 state changes per second
// that each take 104 us

void serial_init(volatile uint8_t* port, unsigned char pin)
{
    *port  = (1 << pin);
}

void serial_tx(volatile uint8_t* port, unsigned int pin, unsigned char c)
{
    *port &= ~(1 << pin);
    _delay_us(104*8);
   
    for (unsigned char i = 0; i < 8; i++) {
        if (c & 0b00000001) {
            *port |= (1 << pin);
        } else {
            *port &= ~(1 << pin);         
        }
        _delay_us(104*8);
        c >>= 1;
    }

    *port |= (1 << pin);
    _delay_us(104*8);
}

void serial_tx_string(volatile uint8_t* port, unsigned int pin, char* s)
{
    while (*s) {
        serial_tx(port, pin, *s++);
    }
}
