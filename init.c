#include <delay.h>
#include <mega328p.h>
#include "eeprom.h" 
#include "interrupt.h" 
#include "flags.h"
#include "init.h"
#include "sms.h"

//*****************************************************************************
//*****************************************************************************
void port_init(void)
{
 PORTB = 0b00000100;
 DDRB  = 0b00101100;  //NB SS pinne må være satt som utgang. Ellers klikker SPI bus.
 PORTC = 0x00;	
 DDRC  = 0x00;
 PORTD = 0b00000000;
 DDRD  = 0b01100110;
}


//*****************************************************************************
//*****************************************************************************
//Watchdog initialisation
// prescale: 2048K cycles
void watchdog_init(void)
{
// Watchdog Timer initialization
// Watchdog Timer Prescaler: OSC/2048
  #pragma optsize-
 	WDR(); //this prevents a timout on enabling
  WDTCSR=0x39;
  WDTCSR=0x29;
  #ifdef _OPTIMIZE_SIZE_
  #pragma optsize+
  #endif
}

//*****************************************************************************
//*****************************************************************************
//TIMER0 initialisation - prescale:Stop
// WGM: Normal
// desired value: 100Hz
// actual value: Out of range
void timer0_init(void)
{
  TCCR0A=0x00;
  TCCR0B=0x00;
  TCNT0=0x00;
  OCR0A=0x00;
  OCR0B=0x00;
}

//*****************************************************************************
//*****************************************************************************
//TIMER1 initialisation - prescale:1024
// WGM: 0) Normal, TOP=0xFFFF
// desired value: 15Sec
// actual value: 15,000Sec (0,0%)
void timer1_init(void)
{
  TCCR1A=0x00;
  TCCR1B=0x05;
  TCNT1H=0x1B;
  TCNT1L=0x1F;
  ICR1H=0x00;
  ICR1L=0x00;
  OCR1AH=0x00;
  OCR1AL=0x00;
  OCR1BH=0x00;
  OCR1BL=0x00;
}

//*****************************************************************************
// Timer/Counter 2 initialization
// Clock source: System Clock
// Clock value: 62,500 kHz
// Mode: Normal top=0xFF
// OC2A output: Disconnected
// OC2B output: Disconnected
//*****************************************************************************
void timer2_init(void)
{
  ASSR=0x00;
  TCCR2A=0x00;
  TCCR2B=0x04;
  TCNT2=0x00;
  OCR2A=0x00;
  OCR2B=0x00;
}


//*****************************************************************************
//*****************************************************************************
//UART0 initialisation
// desired baud rate: 19200
// actual: baud rate: (0,2%)
// char size: 8 bit
// parity: Disabled
void uart0_init(void)
{
  UCSR0A=0x00;
  UCSR0B=0x98;
  UCSR0C=0x06;
  UBRR0H=0x00;
  UBRR0L=0x0C; //19 for 9600 baud
}


//*****************************************************************************
//*****************************************************************************
//ADC initialisation
// Conversion time: 6uS
// Reference voltage = AREF
// 8 bit result in ADCH
// Lowest sampling freq.
void adc_init(void)
{
  ADCSRA = 0x00;            //disable adc
  DIDR0 = 0x00;
  ADMUX = 0x60;             //Cap on aref. AVCC source. ADLAR=1(left adjust)
  ACSR  = 0x80;             //Analog Comparator disable
  ADCSRA = 0b10000111;      //Prescale clock/128 No ints. Enable ADC
  ADCSRB = 0;               //Start ADC
}


//*****************************************************************************
// EEprom strcat function
//*****************************************************************************
void strcpye(char eeprom *str, char *estr) 
{ 
  while (*estr)         //Copy while string[] is not zero
    *str++ = *estr++;
  *str = 0;             //Zero terminate string.
}


//*****************************************************************************
//*****************************************************************************
void reseteeprom(void)
{
  char a;
  
  Ekode1 = '1';
  Ekode2 = '2';
  Ekode3 = '3';
  Ekode4 = '4';
  ERXadr = 0;    
  EFlag = 0;  
  
  a = 0;
  while (a < 20)
  {
    EID[a] = '0';
    a++;
  }   

  a = 0;
  while (a < 9)
  {
    WDR();
    Ereg[a].adr = 0;
    Ereg[a].subadr = 0;
    Ereg[a].type = 0;
    Ereg[a].numreg = 0;
    Ereg[a].outputs = 0;
    Ereg[a].inputs = 0;
    Ereg[a].model = 0;
    Ereg[a].majversion = 0;
    Ereg[a].minversion = 0;
    Ereg[a].battery = 0;
    Ereg[a].siren = 0;
    Ereg[a].name[0] = 0;
    Ereg[a].modelname[0] = 0;
    Ereg[a].intime = 0;
    Ereg[a].pancont = 0;
    Ereg[a].temp[0] = 0;
    Ereg[a].temp[1] = 0;
    Ereg[a].temp[2] = 0;
    Ereg[a].registerred = 0;
    a++;
  }     
}

//*****************************************************************************
// SPI initialization
// SPI Type: Master
// SPI Clock Rate: 62,500 kHz
// SPI Clock Phase: Cycle Start
// SPI Clock Polarity: Low
// SPI Data Order: MSB First
//*****************************************************************************
void SPI_init(void)
{
	SPCR=0x53;  //52 = 62,5kHz 
	SPSR=0x00;
}

//*****************************************************************************
//*****************************************************************************
//call this routine to initialise all peripherals
void init_devices(void)
{
  //stop errant interrupts until set up
  CLI(); //disable all interrupts
  port_init();
  watchdog_init();
  timer0_init();
  timer1_init();
  timer2_init();
  uart0_init();
  adc_init();
  SPI_init();                                            
  
  DIDR0 = 0b00000001; //Disable digital input on port PC0
  EICRA=0x00;
  EIMSK=0x00;
  PCICR=0x00;
  // Timer/Counter 0 Interrupt(s) initialization
  TIMSK0=0x01;
  // Timer/Counter 1 Interrupt(s) initialization
  TIMSK1=0x01;
  // Timer/Counter 2 Interrupt(s) initialization
  TIMSK2=0x01;
  ACSR=0x80;
  ADCSRB=0x00;
  
  SEI(); //re-enable interrupts
  //all peripherals are now initialised
}
