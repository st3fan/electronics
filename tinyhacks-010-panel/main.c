// Blink2313 - Blink a LED on PD5 of a ATTiny 2313

#include <avr/io.h>
#include <util/delay.h>

// Clock    green          pb2
// Data     orange/white   pb1
// Strobe   green/white    pb3
// Mode     orange         pb4

#define MODE_A PORTB &= ~(1 << PB4);
#define MODE_B PORTB |= (1 << PB4);

#define STROBE_HIGH PORTB |= (1 << PB3);
#define STROBE_LOW PORTB &= ~(1 << PB3);

#define CLOCK_HIGH PORTB |= (1 << PB2);
#define CLOCK_LOW PORTB &= ~(1 << PB2);

#define DATA_HIGH PORTB |= (1 << PB1);
#define DATA_LOW PORTB &= ~(1 << PB1);

uint8_t digits[] = {
    0x03,0xe0,0x05,0x10,0x04,0x90,0x04,0x50,0x03,0xe0,0x00,0x00,
    0x00,0x00,0x04,0x20,0x07,0xf0,0x04,0x00,0x00,0x00,0x00,0x00,
    0x06,0x20,0x05,0x10,0x04,0x90,0x04,0x90,0x04,0x60,0x00,0x00,
    0x02,0x20,0x04,0x10,0x04,0x90,0x04,0x90,0x03,0x60,0x00,0x00,
    0x01,0x80,0x01,0x40,0x01,0x20,0x07,0xf0,0x01,0x00,0x00,0x00,
    0x02,0x70,0x04,0x50,0x04,0x50,0x04,0x50,0x03,0x90,0x00,0x00,
    0x03,0xc0,0x04,0xa0,0x04,0x90,0x04,0x90,0x03,0x00,0x00,0x00,
    0x00,0x10,0x07,0x10,0x00,0x90,0x00,0x50,0x00,0x30,0x00,0x00,
    0x03,0x60,0x04,0x90,0x04,0x90,0x04,0x90,0x03,0x60,0x00,0x00,
    0x00,0x60,0x04,0x90,0x04,0x90,0x02,0x90,0x01,0xe0,0x00,0x00,
};

uint8_t blinker[4] = {
    0b00000011, 0b01100000,
    0b00000011, 0b01100000
};

uint8_t digitOffsets[4] = {
    1, 8, 19, 26
};

uint8_t framebuffer[64];

void drawBlinker(uint8_t on)
{
    uint8_t* start = framebuffer + (2 * 15);
    
    if (on) {
        for (uint8_t i = 0; i < 4; i++) {
            *start++ = blinker[i];
        }
    } else {
        for (uint8_t i = 0; i < 4; i++) {
            *start++ = 0x00;
        }
    }
}

void drawDigit(uint8_t digit, uint8_t position)
{
    uint8_t* bufferStart = framebuffer + (2 * digitOffsets[position]);
    uint8_t* digitStart = &digits[digit * 12];
  
    for (uint8_t i = 0; i < 12; i++) {
        *bufferStart++ = *digitStart++;
    }
}

// convenience routine for clocking a byte LSB first
#if 1
inline void clock_byte_lsb_first(char val)
{
    for (char i = 0; i < 8; i++) {
        CLOCK_LOW;
        if (val & 0x01) {
            DATA_HIGH;
        } else {
            DATA_LOW;
        }
        CLOCK_HIGH;
        val = val >> 1;
    }
}
#endif

#if 0
inline void clock_byte_lsb_first(char val)
{
    USICR = (1 << USIWM0);
    
    USIDR = val;
    for (int i = 0; i < 8; i++) {
        USICR = (1 << USIWM0) | (1<<USITC) | (1 << USICLK);
        USICR = (1 << USIWM0) | (1<<USITC);
    }
}
#endif

int main(void)
{
    // Setup

    // init I/Os
    DDRB |= (1 << PB1) | (1 << PB2) | (1 << PB3) | (1 << PB4) | (1 << PB0);
    PORTB |= (1 << PB1) | (1 << PB2) | (1 << PB3) | (1 << PB4) | (1 << PB0);
  
    // clear framebuffer
    for (uint8_t i = 0; i < 64; i++) {
        framebuffer[i] = 0x00;
    }
    
    drawDigit(0,0);
    drawDigit(0,1);
    drawDigit(0,2);
    drawDigit(0,3);

    drawBlinker(1);

    // Loop

    while (1)
    {
        // scan all rows
        uint8_t rowAddr = 1;
        //uint8_t bufferPos = 0;

        uint8_t* p = &framebuffer[0];

        // On
        
        for (char row = 0; row < 8; row++)
        {
            for (char bank = 3; bank >= 0; bank--)
            {
                // set a pixel bank
                MODE_B;
                clock_byte_lsb_first(bank);
                
                // load 16 pixels per board
                MODE_A;
                STROBE_HIGH;
            
                clock_byte_lsb_first(p[1]);
                clock_byte_lsb_first(p[0]);
     
                _delay_us(25);
           
                p += 2;
            }
            
            // select row
            MODE_B;
            clock_byte_lsb_first(rowAddr);
            STROBE_LOW;
            MODE_A;
            
            rowAddr = rowAddr << 1;
        }
    }
    
    return 0;
}
