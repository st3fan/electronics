// tiny2313-hacks-002-adc/main.c

#include <stdlib.h>
#include <string.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

void usart_setup()
{
    UBRRH = 0;
    UBRRL = 51; // 9600 @ 8 Mhz
    //UBRRL = 207; // 2400 @ 8 Mhz
    UCSRA = 0;
    UCSRB = (1 << TXEN); // | (1 << RXEN);
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

void usart_tx_bytes(uint8_t* p, uint8_t n)
{
    while (n--) {
        usart_tx(*p++);
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

void usart_tx_int8(int8_t n)
{
    char digit;

    if (n < 0) {
        usart_tx('-');
        n = abs(n);
    }

    if (n >= 100) {
        digit = '0';
        while (n >= 100) {
            n = n - 100;
            digit++;
        }
        usart_tx(digit);
        if (n == 0) {
            usart_tx_string("00");
            return;
        } else if (n < 10) {
            usart_tx('0');
        }
    }

    if (n >= 10) {
        digit = '0';
        while (n >= 10) {
            n = n - 10;
            digit++;
        }
        usart_tx(digit);
        if (n == 0) {
            usart_tx('0');
            return;
        }
    }

    digit = '0' + n;
    usart_tx(digit);
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

#define DS18B20_PORT PORTD
#define DS18B20_DDR  DDRD
#define DS18B20_PIN  PIND
#define DS18B20_DQ   PD6

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

void display_write_int8(int8_t v)
{
    //usart_tx('!');
    //if (v >= 0) {
    //    usart_tx('x');
    //}
    //if (abs(v) < 10) {
    //    usart_tx('x');
    //}
    //if (abs(v) < 100) {
    //    usart_tx('x');
    //}
    usart_tx_int8(v);
    usart_tx('\r');
    usart_tx('\n');
}

#if 0
void xbee_write_int8(int8_t v)
{
    unsigned char header[3] = { 0x7e, 0x00, 0x0f };
    usart_tx_bytes(header, 3);

    unsigned char data[14] = { 0x10, 0x00, 0x00, 0x13, 0xa2, 0x00, 0x40, 0x32, 0x12, 0x4f, 0xff, 0xfe, 0x00, 0x00 };
    usart_tx_bytes(data, 14);

    usart_tx(v);
    
    uint8_t checksum = 0xff - ((0x10 + 0x13 + 0xa2 + 0x40 + 0x32 + 0x12 + 0x4f + 0xff + 0xfe + 0x00 + 0x00  + v) & 0xff);

    usart_tx(checksum);
}
#endif

//

uint16_t xbee_checksum;

void xbee_tx(uint8_t c)
{
    xbee_checksum += c;
    usart_tx(c);
}

void xbee_tx_bytes(uint8_t* bytes, uint8_t length)
{
    while (length--) {
        xbee_checksum += *bytes;
        usart_tx(*bytes++);
    }
}

void xbee_transmit_bytes(uint8_t* address, uint8_t* network, uint8_t* data, uint8_t length)
{
    xbee_tx(0x7e);               // Start Delimiter
    xbee_tx(0x00);               // Length
    xbee_tx(length + 14);
    {
        xbee_checksum = 0;

        xbee_tx(0x10);               // API Identifier
        {
            xbee_tx(0x00);               // Frame ID
            xbee_tx_bytes(address, 8);   // Destination Address
            xbee_tx_bytes(network, 2);   // Destination Network Address
            xbee_tx(0x00);               // Broadcast Radius
            xbee_tx(0x00);               // Options
            xbee_tx_bytes(data, length); // RF Data
        }
    }
    usart_tx(0xff - (xbee_checksum & 0x00ff));  // Checksum
}

int xbee_enable()
{
    return 0;
}

void xbee_disable()
{
}

void xbee_transmit_string(uint8_t* address, uint8_t* network, uint8_t* string)
{
    xbee_transmit_bytes(address,network, string, strlen(string));
}

void xbee_transmit_strings(uint8_t* address, uint8_t* network, char** strings, uint8_t count)
{
    uint8_t length = 0;
    for (uint8_t i = 0; i < count; i++) {
        length += strlen(strings[i]);
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
                xbee_tx_bytes(strings[i], strlen(strings[i])); // RF Data
            }
        }
    }
    usart_tx(0xff - (xbee_checksum & 0x00ff));   // Checksum
}

//

static const uint8_t coordinator_address[8] = { 0x00, 0x13, 0xa2, 0x00, 0x40, 0x32, 0x12, 0x4f };
static const uint8_t coordinator_network[2] = { 0xff, 0xfe };

void stringify_int8(int8_t n, char* p)
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

void transmit_temperature(int8_t temperature0, int8_t temperature1, int8_t temperature2)
{
    char sensor0Reading[5];
    char sensor1Reading[5];
    char sensor2Reading[5];

    stringify_int8(temperature0, sensor0Reading);
    
    sensor1Reading[0] = 'n';
    sensor1Reading[1] = 'i';
    sensor1Reading[2] = 'l';
    sensor1Reading[3] = 0x00;

    sensor2Reading[0] = 'n';
    sensor2Reading[1] = 'i';
    sensor2Reading[2] = 'l';
    sensor2Reading[3] = 0x00;

    char* strings[] = {
        "{\"type\":\"thermometer\",\"sensors\":[",
        sensor0Reading,
        ",",
        sensor1Reading,
        ",",
        sensor2Reading,
        "]}"
    };
    
    xbee_transmit_strings(coordinator_address, coordinator_network, strings, 7);
}

int main(void)
{
    usart_setup();

    for (int i = 0; i < 15; i++) {
        _delay_ms(1000);
    }

#if 0
    while (1)
    {
        static uint8_t data[21] = {
            0x7e, 0x00, 17,
            0x10,
            0x00,
            0x00, 0x13, 0xa2, 0x00, 0x40, 0x32, 0x12, 0x4f,
            0x00, 0x00,
            0x00,
            0x00,
            'H', 'i', '!',
            0xff - ((0x10 + 0x00 + 0x00 + 0x13 + 0xa2 + 0x00 + 0x40 + 0x32 + 0x12 + 0x4f + 0x00 + 0x00 + 0x00 + 0x00 + 'H' + 'i' + '!') & 0xff)
        };
        
        usart_tx_bytes(data, 21);

        _delay_ms(2500);
    }
#endif
    
    while (1)
    {
        if (onewire_reset() == 0)
        {
            onewire_write(DS18B20_CMD_SKIP_ROM);
            onewire_write(DS18B20_CMD_CONVERT_TEMP);
        
            _delay_ms(250);
            
            if (onewire_reset() == 0)
            {
                onewire_write(DS18B20_CMD_SKIP_ROM);
                onewire_write(DS18B20_CMD_READ_SCRATCHPAD);

                uint8_t lsb = onewire_read();
                uint8_t msb = onewire_read();
                
                int8_t temperature = (msb << 4) | (lsb >> 4) | (msb & 0b10000000);
                
                if (xbee_enable() == 0) {
                    transmit_temperature(temperature, 0, 0);
                    xbee_disable();
                }
            }
        }
        
        for (uint8_t i = 0; i < 2; i++) {
            _delay_ms(1000);
        }
    }
}
