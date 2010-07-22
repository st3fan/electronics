// tiny2313-hacks-002-adc/main.c

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

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

//

#define DISPLAY_DDR DDRB
#define DISPLAY_OUT PORTB
#define DISPLAY_PIN PB1

#define MAX7219_OP_NOOP   0
#define MAX7219_OP_DIGIT0 1
#define MAX7219_OP_DIGIT1 2
#define MAX7219_OP_DIGIT2 3
#define MAX7219_OP_DIGIT3 4
#define MAX7219_OP_DIGIT4 5
#define MAX7219_OP_DIGIT5 6
#define MAX7219_OP_DIGIT6 7
#define MAX7219_OP_DIGIT7 8
#define MAX7219_OP_DECODEMODE  9
#define MAX7219_OP_INTENSITY   10
#define MAX7219_OP_SCANLIMIT   11
#define MAX7219_OP_SHUTDOWN    12
#define MAX7219_OP_DISPLAYTEST 15


static const uint8_t MAX7219_CharTable[128] = {
  0B01111110,0B00110000,0B01101101,0B01111001,0B00110011,0B01011011,0B01011111,0B01110000,
  0B01111111,0B01111011,0B01110111,0B00011111,0B00001101,0B00111101,0B01001111,0B01000111,
  0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,
  0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,
  0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,
  0B00000000,0B00000000,0B00000000,0B00000000,0B10000000,0B00000001,0B10000000,0B00000000,
  0B01111110,0B00110000,0B01101101,0B01111001,0B00110011,0B01011011,0B01011111,0B01110000,
  0B01111111,0B01111011,0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,
  0B00000000,0B01110111,0B00011111,0B00001101,0B00111101,0B01001111,0B01000111,0B00000000,
  0B00110111,0B00000000,0B00000000,0B00000000,0B00001110,0B00000000,0B00000000,0B00000000,
  0B01100111,0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,
  0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,0B00001000,
  0B00000000,0B01110111,0B00011111,0B00001101,0B00111101,0B01001111,0B01000111,0B00000000,
  0B00110111,0B00000000,0B00000000,0B00000000,0B00001110,0B00000000,0B00000000,0B00000000,
  0B01100111,0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,
  0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,0B00000000,0B00000000
};

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
   DISPLAY_OUT |= (1 << DISPLAY_PIN);
}

inline void display_write_command(uint8_t address, uint8_t command, uint8_t value)
{
   display_select();
   {
      spi_write_bits(address, 8);
      spi_write_bits(command, 8);
      spi_write_bits(value, 8);
   }
   display_deselect();
}

void display_clear(uint8_t address)
{
   for (uint8_t i = MAX7219_OP_DIGIT0; i <= MAX7219_OP_DIGIT7; i++) {
      display_write_command(address, i, 0);
   }
}

void display_set_digit(uint8_t address, uint8_t digit, uint8_t value, uint8_t dp)
{
   display_write_command(address, digit + 1, MAX7219_CharTable[value] | (dp << 7));
}

void display_init(uint8_t count)
{
   for (uint8_t address = 0; address < count; address++) {
      display_write_command(address, MAX7219_OP_DISPLAYTEST, 0);
      display_write_command(address, MAX7219_OP_SCANLIMIT, 7);
      display_write_command(address, MAX7219_OP_DECODEMODE, 0);
      display_clear(address);
   }
}

//

void led_init()
{
   DDRB |= (1 << PB0);
}

void led_on()
{
   PORTB |= (1 << PB0);
}

void led_off()
{
   PORTB &= ~(1 << PB0);
}

//

int main(void)
{
   spi_setup();
   display_setup();
   display_init(1);
   led_init();
   
   while (1)
   {
      for (uint8_t value = 0; value <= 9; value++) {
	 for (uint8_t digit = 0; digit <= 7; digit++) {
	    display_set_digit(0, digit, value, 0);
	    _delay_ms(1);
	 }
	 led_on();
	 _delay_ms(125);
	 led_off();
	 _delay_ms(125);
      }
   }
   
   return 0;
}
