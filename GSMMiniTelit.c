#include <mega328p.h>
#include <delay.h>
#include "eeprom.h"
#include "flags.h"
#include "sms.h"
#include "init.h"
#include "temperature.h"
#include "radio.h"
#include "rcom.h"
#include "gprs.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

unsigned int minutt=0;				//Timechk Skal telle opp til  12 timer = 720 min
unsigned char tim1=0;					//Timechk 60 sek teller
unsigned char tim2=0;					//Timechk 10min sek teller
char Rrnd = '*';

//*****************************************************************************
//Main routine
//*****************************************************************************
void main(void)
{
  
  // Crystal Oscillator division factor: 1
  #pragma optsize-
  CLKPR=0x80;
  CLKPR=0x00;
  #ifdef _OPTIMIZE_SIZE_
  #pragma optsize+
  #endif
  
  
  srand(SampleRandom());    //Get seed for random numbers
  
	init_devices();
	if (MCUSR & BIT(char,3)) 	//Watchdog
		Ewatch++;
	if (MCUSR & BIT(char,2))	//Brownout
		Ebrown++;
	if (MCUSR & BIT(char,1))	//External reset
		Ereset++;
	if (MCUSR & BIT(char,0))	//Power up
		Epower++;
	MCUSR = 0;

  Sentralinit();
  
  // Init Radio
  Radio_init();
  CheckRadio();
  gsm_int_gprs();
  
	while (1)
	{
		WDR();
		Timechk();		//Sjekk om 15 og 60 sekunder telling er over
    Radio();
    gsm_gprs_process();
    if (CheckFlag(FRamcd))
      Check_cdam();
	}
}

//*****************************************************************************
//*****************************************************************************
void Check_cdam(void)
{
  if (CD_chk)
  {
    gsm_sendtx("CD");
    gsm_serial_send(Rrnd);
    gsm_sendtx("\n");
  } 
  
  if (Rrnd == '*')
    Rrnd = '-';              
  else if (Rrnd == '-')
    Rrnd = '|';              
  else if (Rrnd == '|')
    Rrnd = '*';
  else
    Rrnd = '*';              
}

//*****************************************************************************
// 15 og 60 sek og 12 tim timeout
//*****************************************************************************
void Timechk(void)
{
	unsigned char a;
  
  if (CheckFlag(FRTim1out)) //15 sek timeout.
	{
		ClearFlag(FRTim1out);		//Rensk flagg
	 	tim1++;
    tim2++;

    //WTEMP RESET
    if (tim2 == 40)
    {
      tim2 = 0;
      a = 0;
      while (a < 9)
      {
        if (WCNT[a] == 1)										//WTEMP has given new temp.
          WCNT[a] = 0;                      //Reset indicator
        else
          temp_info[a] = 0xFF;		//No temp from WTEMP within 10 mins.
        a++;
      }
    }

		if (tim1 == 4)						//60 sek timeout.
 		{
 			tim1=0;
			minutt++;
      
			if (minutt == 1440)
			{
				Etime++;
        CheckRadio();
				minutt=0;
			}
		}
	}	
}

