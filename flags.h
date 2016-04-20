#ifndef _flags_INCLUDED_
#define _flags_INCLUDED_

void SetFlag(char flag);
void ClearFlag(char flag);
char CheckFlag(char flag);
extern unsigned long int     RFlag;

//***************************************
//Flags for storing in RAM. Will be lost at reset
//Will always have range 0-31
//***************************************
#define   FRTim1out   0      //Settes hver 15 sekund timer 1
#define   FRBtimO     1      //Timer 0 utløpt
#define   FRTim0      2      //For timeout ved kommunikasjon timer 0   //Benyttes bare i innmeldings rutine
#define   FRTim1      3      //For timeout ved kommunikasjon timer 1   //Benyttes i bryter sjekk rutine
#define   FRTim2      4      //For timeout ved kommunikasjon timer 2   //Blink reset
#define   FRTim3      5      //For timeout ved kommunikasjon timer 3   //
#define   FRTim4      6      //For timeout ved kommunikasjon timer 4   //Benyttes i waitrxe rutine
#define   FRTim5      7     //For timeout ved kommunikasjon timer 5   //Benyttes i status og info meldinger og for sending av alarm
#define   FRTim6      8     //For timeout ved kommunikasjon timer 6   //
#define   FRTim7      9     //For timeout ved kommunikasjon timer 7   //
#define   FRIntRXsms  10     //RX interrupt Benytter denne for å indikere sms mottak på gang.
#define   FRradio     11     //Radio ok flag. 0 if ok
#define   FROverFlow  12     //Radio ok flag. 0 if ok
#define   FRraw       13      //Raw radio mode
#define   FRamcd      14      //
#define   FRcd        15      //
#define   FRam        16      //

//***************************************
//Flags for storing in EEPROM
//Will always have range 32-63
//***************************************
#define   FEAlarm     32         //Flagg for innbruddsalarm på eller av.
#define   FEactinp    33         //Activate alarm input1
#define   FEdisall    34         //Disable all messages, alarms etc

#endif
