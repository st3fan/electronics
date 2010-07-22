#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

//

#define SHIFTER_DDR      DDRB
#define SHIFTER_PORT     PORTB

#define SHIFTER_SH_CP PB2 // Clock
#define SHIFTER_DS    PB1 // Data
#define SHIFTER_ST_CP PB4 // Latch

void shifter_setup()
{
   SHIFTER_DDR |= (1 << SHIFTER_SH_CP) | (1 << SHIFTER_DS) | (1 << SHIFTER_ST_CP);
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

#define DISPLAY_SEGMENT_A 0b00000010
#define DISPLAY_SEGMENT_B 0b00000100
#define DISPLAY_SEGMENT_C 0b00001000
#define DISPLAY_SEGMENT_D 0b00010000
#define DISPLAY_SEGMENT_E 0b00100000
#define DISPLAY_SEGMENT_F 0b01000000
#define DISPLAY_SEGMENT_G 0b10000000

#define DISPLAY_NUMERAL_0 (DISPLAY_SEGMENT_A | DISPLAY_SEGMENT_B | DISPLAY_SEGMENT_C | DISPLAY_SEGMENT_D | DISPLAY_SEGMENT_E | DISPLAY_SEGMENT_F)
#define DISPLAY_NUMERAL_1 (DISPLAY_SEGMENT_B | DISPLAY_SEGMENT_C)
#define DISPLAY_NUMERAL_2 (DISPLAY_SEGMENT_A | DISPLAY_SEGMENT_B | DISPLAY_SEGMENT_G | DISPLAY_SEGMENT_E | DISPLAY_SEGMENT_D)
#define DISPLAY_NUMERAL_3 (DISPLAY_SEGMENT_A | DISPLAY_SEGMENT_B | DISPLAY_SEGMENT_C | DISPLAY_SEGMENT_D | DISPLAY_SEGMENT_G)
#define DISPLAY_NUMERAL_4 (DISPLAY_SEGMENT_F | DISPLAY_SEGMENT_B | DISPLAY_SEGMENT_C | DISPLAY_SEGMENT_G)
#define DISPLAY_NUMERAL_5 (DISPLAY_SEGMENT_A | DISPLAY_SEGMENT_F | DISPLAY_SEGMENT_G | DISPLAY_SEGMENT_C | DISPLAY_SEGMENT_D)
#define DISPLAY_NUMERAL_6 (DISPLAY_SEGMENT_A | DISPLAY_SEGMENT_C | DISPLAY_SEGMENT_D | DISPLAY_SEGMENT_E | DISPLAY_SEGMENT_F | DISPLAY_SEGMENT_G)
#define DISPLAY_NUMERAL_7 (DISPLAY_SEGMENT_A | DISPLAY_SEGMENT_B | DISPLAY_SEGMENT_C)
#define DISPLAY_NUMERAL_8 (DISPLAY_SEGMENT_A | DISPLAY_SEGMENT_B | DISPLAY_SEGMENT_C | DISPLAY_SEGMENT_D | DISPLAY_SEGMENT_E | DISPLAY_SEGMENT_F | DISPLAY_SEGMENT_G)
#define DISPLAY_NUMERAL_9 (DISPLAY_SEGMENT_A | DISPLAY_SEGMENT_B | DISPLAY_SEGMENT_C | DISPLAY_SEGMENT_D | DISPLAY_SEGMENT_F | DISPLAY_SEGMENT_G)

void display_clear()
{
   shifter_write(0);
}

//

void display_out(uint8_t value)
{
   static const uint8_t values[10] = {
      DISPLAY_NUMERAL_0,
      DISPLAY_NUMERAL_1,
      DISPLAY_NUMERAL_2,
      DISPLAY_NUMERAL_3,
      DISPLAY_NUMERAL_4,
      DISPLAY_NUMERAL_5,
      DISPLAY_NUMERAL_6,
      DISPLAY_NUMERAL_7,
      DISPLAY_NUMERAL_8,
      DISPLAY_NUMERAL_9
   };
   
   shifter_write(values[value]);
}

int main(void)
{
   shifter_setup();
   display_clear();

   // Setup a timer

   TCCR1 |= (1<<CS10) | (1<<CS11)/* | (1<<CS12)*/ | (1<<CS13);
   TIMSK |= (1<<TOIE1);

   sei();   //   Enable all interrupts 

   while (1) {
     // Nothing
   }

   return 0;
}

ISR(TIMER1_OVF_vect)      //   Timer1 Interrupt Routine 
{
  static volatile value = 0;

  display_out(value);
  
  value++;
  if (value == 10) {
    value = 0;
  }
}
