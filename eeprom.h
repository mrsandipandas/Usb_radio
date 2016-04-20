#ifndef _eeprom_INCLUDED_
#define _eeprom_INCLUDED_
#include "rcom.h"
//EEprom
extern eeprom char Ekode1;
extern eeprom char Ekode2;
extern eeprom char Ekode3;
extern eeprom char Ekode4;
extern eeprom unsigned int Epower;
extern eeprom unsigned int Ewatch;
extern eeprom unsigned int Ebrown;
extern eeprom unsigned int Ereset;
extern eeprom unsigned int Etime;
extern eeprom unsigned long int EFlag;
extern eeprom unsigned long int        ERXadr;
extern eeprom RADIO_REGISTERED Ereg[9];
extern eeprom char EID[20];
#endif
