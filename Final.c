#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint-gcc.h>
#include <avr/signal.h>
#include <avr/eeprom.h>
#include <math.h>

#include <avr/delay.h>


#define F_CPU 16000000
#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1

volatile uint16_t contor;
uint16_t adc_result;
uint16_t nr_apasari=0;

void USART_Init( unsigned int ubrr)
{
	/*Set baud rate */
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)ubrr;
	/*Enable receiver and transmitter */
	UCSR0B = (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0); //|(1<<RXCIE0);
	/* Set frame format: 8data, 2stop bit */
	UCSR0C = (1<<USBS0)|(3<<UCSZ00);
}

void USART_Transmit(unsigned char data)
{
	/* Wait for empty transmit buffer */
	while ( !( UCSR0A & (1<<UDRE0)) );
	/* Put data into buffer, sends the data */
	UDR0 = data;
}


unsigned char USART_Receive( void )
{
	/* Wait for data to be received */
	while ( !(UCSR0A & (1<<RXC0)) );
	/* Get and return received data from buffer */
	return UDR0;
}

void InitADC()
{
	
	ADMUX=(1<<REFS0);									// For Aref=AVcc;
	ADCSRA=(1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);  //Rrescalar div factor =128, read from A0
}

uint16_t ReadADC(uint8_t ch)
{
	//Select ADC Channel ch must be 0-7
	ch=ch&0b00000111;
	ADMUX|=ch;

	//Start Single conversion
	ADCSRA|=(1<<ADSC);

	//Wait for conversion to complete
	while(!(ADCSRA & (1<<ADIF)));

	//Clear ADIF by writing one to it
	//Note you may be wondering why we have write one to clear it
	//This is standard way of clearing bits in io as said in datasheets.
	//The code writes '1' but it result in setting bit to '0' !!!

	//ADCSRA|=(1<<ADIF);

	return(ADC);
}

void SerialSend(float value){
	int x = value * 100;
	int p = 1000;
	char chr;
	while(x != 0){
		chr = x/p + 48;
		USART_Transmit(chr);
		x = x % p;
		p=p/10;
		if (p == 10) {
			chr = ',';
			USART_Transmit(chr);
		}
	}
	//chr = 10; //new line
	chr='\n';
	USART_Transmit(chr);
}

void EEPROM_write(unsigned int uiAddress, unsigned char ucData)
{
	/* Wait for completion of previous write */
	while(EECR & (1<<EEPE))
	{

	};
	/* Set up address and Data Registers */
	EEAR = uiAddress;
	EEDR = ucData;
	/* Write logical one to EEMPE */
	EECR |= (1<<EEMPE);
	/* Start eeprom write by setting EEPE */
	EECR |= (1<<EEPE);
}


unsigned char EEPROM_read(unsigned int uiAddress)
{
	/* Wait for completion of previous write */
	while(EECR & (1<<EEPE))
	{

	};
	/* Set up address register */
	EEAR = uiAddress;
	/* Start eeprom read by writing EERE */
	EECR |= (1<<EERE);
	/* Return data from Data Register */
	return EEDR;
}

void Afisaj_7Segments(int nr_apasari)
{
	switch(nr_apasari)
	{
		case 0:
			PORTD&=0b00101111;
			PORTB&=0b11100001;
			PORTD|=0b11010000;
			PORTB|=0b00011100;
			break;
			
		case 1:
			PORTD&=0b00101111;
			PORTB&=0b11100001;
			PORTD|=0b10000000;
			PORTB|=0b00010000;
			break;
		
		case 2:
			PORTD&=0b00101111;
			PORTB&=0b11100001;
			PORTD|=0b01010000;
			PORTB|=0b00011010;
		break;
		
		case 3:
			PORTD&=0b00101111;
			PORTB&=0b11100001;
			PORTD|=0b11000000;
			PORTB|=0b00011010;
		break;
		
		case 4:
			PORTD&=0b00101111;
			PORTB&=0b11100001;
			PORTD|=0b10000000;
			PORTB|=0b00010110;
		break;
		
		case 5:
			PORTD&=0b00101111;
			PORTB&=0b11100001;
			PORTD|=0b11000000;
			PORTB|=0b00001110;
		break;
		
		case 6:
			PORTD&=0b00101111;
			PORTB&=0b11100001;
			PORTD|=0b11010000;
			PORTB|=0b00001110;
		break;
		
		case 7:
			PORTD&=0b00101111;
			PORTB&=0b11100001;
			PORTD|=0b10000000;
			PORTB|=0b00011000;
		break;
		
		case 8:
			PORTD&=0b00101111;
			PORTB&=0b11100001;
			PORTD|=0b11010000;
			PORTB|=0b00011110;
		break;
		
		case 9:
			PORTD&=0b00101111;
			PORTB&=0b11100001;
			PORTD|=0b11000000;
			PORTB|=0b00011110;
		break;
		
		
		default:
			//PORTD&=0b00101111;
			//PORTB&=0b11100001;
			break;
	}		
}

int main(void)
{
	TCCR1A=0;
	TCCR1B=0;
	TCCR1B |= (1<<WGM12)|(1<<CS12)|(1<<CS10); //1024 prescaler (CS12 & CS10) / (WGM12- compare match - Top=OCR1A)
	TIMSK1 |= (1<<OCIE1A);					 //Output Compare A Match Interrupt Enable
	
	OCR1A=15625; //1 sec interrupt
	DDRB = 0xFF;
	
	contor=0;
	DDRD=0xFF;	//PORTD set as output
	DDRD &= ~(1<<PIND3);	//pin D3 set as input
	
	TCCR0A = _BV(WGM01) | _BV(WGM00) | _BV(COM0A0) | _BV(COM0B1); //normal port operation OCR0A disconnected (COM0A0), clear OCR0B on Compare Match | Fast Mode PWM WGM00 & WGM01
	TCCR0B =  _BV(CS02);										 //256 prescaler => interrupt at 4 miliseconds
	//OCR0A = 0;	// pin 6 OCR0A disconected
	OCR0B = 0;		// pin 5 (PWM)
	
	TIMSK0=_BV(TOIE0);
	//interrupt on falling edge
	EICRA|=(1<<ISC01);	
	//enable external INT0	
	EIMSK|=(1<<INT0);
	
	
	InitADC();
	
	//first appear 0 on 7 segments display
	Afisaj_7Segments(nr_apasari);
	
	sei(); //enable global interrupts
	USART_Init(MYUBRR); 
	
	/* Replace with your application code */
	while (1)
	{
		//Citeste temperatura de la LM35 o converteste in grade celsius si o afiseaza
		adc_result=ReadADC(0);
		float mv = ( adc_result/1024.0)*5000;
		float celsius = mv/10;
		
		if(contor%100==0)
			SerialSend(celsius);
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

ISR(USART_RX_vect)
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


ISR(INT0_vect)
{
	
	if(nr_apasari<=9)
	{
		nr_apasari++;
		
	}
	else
		nr_apasari=0;
	EEPROM_write(300,nr_apasari);
	//int nr = EEPROM_read(300);
	Afisaj_7Segments(nr_apasari);
}
