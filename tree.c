// define CPU speed - actual speed is set using CLKPSR in main()
#include <stdint.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

const uint8_t loop_ddrb[] PROGMEM = {
  0b00011,
  0b00011,
  0b00101,
  0b00101,
  0b01001,
  0b01001,
  0b10001,
  0b10001,
  0b00110,
  0b00110,
  0b01010,
  0b01010,
  0b10010,
  0b10010,
  0b01100,
  0b01100,
  0b10100,
  0b10100,
  0b11000,
  0b11000,
};

const uint8_t loop_portb[] PROGMEM = {
  0b00001,
  0b00010,
  0b00001,
  0b00100,
  0b00001,
  0b01000,
  0b00001,
  0b10000,
  0b00010,
  0b00100,
  0b00010,
  0b01000,
  0b00010,
  0b10000,
  0b00100,
  0b01000,
  0b00100,
  0b10000,
  0b01000,
  0b10000,
};

#define SIN_FRAMES 4
#define NUM_LEDS 20
#define NUM_POSITIONS 10
const uint8_t sin_vals[] PROGMEM = {
  0, 35, 99, 195, 255
};
int8_t d_status[] = {
  3,
  4,
  3,
  2,
  1,
  0,
  1,
  2,
  3,
  4,
};

volatile uint8_t d_pwm[] = {
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
};
volatile uint8_t  pwmCount = 0;
volatile uint8_t  loop_portb_mem = 0;
volatile uint16_t counter0 = 0;
const uint16_t colorInterval = 512;
volatile uint8_t x = 0, a = 0, b = 0, c = 0;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wparentheses"
void init_rng(uint8_t s1,uint8_t s2,uint8_t s3) //Can also be used to seed the rng with more entropy during use.
{
	//XOR new entropy into key state
	a ^=s1;
	b ^=s2;
	c ^=s3;

	x++;
	a = (a^c^x);
	b = (b+a);
	c = (c+(b>>1)^a);
}

uint8_t randomize()
{
	x++;               //x is incremented every round and is not affected by any other variable
	a = (a^c^x);       //note the mix of addition and XOR
	b = (b+a);         //And the use of very few instructions
	c = (c+(b>>1)^a);  //the right shift is to ensure that high-order bits from b can affect  
	return(c);          //low order bits of other variables
}
#pragma GCC diagnostic pop

int main() {
  // Allow changes to the clock prescaler
  CLKPR = 1<<CLKPCE;
  //  CLKPR[3:0] sets the clock division factor
  CLKPR = 0;

  init_rng(0x14, 0x67, 0x18);

  // set initial PWM value
  for(uint8_t i = 0; i < NUM_POSITIONS; i++) {
    int8_t state = d_status[i];
    d_pwm[i] = pgm_read_byte(&sin_vals[state<0?-state:state]);
  }

  // main loop
  while (1) {
    for(uint8_t currLed = 0; currLed < NUM_LEDS; currLed++) {
      PORTB = 0;
      DDRB = pgm_read_byte(&loop_ddrb[currLed]);
      loop_portb_mem = pgm_read_byte(&loop_portb[currLed]);
      if(pwmCount < d_pwm[currLed/2]) {
        PORTB = loop_portb_mem;
      } else {
        PORTB = 0;
      }
    }
    // increment PWM count
    pwmCount++;
    // increment interval counter
    counter0++;
    if(counter0 % colorInterval == 0) {
      for(uint8_t i = 0; i < NUM_POSITIONS; i++) {
        int8_t state = d_status[i];
        if( (state == 4 || state == 0) && randomize() % 21 != 0 ) {
          continue;
        }
        state = state == -1? 0 : state == SIN_FRAMES? -(state-1) : state+1;
        d_status[i] = state;
        d_pwm[i] = pgm_read_byte(&sin_vals[state<0?-state:state]);
      }
    }
  }
  return 0;
}
