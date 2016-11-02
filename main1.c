/*
 * PS_1.c
 *
 * Created: 10/25/2016 6:48:24 PM
 * Author : Christian
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint8_t count;
int main(void)
{
	TCCR1A=0;
	TCCR1B=0;
	TCCR1B |= (1<<WGM12)|(1<<CS12)|(1<<CS10); //1024 prescaler (CS12 & CS10) / (WGM12- compare match - Top=OCR1A)
	TIMSK1 |= (1<<OCIE1A);	//Output Compare A Match Interrupt Enable
	
	OCR1A=15625; //1 sec interrupt
	DDRB = 0xFF;
	sei();
	
    /* Replace with your application code */
    while (1) 
    {
    }
}

ISR(TIMER1_COMPA_vect)
{		
		PORTB ^=0x20; //pin 13 (PB5)
}
