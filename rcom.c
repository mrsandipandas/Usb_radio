#include <mega328p.h>
#include <delay.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "eeprom.h"
#include "radio.h"
#include "init.h"
#include "flags.h"
#include "rcom.h"
#include "interrupt.h"
#include "init.h"
#include "alarm.h"
#include "gprs.h"
#include "sms.h"

unsigned char  temp_info[9];      //temperature from wtemp
unsigned char  WCNT[9];           //Indication of new temp

//*****************************************************************************
//*****************************************************************************
char Radio_Control_Termval(unsigned char nr, unsigned char a, unsigned char b, unsigned char c, unsigned char d, unsigned char e, unsigned char f)
{
  char ret;
  char buf[10];
  
  if (nr > 8)
    return(1);

  if (a != '+')
    if (a != '-')
      return(1);
  
  if (d != '+')
    if (d != '-')
      return(1);

  buf[0] = RTX_COM_ECOCOMF;
  buf[1] = RSUB_THERMOSTAT_LOWHIGH;
  buf[2] = a;
  buf[3] = b;
  buf[4] = c;
  buf[5] = d;
  buf[6] = e;
  buf[7] = f;
  ret = Radio_Control_Thermostat(nr, (char *)buf);        //Send command to radio.
  if (ret != 0)
    return(1);

  delay_ms(100);
  return(0);
}

//*****************************************************************************
//*****************************************************************************
char Radio_Control_Termonoff(unsigned char nr, unsigned char onoff)
{
  char ret;
  char buf[5];
  
  if (nr > 8)
    return(1);
    
  buf[0] = RTX_COM_ECOCOMF;
  if (onoff == 0)
    buf[1] = RSUB_THERMOSTAT_OFF;
  else
    buf[1] = RSUB_THERMOSTAT_ON;

  ret = Radio_Control_Thermostat(nr, (char *)buf);				//Send command to radio.
  if (ret != 0)
    return(1);

  delay_ms(100);
  return(0);
}

//*****************************************************************************
//*****************************************************************************
char Radio_Control_Limval(unsigned char nr, unsigned char a, unsigned char b, unsigned char c, unsigned char d, unsigned char e, unsigned char f)
{
  char ret;
  char buf[10];
  
  if (nr > 8)
    return(1);

	if (a != '+')
  	if (a != '-')
      return(1);
  
	if (d != '+')
  	if (d != '-')
      return(1);

  buf[0] = RTX_COM_TEMPSURV;
  buf[1] = RSUB_LIMITS_LOWHIGH;
  buf[2] = a;
  buf[3] = b;
  buf[4] = c;
  buf[5] = d;
  buf[6] = e;
  buf[7] = f;
  ret = Radio_Control_Limits(nr, (char *)buf);				//Send command to radio.
  if (ret != 0)
    return(1);

  delay_ms(100);
  return(0);
}

//*****************************************************************************
//*****************************************************************************
char Radio_Control_Limonoff(unsigned char nr, unsigned char onoff)
{
  char ret;
  char buf[5];
  
  if (nr > 8)
    return(1);
    
  buf[0] = RTX_COM_TEMPSURV;
  if (onoff == 0)
    buf[1] = RSUB_LIMITS_OFF;
  else
    buf[1] = RSUB_LIMITS_ON;

  ret = Radio_Control_Limits(nr, (char *)buf);				//Send command to radio.
  if (ret != 0)
    return(1);

  delay_ms(100);
  return(0);
}

//*****************************************************************************
//*****************************************************************************
char Radio_Control_Onoff(unsigned char nr, unsigned char onoff)
{
  char ret;
  char buf[5];
  
  if (nr > 8)
    return (1);

  buf[0] = RTX_COM_OUTCHG;
  if (onoff == 0)
    buf[1] = RSUB_OUTPUT_OFF;
  else
    buf[1] = RSUB_OUTPUT_ON;

  ret = Radio_Control(nr, (char *)buf);				//Send command to radio.
  if (ret != 0)
    return(1);
  
  delay_ms(100);
  return(0);
}

//*****************************************************************************
//** Get status from node
//** Param: nr = 0-8
//** Returns status in data
//** Returns OK if transmission was ok. Else Error.
//*****************************************************************************
char Radio_Get_Status1(unsigned char nr, volatile char *data)
{
  unsigned int cnt = 0;
  unsigned char a = 0;

  if (nr > 8)
    return (1);
  
  if (Ereg[nr].registerred == 0)
    return (1);
  
  	//** Is this a temperature reader?
	if ((Ereg[nr].type == RTYPE_TEMP) || (Ereg[nr].type == RTYPE_OUTTEMP))
	{
		while (a < 32)
		{
			data[a] = 0;
			a++;
		}
		data[0] = RRX_COM_STATUS1;
    data[1] = 0;                    //Subadr
		data[5] = RALARMTYPE_TEMP;
		if (Ereg[nr].temp[2] == 1)
		  data[6] = 1;                  //Surv active
		data[7] = temp_info[nr];
		data[8] = Ereg[nr].temp[0];
		data[9] = Ereg[nr].temp[1];
		data[28] = Ereg[nr].adr>>24;
		data[29] = Ereg[nr].adr>>16;
		data[30] = Ereg[nr].adr>>8;
		data[31] = Ereg[nr].adr;
		return (0);
	}

  //** This is normal equ.
  data[1] = Ereg[nr].subadr;        //If more than 1 output in equipment.
  TXradio(Ereg[nr].adr, (char *)data);

  while (1)
  {
    delay_ms(10);
    cnt++;

    //**Delay max 3 sec for receive
		if (cnt >= 300) 								//Timeout return error
    {
			return (1);
    }

		else if (RXradio((char *)data) == RADIO_RXOK)
		{
			if (data[0] == RRX_COM_STATUS1)
      {
        delay_ms(100);
        return (0);
      }
		}
	}
}

//*****************************************************************************
//** Turn thermostat on and off. Set limits.
//** nr = 0-8  Address of receiver.
//** data[0]:  RTX_COM_ECOCOMF command for thermostat control
//** data[1]:  0 = Deactivate
//**           1 = Activate
//**          2 = Limits Lo          in data[2-4] ASCII ("+20")
//**          3 = Limits Hi           in data[2-4] ASCII
//**          4 = Limits Lo and Hi   in data[2-7] ASCII
//** Returns OK if transmission was ok. Else Error.
//*****************************************************************************
char Radio_Control_Thermostat(unsigned char nr, volatile char *data)
{
  RADIO_SEND_MSG smsg;
  RADIO_SEND_MSG *TXmsg = &smsg;
  RADIO_RECEIVED_MSG rmsg;
  RADIO_RECEIVED_MSG *RXmsg = &rmsg;
  unsigned int cnt=0;

  if (nr > 8)
    return (1);

  TXmsg->command = data[0];                    //Economy Comfort command
  TXmsg->sub = Ereg[nr].subadr;          //If more than 1 output in equipment.
  TXmsg->pad[0] = data[1];                    //Activate/deactivate/limits command
  TXmsg->pad[1] = data[2];                    //Payload if applicable
  TXmsg->pad[2] = data[3];
  TXmsg->pad[3] = data[4];
  TXmsg->pad[4] = data[5];
  TXmsg->pad[5] = data[6];
  TXmsg->pad[6] = data[7];
  TXradio(Ereg[nr].adr, (char *)TXmsg);

  while (1)
  {
    delay_ms(10);
    cnt++;

    //**Delay max 3 sec for receive
    if (cnt >= 300)                 //Timeout return error
      return (1);

    else if (RXradio((char *)RXmsg) == RADIO_RXOK)
      return (0);
  }
}

//*****************************************************************************
//** Turn limits check on and off. Set limits.
//** nr = 0-8  Address of receiver.
//** data[0]:  RTX_COM_TEMPSURV command for limits control
//** data[1]:  0 = Deactivate
//**           1 = Activate
//**          2 = Limits Lo          in data[2-4] ASCII ("+20")
//**          3 = Limits Hi           in data[2-4] ASCII
//**          4 = Limits Lo and Hi   in data[2-7] ASCII
//** Returns OK if transmission was ok. Else Error.
//*****************************************************************************
char Radio_Control_Limits(unsigned char nr, volatile char *data)
{
  RADIO_SEND_MSG smsg;
  RADIO_SEND_MSG *TXmsg = &smsg;
  RADIO_RECEIVED_MSG rmsg;
  RADIO_RECEIVED_MSG *RXmsg = &rmsg;
  unsigned int cnt=0;
  unsigned char temp;
  
  if (nr > 8)
    return (1);

	//** Is this a temperature reader?
  if ((Ereg[nr].type == RTYPE_TEMP) || (Ereg[nr].type == RTYPE_OUTTEMP))
  {
    if (data[1] == RSUB_LIMITS_OFF)
      Ereg[nr].temp[2] = 0;   //On/off

    else if (data[1] == RSUB_LIMITS_ON)
      Ereg[nr].temp[2] = 1;   //On/off

    else if (data[1] == RSUB_THERMOSTAT_LOW)    //Lo
    {
      temp = (data[3]-48)*10; //Lo
      temp = temp + (data[4]-48);
      if (data[2] == '-')
        temp = -temp + 100;
      else
        temp = temp + 100;
      Ereg[nr].temp[1] = temp;
    }

    else if (data[1] == RSUB_LIMITS_HIGH)    //Hi
    {
      temp = (data[3]-48)*10;
      temp = temp + (data[4]-48);
      if (data[2] == '-')
        temp = -temp + 100;
      else
        temp = temp + 100;
      Ereg[nr].temp[0] = temp;
    }

    else if (data[1] == RSUB_THERMOSTAT_LOWHIGH)    //Lo-hi
    {
      temp = (data[3]-48)*10;
      temp = temp + (data[4]-48);
      if (data[2] == '-')
        temp = -temp + 100;
      else
        temp = temp + 100;
      Ereg[nr].temp[1] = temp;  //Lo
      temp = (data[6]-48)*10;
      temp = temp + (data[7]-48);
      if (data[5] == '-')
        temp = -temp + 100;
      else
        temp = temp + 100;
      Ereg[nr].temp[0] = temp;  //Hi
    }
  }
  
  TXmsg->command = data[0];                    //Limits command
  TXmsg->sub = Ereg[nr].subadr;          //If more than 1 output in equipment.
  TXmsg->pad[0] = data[1];                    //Activate/deactivate/limits command
  TXmsg->pad[1] = data[2];                    //Payload if applicable
  TXmsg->pad[2] = data[3];
  TXmsg->pad[3] = data[4];
  TXmsg->pad[4] = data[5];
  TXmsg->pad[5] = data[6];
  TXmsg->pad[6] = data[7];
  
   
  TXradio(Ereg[nr].adr, (char *)TXmsg);

  while (1)
	{
		delay_ms(10);
		cnt++;

		//**Delay max 3 sec for receive
		if (cnt >= 300) 								//Timeout return error
    {
			return (1);
    }
		else if (RXradio((char *)RXmsg) == RADIO_RXOK)
    {
			return (0);
    }
	}
}

//*****************************************************************************
//** Turn relay on or off. Economy or Comfort.
//** Param: nr = 0-8
//** data[0]:  RTX_COM_OUTCHG command for output control
//** data[1]:  0 = OFF/ECO
//**           1 = ON/COMFORT
//**          2 = TOGGLE
//**          3 = Values
//** Returns OK if transmission was ok. Else Error.
//** Also returns if relay was controlled on or off/comf eco in data[32]
//*****************************************************************************
char Radio_Control(unsigned char nr, volatile char *data)
{
  RADIO_SEND_MSG smsg;
  RADIO_SEND_MSG *TXmsg = &smsg;
  RADIO_RECEIVED_MSG rmsg;
  RADIO_RECEIVED_MSG *RXmsg = &rmsg;
  unsigned int cnt=0;
  
  if (nr > 8)
    return (2);
    
  TXmsg->command = data[0];                      //Economy Comfort command
  TXmsg->sub = Ereg[nr].subadr;                  //If more than 1 output in equipment.
  TXmsg->pad[0] = data[1];                      //Activate/deactivate/toggle command
  TXmsg->pad[1] = data[2];                      //Other values. T.ex IVT
  TXradio(Ereg[nr].adr, (char *)TXmsg);

  while (1)
  {
    delay_ms(10);
    cnt++;

    //**Delay max 3 sec for receive
    if (cnt >= 300) 								//Timeout return error
			return (1);

		else if (RXradio((char *)RXmsg) == 1)
		{
      if (RXmsg->command == RRX_COM_OK)
      {
        data[32] = RXmsg->pad[0];
        return (0);
      }
		}
	}
}

//*****************************************************************************
//** Initier sentral.
//** Hvis sentral addresse ikke eksisterer, assign en.
//*****************************************************************************
void Sentralinit(void)
{
  if (ERXadr == 0x00000000)                 //Does the central have an adress?
  {                                         //No
    ERXadr = int_random() + 0x10000000 + rand();     //Get random adress.
  }
  W_RX_ADDRESS(ERXadr);                     //Write new adress to central
}

//*****************************************************************************
//*****************************************************************************
unsigned long int endian_swap(unsigned long int x)
{
  unsigned long int res;
  res = (x>>24) | ((x<<8) & 0x00FF0000) | ((x>>8) & 0x0000FF00) | (x<<24);
  return(res);
}


//*****************************************************************************
//** Innmelding av noder og tastatur
//** Retur 0 hvis ok.
//** Retur 1 hvis ventet 1 minutt uten ny registrering
//** Fjernet: Retur 2 Hvis det ikke er noen registrerings request, men annen com.
//** Retur 3 Timeout ping retur
//** Retur 4 software versjon er feil
//** Retur 5 ikke ledig plass
//** Retur 6 Ping svar ikke mottatt.
//** Retur 7 Abort.
//*****************************************************************************
char NewIdent(void)
{
  unsigned long int newadr;
	volatile RADIO_SEND_MSG smsg;
	RADIO_SEND_MSG *TXmsg = &smsg;
	volatile RADIO_REG_EQUIP rmsg;
	volatile RADIO_REG_EQUIP rmsgp;
	RADIO_REG_EQUIP *RXmsg = &rmsg;
	RADIO_REG_EQUIP *RXequip = &rmsgp;
  unsigned char a,b,cc, newid;

// Clean TX registers.
  TXmsg->command = 0;   
  TXmsg->fromadr = 0;
  TXmsg->sub = 0;
  a = 0;
  while (a < 26)
  {
    TXmsg->pad[a] = 0;
    a++;
  }

// Get new random adress
  newadr = int_random();                   //Random value. Ny addresse for node

  if (newadr == 0)                         //Aldri gi addresse 0 og 1.
    newadr = 0x65AC6543;
  if (newadr == 1)
    newadr = 0x62AC6744;

// Wait for receive
  Radio_init();                            //Resett radio

  W_RX_ADDRESS(0x00000001);                //Sentral addresse 0x00000001 for innmelding

  start0(60,FRTim0);                       //1 minutt til å sette inn node
  a = 0;

  while (a == 0)                           //Sjekk mottak
  {
    if(CheckFlag(FRTim0))                   //Timeout skjedd?
    {
      Radio_init();                        //Sett min originale adresse i Radio
      return(1);                           //Retur feil
    }
    if (RXradio((char *)RXequip) != 0)
    {
    	if (RXequip->command == RRX_COM_REGISTER) // Is this a register request?
        a = 1;
    }
  }

  
// Send answer
 	TXmsg->command = RTX_COM_REGANS;
	TXmsg->pad[0] = (unsigned long int)newadr >> 24 & 0xFF;		//NB: byte[1] is subadr... So this is byte[2].
	TXmsg->pad[1] = (unsigned long int)newadr >> 16 & 0xFF;
	TXmsg->pad[2] = (unsigned long int)newadr >> 8 & 0xFF;
	TXmsg->pad[3] = (unsigned long int)newadr & 0xFF;
	TXmsg->fromadr = endian_swap(ERXadr);
  delay_ms(200);
	TXradio(0x00000000, (char *)TXmsg);


// Wait for ping reception
//  W_RX_ADDRESS(ERXadr);		//Sett min adresse
  Radio_init();                        //Sett min originale adresse i Radio
  start0(5,FRTim0);
  while (RXradio((char *)RXmsg) == 0)                    //Det er mottak!
  {
   if(CheckFlag(FRTim0))                    //Timeout skjedd?
    {
      Radio_init();                        //Sett min originale adresse i Radio
      return(3);                          //Retur feil
    }
  }

//Check PING and software. If ok, store information
  if (RXmsg->command == RRX_COM_PINGREG)	//PING ok. Store registration.
  {
    if (RXequip->softreq > 0x16)					//Check software requirements
      return(4);									//Radio Equip requires higher ver of software in ECO starter.
    a = 0;
    b = 0;
    while (a < 9)															//Only 9 positions
    {
      if (Ereg[a].registerred == 0)			//Free position?
      {
        Ereg[a].adr = (UNS_32)newadr;
        Ereg[a].subadr = b;
        Ereg[a].type = RXequip->type;
        Ereg[a].numreg = RXequip->numreg;
        Ereg[a].outputs = RXequip->outputs;
        Ereg[a].inputs = RXequip->inputs;
        Ereg[a].model = RXequip->model;
        Ereg[a].majversion = RXequip->majversion;
        Ereg[a].minversion = RXequip->minversion;
        Ereg[a].battery = RXequip->battery;
        Ereg[a].siren = RXequip->siren;
        Ereg[a].name[0] = 0;
        Ereg[a].intime = 0;
        Ereg[a].temp[0] = 130;    //30C High
        Ereg[a].temp[1] = 100;    //0C  Low
        Ereg[a].temp[2] = 0;      //Off
        Ereg[a].registerred = 0x01;
        Ereg[a].pancont = 0xFF;	  //No control for panel yet.
        temp_info[a] = 0xFF;       //Temp is set to X if WTEMP
        newid = a;
        cc = 0;
        while (cc < 10)
        {
          Ereg[a].modelname[cc] = RXequip->modelname[cc];
          cc++;
        }  
        gprs_zsend(a);
        b++;
        if (RXequip->numreg == b)			//Make new positions if theres more to be registered.
          a = 8;										//Else end.
      }
      a++;
    }
  	if (b == 0)
	  	return(5);
    newid++;
    return(0);
  }
  else
    return(6);                          //Retur feil
}

//*****************************************************************************
// Sjekker kommandoer fra radio etc.
//*****************************************************************************
void Radio(void)
{
	volatile RADIO_SEND_MSG      smsg;
	RADIO_SEND_MSG      *TXmsg = &smsg;
	volatile RADIO_RECEIVED_MSG  rmsg;
	RADIO_RECEIVED_MSG  *RXmsg = &rmsg;
	volatile RADIO_RECEIVED_MSG  rmsgT;
	RADIO_RECEIVED_MSG  *RXmsgT = &rmsgT;
  unsigned long int   txadr;
  unsigned char d, res, nodeadr;
  char buf[10];
  char *ptrdata;
      
  if (RXradio((char *)RXmsg))                   //Det er mottak!
  {
    //RAW Mode. Just dump received data.
    if (CheckFlag(FRraw))
    {
     	d = 0;
      ptrdata = (char *)RXmsg;
      while (d < 32)
      {
        sprintf(buf,"%02X ",*ptrdata);
        sendtxR(buf);
        ptrdata++;
        d++;
      }
    	gsm_sendtx("\r\n");                                            
      return;
    }

    // Check if node is allowed in system
		nodeadr = 9;
    d = 0;
    while (d < 9)
    {
      if (Ereg[d].registerred != 0)
      {
        if (Ereg[d].adr == endian_swap(RXmsg->fromadr))
        {
          if (Ereg[d].subadr == RXmsg->sub)
            nodeadr = d;
        }
      }         
      d++;
    }  
    if (nodeadr == 9)
    {
      return;
    }
    
    // Clean TX registers.
		TXmsg->command = 0;   
    TXmsg->fromadr = 0;
    TXmsg->sub = 0;
    d = 0;
    while (d < 26)
    {
      TXmsg->pad[d] = 0;
      d++;
    }

    //** Alarmcheck wanted
    //*************************************************************
    if (RXmsg->command == RRX_COM_ALARMCHK)
    {
      txadr = endian_swap(RXmsg->fromadr);
      TXmsg->command = RTX_COM_ALARMSTAT;
      TXmsg->pad[0] = CheckFlag(FEAlarm);
      TXradio(txadr, (char *)TXmsg);
    }

		//** Temp from wireless tempreader
    //*************************************************************
		else if (RXmsg->command == RRX_COM_TEMP)
		{
			WCNT[nodeadr] = 1;													  //No reset of temperature.
			temp_info[nodeadr] = RXmsg->pad[0]; 		  //Store new temperature

			if (Ereg[nodeadr].temp[2] == 1)         //Check limits if active
			{
			  if (temp_info[nodeadr] >= Ereg[nodeadr].temp[0])  //Hi
			  {
			    RXmsg->pad[0] = 1;                            //Input
          RXmsg->pad[1] = RALARMTYPE_TEMPHI;            //Type
          RXmsg->pad[2] = 3;                            //Pri
          RXmsg->pad[3] = Ereg[nodeadr].temp[0];  //Temphi limit
          Ereg[nodeadr].temp[2] = 0;              //Disable
          alarm_Radio((RADIO_ALARM_MSG *) RXmsg, nodeadr);           //Enter alarmmsg into queue
			  }
			  else if (temp_info[nodeadr] < Ereg[nodeadr].temp[1])  //Lo
        {
          RXmsg->pad[0] = 1;                            //Input
          RXmsg->pad[1] = RALARMTYPE_TEMPLO;            //Type
          RXmsg->pad[2] = 3;                            //Pri
          RXmsg->pad[3] = Ereg[nodeadr].temp[1];  //Templo limit
          Ereg[nodeadr].temp[2] = 0;              //Disable
          alarm_Radio((RADIO_ALARM_MSG *) RXmsg, nodeadr);           //Enter alarmmsg into queue
        }
			}
		}


    //** Alarm
    //*************************************************************
    else if (RXmsg->command == RRX_COM_ALARMGSM)
    {
      txadr = endian_swap(RXmsg->fromadr);
      TXmsg->command = RTX_COM_ALARMREC;                      //Alarm is received. Send ack.
      TXradio(txadr, (char *)TXmsg);                          //Transmit
      alarm_Radio((RADIO_ALARM_MSG *) RXmsg, nodeadr);        //Enter alarmmsg into queue
    }

    //** Turn alarm on/off or toggle. Without code.
    //*************************************************************
    else if (RXmsg->command == RRX_COM_ALARMSIMP)
    {
      if (RXmsg->pad[0] == 0)          //Off
        alarm_DeAktAlarm();
      else if (RXmsg->pad[0] == 1)    //On
        alarm_AktAlarm();
      else if (RXmsg->pad[0] == 2)    //Toggle
      {
        if (CheckFlag(FEAlarm) == 1)
          alarm_DeAktAlarm();
        else
          alarm_AktAlarm();
      }
      txadr = endian_swap(RXmsg->fromadr);
      TXmsg->command = RTX_COM_ALARMSTAT;
      TXmsg->pad[0] = CheckFlag(FEAlarm);
      TXradio(txadr, (char *)TXmsg);
    }

    //** Turn alarm on/off or toggle. With code
    //*************************************************************
    else if (RXmsg->command == RRX_COM_ALARMCHG)
    {
      if ((RXmsg->pad[0] == Ekode1) && (RXmsg->pad[1] == Ekode2) &&  (RXmsg->pad[2] == Ekode3) && (RXmsg->pad[3] == Ekode4))
      {
        if (RXmsg->pad[4] == 0)          //Off
          alarm_DeAktAlarm();
        else if (RXmsg->pad[4] == 1)    //On
          alarm_AktAlarm();
        else if (RXmsg->pad[4] == 2)    //Toggle
        {
          if (CheckFlag(FEAlarm) == 1)
            alarm_DeAktAlarm();
          else
            alarm_AktAlarm();
        }
      }
      txadr = endian_swap(RXmsg->fromadr);
      TXmsg->command = RTX_COM_ALARMSTAT;
      TXmsg->pad[0] = CheckFlag(FEAlarm);
      TXradio(txadr, (char *)TXmsg);
    }

    //** Control single device number x on/off or toggle
    //*************************************************************
    else if (RXmsg->command == RRX_COM_ONOFF)
    {
      if (RXmsg->pad[1] > 9)                                                //9 devices max.
        return;
      TXmsg->command = RTX_COM_OUTCHG;                                      //Param for controlling on/off
      TXmsg->sub = Ereg[RXmsg->pad[1]].subadr; 
      TXmsg->pad[0] = RXmsg->pad[0];                                        //Off ON Toggle 0 1 2
    	TXradio(Ereg[RXmsg->pad[1]].adr, (char *)TXmsg);

      start0(3,FRTim0);
      res = 0;
      while ((CheckFlag(FRTim0) == 0) && res == 0)                    //Det er mottak!
      {
        if (RXradio((char *)RXmsgT))
        {
          if (RXmsgT->command == RRX_COM_OK)
            res = 1;
        }
        else if (CheckFlag(FRTim0))                    //Timeout skjedd?
          res = 0;
      }
      
      txadr = endian_swap(RXmsg->fromadr);
      TXmsg->command = RTX_COM_ACK ;                                      //Message ok received. send back answer.
      TXmsg->sub = Ereg[nodeadr].subadr;
      if (res == 1)
      {
        TXmsg->pad[0] = 0;                //Device ACK'ed
        TXmsg->pad[1] = RXmsgT->pad[0];    //Was relay controlled ON/off or ECO/Comf (1=ON/COMF 0=ECO/OFF)
      }
      else
        TXmsg->pad[0] = 1;          //NO ACK from device
      TXradio(txadr, (char *)TXmsg);             //Transmit RTX_COM_ACK back to controlling device.
    }
  }
}


//*****************************************************************************
// Siren Control
// Turn on/off all sirens.
// Arg: 0 = Deactivate 1 = Activate
//*****************************************************************************
void Siren(unsigned char command)
{
	RADIO_SEND_MSG smsg;
	RADIO_SEND_MSG *TXmsg = &smsg;
	unsigned char a = 0;

	TXmsg->command = RTX_COM_SIREN;										//Siren
	TXmsg->pad[0] = command;													//Activate/deactivate

	while (a < 9)
	{
		if ((Ereg[a].registerred != 0) && (Ereg[a].siren == 1))
			TXradio(Ereg[a].adr, (char *)TXmsg);
		a++;
	}
}
