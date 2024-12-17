#include <avr/io.h>
#include <avr/interrupt.h>

//volatile -tells the compiler that the object is subject to sudden change 
//for reasons which cannot be predicted from a study of the program itself, 
//and forces every reference to such an object to be a genuine reference
volatile uint8_t count;

int main(void)
{
   // Prescaler = FCPU/1024
   TCCR0|=(1<<CS02)|(1<<CS00);

   //Enable Overflow Interrupt Enable
   TIMSK|=(1<<TOIE0);

   //Initialize Counter
   TCNT0=0;

   //Initialize our varriable
   count=0;

   //Port C[3,2,1,0] as output
   DDRC|=0x0F;

   //Enable Global Interrupts
   sei();

   //Infinite loop
   while(1);

   return 0;
}

ISR(TIMER0_OVF_vect)
{
   //This is the interrupt service routine for TIMER0 OVERFLOW Interrupt.
   //CPU automatically call this when TIMER0 overflows.

   //Increment our variable
   count++;
   if(count == 61)
   {
      PORTC ^= 0x0F; //Invert the Value of PORTC, first 4 bits
      count=0;
   }
}
