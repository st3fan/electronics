// tiny2313-hacks-002-adc/main.c

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

//

#define SHIFTER_DDR      DDRB
#define SHIFTER_PORT     PORTB

#define SHIFTER_OE    PB0
#define SHIFTER_MR    PB1
#define SHIFTER_SH_CP PB2
#define SHIFTER_DS    PB3
#define SHIFTER_ST_CP PB4

void shifter_setup()
{
    // All output pins
    SHIFTER_DDR |= (1 << SHIFTER_OE) | (1 << SHIFTER_MR) | (1 << SHIFTER_SH_CP) | (1 << SHIFTER_DS) | (1 << SHIFTER_ST_CP);

    // Reset is active low, so we set it high
    SHIFTER_PORT |= (1 << SHIFTER_MR);
}

void shifter_write(uint8_t b)
{
    SHIFTER_PORT &= ~(1 << SHIFTER_ST_CP);
    {
        for (int8_t i = 7; i >= 0; i--)
        {
            // Set the data
            if ((b >> i) & 0b00000001) {
                SHIFTER_PORT |= (1 << SHIFTER_DS);
            } else {
                SHIFTER_PORT &= ~(1 << SHIFTER_DS);
            }

            // Toggle the clock
            SHIFTER_PORT |= (1 << SHIFTER_SH_CP);
            SHIFTER_PORT &= ~(1 << SHIFTER_SH_CP);            
        }
    }

    SHIFTER_PORT |= (1 << SHIFTER_ST_CP);
}

//

int main(void)
{
    shifter_setup();
    
    while (1)
    {
        for (uint8_t i = 0; i < 255; i++)
        {
            shifter_write(i);
            _delay_ms(250);            
        }
    }

    return 0;
}
