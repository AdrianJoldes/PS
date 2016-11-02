/*
 * PS_2.c
 *
 * Created: 10/25/2016 10:52:01 PM
 * Author : Christian
 */ 

#include <avr/io.h>
#include <stdio.h>
#include <avr/interrupt.h>
//#include <util/delay.h>

volatile uint16_t contor;

int main(void)
{
	contor=0; 
	DDRD=0xFF;	//PORTD set as output
	TCCR0A = _BV(WGM01) | _BV(WGM00) | _BV(COM0A0) | _BV(COM0B1); //normal port operation OCR0A disconnected (COM0A0), clear OCR0B on Compare Match | Fast Mode PWM WGM00 & WGM01
	TCCR0B =  _BV(CS02); //256 prescaler => interrupt at 4 miliseconds
	//OCR0A = 0;	// pin 6 OCR0A disconected
	OCR0B = 0;		// pin 5
	
	TIMSK0=_BV(TOIE0);
	sei();
	
    /* Replace with your application code */
    while (1) 
    {
		if(contor<=255)
		OCR0B++;									//fade in 0-255
		else if( (contor>255) && (contor<=510))
		OCR0B=255;									// keep on 255
		else if((contor>510) && (contor<=765))
		OCR0B--;									//fade out 255-0
		else if((contor>765) && (contor<=1020))
		{
			OCR0B=0;								//keep on 0
		}
		else
		contor=0;								
    }
}

ISR(TIMER0_OVF_vect)
{
	contor++;
}


