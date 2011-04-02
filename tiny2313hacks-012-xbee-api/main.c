// tiny2313-hacks-002-adc/main.c

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

void usart_setup()
{
    UBRRH = 0;
    UBRRL = 51; // 9600 @ 8 Mhz
    //UBRRL = 207; // 2400 @ 8 Mhz
    UCSRA = 0;
    UCSRB = (1 << TXEN) | (1 << RXEN);
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

void xbee_send_data()
{
    static uint8_t packet[10] = {
        0x7e,                                                   // start delimiter
        0x00, 0x06,                                             // length
            0x01,                                               // api identifier TX Request: 16-bit address
                0x00,                                           // frame id
                0x00, 0x00,                                     // destination
                0x01,                                           // options
                0x21,                                           // data
        0xff - ((0x01 + 0x01 + 0x21) & 0xff)                    // checksum
    };

    usart_tx_bytes(packet, 10);
}

int main(void)
{
    usart_setup();

    _delay_ms(2500);

    while (1)
    {
        xbee_send_data();
        _delay_ms(1000);
    }
}

// COORDINATOR: MY 0     SH 13a200 SL 4032124f ?
// E-API:       MY efdf  SH 13a200 SL 40321241
// E-AT:        MY ffba  SH 13a200 SL 4032128b

