// Serial

#include <avr/io.h>

int main(void)
{
    serial_init(DDRB, PB2);
    
    while (1) {
        serial_tx_string(DDRB, PB2, "Hello, world!");
    }
    
    return 0;
}
