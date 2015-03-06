/**************************************************
  Written By: Tanner Christensen

  Program Description: Music Collab Board Firmware
  This is a program for a MCU driving
  a Nordic 2.4ghz tranceiver. This program reads in
  data from peripherals, including an accelorometer
  and ADC, and sends the data via the transceiver. 

 **************************************************/




#define F_CPU 8000000

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <avr/cpufunc.h>
#include "../../lib/spi/spi.h"
#include <stdlib.h>
#include "../../lib/radio_control/radioctl.h"
#include "../../lib/i2c/i2c.h"

#define ADC_PRESCALER 0
#define LED_CONFIG	(DDRD |= (1<<6))
#define CPU_PRESCALE(n)	(CLKPR = 0x80, CLKPR = (n))
#define F_CPU 8000000

#define THREE_BYTES 0b00000001
#define FOUR_BYTES 0b10
#define FIVE_BYTES 0b11

#define DATA_PIPE_0 0x11

#define PACKET_SIZE 16 
#define FALSE 0
#define TRUE 1


void initUART(void){

  //UBRR1 = (F_CPU / 4/ baud - 1) / 2;
  UBRR1 = 25;  //38.4k baud
  UCSR1A = (1<<U2X1);
  UCSR1B = (1<<RXEN1) | (1<<TXEN1);
  UCSR1C = (1<<UCSZ11) | (1<<UCSZ10);  //1 stop bit
}

void setDigiPot(uint8_t resistance){

  SPI_masterInit();
  digiPotInitWiper(resistance);
}

int8_t getADC(int8_t* frame){

  int8_t ADCval;  

  while(ADCSRA & (1 << ADSC)){
    ADCval = ADCH; 
  }
  if(ADCval == 0){

    setDigiPot(0x00);
    frame[2] = 0;
    ADCval = getADC(frame); 
  }
  if(ADCval == 255){

    setDigiPot(0xff);
    frame[2] = 1; 
    ADCval = getADC(frame); 
  }

  return ADCval;
}

void reset(void){
  
  wdt_enable(1);
  _delay_ms(1000);
}

void initAll(void){

  cli();

  int8_t frame[PACKET_SIZE] = {0};
  int8_t *temp = (int8_t *)malloc((PACKET_SIZE - 10)*sizeof(int8_t));
  int8_t set_flag = FALSE;
  int8_t pin_count = 0; 
  int i;
  CPU_PRESCALE(0x01);  // run at 8 MHz

  INIT_CSN;
  INIT_CE;
  CSN_HIGH;

  /****** Initialize Pin Stuff ******/
  /* for reset */

  DDRE = 0x00; //for reset
  PORTE &= ~(1<<PE6); //enable pull up resistor 
  EIMSK |= 0b01000000; //set only INT6
  EICRB = 0b00000000; //generate on low level (INT6) 
  

  /* other pin stuff */  
  DDRC = (1<<DDC7); //for Event button which is on PC6
  PORTC = (1<<PC6);  //enables pull up resistor

  DDRD |= (1<<6);  //sets D6 as output
  DDRD |= (1<<7);  //sets D7 as output
  PORTD |= (1<<6);
  PORTD |= (1<<7);  //sets the SPI select bits high

  DDRF|= (1<<6);
  PORTF |= (1<<6); 

  /****** ADC Stuff ******/

  ADMUX |= 0 << REFS1 | 1 << REFS0 | 1 << ADLAR;
  ADCSRA |= 1 << ADEN | 1 << ADPS2 | 1 << ADPS0;
  // ADCH holds analog value
  ADCSRB = 0;
  DIDR0 = 1 << ADC0D;
  DIDR2 = 0;


  SPI_masterInit();

  initRadioTX();
  setRadioAddressWidth(THREE_BYTES);
  setRadioTXAddress(0xABC123); 
  setRadioRXAddress(0xABC123);  // must match RX device

  eeprom_busy_wait();
  uint8_t val = eeprom_read_byte(0x00);
  setRadioFrequency(val);
  frame[0] = val; 

  /***** Initialiize Peripherals ********/

  initI2C();

  batteryICI2C(5, 0xA0);  //disables TS pin
  batteryICI2C(2, 0x8F);  //program battery regulation to 4.2 V
  batteryICI2C(1, 0x6C);  //programs current limit to external settings
  batteryICI2C(3, 0x80);  //sets charge current to 800mA


  SPI_masterInit();
  digiPotInitWiper(0xFF); //change to 0xFF


  _delay_ms(5);

  int pin_count = 0; 
  int i;
  char data1[PACKET_SIZE];

}

ISR(INT6_vect){
  
  /* reinitializes system  */
  initAll(); 
  
}

int main(void){

  initAll(); 

  while(1){

    if (PINE & (1 << 6)){

      i++;

      if(i >= 663000){
        set_flag = TRUE;
      }
    } 

    else i = 0;

    ADCSRA |= 1 << ADSC; 

    initI2C();
    batteryICI2C(5, 0xA0);  //disables TS pin

    frame[1] = getADC(frame);

    frame[3] = 0x00;

    if(!(PINC & (1 << PC6))){   // get push button

      frame[3] = 0x01;
    }

    //get accelerometer data
    SPI_masterInit();
    init_accel();	
    accelerometerReceive(temp);
    SPI_masterInit();			// resets SPDR register 

    //store in array
    for(int j = 0; j < 6; ++j){
      frame[j+4] = temp[j];
    }

    setRadioTXPayload(frame, PACKET_SIZE);
    CE_HIGH;
    _delay_us(15);  // pulse CE to start transmition
    CE_LOW;

    while(!(getTX_DS())){

      _delay_us(300);  //retransmit time determined by SETUP_RETR register

      if(getMAX_RT()){

        clearMAX_RT();
        CE_HIGH;
        _delay_us(15);
        CE_LOW;
      }
    }
    clearTX_DS();

    _delay_ms(5);

    /* sleep function 
     * sets sleep mode to low power (<1mA)
     * enables sleep register
     * puts the NRF to sleep
     * calls the WDT reset function
     * enables the interrupt for waking
     * puts the MCU into sleep mode
     */
    if(set_flag == TRUE){

      set_sleep_mode(0b00000101); 
      sleep_enable();
      sleepRadio();
      DDRB = 0x00;
      PORTB = 0xFF;
      DDRF = 0x00;
      PORTF = 0xFF;
      DDRD = 0x00;
      PORTD = 0xFF;
      reset();
      sei();
      sleep_cpu();
      sleep_disable();
    } 
  }
  free(temp);
}


