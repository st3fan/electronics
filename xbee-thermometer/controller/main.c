// tiny2313-hacks-002-adc/main.c

#include <stdlib.h>
#include <string.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/delay.h>

//

#define DS18B20_ONE_PORT PORTB
#define DS18B20_ONE_DDR  DDRB
#define DS18B20_ONE_PIN  PINB
#define DS18B20_ONE_DQ   PB2
#define DS18B20_ONE_VCC  PB3

#define DS18B20_TWO_PORT PORTB
#define DS18B20_TWO_DDR  DDRB
#define DS18B20_TWO_PIN  PINB
#define DS18B20_TWO_DQ   PB0
#define DS18B20_TWO_VCC  PB1

#define DS18B20_CMD_SKIP_ROM 0xcc
#define DS18B20_CMD_CONVERT_TEMP 0x44
#define DS18B20_CMD_READ_SCRATCHPAD 0xbe

// Returns 0 in case of no error

uint8_t onewire_reset(volatile uint8_t* port, volatile uint8_t* ddr, volatile uint8_t* pin, uint8_t dq)
{
    uint8_t result = 0;

    *port &= ~(1 << dq); // onewire_low(port, dq);
    *ddr |= (1 << dq); // onewire_output_mode(ddr, dq);
    _delay_us(480);
    *ddr &= ~(1 << dq); // onewire_input_mode(ddr, dq);
    
    _delay_us(70);
    result = *pin & (1 << dq);
    _delay_us(410);
    
    return result;
}

void onewire_write(volatile uint8_t* port, volatile uint8_t* ddr, volatile uint8_t* pin, uint8_t dq, uint8_t data)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        if (data & 0x01)
        {
            *port &= ~(1 << dq); // onewire_low(port, dq);
            *ddr |= (1 << dq);   // onewire_output_mode(ddr, dq);
            _delay_us(6);
            *port |= (1 << dq);  // onewire_high(port, dq);
            _delay_us(64);
        }
        else
        {
            *port &= ~(1 << dq); //onewire_low(port, dq);
            *ddr |= (1 << dq);   //onewire_output_mode(ddr, dq);
            _delay_us(60);
            *port |= (1 << dq);  //onewire_high(port, dq);
            _delay_us(10);
        }

        // shift the data byte for the next bit
        data >>= 1;
    }
}

uint8_t onewire_read(volatile uint8_t* port, volatile uint8_t* ddr, volatile uint8_t* pin, uint8_t dq)
{
    uint8_t result = 0;

    for (uint8_t loop = 0; loop < 8; loop++)
    {
        // shift the result to get it ready for the next bit
        result >>= 1;

        // if result is one, then set MS bit

        *port &= ~(1 << dq); // onewire_low(port, dq);
        _delay_us(6);
        *port |= (1 << dq);  // onewire_high(port, dq);
        _delay_us(9);
        uint8_t bit = *pin & (1 << dq);
        _delay_us(55);

        if (bit) {
            result |= 0x80;
        }
    }
    
    return result;
}

//

uint8_t xbee_checksum;

void xbee_setup()
{
    // Setup the UART

    UBRRH = 0;
    UBRRL = 51; // 9600 @ 8 Mhz
    UCSRA = 0;
    UCSRC = (1 << UCSZ1) | (1 << UCSZ0);

    // Setup the pins that we use to power the XBee

    DDRD |= (1 << PD4) | (1 << PD5);
}

void usart_tx(uint8_t c)
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

void usart_tx_bytes(uint8_t* p, uint8_t n)
{
    while (n--) {
        usart_tx(*p++);
    }
}

void xbee_tx(uint8_t c)
{
    xbee_checksum += c;
    
    while ((UCSRA & (1 << UDRE)) == 0x00) {
        // Do nothing
    }
    UDR = c;
}

void xbee_tx_bytes(const uint8_t* bytes, uint8_t length)
{
    while (length--) {
        xbee_checksum += *bytes;
        while ((UCSRA & (1 << UDRE)) == 0x00) {
            // Do nothing
        }
        UDR = *bytes++;
    }
}

//

void xbee_transmit_bytes(const uint8_t* address, const uint8_t* network, uint8_t* bytes, uint8_t length)
{
    xbee_tx(0x7e);                               // Start Delimiter
    xbee_tx(0x00);                               // Length
    xbee_tx(length + 14);
    {
        xbee_checksum = 0;

        xbee_tx(0x10);                               // API Identifier
        {
            xbee_tx(0x00);                               // Frame ID
            xbee_tx_bytes(address, 8);                   // Destination Address
            xbee_tx_bytes(network, 2);                   // Destination Network Address
            xbee_tx(0x00);                               // Broadcast Radius
            xbee_tx(0x00);                               // Options
            xbee_tx_bytes(bytes, length);                // RF Data
        }
    }
    xbee_tx(0xff - (xbee_checksum & 0x00ff));   // Checksum
}

//

int xbee_enable()
{
    // Enable power
    PORTD |= (1 << PD4) | (1 << PD5);

    // Setup the USART
    UCSRB |= (1 << TXEN); // | (1 << RXEN);

    // TODO: Ask the XBee if it is ready and connected

    for (uint8_t i = 0; i < 10; i++) {
        _delay_ms(1000);
    }

    return 0;
}

void xbee_disable()
{
    // TODO: Ask the XBee if it is done transmitting

    for (uint8_t i = 0; i < 10; i++) {
        _delay_ms(1000);
    }

    // Disable the USART
    UCSRB &= ~(1 << TXEN); // | (1 << RXEN);


    // Turn off the power
    PORTD &= ~((1 << PD4) | (1 << PD5));
}

//

const uint8_t coordinator_address[8] = { 0x00, 0x13, 0xa2, 0x00, 0x40, 0x32, 0x12, 0x4f };
const uint8_t coordinator_network[2] = { 0xff, 0xfe };

int8_t read_temperature(volatile uint8_t* port, volatile uint8_t* ddr, volatile uint8_t* pin, uint8_t dq, uint8_t vcc)
{
    int8_t temperature = 0xff;

    *port |= (1 << vcc);

    _delay_ms(1000);

    if (onewire_reset(port, ddr, pin, dq) == 0)
    {
        onewire_write(port, ddr, pin, dq, DS18B20_CMD_SKIP_ROM);
        onewire_write(port, ddr, pin, dq, DS18B20_CMD_CONVERT_TEMP);

        _delay_ms(750);
        
        if (onewire_reset(port, ddr, pin, dq) == 0)
        {
            onewire_write(port, ddr, pin, dq, DS18B20_CMD_SKIP_ROM);
            onewire_write(port, ddr, pin, dq, DS18B20_CMD_READ_SCRATCHPAD);
            
            uint8_t lsb = onewire_read(port, ddr, pin, dq);
            uint8_t msb = onewire_read(port, ddr, pin, dq);
            
            temperature = (msb << 4) | (lsb >> 4) | (msb & 0b10000000);
        }
    }

    *port &= ~(1 << vcc);

    return temperature;
}

volatile uint8_t counter = 0;

ISR(WDT_OVERFLOW_vect)
{
    // Disable the watchdog
    WDTCSR |= (1 << WDCE) | (1 << WDE);
    WDTCSR &= ~(1 << WDE);

    // Increment the counter so we can track iterations
    counter++;
}

int main(void)
{
    // Setup the VCC ports for the sensors

    DS18B20_ONE_DDR |= (1 << DS18B20_ONE_VCC);
    DS18B20_TWO_DDR |= (1 << DS18B20_TWO_VCC);    

    xbee_setup();

    _delay_ms(1000); // TODO: Is this still needed?

    sei();

    while (1)
    {
        // Disable the one-wire ports

        DS18B20_ONE_PORT &= ~(1 << DS18B20_ONE_DQ); // onewire_low(&DS18B20_ONE_PORT, DS18B20_ONE_DQ);
        DS18B20_TWO_PORT &= ~(1 << DS18B20_TWO_DQ); // onewire_low(&DS18B20_TWO_PORT, DS18B20_TWO_DQ);

        // Enable the watchdog. It will fire every 4 seconds.
        
        WDTCSR = (1 << WDIE) | (1 << WDE) | (1 << WDP3);

        // Sleep

        set_sleep_mode(SLEEP_MODE_PWR_DOWN);
        sleep_enable();
        sleep_cpu();

#if 0        
        // Debug - Flash the led three times
        
        DDRB |= (1 << PB7);
        PORTB |= (1 << PB7);
        _delay_ms(100);
        PORTB &= ~(1 << PB7);
        DDRB &= ~(1 << PB7);
#endif

        // Do work

        if (counter >= (60 / 4))
        {
            int8_t temperatureValues[2] = {
                read_temperature(&DS18B20_ONE_PORT, &DS18B20_ONE_DDR, &DS18B20_ONE_PIN, DS18B20_ONE_DQ, DS18B20_ONE_VCC),
                read_temperature(&DS18B20_TWO_PORT, &DS18B20_TWO_DDR, &DS18B20_TWO_PIN, DS18B20_TWO_DQ, DS18B20_TWO_VCC)
            };

            if (xbee_enable() == 0)
            {
                char packet[4] = {
                    0x01, // Temperature Sensor
                    0x02, // Two values
                    temperatureValues[0],
                    temperatureValues[1]
                };
                
                xbee_transmit_bytes(coordinator_address, coordinator_network, packet, 4);
                
                xbee_disable();
            }

            counter = 0;
        }
    }

    return 0;
}
