#include <mega328p.h>
#include <delay.h>
#include "init.h"

//*****************************************************************************
//** Sample aktuell inngang 50 ganger og returner gjennomsnitt.
//*****************************************************************************
int SampleRandom(void)
{
  char a=0, result;
  int res=0;

  while (a < 15)
  {  
    WDR();
    ADMUX = 0x60;
    ADMUX |= 0;                //Sett innport (Andre bit uforandret)  NTC
    ADCSRA |= BIT(char,6);         //ADSC = bit 6
    while (ADCSRA & BIT(char,6));  //Vent til ferdig konvertert
    result = ADCL;
    res = res << 1;
    if (result & 0b01000000)
      res = res + 1;
		a++;
	}                 
	return (res);	
}

//*****************************************************************************
//** Sample aktuell inngang 50 ganger og returner gjennomsnitt.
//*****************************************************************************
char Sample(char input)
{
  char a=0, result;
  int res=0;

  while (a < 10)
  {  
    WDR();
    ADMUX = 0x60;
    ADMUX |= input;                //Sett innport (Andre bit uforandret)
    ADCSRA |= BIT(char,6);         //ADSC = bit 6
    while (ADCSRA & BIT(char,6));  //Vent til ferdig konvertert
    result = ADCH;
    res = res + result;
		a++;
	}                 
	result = res / 10;
	return (result);	
}

