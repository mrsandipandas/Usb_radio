#ifndef _radio_INCLUDED_
#define _radio_INCLUDED_

#define TransDelay			200		//antall ms transmit foregår.
#define	Junkinfo				0			//Ingen mer informasjon skal sendes. Sett til 1 hvis det 

// Radio
void W_CONFIG(char,char);
void W_TX_PAYLOAD(char *buf);
void TXradio(unsigned long int, char *buf);
void Radio_init(void);
//void DebugRadio(void);
void W_TX_ADDRESS(unsigned long int);
char RXradio(char *buf);
void R_RX_PAYLOAD(char *buf);
char Get_Byte(void);
void W_RX_ADDRESS(unsigned long int);
char CheckRadio(void);
char R_CONFIG(char reg);
//void DebugRadio(void);

#endif
