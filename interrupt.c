#include <mega328p.h>
#include "sms.h"
#include "flags.h"
#include "init.h"
#include "gprs.h"
#include "interrupt.h"

volatile unsigned char         Rtim[8];         //Teller for timer 0 interrupt
volatile unsigned char         RtimS[8];        //Teller for timer 0 interrupt (antall sek)
volatile unsigned long int     R_int_tim2 = 0;  //Teller for timer 2 interrupt (random verdi)

volatile unsigned char         Rrxtell=0;        //Teller for RX int.
volatile char                  GPRSbuf[NGPRSBUF][65];	 				//5 GPRS buffer on receive 65 byte each. The 65[64] byte indicates buffer in use. 0=free 0xFF=in use
volatile unsigned char         Gnum = 0;								//Point to next GPRS buffer. RXint

//*****************************************************************************
// Return random 32 bit val
//*****************************************************************************
unsigned long int int_random(void)
{
  return (R_int_tim2);
}

//*****************************************************************************
// Start timer 0 med x sek timeout
// Timout flagg settes ved timeout
//*****************************************************************************
void start0(char time, char flag)
{
  CLI();
  if (flag == FRTim0)
  {
    RtimS[0] = time;
    Rtim[0] = 0;
    ClearFlag(FRTim0);
  }
  if (flag == FRTim1)
  {
    RtimS[1] = time;
    Rtim[1] = 0;
    ClearFlag(FRTim1);
  }
  if (flag == FRTim2)
  {
    RtimS[2] = time;
    Rtim[2] = 0;
    ClearFlag(FRTim2);
  }
  if (flag == FRTim3)
  {
    RtimS[3] = time;
    Rtim[3] = 0;
    ClearFlag(FRTim3);
  }
  if (flag == FRTim4)
  {
    RtimS[4] = time;
    Rtim[4] = 0;
    ClearFlag(FRTim4);
  }
  if (flag == FRTim5)
  {
    RtimS[5] = time;
    Rtim[5] = 0;
    ClearFlag(FRTim5);
  }
  if (flag == FRTim6)
  {
    RtimS[6] = time;
    Rtim[6] = 0;
    ClearFlag(FRTim6);
  }
  if (flag == FRTim7)
  {
    RtimS[7] = time;
    Rtim[7] = 0;
    ClearFlag(FRTim7);
  }
	TCCR0B=5;				//Start counter prescale 1024	
  SEI();
}

//*****************************************************************************
// Timer 0 overflow interrupt service routine
//*****************************************************************************
interrupt [TIM0_OVF] void timer0_ovf_isr(void)
{
  char a = 0;
  
  while (a < 8)
  {
  	if ((Rtim[a] != 0) || (RtimS[a] != 0))
    {
    	WDR();				 		//Watchdog reset
    	Rtim[a]++;
  	  if (Rtim[a] == 15)	//1024*255*15 = 1 sek
    	{
    		Rtim[a] = 0;
  	  	RtimS[a]--;
  	  }
    	if (RtimS[a] == 0)
      {
        if (a == 0)
          SetFlag(FRTim0);  //Set Overflow
        if (a == 1)
          SetFlag(FRTim1);  //Set Overflow
        if (a == 2)
          SetFlag(FRTim2);  //Set Overflow
        if (a == 3)
          SetFlag(FRTim3);  //Set Overflow
        if (a == 4)
          SetFlag(FRTim4);  //Set Overflow
        if (a == 5)
          SetFlag(FRTim5);  //Set Overflow
        if (a == 6)
          SetFlag(FRTim6);  //Set Overflow
        if (a == 7)
          SetFlag(FRTim7);  //Set Overflow 
      }
  	}
    a++;
  }
}  

//*****************************************************************************
// Timer 1 overflow interrupt service routine
//*****************************************************************************
interrupt [TIM1_OVF] void timer1_ovf_isr(void)
{
 //TIMER1 has overflowed  15 sekunder
 	TCNT1H = 0x1B; //reload counter high value
 	TCNT1L = 0x1F; //reload counter low value
	SetFlag(FRTim1out);
}

//*****************************************************************************
// Timer 2 overflow interrupt service routine
// Regner seg frem til en random verdi.
//*****************************************************************************
interrupt [TIM2_OVF] void timer2_ovf_isr(void)
{
	R_int_tim2++;
  WDR();
}


//*****************************************************************************
// USART Receiver interrupt service routine
//*****************************************************************************
interrupt [USART_RXC] void usart_rx_isr(void)
{
	char rxb, a = 0;

	rxb = UDR0;													//Hent byte i UART

//***** Sjekk om det er overflow
  if (CheckFlag(FROverFlow))          //Overflow i byter?
  {                                   //Ja. Sjekk om vi kan synke inn på LF.
    if (rxb == LF)
    {
      ClearFlag(FROverFlow);
      Rrxtell = 0;
      return;
    }    
  }
//***** Ingen buffer tidligere ledig. Sjekk nå.
	else if (Rrxtell == 64)  					 		 	//Ingen ledig buffer?
  {
    a = 0;
    while (GPRSbuf[Gnum][64] != 0)    //Next buffer
    {
      a++;
      Gnum++;
      if (Gnum == NGPRSBUF)
        Gnum = 0;
      if (a == NGPRSBUF)              //Ingen ledig buffer
        return;
    }                                                              
		Rrxtell=0;												//Sett teller til 0 igjen.
  }
//***** Lagre char
  else if ((rxb != CR) && (rxb != LF))			//Aldri lagre CR eller LF
	{			 							 								//LF indikerer slutt på transmisjon etc.
    GPRSbuf[Gnum][Rrxtell] = rxb;     //Get data and store
		Rrxtell++;
    if (Rrxtell == 64)
    {
      SetFlag(FROverFlow);            //Set Overflow 
      return;
    }
 	}

//***** Fullfør buffer. 
  if (rxb == LF)			                //Lagre buffer og gjør klar ny buffer.
  {
    GPRSbuf[Gnum][Rrxtell] = 0;       //0 terminer buffer
    GPRSbuf[Gnum][64] = 0xFF;         //Mark buffer in use.
		Rrxtell=64;												//Indiker ingen buffer ledig før sjekk nedenfor
    a = 0;
    while (GPRSbuf[Gnum][64] != 0)    //Next buffer
    {
      a++;
      Gnum++;
      if (Gnum == NGPRSBUF)
        Gnum = 0;
      if (a == NGPRSBUF)              //Ingen ledig buffer
        return;
    }
		Rrxtell=0;												//Sett teller til 0 igjen.
  }
}

