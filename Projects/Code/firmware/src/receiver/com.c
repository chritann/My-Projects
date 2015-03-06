/**************************************************
Written By: Tanner Christensen

Program Description:
This is a program for an AVR MCU driving
a Nordic 2.4ghz tranceiver. This function receivers
an 8 byte packet from a transmitter and sends it
via UART
**************************************************/



#define F_CPU 8000000

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/cpufunc.h>
#include "../../lib/spi/spi.h"
#include <stdlib.h>
#include "../../lib/radio_control/radioctl.h"

// Teensy 2.0: LED is active high
#if defined(__AVR_ATmega32U4__) || defined(__AVR_AT90USB1286__)
#define LED_ON		(PORTD |= (1<<6))
#define LED_OFF		(PORTD &= ~(1<<6))

// Teensy 1.0: LED is active low
#else
#define LED_ON	(PORTD &= ~(1<<6))
#define LED_OFF	(PORTD |= (1<<6))
#endif

#define ADC_PRESCALER 0
#define LED_CONFIG	(DDRD |= (1<<6))
#define CPU_PRESCALE(n)	(CLKPR = 0x80, CLKPR = (n))
#define DIT 80		/* unit time for mors e code */
#define BAUD_RATE 38.4k
#define F_CPU 8000000
#define PACKET_SIZE 16

void initUART(void)
{
	//UBRR1 = (F_CPU / 4/ baud - 1) / 2;
	UBRR1 = 25;  //38.4k baud
	UCSR1A = (1<<U2X1);
  	UCSR1B = (1<<RXEN1) | (1<<TXEN1);
	UCSR1C = (1<<UCSZ11) | (1<<UCSZ10);  //1 stop bit
}



int main(void)
{

	uint8_t *load = (uint8_t *)malloc(PACKET_SIZE*sizeof(uint8_t));


	CPU_PRESCALE(0x01);  // run at 8 MHz

	INIT_CSN;
	INIT_CE;
	CSN_HIGH;
	CE_HIGH;


	initUART();

	// Put receiver in sleep mode

	SPI_masterInit();
	initRadioRX();

	setRadioAddressWidth(THREE_BYTES);
	setRadioRXAddress(0xABC123);
	setRadioRXPayloadSize(DATA_PIPE_0, PACKET_SIZE);

	_delay_ms(5);

  	int8_t radioFrequency = 0b00000000;
  	setRadioFrequency(radioFrequency);

	while (1) {


		uint8_t radioStatus = 0;

		getRadioStatus(&radioStatus);

		if(getRX_DR())  // if RX data received
		{
			clearRX_DR();

      		getRadioRXPayload(load, PACKET_SIZE); //read payload

			CE_LOW;

			for(int i = 0; i<PACKET_SIZE; i++)
			{
       		while(!(UCSR1A & (1 << UDRE1)));
        	UDR1 = load[i];
			}

			CE_HIGH;
		}

    /* while(!(UCSR1A & (1 << UDRE1))); */
    /* UDR1 = radioStatus; */

		
    	radioFrequency += 0b00000001;
    	if (radioFrequency == 30) {
      		radioFrequency = 0b00000000;
    	}
    	setRadioFrequency(radioFrequency); 
    	_delay_ms(8);
	}

	free(load);

}
