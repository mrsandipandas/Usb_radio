#include <mega328p.h>
#include <delay.h>
#include "eeprom.h"
#include "flags.h"
#include "init.h"
#include "radio.h"

//For Radio
//unsigned char         Txdata[32];			//Buffer for sending.
//unsigned char         Rxdata[32];			//Buffer for mottak
 
//*****************************************************************************
//*****************************************************************************
// Initialiser og start opp radio
// OBS: Radio er i "Stand By" etter funskjon er utført
//*****************************************************************************
//*****************************************************************************
void Radio_init(void)
{
	WDR();
	PWR_UP0           			//Power off radio
	delay_ms(5);      			//Vent til power off ok
	TXENC0                   //
	TRX_CE0									//Power save for å kunne konfigurere
	CSN1;						 				//Chip select for radio
	PWR_UP1           			//Power on radio
	delay_ms(5);      			//Vent til power on ok
	W_RX_ADDRESS(ERXadr);		//Sett min adresse
	W_CONFIG(0x00,0x73);		//Kanal nummer
	W_CONFIG(0x01,0x2C);		//Retrans:ON redpow:OFF PWR:10dBm PLL:433MHz  **OBS:Sjekkes også i checkradio**
	W_CONFIG(0x02,0b01000100);		//Address 32 bit RX TX
	W_CONFIG(0x03,0x20);		//Payload 32 byte receiver
	W_CONFIG(0x04,0x20);		//Payload 32 byte transmitter
	W_CONFIG(0x09,0xDB);		//CRC:16bit CRC:ON Xtal:16MHz UPCLK:OFF UPCLK:500KHz
}

//*****************************************************************************
// Les data tilbake fra radio. 
// Prøv å skriv en gang til hvis feil
// Hvis fortsatt feil, sett flagg.
//*****************************************************************************
char CheckRadio(void)
{
	char a,b,c,d;
	
	a = ERXadr & 0xFF;
	b = (ERXadr >> 8) & 0xFF;
	c = (ERXadr >> 16) & 0xFF;
	d = (ERXadr >> 24) & 0xFF;
	if ((R_CONFIG(0) != 0x73) || (R_CONFIG(1) != 0x2C) || (R_CONFIG(2) != 0x44) || (R_CONFIG(3) != 0x20) || (R_CONFIG(4) != 0x20) || (R_CONFIG(9) != 0xDB) || (R_CONFIG(5) != a) || (R_CONFIG(6) != b) || (R_CONFIG(7) != c) || (R_CONFIG(8) != d))
		Radio_init();
	if ((R_CONFIG(0) != 0x73) || (R_CONFIG(1) != 0x2C) || (R_CONFIG(2) != 0x44) || (R_CONFIG(3) != 0x20) || (R_CONFIG(4) != 0x20) || (R_CONFIG(9) != 0xDB) || (R_CONFIG(5) != a) || (R_CONFIG(6) != b) || (R_CONFIG(7) != c) || (R_CONFIG(8) != d))
	{
		SetFlag(FRradio);		//Sett flagg feil
		return(1);
	}
	ClearFlag(FRradio);		//OK
	return(0);
}          


//*****************************************************************************
// Les konfigurasjonsregister i Radio
// Argument: 1. register som det skal leses fra
// OBS: Radio er i "Stand By" etter funksjon er utført
//*****************************************************************************
char R_CONFIG(char reg)
{
	char a;
	
	TXENC0  								 			//Receive mode
	TRX_CE0											//Deaktiver ShockBurst
	CSN0;
	delay_ms(1);
	SPDR = reg | 0x10;
	while(!(SPSR & (1<<7)));		//Vent til sending ok
	a = Get_Byte();
	CSN1;
	return(a);
}

//*****************************************************************************
// Send data ut på radio. Sender i 100ms.
// Argument. TIL adresse. 0x00000000
// Sender det som ligger i payload (TXdata[])
// OBS: Radio er i "Stand By" etter funksjon er utført
//*****************************************************************************
void TXradio(unsigned long int adr, char *buf)
{
	unsigned int a = 0;
	W_TX_ADDRESS(adr);			//Sett til adresse
	W_TX_PAYLOAD(buf);					//Legg payload inn i radio
	TXENC0  									//Receive mode
	TRX_CE1									//Aktiver ShockBurst RX
	delay_ms(1);
	while (CD_chk && a < 10000)					//Noen andre som sender? Vent til kanal ledig!  
	{
		a++;
		WDR();
	}
	TXENC1										//Transmit mode Aktiver ShockBurst TX. OBS: Må være minst 10us lang puls.
	delay_ms(TransDelay);		//Retransmit foregår i 200 ms.
	TXENC0  								 	//Stand by
	TRX_CE0								 	//Deaktiver ShockBurst
}


//*****************************************************************************
// Sett addresse det skal sendes til. 4 byte
// Argument: 1. Adresse 4 byte. 0x00000000
// OBS: Radio er i "Stand By" etter funskjon er utført
//*****************************************************************************
void W_TX_ADDRESS(unsigned long int adr)
{
	char a;
	WDR();
	TXENC0  								 			//Stand by mode
	TRX_CE0											//Deaktiver ShockBurst
	CSN0;
	delay_ms(1);
	SPDR = 0x22;
	while(!(SPSR & (1<<7)));		//Vent til sending ok
	a = adr & 0xFF;
	delay_us(100);						//Vent til aktivert
	SPDR = a;
	while(!(SPSR & (1<<7)));		//Vent til sending ok
	a = (adr >> 8) & 0xFF;
	delay_us(100);						//Vent til aktivert
	SPDR = a;
	while(!(SPSR & (1<<7)));		//Vent til sending ok
	a = (adr >> 16) & 0xFF;
	delay_us(100);						//Vent til aktivert
	SPDR = a;
	while(!(SPSR & (1<<7)));		//Vent til sending ok
	a = (adr >> 24) & 0xFF;
	delay_us(100);						//Vent til aktivert
	SPDR = a;
	while(!(SPSR & (1<<7)));		//Vent til sending ok
	CSN1;
}

//*****************************************************************************
// Skriv til Payloadregister i Radio                            
// Data som skal sendes legges i Global RAM TXdata[31]
// OBS: Radio er i "Stand By" etter funksjon er utført
//*****************************************************************************
void W_TX_PAYLOAD(char *buf)
{
	char a=0;
	TXENC0  								 			//Receive mode
	TRX_CE0											//Deaktiver ShockBurst
	CSN0;
	delay_ms(1);
	SPDR = 0x20;								//WTP
	while(!(SPSR & (1<<7)));		//Vent til sending ok
	while (a != 32)             //Skriv 32 byte
	{
		WDR();
		SPDR = buf[a];
		while(!(SPSR & (1<<7)));		//Vent til sending ok
		a++;
	}
	CSN1;
}

//*****************************************************************************
// Motta data fra radio.
// Mottar det som ligger i payload (TXdata[])
// Returnerer 1 hvis det er mottak.
// OBS: Radio står i mottaks modus etter at funskjon er kjørt. Hvis intet er blitt motatt
// OBS: Står i stand by hvis det har vært mottak som skal behandles.
//*****************************************************************************
char RXradio(char *buf)
{
	WDR();
	TXENC0  									//Receive mode
	TRX_CE1									//Aktiver ShockBurst RX
	if (DR_chk)							//Mottak ok
	{
		delay_ms(TransDelay);	//Retransmit foregår i 200 ms.
		R_RX_PAYLOAD(buf);				//Klokk inn data
		return(1);
	}
	return(0);
}

//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
//*****************************************************************************

//*****************************************************************************
// Sett min addresse. 4 byte
// Argument: 1. Adresse 4 byte. 0x00000000
// OBS: Radio er i "Stand By" etter funskjon er utført
//*****************************************************************************
void W_RX_ADDRESS(unsigned long int adr)
{
	char a;
	a = adr & 0xFF;
	W_CONFIG(0x05,a);	
	a = (adr >> 8) & 0xFF;
	W_CONFIG(0x06,a);		
	a = (adr >> 16) & 0xFF;
	W_CONFIG(0x07,a);		
	a = (adr >> 24) & 0xFF;
	W_CONFIG(0x08,a);		
}

//*****************************************************************************
// Skriv til konfigurasjonsregister i Radio
// Argument: 1. Byte det skal skrives til 2. Data
// OBS: Radio er i "Stand By" etter funskjon er utført
//*****************************************************************************
void W_CONFIG(char byt, char data)
{
	WDR();
	TXENC0  								 			//Stand by mode
	TRX_CE0											//Deaktiver ShockBurst
	CSN0;
	delay_ms(1);
	SPDR = byt;
	while(!(SPSR & (1<<7)));		//Vent til sending ok
	delay_us(100);
	SPDR = data;
	while(!(SPSR & (1<<7)));		//Vent til sending ok
	CSN1;
}

//*****************************************************************************
// Hent Payloadregister fra Radio
// Argument: Data som skal sendes legges i Global RAM TXdata[31]
// OBS: Radio er i "Stand By" etter funskjon er utført
//*****************************************************************************
void R_RX_PAYLOAD(char *buf)
{
	char a=0;
	TXENC0  								 			//Receive mode
	TRX_CE0											//Deaktiver ShockBurst
	CSN0;
	delay_ms(1);
	SPDR = 0x24;								//RRP
	while(!(SPSR & (1<<7)));		//Vent til sending ok
	while (a != 32)             //Skriv 32 byte
	{
		WDR();
		buf[a] = Get_Byte();		//Hent byte.
		a++;
	}
	CSN1;
}

//*****************************************************************************
// Hent 1 byte fra MISO
// OBS: CSN må fortløpende liggende lav mens 32 byte data hentes.
//*****************************************************************************
char Get_Byte(void)
{
	char a,data;
	
	SPCR=0x00;						//Slå av SPI mens data klokkes inn.
	a = 0;
	data = 0;
	while (a != 8)
	{
		WDR();
		SCK1;
		delay_us(10);
		data = data * 2;
		if (MISO_chk)
			data++;
		SCK0;
		delay_us(10);
		a++;
	}
	SPI_init();						//Aktiver SPI igjen
	return(data);
}


//*****************************************************************************
// Skap konstant bærebølge på radio
//*****************************************************************************
/*
void DebugRadio(void)
{
	TRX_CE0
	delay_ms(1);
	W_CONFIG(0x00,0x73);		//Kanal nummer
	W_CONFIG(0x01,0x0C);		//OBSOBSOBS: Retrans:OFF redpow:OFF PWR:10dBm PLL:433MHz
	W_CONFIG(0x03,0x20);		//Payload 32 byte receiver
	W_CONFIG(0x04,0x20);		//Payload 32 byte transmitter
	W_CONFIG(0x09,0xDB);		//CRC:16bit CRC:ON Xtal:16MHz UPCLK:OFF UPCLK:500KHz
	TRX_CE1  								//OBS: Må være minst 10us lang puls.
	TXENC1										//Transmit mode
	while(1)
		WDR();
}
*/
