// tiny2313-hacks-002-adc/main.c

#include <stdlib.h>
#include <string.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

//

struct string_descriptor_t {
    uint8_t type;
    char* string;
};

//

#define DS18B20_PORT PORTB
#define DS18B20_DDR  DDRB
#define DS18B20_PIN  PINB
#define DS18B20_DQ   PB2
#define DS18B20_VCC  PB3

#define DS18B20_INPUT_MODE()  (DS18B20_DDR &= ~(1 << DS18B20_DQ))
#define DS18B20_OUTPUT_MODE() (DS18B20_DDR |= (1 << DS18B20_DQ))
#define DS18B20_LOW()         (DS18B20_PORT &= ~(1 << DS18B20_DQ))
#define DS18B20_HIGH()        (DS18B20_PORT |= (1 << DS18B20_DQ))

#define DS18B20_CMD_SKIP_ROM 0xcc
#define DS18B20_CMD_CONVERT_TEMP 0x44
#define DS18B20_CMD_READ_SCRATCHPAD 0xbe

// Returns 0 in case of no error

uint8_t onewire_reset()
{
    uint8_t result = 0;

    DS18B20_LOW();
    DS18B20_OUTPUT_MODE();
    _delay_us(480);
    DS18B20_INPUT_MODE();
    
    _delay_us(70);
    result = DS18B20_PIN & (1 << DS18B20_DQ);
    _delay_us(410);
    
    return result;
}

inline void onewire_write_bit(uint8_t bit)
{
    if (bit)
    {
        DS18B20_LOW();
        DS18B20_OUTPUT_MODE();
        _delay_us(6);
        DS18B20_HIGH();
        _delay_us(64);
    }
    else
    {
        DS18B20_LOW();
        DS18B20_OUTPUT_MODE();
        _delay_us(60);
        DS18B20_HIGH();
        _delay_us(10);
    }
}

void onewire_write(uint8_t data)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        onewire_write_bit(data & 0x01);
        // shift the data byte for the next bit
        data >>= 1;
    }
}

inline uint8_t onewire_read_bit()
{
    DS18B20_LOW();
    _delay_us(6);
    DS18B20_HIGH();    
    _delay_us(9);
    uint8_t result = DS18B20_PIN & (1 << DS18B20_DQ);
    _delay_us(55);
    return result;
}

uint8_t onewire_read()
{
    uint8_t result = 0;

    for (uint8_t loop = 0; loop < 8; loop++)
    {
        // shift the result to get it ready for the next bit
        result >>= 1;

        // if result is one, then set MS bit
        if (onewire_read_bit()) {
            result |= 0x80;
        }
    }
    
    return result;
}

//

inline void stringify_int8(int8_t n, char* p)
{
    char digit;

    if (n < 0) {
        *p++ = '-';
        n = abs(n);
    }

    if (n >= 100) {
        digit = '0';
        while (n >= 100) {
            n = n - 100;
            digit++;
        }
        *p++ = digit;
        if (n == 0) {
            *p++ = '0';
            *p++ = '0';
            return;
        } else if (n < 10) {
            *p++ = '0';
        }
    }

    if (n >= 10) {
        digit = '0';
        while (n >= 10) {
            n = n - 10;
            digit++;
        }
        *p++ = digit;
        if (n == 0) {
            *p++ = '0';
            return;
        }
    }

    digit = '0' + n;
    *p++ = digit;
    
    *p = 0x00;
}

//

uint8_t xbee_checksum;

void xbee_setup()
{
    UBRRH = 0;
    UBRRL = 51; // 9600 @ 8 Mhz
    UCSRA = 0;
    UCSRB = (1 << TXEN); // | (1 << RXEN);
    UCSRC = (1 << UCSZ1) | (1 << UCSZ0);
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

void xbee_tx_bytes_P(const uint8_t* bytes, uint8_t length)
{
    while (length--) {
        xbee_checksum += pgm_read_byte(bytes);
        while ((UCSRA & (1 << UDRE)) == 0x00) {
            // Do nothing
        }
        UDR = pgm_read_byte(bytes++);
    }
}

//

inline void xbee_transmit_strings(const uint8_t* address, const uint8_t* network, struct string_descriptor_t* strings, uint8_t count)
{
    uint8_t length = 0;
    for (uint8_t i = 0; i < count; i++) {
        if (strings[i].type) {
            length += strlen_P(strings[i].string);
        } else {
            length += strlen(strings[i].string);
        }
    }

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
            for (uint8_t i = 0; i < count; i++) {
                if (strings[i].type) {
                    xbee_tx_bytes_P(strings[i].string, strlen_P(strings[i].string)); // RF Data
                } else {
                    xbee_tx_bytes(strings[i].string, strlen(strings[i].string)); // RF Data
                }
            }
        }
    }
    xbee_tx(0xff - (xbee_checksum & 0x00ff));   // Checksum
}

//

int xbee_enable()
{
    return 0;
}

void xbee_disable()
{
}

//

const uint8_t coordinator_address[8] = { 0x00, 0x13, 0xa2, 0x00, 0x40, 0x32, 0x12, 0x4f };
const uint8_t coordinator_network[2] = { 0xff, 0xfe };

const char s1[] PROGMEM = "{\"t\":\"t\",\"v\":1,\"sensors\":[";
const char s2[] PROGMEM = ",";
const char s3[] PROGMEM = "]}";

int main(void)
{
    xbee_setup();

    //for (int i = 0; i < 15; i++) {
    //    _delay_ms(1000);
    //}

    while (1)
    {
        if (onewire_reset() == 0)
        {
            onewire_write(DS18B20_CMD_SKIP_ROM);
            onewire_write(DS18B20_CMD_CONVERT_TEMP);
        
            if (onewire_reset() == 0)
            {
                onewire_write(DS18B20_CMD_SKIP_ROM);
                onewire_write(DS18B20_CMD_READ_SCRATCHPAD);

                uint8_t lsb = onewire_read();
                uint8_t msb = onewire_read();
                
                int8_t temperature = (msb << 4) | (lsb >> 4) | (msb & 0b10000000);
                
                if (xbee_enable() == 0)
                {
                    int8_t temperatureValues[2] = { temperature, 0 };
                    char temperatureStrings[2][5];

                    for (uint8_t i = 0; i < 2; i++)
                    {
                        stringify_int8(temperatureValues[i], &temperatureStrings[i][0]);

#if 0
                        if ((uint8_t) temperatureValues[i] != 0xff) {
                            stringify_int8(temperatureValues[i], &temperatureStrings[i][0]);
                        } else {
                            temperatureStrings[i][0] = 'n';
                            temperatureStrings[i][1] = 'i';
                            temperatureStrings[i][2] = 'l';
                            temperatureStrings[i][3] = 0x00;
                        }
#endif
                    }
                    
                    struct string_descriptor_t strings[5] = {
                        { 1, s1 },
                        { 0, temperatureStrings[0] },
                        { 1, s2 },
                        { 0, temperatureStrings[1] },
                        { 1, s3 }
                    };
                    
                    usart_tx_string(temperatureStrings[0]);
                    usart_tx(',');
                    usart_tx_string(temperatureStrings[1]);
                    usart_tx('\r');
                    usart_tx('\n');

                    //xbee_transmit_strings(coordinator_address, coordinator_network, strings, 7);
                    
                    xbee_disable();
                }
            }
        }
        
        for (uint8_t i = 0; i < 2; i++) {
            _delay_ms(1000);
        }
    }
}
