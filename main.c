#include <avr/io.h>
#include <stdio.h>
#include <avr/interrupt.h>
#define F_CPU 16000000
#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1

volatile uint16_t contor;

void USART_Init( unsigned int ubrr)
{
	/*Set baud rate */
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)ubrr;
	/*Enable receiver and transmitter */
	UCSR0B = (1<<RXEN0)|(1<<TXEN0); //|(1<<RXCIE0);
	/* Set frame format: 8data, 2stop bit */
	UCSR0C = (1<<USBS0)|(3<<UCSZ00);
}

void USART_Transmit(unsigned char data)
{
	/* Wait for empty transmit buffer */
	while ( !( UCSR0A & (1<<UDRE0)) );
	/* Put data into buffer, sends the data */
	//for(int i=0;i<strlen(data);i++)
	UDR0 = data;
}


unsigned char USART_Receive( void )
{
	/* Wait for data to be received */
	while ( !(UCSR0A & (1<<RXC0)) );
	/* Get and return received data from buffer */
	return UDR0;
}


int main(void)
{
	TCCR1A=0;
	TCCR1B=0;
	TCCR1B |= (1<<WGM12)|(1<<CS12)|(1<<CS10); //1024 prescaler (CS12 & CS10) / (WGM12- compare match - Top=OCR1A)
	TIMSK1 |= (1<<OCIE1A);	//Output Compare A Match Interrupt Enable
	
	OCR1A=15625; //1 sec interrupt
	DDRB = 0xFF;
	
	contor=0;
	DDRD=0xFF;	//PORTD set as output
	TCCR0A = _BV(WGM01) | _BV(WGM00) | _BV(COM0A0) | _BV(COM0B1); //normal port operation OCR0A disconnected (COM0A0), clear OCR0B on Compare Match | Fast Mode PWM WGM00 & WGM01
	TCCR0B =  _BV(CS02); //256 prescaler => interrupt at 4 miliseconds
	//OCR0A = 0;	// pin 6 OCR0A disconected
	OCR0B = 0;		// pin 5
	
	TIMSK0=_BV(TOIE0);
	USART_Init(MYUBRR); 
	
	sei();
	
	/* Replace with your application code */
	while (1)
	{
	
		char c=USART_Receive();
		if(c=='a')
		{
			PORTB|=0x01;
		}
		else
		if(c=='s')
		{
			PORTB&=0b11111110;;
		}
	}
}

ISR(TIMER1_COMPA_vect)
{
	PORTB ^=0x20; //pin 13 (PB5)
}

ISR(TIMER0_OVF_vect)
{
	contor++;
	
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

