#include <avr/io.h>
#include <stdlib.h>

#define F_CPU 8000000UL   //internal RC oscillator 8MHz
#include <util/delay.h>

#define nrCol  8  //nr. of columns
#define PERIOD 10 //in milliseconds

#define DUTY_CYCLE 32 //used in start_buzzer() funct.

//shift register output pins from PORTB
#define CLK   0 //PB0
#define S_IN  1 //PB1
#define LATCH 2 //PB2

#define _B_TIME 200

uint8_t column[nrCol];
uint8_t l[12]={0,0,1,2,4,8,16,32,64,128,0,0}; //mapare linii
uint8_t sw,sw1,sw2,score1,score2;
uint8_t PSPEED=10, BSPEED=20;
uint8_t go;

struct bila
{
	int8_t x;
	int8_t y;
	
	int8_t old_x;
	int8_t old_y;

	
	int8_t speed_x;
	int8_t speed_y;
}bila;

//+++ basic functions
//---------------------------------------------------------------------

void initialize(void);

//this function takes <PERIOD> milliseconds to execute
void display(uint8_t *column);

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

void init_matrix(uint8_t *column)
{
    uint8_t i;
    for (i = 0; i < nrCol; i++)
        column[i] = 0xFF; //turn all LEDs on
        //column[i] = 0x00;
}

void clear()
{
	uint8_t i;
    for (i = 0; i < nrCol; i++)
        column[i] = 0;
}

void long_display()
{
	uint8_t i;
	for(i=0 ;i<50; i++)
	 display(column);
}
void initBall()
{
	int8_t v[2]={-1,1};
	int8_t v2[3]={-1,0,1};

	bila.x=3+rand()%2;
	bila.y=3+rand()%2;
	bila.old_x=bila.x;
	bila.old_y=bila.y;
	bila.speed_x=v[rand()%2];
	bila.speed_y=0;

	column[0]=column[7]=0x18;

	
//	long_display();

}

void move_ball()
{
		
	uint8_t i;
	bila.old_x=bila.x;
	bila.x=bila.x + bila.speed_x;
	
	bila.old_y=bila.y;
	bila.y=bila.y + bila.speed_y;


	//coliziune cu perete(sus/jos)

	if(bila.y==7 && bila.speed_y==1)
		bila.speed_y=-1;

	if(bila.y==0 && bila.speed_y==-1)
		bila.speed_y=1;
		
	sw1=0; sw2=0;



	//Coliziune cu palete

// coliziune cu paleta dreapta
	if(bila.x==6)
	{
		if((l[2+bila.y]+l[2+bila.y-1])==column[7]) 
		{
			bila.speed_x=-1;
			if(bila.speed_y!=1)
				bila.speed_y++;
			sw2=1;
		}
		

		if ((l[2+bila.y+1]+l[2+bila.y])==column[7])
		{
			bila.speed_x=-1;
			if(bila.speed_y!=-1)
				bila.speed_y--;
			sw2=1;
		}
		

		if ( ((l[2+bila.y+1]+l[2+bila.y+2])==column[7]) && (bila.speed_y==1))
		{
			bila.speed_x=-1;
			bila.speed_y=-1;
			sw2=1;
			
		}

		if ( ((l[2+bila.y-1]+l[2+bila.y-2])==column[7]) && (bila.speed_y==-1))
		{
			bila.speed_x=-1;
			bila.speed_y=1;
			sw2=1;
			
		}

	}

// castiga stangaciul
	if (sw2==0 && bila.x==6)
	{
		score1++;
		//beep_once(_B_TIME);
		go=1;
		update_score(score1, score2);


		column[7]=255;
		column[bila.x]=l[2+bila.y];
		for (sw=0;sw<100;sw++) display(column);
		initBall();	}

// coliziune cu paleta stanga
	if(bila.x==1)
	{
		if((l[2+bila.y]+l[2+bila.y-1])==column[0])  
		{
			bila.speed_x=1;
			if(bila.speed_y!=1)
				bila.speed_y++;
			
			sw1=1;
		}
	

		if ((l[2+bila.y+1]+l[2+bila.y])==column[0]) 
		{
			bila.speed_x=1;
			if(bila.speed_y!=-1)
				bila.speed_y--;
			sw1=1;
		}
		

		if ( ((l[2+bila.y+1]+l[2+bila.y+2])==column[0]) && (bila.speed_y==1))
		{
			bila.speed_x=1;
			bila.speed_y=-1;
			sw1=1;
		}

		if ( ((l[2+bila.y-1]+l[2+bila.y-2])==column[0]) && (bila.speed_y==-1))
		{
			bila.speed_x=1;
			bila.speed_y=1;
			sw1=1;
		}

		
	}

// castiga dreptaciul
	if (sw1==0 && bila.x==1)
	{
		score2++;
		//beep_once(_B_TIME);
		go=1;
		update_score(score1, score2);

		column[0]=255;
		column[bila.x]=l[2+bila.y];
		for (sw=0;sw<100;sw++) display(column);
		initBall();
	}
// conditiile pt. colturile terenului
	if (bila.x==1 && bila.y==0 && bila.speed_y==-1)
		bila.speed_y=1;

	if (bila.x==6 && bila.y==0 && bila.speed_y==-1)
		bila.speed_y=1;

	if (bila.x==1 && bila.y==7 && bila.speed_y==1)
		bila.speed_y=-1;

	if (bila.x==6 && bila.y==7 && bila.speed_y==1)
		bila.speed_y=-1;




//LED-ul bilei on
	column[bila.x]=l[2+bila.y];


}

void game()
{


	uint8_t i;
	
	clear();

	column[0] = 0x18;
	column[7] = 0x18;
	sw1=0; sw2=0;
	score1=0; score2=0;	

	uint8_t btn1 = 0, btn2 = 0, btn3 = 0, btn4 = 0;
	uint8_t counter = 0;
	uint8_t counter2 = 0;
	uint8_t c1=column[0],c2=column[7];

	initBall();

	for(;;)
	{

///////////////////////////////
///////////Button Read/////////
		btn1 = readButton(1);
		btn2 = readButton(2);
		btn3 = readButton(3);
		btn4 = readButton(4);

        if(counter>PSPEED)
		{
			if (btn1)
			{
				if((column[7] << 1) <=192)
				c2=column[7] = column[7] << 1;
			}

			if (btn2)
			{
				if((column[7] >> 1) >= 3)
				c2=column[7] = column[7] >> 1;
			}

			if (btn3)
			{
				if((column[0] << 1) <=192)
				c1=column[0] = column[0] << 1;
			}

			if (btn4)
			{
				if((column[0] >> 1) >= 3)
				c1=column[0] = column[0] >> 1;
			}
			counter=0;
		}
		counter++;
////////////////////////////
////////////////////////////


////////////////////////
////////Score Update////
		if(score1>9 || score2>9)
		 { 
		   score1=0;
		   score2=0;
		   update_score(score1,score2);
		  }
////////////////////////
////////////////////////

		for(i=1; i<nrCol-1; i++)
			column[i]=0;

		//column[7]=c2; 
		//column[0]=c1;

///////////////////////
////////Ball movement//
		if(counter2>BSPEED)
		{
		 move_ball();
		 counter2=0;
		}
		counter2++;
//////////////////////
//////////////////////

		display(column);
				
	}
	
}

int main(void)
{
    initialize();
    init_matrix(column);
    update_score(0, 0);
	game(); 
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
    TCCR0 |= (1<<WGM01) | (1<<WGM00) | (1<<COM01) | (0<<CS02) | (1<<CS01) | (1<<CS00);
    TCCR0 &= ~((1<<COM01) | (1<<COM00)); //for now deactivate output to buzzer
}

void display(uint8_t *column)
{
    uint8_t i;
    uint8_t seq;

    seq = 0x01;
        
	PORTA=l[bila.x+2];
	PORTD=l[bila.y+2];
	_delay_ms(PERIOD /(nrCol+1));

    for (i = 0; i < nrCol; i++)
    {

        PORTA = seq;
        PORTD = column[i];
		_delay_ms(PERIOD /(nrCol+1));

        seq = seq << 1;
    }

	if (go==1) {
		beep_once(_B_TIME);
		go=0;
		}
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
