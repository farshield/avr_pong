#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 8000000UL   //internal RC oscillator 8MHz
#include <util/delay.h>

#define nrCol  8  //nr. of columns

#define DUTY_CYCLE 32 //used in start_buzzer() funct.

//shift register output pins from PORTB
#define CLK   0 //PB0
#define S_IN  1 //PB1
#define LATCH 2 //PB2

/*
	Global variables
*/
uint8_t column[nrCol];
volatile uint8_t seq = 0;
//volatile tells the compiler that the object is subject to sudden
//change for reasons which cannot be predicted from a study of the
//program itself

//+++ basic functions
//---------------------------------------------------------------------

void initialize(void);

//reads current state of a button: 1 if pressed, 0 otherwise
//index: values from 1->4 (4 buttons, the 5th is the reset button)
uint8_t readButton(uint8_t index);
//Player 1 (right side of the board): 1(up), 2(down)
//Player 2 (left side of the board): 3(up), 4(down)

/* Buzzer related functions */
void start_buzzer(); //continue annoying until stop_buzzer is called
void stop_buzzer();  //end torment
void beep_once(uint16_t d_ms); //d_ms - time in milliseconds

/* 7 segment display related functions */
void shift_data(uint8_t data); //used in update_score() funct.
void update_score(uint8_t score_p1, uint8_t score_p2); //use this directly

//---------------------------------------------------------------------

void init_matrix()
{
	uint8_t i;
	for (i = 0; i < nrCol; i++)
		column[i] = 0xFF; //turn all LEDs on
		//column[i] = 0x00;
}

int main(void)
{	
	initialize();
	sei(); //enable global interrupts

	init_matrix();
	update_score(0, 0);
	
	for(;;)
	{
	}
	
	return 0;
}

void initialize(void)
{
	/*
		Port A (sequence)

		Set all 8 pins as output pins
		One hot code will be stored in PORTA register
		Column driver
	*/
	DDRA = 0xFF;
	PORTA = 0x00;

	/*
		Port D (data)

		Set all 8 pins as output pins
		Column data will be stored in PORTD register
	*/
	DDRD = 0xFF;
	PORTD = 0x00;

	/*
		Port B (buttons)

		Set last 4 pins as input pins
		Button state will be read from PINB
	*/
	DDRB &= ~0xF0;

	/*
		Port B (buzzer + reg_shift)

		Set first 4 pins as output pins
		PB0, PB1, PB2 - for the shift register
		PB3 - for the buzzer (PWM controlled)
	*/
	DDRB |= 0x0F;

	/*
		FCPU / Prescaler = frequency at which counter increments by 1

		PWM - power width modulation setup
		Fast PWM (WGM)
		Non inverting (COM)
		
		Prescaler
		CS02 CS01 CS00
		001 - 1; 010 - 8; 011 - 64; 100 - 256; 101 - 1024
	*/
	TCCR0 |= (1<<WGM01) | (1<<WGM00) | (1<<COM01) | (0<<CS02) | (1<<CS01) | (0<<CS00);
	TCCR0 &= ~((1<<COM01) | (1<<COM00)); //for now deactivate output to buzzer

	/*
		Enable overflow interrupt for TIMER0
	*/
	TIMSK |= (1 << TOIE0);
}

uint8_t readButton(uint8_t index)
{
	if (index < 1 || index > 4) return 0;

	return !(PINB & (0x10 << (index - 1)));
}

void start_buzzer()
{
	TCCR0 |= (1<<COM01);  //activate output to buzzer
	OCR0 = DUTY_CYCLE;  //apply signal to buzzer
	//mean voltage: DUTY_CYCLE / 256 * 5V
}

void stop_buzzer()
{
	TCCR0 &= ~((1<<COM01) | (1<<COM00));  //deactivate output to buzzer
	OCR0 = 0; //even so, mean voltage 20mV (buzzer would still sound because of this
			  //if output wasn't deactivated)
}

void beep_once(uint16_t d_ms)
{
	start_buzzer();
	_delay_ms(d_ms);
	stop_buzzer();
}

//test this for real
//delays?
void shift_data(uint8_t data)
{
	uint8_t i;
	uint8_t bit;

	PORTB &= ~(1 << LATCH); //deactivate latch output

	for (i = 0; i < 8; i++)
	{
		PORTB &= ~(1 << S_IN);
		PORTB &= ~(1 << CLK);

		if (data & (1 << i))
			bit = 0x01;
		else
			bit = 0x00;

		PORTB |= (bit << S_IN);
		PORTB |= (1 << CLK);
	}

	PORTB &= ~(1 << CLK);  //set clock to zero
	PORTB |= (1 << LATCH); //activate latch output
}

void update_score(uint8_t score_p1, uint8_t score_p2)
{
	shift_data((score_p2 & 0x0F) | ((score_p1 & 0x0F) << 4));
}

/*
	Interrupt service routine

	This routine is called in background at a frequency of FCPU / Prescaler / 256
		when counter overflows.
	Prescaler is currently 8 (CS02 CS01 CS00 = 0 1 0)
	256 because of 8 bits counter
	Currently: 8MHz / 8 / 256 = 3.9 kHz
*/
ISR(TIMER0_OVF_vect)
{
	if (seq >= 8)
		seq = 0;

	PORTA = 0x01 << seq;
	PORTD = column[seq];

	seq++;
}
