#include <mega328p.h>
#include <delay.h>
#include <stdio.h>
#include <string.h>

#include "eeprom.h"
#include "flags.h"
#include "init.h"
#include "rcom.h"
#include "alarm.h"
#include "gprs.h"
//*****************************************************************************
// Aktiver innbrudd Radio
//*****************************************************************************
void alarm_AktAlarm(void)
{
	WDR();
  SetFlag(FEAlarm);         //Activate main alarm
	ClearFlag(FEdisall);			//Alarmer aktive igjen
  Siren(0);
}

//*****************************************************************************
// DeAktiver innbrudd Radio
//*****************************************************************************
void alarm_DeAktAlarm(void)
{
	WDR();
  ClearFlag(FEAlarm);         //Turn off main alarm
	ClearFlag(FEdisall);			  //Alarmer aktive igjen
  Siren(0);
}

//*****************************************************************************
//** Sjekk alarmer fra radio
//** Sendes bare hvis aktiv, og skiller mellom innbrudd og andre alarmer.
//*****************************************************************************
void alarm_Radio(RADIO_ALARM_MSG *myalarm, char nodeadr)
{
  WDR();
 	if (CheckFlag(FEdisall))					  //All messages disabled?
    return;

 	if (CheckFlag(FEAlarm) == 0)        //Innbrudd deaktivert?
  {                                   //Send alle andre alarmer.
    if ((myalarm->type == RALARMTYPE_PIR) || (myalarm->type == RALARMTYPE_MAGNET) || (myalarm->type == RALARMTYPE_GENPIR))
      return;
    else if (myalarm->type == RALARMTYPE_FIRE)
      Siren(1);
    AlarmRadioSMS((RADIO_ALARM_MSG *) myalarm, nodeadr);          //Enter alarmmsg into queue 
  }
  else    
  {                                   //Send alle alarmer.
    if ((myalarm->type == RALARMTYPE_PIR) || (myalarm->type == RALARMTYPE_MAGNET) || (myalarm->type == RALARMTYPE_GENPIR))
    {
        Siren(1);
        AlarmRadioSMS((RADIO_ALARM_MSG *) myalarm, nodeadr);          //Enter alarmmsg into queue
    }
    else if (myalarm->type == RALARMTYPE_FIRE)
    {
      Siren(1);
      AlarmRadioSMS((RADIO_ALARM_MSG *) myalarm, nodeadr);          //Enter alarmmsg into queue
    }
    else
      AlarmRadioSMS((RADIO_ALARM_MSG *) myalarm, nodeadr);          //Enter alarmmsg into queue
  }
}


//*****************************************************************************
// Sender SMS til alle aktive nummer med alarmmelding.   
//*****************************************************************************
void AlarmRadioSMS(RADIO_ALARM_MSG *myalarm, char nr)
{
  char mytext[200];               //Max 160 chars but for safety...
  char tmpbuf[50];
  unsigned char errcode;
  char temp;
      
  sprintf(mytext, "Alarm: Device %d ", nr+1);
  
  switch (myalarm->type)
  {
    case RALARMTYPE_PIR:
      sprintf(tmpbuf,"PIR");
      break;
    case RALARMTYPE_FIRE:
      sprintf(tmpbuf,"FIRE");
      break;
    case RALARMTYPE_WATER:
      sprintf(tmpbuf,"WATER");
      break;
    case RALARMTYPE_MAGNET:
      sprintf(tmpbuf,"MAGNET");
      break;
    case RALARMTYPE_GAS:
      sprintf(tmpbuf,"GAS");
      break;
    case RALARMTYPE_GENERIC:
      sprintf(tmpbuf,"GENERIC");
      break;
    case RALARMTYPE_WARN:
      sprintf(tmpbuf,"OVERTEMP");
      break;
    case RALARMTYPE_TEMPHI:
      sprintf(tmpbuf,"TEMPERATURE HIGH");
      break;
    case RALARMTYPE_TEMPLO:
      sprintf(tmpbuf,"TEMPERATURE LOW");
      break;
    case RALARMTYPE_GENPIR:
      sprintf(tmpbuf,"GENERIC PIR");
      break;
    case RALARMTYPE_TEMP:
      sprintf(tmpbuf,"TEMPERATURE");
      break;
    case RALARMTYPE_BATT:
      sprintf(tmpbuf,"BATTERY");
      break;
    default:
      sprintf(tmpbuf,"UNKNOWN");
      return;
  }
  strcat(mytext,tmpbuf);

	if (myalarm->type == RALARMTYPE_IVTFAIL)
	{
	  errcode = myalarm->pad[0] & 0x1F;
	  sprintf(tmpbuf," Code %d-", errcode);
    strcat (mytext,tmpbuf);
    errcode = myalarm->pad[0] & 0xE0;
    errcode = errcode >> 5;
    sprintf(tmpbuf,"%d ", errcode);
    strcat (mytext,tmpbuf);
	}
	else if (myalarm->pad[0] != 0)
	{
	  if (myalarm->pad[0] < 100)
    { 
      temp = myalarm->pad[0] - 100;
      temp = ~temp + 1;
      sprintf(tmpbuf," -%dc", temp);
    }
    else
      sprintf(tmpbuf," %dc", myalarm->pad[0] - 100);
	  strcat (mytext,tmpbuf);
	}

	strcatf(mytext," ");
  strcate(mytext, Ereg[nr].modelname);
	strcatf(mytext,". ");
  strcate(mytext,Ereg[nr].name);
	strcatf(mytext,".");
  gprs_alarm(mytext);
}

//*****************************************************************************
// EEprom strcat function
//*****************************************************************************
void strcate(char *str, char eeprom *estr) 
{ 
  while (*str) *str++;  //Find end of string
  while (*estr)         //Copy while string[] is not zero
    *str++ = *estr++;
  *str = 0;             //Zero terminate string.
}
 