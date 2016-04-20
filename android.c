#include <mega328p.h>
#include <delay.h>
#include <string.h>
#include <stdio.h>
#include "android.h"
#include "eeprom.h"
#include "temperature.h"
#include "flags.h"
#include "interrupt.h"
#include "alarm.h"
#include "messages.h"
#include "init.h"
#include "sms.h"

//*****************************************************************************
//*****************************************************************************
void Android_Control(char *buf)
{
  unsigned char len;
              
  len = strlen(buf) - 3;

  if (buf[0] == '1')
  {
    if (len != 101)
      return;
    if (buf[99] != 'X')
      return;
    if (buf[100] != 'X')
      return;
    Android_message_1(buf);    
  }
  else if (buf[0] == '2')
    Android_status();
  else if (buf[0] == '3')
    Android_info(buf[1]-48);
}

//*****************************************************************************
//*****************************************************************************
void Android_info(unsigned char nr)
{
  char buf[160];
  char tmp[20];
  unsigned char r;
	volatile RADIO_STATUS_EQUIP smsg;
	volatile RADIO_STATUS_EQUIP *STATUSmsg = &smsg;
  
  sprintf(buf,"AND3%d",nr);
  sprintf(tmp,"%02X",Ereg[nr].registerred);
  strcat(buf,tmp);  
  sprintf(tmp,"%04X%04X",Ereg[nr].model>>16,Ereg[nr].model);
  strcat(buf,tmp);  
  sprintf(tmp,"%02X",Ereg[nr].type);
  strcat(buf,tmp);  
  sprintf(tmp,"%02X",Ereg[nr].outputs);
  strcat(buf,tmp);  
  sprintf(tmp,"%02X",Ereg[nr].inputs);
  strcat(buf,tmp);  
  sprintf(tmp,"%02X",Ereg[nr].majversion);
  strcat(buf,tmp);  
  sprintf(tmp,"%02X",Ereg[nr].minversion);
  strcat(buf,tmp);  
  sprintf(tmp,"%02X",Ereg[nr].battery);
  strcat(buf,tmp);  
  sprintf(tmp,"%02X",Ereg[nr].siren);
  strcat(buf,tmp);  
  sprintf(tmp,"#%s#",Ereg[nr].name);
  strcat(buf,tmp);  
  sprintf(tmp,"#%s#",Ereg[nr].modelname);
  strcat(buf,tmp);  

  if (Ereg[nr].registerred != 0)
  {
    if ((Ereg[nr].type != RTYPE_TAST) && (Ereg[nr].type != RTYPE_DETECTOR))
    {
      STATUSmsg->command = RTX_COM_STATUS1;                  //Get status
      r = Radio_Get_Status1(nr, (char *)STATUSmsg);          //Wait for status.
      if (r != 0)                                           
      {
        sprintf(tmp, "0");    //Error getting status
        strcat(buf,tmp);  
      }
      else
      {
        sprintf(tmp, "1");
        strcat(buf,tmp);  
        sprintf(tmp,"%02X",STATUSmsg->status1);
        strcat(buf,tmp);  
        sprintf(tmp,"%02X",STATUSmsg->status2);
        strcat(buf,tmp);  
        sprintf(tmp,"%02X",STATUSmsg->status3);
        strcat(buf,tmp);  
        sprintf(tmp,"%02X",STATUSmsg->inptype1);
        strcat(buf,tmp);  
        sprintf(tmp,"%02X",STATUSmsg->inpstat1);
        strcat(buf,tmp);  
        sprintf(tmp,"%02X",STATUSmsg->inptemp1);
        strcat(buf,tmp);  
        sprintf(tmp,"%02X",STATUSmsg->inpsurvH1);
        strcat(buf,tmp);  
        sprintf(tmp,"%02X",STATUSmsg->inpsurvL1);
        strcat(buf,tmp);  
        sprintf(tmp,"%02X",STATUSmsg->inptermH1);
        strcat(buf,tmp);  
        sprintf(tmp,"%02X",STATUSmsg->inptermL1);
        strcat(buf,tmp);  
        sprintf(tmp,"%02X",STATUSmsg->inptype2);
        strcat(buf,tmp);  
        sprintf(tmp,"%02X",STATUSmsg->inpstat2);
        strcat(buf,tmp);  
        sprintf(tmp,"%02X",STATUSmsg->inptemp2);
        strcat(buf,tmp);  
        sprintf(tmp,"%02X",STATUSmsg->inpsurvH2);
        strcat(buf,tmp);  
        sprintf(tmp,"%02X",STATUSmsg->inpsurvL2);
        strcat(buf,tmp);  
        sprintf(tmp,"%02X",STATUSmsg->statusnr);
        strcat(buf,tmp);  
        sprintf(tmp,"%02X",STATUSmsg->dimval);
        strcat(buf,tmp);  
      }
    }
  }      

  SendSMS(buf);  
}

//*****************************************************************************
//*****************************************************************************
void Android_status(void)
{
  char buf[160];
  unsigned char cnt;
   
  buf[0] = 'A';
  buf[1] = 'N';
  buf[2] = 'D';
  buf[3] = '2';
  
//Relay 1
  if (CheckFlag(FERel1))
    buf[1] = '1';
  else      
    buf[1] = '0';

//Alarm
  if (CheckFlag(FEdisall))            //Alarm aktiv?
    buf[2] = '2';                    //All disabled
  else
  {
    if (CheckFlag(FEAlarm))            //Alarm aktiv?
      buf[2] = '1';                    //Aktiv
    else
      buf[2] = '0';                    //Deaktiv
  }

//Number 1
  cnt = 0;
  while (cnt < 16)
  {       
    if (EGSMnummer1[cnt] != 0)
      buf[3+cnt] = EGSMnummer1[cnt];
    else
      buf[3+cnt] = '-';
    cnt++;
  }                                
  
//Number 2
  cnt = 0;
  while (cnt < 16)
  {       
    if (EGSMnummer2[cnt] != 0)
      buf[19+cnt] = EGSMnummer2[cnt];
    else
      buf[19+cnt] = '-';
    cnt++;
  }                                

  if (CheckFlag(FENUM2))
  	buf[35] = '1';
  else
  	buf[35] = '0';

  buf[36] = 0;

  SendSMS(buf);  
}


//*****************************************************************************
//*****************************************************************************
void Android_message_1(char *buf)
{
	char d=0;

// "A" Alarm innbrudd
  if (buf[1] == '1')
    alarm_AktAlarm();                              //Aktiver alarm
  if (buf[1] == '0')
    alarm_DeAktAlarm();                            //DeAktiver alarm
  if (buf[1] == '2')
    SetFlag(FEdisall);                            //DeAktiver alle alarmer

// "W" Relestyring intern rele
//  if ((buf[30] == '1'))
//     Releon();
//  if ((buf[30] == '0'))
//    Releoff();

// "C" Code
  if (buf[33] != 'X')
  {
    Ekode1 = buf[34];
    Ekode2 = buf[35];
    Ekode3 = buf[36];
    Ekode4 = buf[37];
  }

// "N" Number 1
  if (buf[37] != 'X')
  {
    d = 0;
    while ((buf[37+d] != '#') && (d < 16))
    {
      EGSMnummer1[d] = buf[37+d];
      d++;
    }
    EGSMnummer1[d]=0;
  }

// "N" Number 2
  if (buf[53] != 'X')
  {
    d = 0;
    while ((buf[53+d] != '#') && (d < 16))
    {
      EGSMnummer2[d] = buf[53+d];
      d++;
    }
    EGSMnummer2[d]=0;
  }

// "O" Number 2 de/activate
  if (buf[69] == '0')
    ClearFlag(FENUM2);
  else if (buf[69] == '1')
    SetFlag(FENUM2);
          
// "S" Status pos 0
   if (buf[70] == '0')
     Status();
  
}