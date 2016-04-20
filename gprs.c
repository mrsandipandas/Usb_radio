#include <sleep.h>
#include <delay.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "eeprom.h"
#include "init.h"
#include "gprs.h"
#include "sms.h"
//#include "gsm fixi.h"
#include "temperature.h"
#include "flags.h"
#include "interrupt.h"
#include "alarm.h" 
#include "radio.h" 

extern unsigned int minutt;				//Timechk Skal telle opp til  12 timer = 720 min

//GPRS
unsigned char       G2num = 0;								//Point to next GPRS buffer.
unsigned char       GPRSALARMACK = 0;       //Acknowledge if alarm is sent by GPRS. Will be set to 0xFF when ack is received.

char RcliID = 'A';
char RserID = 'A';

//*****************************************************************************
// GPRS init
//*****************************************************************************
int gsm_int_gprs(void)
{
  RcliID = 'A';         //Reset ID.
  RserID = 'A';
  gsm_gprs_clear_buf();
  gprs_send_id();
	return (0);
}

//*****************************************************************************
//** Check for ACK in buffers.
//** Scans and clears all buffers with ACK.
//*****************************************************************************
void gsm_gprs_process_ack(void)
{
  unsigned char e=0;
	unsigned int a = 0;
  
	//** Find used buffer
	a = 0;															  //Find free GPRS buffer for storing data
	while (a < NGPRSBUF)		    	        //Will always use next buffer with turnover to first buffer
	{
  	if (GPRSbuf[a][64] == 0xFF)		    	//Buffer in use. Check if this is ACK.
    {
      if (GPRSbuf[a][0] == 'A' && GPRSbuf[a][1] == 'C' && GPRSbuf[a][2] == 'K') //Alarm Acknowledge
      {
        GPRSALARMACK = 0xFF;
      	//** Clear buffer and mark as free
      	e = 0;
      	while (e < 65)
      	{
      		GPRSbuf[a][e] = 0;
      		e++;
      	}
      }
    } 
 		a++;															//5 buffers scanned?
  }
}  

//*****************************************************************************
//** Clear All GPRS buffers.
//*****************************************************************************
void gsm_gprs_clear_buf(void)
{
	unsigned char a,b;

	// Clear GPRS buffer
	a = 0;
	while (a < NGPRSBUF)						//Clear GPRS buffer
	{
		b = 0;
		while (b < 65)
		{
			GPRSbuf[a][b] = 0;
			b++;
		}
		a++;
	}
}

  
//*****************************************************************************
//** Send ID
//** BAXGMN10:00000000000000000000:356917059000153:16.00.142-B009:242010106339593:89470010090427005671:GL865-QUAD-V3;
//*****************************************************************************
void gprs_send_id(void)
{
	char buf[120];
	char tbuf[10];
	char a = 0;
	
	sprintf(buf,"X");
  strcatf(buf,GPRSVertxt);
  strcatf(buf,":");
  
	while (a < 20)
	{
	  sprintf(tbuf,"%c",EID[a]);
	  strcat(buf,tbuf);
	  a++;
	}
	strcat(buf,";");

	gsm_sem_sendgprs(buf,0);
}


//*****************************************************************************
//*****************************************************************************
//** GPRS driver routines. Semaphore protected.
//** Only for internal use in this driver.
//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
// Send on GPRS
// Max 1024 chars, or GSM will fail.
// Returns: 0 if message is sent, or GPRS is supposed to be offline.
//*****************************************************************************
int gsm_sem_sendgprs(char *gprsbuf, unsigned char resp)
{
	char buf[10];
	
  RcliID++;                                       //Put new client id in the array.
  if (RcliID > 'Z')                              //Data is prebuilt with the rest.
	  RcliID = 'A';
	if (resp == 2)                                 //
    sprintf(buf,"%c%cA",RcliID,RserID);          //Alarm
  else if (resp == 1)                            //Is this response data?
    sprintf(buf,"%c%cR",RcliID,RserID);          //Response
  else                                           //
    sprintf(buf,"%c%c",RcliID,RserID);           //
	sendtxR(buf);                                  //Send text
	sendtxR(gprsbuf);                              //Send text
	gsm_sendtx("\r\n");                                            

	return (0);
}
  
//*****************************************************************************
//** Add response 'R' to all sent text before sending
//*****************************************************************************
int gsm_sem_response_sendgprs(char *buf)
{
	return(gsm_sem_sendgprs(buf,1));	
}
   

//*****************************************************************************
// Send Info melding!
// nr er ASCII kode for 1-9
//*****************************************************************************
void gprs_info(char nr)
{
	char tbuf[100];
	char tmp[30];
  
//Alarm og nummer
  if (nr == '1')
  {
    sprintf(tbuf,"I1:");
    if (CheckFlag(FEdisall))
      sprintf(tmp,"2:");           //All deactivated
    else if (CheckFlag(FEAlarm))
      sprintf(tmp,"1:");           //All activated
    else      
      sprintf(tmp,"0:");           //Burglary deactivated
    strcat(tbuf,tmp);
  }
//ADC Verdier
  else if (nr == '2')
  {
  }
//Log informasjon
  else if (nr == '3')
  {
    sprintf(tbuf,"I3:%04X:%04X:%04X:%04X:%04X:",Epower,Ewatch,Ebrown,Ereset,Etime);
  }
//Div informasjon
  else if (nr == '4')
  {
  }
 	else
	  sprintf(tbuf,"RI%cE",nr);          //Error er en response med R

	gsm_sem_sendgprs(tbuf,0);
}

//*****************************************************************************
// Send info about new registerred equipment.
// Argument 0-8
//*****************************************************************************
void gprs_zsend(unsigned char d)
{
	char buf[120];
	char eeprom *ptrdata;
	char buf2[10];
	unsigned char val;
	
	sprintf(buf,"Z%d",d+1);
	ptrdata = (char eeprom *)&Ereg[d];
	val = 0;
	while (val < 50)
	{
		sprintf(buf2,"%02X",*ptrdata);
		strcat(buf,buf2);
		ptrdata++;
		val++;
	}
	gsm_sem_response_sendgprs(buf);
}
 
//*****************************************************************************
// Send ALARM melding!  
// Return 0 if OK. Or GPRS is deactivated
// Return 1 if ERROR
//*****************************************************************************
char gprs_alarm(char *mdata)
{
	unsigned int i;
  
	gsm_sem_sendgprs(mdata,2);

	//Wait for ACK
	GPRSALARMACK = 0x00;                              //10 secs for ACK or reconnect and resend.
	i = 0;
	while (i < 300)
	{
		delay_ms(100);
		gsm_gprs_process_ack();
		if (GPRSALARMACK != 0)
		  return(0);
		i++;
	}
	gsm_sem_sendgprs(mdata,2);
	return(1);                                        //Two tries failed. Error
}
 
                                                                         
//*****************************************************************************
//** Check new communication on GPRS channel.
//*****************************************************************************
void gsm_gprs_process(void)
{
  unsigned char e=0, d=0, nr=0;
	unsigned int a = 0, c = 0;
	char buf[120];
	char buf2[33];
	char tbuf[10];
	char res;
	char eeprom *ptrdata;
  unsigned int regval = 0;
  unsigned long int adr;
  
	//** Find used buffer
	a = 0;															//Find free GPRS buffer for storing data
	while (GPRSbuf[G2num][64] == 0)			//Will always use next buffer with turnover to first buffer
	{
		a++;															//5 buffers scanned?
		if (a == NGPRSBUF)								//Yes, all empty.
		  return;													//Return
		G2num++;													//Find used buffer.
		if (G2num == NGPRSBUF)
			G2num = 0;
	}
	//******************************************
	//** Process buffer
	//******************************************
  if (GPRSbuf[G2num][0] == 'A' && GPRSbuf[G2num][1] == 'C' && GPRSbuf[G2num][2] == 'K') //Alarm Acknowledge
    c = 3;  //Does nothing. Only to clear ACK from buffers if accidently occuring.
  else if (GPRSbuf[G2num][0] == 'S' && GPRSbuf[G2num][1] == 'k' && GPRSbuf[G2num][2] == 'm')	//MUST MATCH
	{
		RserID = GPRSbuf[G2num][3];
    c = 4;
		while ((GPRSbuf[G2num][c] != 0x00) && (c < 64))		                  //If buffer position is 0x00, then return.
		{                                                                   //Check for multiple Skm in this buffer.
      if ((GPRSbuf[G2num][c] == CR) && (GPRSbuf[G2num][c+1] == LF))     //Break if CR LF ic received. 
      {                                                                 //Next check for new command SkmX
        if (GPRSbuf[G2num][c+2] == 'S' && GPRSbuf[G2num][c+3] == 'k' && GPRSbuf[G2num][c+4] == 'm')	//MUST MATCH
        {  
      		RserID = GPRSbuf[G2num][c+5];                                 //Store new Server ID.
          c = c + 6;
        } 
      }    
      
			//*******************************
			// Relestyring
			//*******************************
			if (GPRSbuf[G2num][c] == 'R')
			{
        sprintf(buf,"RE");     
        if (GPRSbuf[G2num][c+2]=='1')
        {
          sprintf(buf,"R1%d",GPRSbuf[G2num][c+1] - 48);
          nr = GPRSbuf[G2num][c+1] - 49;
          Radio_Control_Onoff(nr,1);    
        } 
        else if (GPRSbuf[G2num][c+2]=='0')
        {
          sprintf(buf,"R0%d",GPRSbuf[G2num][c+1] - 48);
          nr = GPRSbuf[G2num][c+1] - 49;
          Radio_Control_Onoff(nr,0);    
        }
				gsm_sem_response_sendgprs(buf);
			}

      //*******************************
      // Alarm Burglary
      //*******************************
      else if (GPRSbuf[G2num][c] == 'A')
      {
				if ((GPRSbuf[G2num][c+1] == '1'))
        {
     			alarm_AktAlarm();			  //Alarm på
	        sprintf(buf,"A1");
	      }
  			else if ((GPRSbuf[G2num][c+1] == '0'))
        {
     			alarm_DeAktAlarm();
          sprintf(buf,"A0");
        }    
				else if ((GPRSbuf[G2num][c+1] == '2'))
        {
     			alarm_DeAktAlarm();
     			SetFlag(FEdisall);
	        sprintf(buf,"A2");
	      }
        else
          sprintf(buf,"AE");
        gsm_sem_response_sendgprs(buf);
      }

      //*******************************
      //Navn på trådløse enheter
      //*******************************
      else if (GPRSbuf[G2num][c]=='Q')
      {
        e = GPRSbuf[G2num][c+1] - 49;
        sprintf(buf,"QE");
        if (e < 9)
        {
          d = 0;
          c = c + 2;
          while ((GPRSbuf[G2num][c] != '#') && (d < 16))    //16 chars max.
          {
            Ereg[e].name[d] = GPRSbuf[G2num][c];
            c++;
            d++;
          }
          c--;
          Ereg[e].name[d] = 0;							//End buffer.
          sprintf(buf,"Q%d",e+1);
        }
        gsm_sem_response_sendgprs(buf);
      }

      //*******************************
      //Slett device fra system
      //*******************************
      else if (GPRSbuf[G2num][c]=='V')
      {
        sprintf(buf,"VE");
        e = GPRSbuf[G2num][c+1] - 49;
        if (e < 9)
        {
          Ereg[e].registerred = 0;
          sprintf(buf,"V%d",nr+1);
        }
        gsm_sem_response_sendgprs(buf);
      }

      //*******************************
      //Legg til device til system
      //*******************************
      else if (GPRSbuf[G2num][c]=='W')
      {
        sprintf(buf,"W");
        gsm_sem_response_sendgprs(buf);
        NewIdent();
      }

      //*******************************
      //Check Radio
      //*******************************
      if (GPRSbuf[G2num][c]=='v')
      {
        if (CheckRadio() == 1)
          sprintf(buf,"vE");
        else
          sprintf(buf,"v");
        gsm_sem_response_sendgprs(buf);
      }  

      //*******************************
      //Init radio
      //*******************************
      if (GPRSbuf[G2num][c]=='c')
      {
        Radio_init();
        sprintf(buf,"c");
        gsm_sem_response_sendgprs(buf);
      }  
      
      //*******************************
      //Radio values
      //*******************************
      if (GPRSbuf[G2num][c]=='x')
      {
      	if ((R_CONFIG(0) != 0x73) || (R_CONFIG(1) != 0x2C) || (R_CONFIG(2) != 0x44) || (R_CONFIG(3) != 0x20) || (R_CONFIG(4) != 0x20) || (R_CONFIG(9) != 0xDB))
          sprintf(buf,"xE %02X %02X %02X %02X %02X %02X ",R_CONFIG(0),R_CONFIG(1),R_CONFIG(2),R_CONFIG(3),R_CONFIG(4),R_CONFIG(9));
        else
          sprintf(buf,"x %02X %02X %02X %02X %02X %02X ",R_CONFIG(0),R_CONFIG(1),R_CONFIG(2),R_CONFIG(3),R_CONFIG(4),R_CONFIG(9));
        gsm_sem_response_sendgprs(buf);
      }  
                      
      //*******************************
      // Status noder
      //*******************************
      else if (GPRSbuf[G2num][c]=='S')
      {
				e = GPRSbuf[G2num][c+1] - 48;
        if (e < 10)
				{
  				e--;
					if (Ereg[e].registerred == 0)              //Nothing registered at this position
						sprintf(buf,"S%dEMPTY", e+1);			//Empty position
					else
					{
						sprintf(buf,"S%d", e+1);
						a = 0;
						while (a < 32)
						{
							buf2[a] = 0;
							a++;
						}
						buf2[0] = RTX_COM_STATUS1;					//Get status
						res = Radio_Get_Status1(e, (char *)buf2);		//Wait for status.
						if (res != 0)
							strcat(buf, "ERROR");                 //No response.
						else
						{
							sprintf(tbuf,"%02X",Ereg[e].type);
							strcat(buf,tbuf);
							a = 0;
							while (a < 32)
							{
								sprintf(tbuf,"%02X",buf2[a]);
								strcat(buf,tbuf);
								a++;
							}
						}
					}
				}
				else
					sprintf(buf,"SE");
				gsm_sem_response_sendgprs(buf);
      }  

			//*******************************
			//** Send device information
			//*******************************
			else if (GPRSbuf[G2num][c] == 'Z')
			{
				sprintf(buf,"ZE");
				d = GPRSbuf[G2num][c+1] - 49;				//
				if (d < 9)
				{
					if (Ereg[d].registerred != 0)       //Send only reg equ
					{
						sprintf(buf,"Z%d",d+1);
          	ptrdata = (char eeprom *)&Ereg[d];
            e = 0;
						while (e < 50)
						{
							sprintf(tbuf,"%02X",*ptrdata);
							strcat(buf,tbuf); 
              ptrdata++;
							e++;
						}
					}
					else
						sprintf(buf,"Z%dEMPTY",d+1);
				}
				gsm_sem_response_sendgprs(buf);
			}

      //*******************************
      // Change Code
      //*******************************
      else if (GPRSbuf[G2num][c] == 'C')
      {
        Ekode1 = GPRSbuf[G2num][c+1];
        Ekode2 = GPRSbuf[G2num][c+2];
        Ekode3 = GPRSbuf[G2num][c+3];
        Ekode4 = GPRSbuf[G2num][c+4];
        sprintf(buf,"C");
        gsm_sem_response_sendgprs(buf);
      }

      //*******************************
      // Termostat PÅ/AV
      //*******************************
      else if (GPRSbuf[G2num][c]=='J')
      {
        sprintf(buf,"JE");
        if (GPRSbuf[G2num][c+1] - 49 < 9)
        {
          if (GPRSbuf[G2num][c+2] == '0')												  //Slå av termostat
          {
            Radio_Control_Termonoff(GPRSbuf[G2num][c+1] - 49, 0);
            sprintf(buf,"J0%d",GPRSbuf[G2num][c+1] - 48);
          }
          else if (GPRSbuf[G2num][c+2] == '1')												  //Slå på termostat
          {
            Radio_Control_Termonoff(GPRSbuf[G2num][c+1] - 49, 1);
            sprintf(buf,"J1%d",GPRSbuf[G2num][c+1] - 48);
          }
        }
        gsm_sem_response_sendgprs(buf);
      }               
        
      //*******************************
      // Termostat Values
      //*******************************
      else if (GPRSbuf[G2num][c]=='L')
      {
        sprintf(buf,"LE");
        if (GPRSbuf[G2num][c+1] - 49 < 9)
        {
          Radio_Control_Termval(GPRSbuf[G2num][c+1] - 49, GPRSbuf[G2num][c+2], GPRSbuf[G2num][c+3], GPRSbuf[G2num][c+4], GPRSbuf[G2num][c+5], GPRSbuf[G2num][c+6], GPRSbuf[G2num][c+7]);
          sprintf(buf,"L%d",GPRSbuf[G2num][c+1] - 48);
        }
        gsm_sem_response_sendgprs(buf);
      }
        
      //*******************************
      // Temperaturovervåkning PÅ/AV
      //*******************************
      else if (GPRSbuf[G2num][c]=='Y')
      {
        sprintf(buf,"YE");
        if (GPRSbuf[G2num][c+1] - 49 < 9)
        {
          if (GPRSbuf[G2num][c+2] == '0')												//Slå av overvåkning
          { 
            Radio_Control_Limonoff(GPRSbuf[G2num][c+1] - 49, 0);
            sprintf(buf,"Y0%d",GPRSbuf[G2num][c+1] - 48);
          }
          else if (GPRSbuf[G2num][c+2] == '1')												//Slå på overvåkning
          {
            Radio_Control_Limonoff(GPRSbuf[G2num][c+1] - 49, 1);
            sprintf(buf,"Y1%d",GPRSbuf[G2num][c+1] - 48);
          }
        }
        gsm_sem_response_sendgprs(buf);
      }

      //*******************************
      // Temperaturovervåkning Grenser
      //*******************************
      else if (GPRSbuf[G2num][c]=='T')
      {
        sprintf(buf,"TE");
        if (GPRSbuf[G2num][c+1] - 49 < 9)
        {
          Radio_Control_Limval(GPRSbuf[G2num][c+1] - 49, GPRSbuf[G2num][c+2], GPRSbuf[G2num][c+3], GPRSbuf[G2num][c+4], GPRSbuf[G2num][c+5], GPRSbuf[G2num][c+6], GPRSbuf[G2num][c+7]);
          sprintf(buf,"T%d",GPRSbuf[G2num][c+1] - 48);
        }
        gsm_sem_response_sendgprs(buf);
      }

      //*******************************
      // Info message
      //*******************************
      else if (GPRSbuf[G2num][c] == 'I')
      {
        e = GPRSbuf[G2num][c+1];
        gprs_info(e);
      }

      //*******************************
			//** Send ID
      //*******************************
			else if (GPRSbuf[G2num][c] == 'X')
				gprs_send_id();

			//*******************************
			// Allocated positions
			//*******************************
			else if (GPRSbuf[G2num][c] == 'b')
			{
				sprintf(buf,"b");
				regval = 0;
				e = 9;
				while (e > 0)
				{
					e--;
					regval = regval << 1;
					if (Ereg[e].registerred == 1)
						regval = regval | 0x01;
				}
				sprintf(tbuf,"%04X",regval);
				strcat(buf,tbuf);
				gsm_sem_response_sendgprs(buf);
			}

      //*******************************
			//** Diverse kommandoer 2
      //*******************************
			else if (GPRSbuf[G2num][c]=='%')
			{
        //Innmeld
   			if (GPRSbuf[G2num][c+1] == '1')					//Text for EID. 20 chars.
				{  
          sprintf(buf,"%%1-Wait for registering complete");
          gsm_sem_response_sendgprs(buf);
          e = NewIdent(); 
          if (e != 0) 
            sprintf(buf,"%%1E%d",e);
          else
            sprintf(buf,"%%1",e);
          gsm_sem_response_sendgprs(buf);
        }                                         
        //EID
   			else if (GPRSbuf[G2num][c+1] == '4')					//Text for EID. 20 chars.
				{                                             //Must contain ; and \r at end if applicable
  				d=0;
  				while (d < 20)
  				{
  					EID[d] = GPRSbuf[G2num][c+d+2];
  					d++;
  				}
          sprintf(buf,"%%4");
          gsm_sem_response_sendgprs(buf);
          c = c + d + 1;
        }
      }

      //*******************************
			//** Diverse kommandoer 3
      //*******************************
			else if (GPRSbuf[G2num][c]=='&')
			{
        //Set Radio adress
  			if (GPRSbuf[G2num][c+1] == '1')					        //
				{                                               //**********
          
          d = 0;           
          adr = 0;
          while (d < 8)
          {
            adr = adr << 4;
            adr = adr + toint(GPRSbuf[G2num][c+2+d]);
            d++;  
          }     
          c = c + 8;
//          ERXadr = adr;
          W_RX_ADDRESS(adr);                     //Write new adress to central
          sprintf(buf,"&1%04X%04X",adr>>16,adr);
          gsm_sem_response_sendgprs(buf);
        }
        //Raw Radio mode
  			else if (GPRSbuf[G2num][c+1] == '2')					        //Factory reset
				{                                                     //**********
          if (CheckFlag(FRraw))
          {
            ClearFlag(FRraw);
            sprintf(buf,"&20");
          }
          else
          {  
            SetFlag(FRraw);
            sprintf(buf,"&21");
          }
          gsm_sem_response_sendgprs(buf);
        }
        //CD AM mode
  			else if (GPRSbuf[G2num][c+1] == '3')
				{                                                     //**********
          if (CheckFlag(FRamcd))
          {
            ClearFlag(FRamcd);
            sprintf(buf,"&30");
          }
          else
          {  
            SetFlag(FRamcd);
            sprintf(buf,"&31");
          }
          gsm_sem_response_sendgprs(buf);
        }
        //Factory reset
  			else if (GPRSbuf[G2num][c+1] == '5')					        //Factory reset
				{                                                     //**********
          reseteeprom();
          sprintf(buf,"&5");
          gsm_sem_response_sendgprs(buf);
        }
      }
      //** Next command.
			c++;

    }
	}
  else
  {
    sprintf(buf,"ERROR");
    gsm_sem_response_sendgprs(buf);
  }
	//** Clear buffer and mark as free
	a = 0;
	while (a < 64)
	{
		GPRSbuf[G2num][a] = 0;
		a++;
	}
	GPRSbuf[G2num][64] = 0;							//Mark free.

}

