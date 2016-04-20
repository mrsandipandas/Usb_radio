#ifndef _alarm_INCLUDED_
#define _alarm_INCLUDED_

void alarm_AktAlarm(void);
void alarm_DeAktAlarm(void);
void alarm_Radio(RADIO_ALARM_MSG *myalarm, char nodeadr);
void AlarmRadioSMS(RADIO_ALARM_MSG *myalarm, char nr);
void strcate(char *str, char eeprom *estr);

#endif
