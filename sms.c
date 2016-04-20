#include <mega328p.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "eeprom.h"
#include <delay.h>
#include "sms.h"
#include "flags.h"
#include "interrupt.h"
#include "init.h"
#include "temperature.h"
#include "alarm.h"
#include "gprs.h"



//*****************************************************************************
// Send byte to uart
//*****************************************************************************
void gsm_serial_send(char a)
{
	WDR();
	while ((UCSR0A & (1<<UDRE0)) == 0);			//Wait for uart ready
	UDR0=a;
}

//*****************************************************************************
// Send string to Uart
// Sender string som er lagret i program minne.
//*****************************************************************************
void gsm_sendtx(char flash *a)
{
	unsigned int i=0;
	WDR();
	while (a[i] != 0)
	{
		while ((UCSR0A & (1<<UDRE0)) == 0);			//Wait for uart ready
		UDR0=a[i];
		i++;
		WDR();
	}
}

//*****************************************************************************
// Send string to Uart
// Sender string som er lagret i program minne.
//*****************************************************************************
void sendtxR(char *a)
{
	unsigned int i=0;
	WDR();
	while (a[i] != 0)
	{
		while ((UCSR0A & (1<<UDRE0)) == 0);			//Wait for uart ready
		UDR0=a[i];
		i++;
		WDR();
		if (i == 255) //In case of hang.
		  return;
	}
}


//*****************************************************************************
// Send string to Uart
// Sender string som er lagret i program minne.
//*****************************************************************************
void sendtxE(char eeprom *a)
{
	unsigned int i=0;
	WDR();
	while (a[i] != 0)
	{
		while ((UCSR0A & (1<<UDRE0)) == 0);			//Wait for uart ready
		UDR0=a[i];
		i++;
		WDR();
		if (i == 255) //In case of hang.
		  return;
	}
}

