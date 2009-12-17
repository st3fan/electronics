/* serial.h */

#ifndef SERIAL_H
#define SERIAL_H

void serial_init(volatile uint8_t* port, unsigned char pin);
void serial_tx(volatile uint8_t* port, unsigned int pin, unsigned char c);
void serial_tx_string(volatile uint8_t* port, unsigned int pin, char* s);

#endif
