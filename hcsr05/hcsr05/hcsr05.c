/*
 * hcsr05.c
 *
 * Created: 10/25/2015 1:55:50 PM
 *  Author: Ripagood
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>


#define F_CPU 16000000
#define BAUDRATEPC 9600                           // define baud
#define BAUDRATE ((F_CPU)/(BAUDRATEPC*16UL)-1)            // set baud rate value for UBRR


volatile uint16_t time1;
volatile uint16_t time2;
volatile uint8_t measureReady=0;

void serialPCtx(uint8_t byte);

ISR (TIMER4_CAPT_vect){

	if ( TCCR4B & (1<<ICES4))// we are on rising edge
	{
		time1= ICR4;
		TCCR4B &= ~(1<<ICES4);// select trigger by falling edge
	}else{
		// we are on falling edge
		time2= ICR4;
		TIMSK4 &= ~(1<<ICIE4);
		TCNT4=0;//reset timer
		TCCR4B =0 ;//disconnect clock source
		measureReady=1;
	}
	
	
}

void initSerialPC(void){
	//UBRR0 = 8;
	UBRR0 = BAUDRATE;
	//UCSR0B|= (1<<TXEN0)|(1<<RXEN0)|(1<<RXCIE0);                // enable receiver and transmitter
	UCSR0B= (1<<TXEN0)|(1<<RXEN0);
	
	UCSR0C= (1<<UCSZ00)|(1<<UCSZ01);   // 8bit data format
}

void serialPCtx(uint8_t byte){
	
	while (!( UCSR0A & (1<<UDRE0)));                // wait while register is free
	UDR0 = byte;
	
	
}


void serialPCtxArray(uint8_t* array,uint8_t tam){
	
	for (uint8_t i=0; i<tam;i++)
	{
		serialPCtx(array[i]);
	}
	
	
}


uint8_t average8(uint8_t value){
	
	static uint8_t average[8];
	uint16_t sum=0;
	for (uint8_t i=7; i>0;i--)
	{
		average[i]=average[i-1];
		sum+=average[i-1];
		
	}
	average[0]=value;
	sum+=average[0];
	
	return (sum >> 4);
	
}
uint8_t average4(uint8_t value){
	
	static uint8_t average[4];
	uint16_t sum=0;
	for (uint8_t i=3; i>0;i--)
	{
		average[i]=average[i-1];
		sum+=average[i-1];
		
	}
	average[0]=value;
	sum+=average[0];
	
	return (sum >> 2);
	
}

uint8_t takeMeasureCm(void){
	uint16_t measure;
	//send pulse on trigger pin
	PORTL |= (1<<PL1);
	_delay_us(30);
	PORTL &= ~(1<<PL1);
	//TCCR4B = (1<<ICNC4)|(1<<ICES4)|(1<<CS41)|(1<<CS40);//rising edge, prescaler = 64
	TCCR4B = (1<<ICNC4)|(1<<ICES4)|(1<<CS41);//rising edge, prescaler = 8 ,
	TIMSK4 = (1<<ICIE4); // enable interrupt
	while(!measureReady);
	measureReady=0;
	measure = time2-time1;
	measure +=4;// beacuse of noise canceller
	//measure *=4;
	measure /= 116; // (58*2)
	
	return (uint8_t) measure;
	
}



int main(void)
{
	uint16_t measure=0;
	char asciiMeasure[2];
	
	initSerialPC();
	//SETUP INPUT CAPTURE
	serialPCtxArray("Initializing ICP1\n",18);
	DDRL |= (1<<PL1);//pl1 as trigger output
	DDRL &= ~(1<<PL0);
	PORTL |= (1<<PL0); //pl0 as input and with pull up
	
	
	
	
	sei();
	
	
    while(1)
    {
		//serialPCtx(takeMeasureCm());
		serialPCtx(average8(takeMeasureCm()));
    }
}