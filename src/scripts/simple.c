#include <avr/io.h>

#define F_CPU 8000000UL
#include <util/delay.h>

int main(void)
{

 DDRB = 0x01;
 for(;;)
 {
  PORTB = 0x00;
  _delay_ms(100);
  PORTB = 0x01;
  _delay_ms(100);
 }

}
