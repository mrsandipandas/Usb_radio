#ifndef _init_INCLUDED_
#define _init_INCLUDED_


//************************************************************************************
//** Må endres for hver versjon
//************************************************************************************
//Produktidentifisering
#define Prodtxt "USB-Central V1.0 "
//GPRS ver
#define GPRSVertxt "USBC10"


void SPI_init(void);
void port_init(void);
void init_devices(void);
void watchdog_init(void);
void timer0_init(void);
void timer1_init(void);
void timer2_init(void);
void uart0_init(void);
void adc_init(void);
//Fra main
void Timechk(void);
void reseteeprom(void);
void Check_cdam(void);

//Radio Definisjoner
#define CSN1            PORTB|=BIT(char,PB2);      //Enable SPI interface
#define CSN0            PORTB&=~BIT(char,PB2);    //Disable SPI interface
#define TRX_CE1         PORTD|=BIT(char,PD6);      //Enable TX-RX
#define TRX_CE0         PORTD&=~BIT(char,PD6);    //Disable TX-RX
#define TXENC1          PORTD|=BIT(char,PD5);      //Enable TX
#define TXENC0          PORTD&=~BIT(char,PD5);    //Enable RX
#define PWR_UP1         PORTD|=BIT(char,PD2);      //Slå på radio
#define PWR_UP0         PORTD&=~BIT(char,PD2);    //Slå av radio
#define CD_chk          PIND&BIT(char,PD4)        //Carrier Detect inngang
#define AM_chk          PINB&BIT(char,PB0)        //Address Match
#define DR_chk          PIND&BIT(char,PD7)        //Receive and Transmit data ready
#define MISO_chk        PINB&BIT(char,PB4)				//
#define SCK1						PORTB|=BIT(char,PB5);    	//Enable SPI interface
#define	SCK0			 			PORTB&=~BIT(char,PB5);		//Disable SPI interface

//Definisjoner for inn ut pinner
#define	CTS_on					PORTC|=BIT(char,PC0);		//
#define	CTS_off					PORTC&=~BIT(char,PC0);		//

#define	CR	0x0D		//Carriage Return
#define	LF	0x0A		//Line Feed

#define BIT(cast,a) (1<<(cast)a)		//cast = casting
#define SEI() #asm("sei")
#define CLI() #asm("cli")
#define WDR() #asm("wdr")

#define RXB8 1
#define TXB8 0
#define UPE 2
#define OVR 3
#define FE 4
#define UDRE0 5
#define RXC 7
#define FRAMING_ERROR (1<<FE)
#define PARITY_ERROR (1<<UPE)
#define DATA_OVERRUN (1<<OVR)
#define DATA_REGISTER_EMPTY (1<<UDRE)
#define RX_COMPLETE (1<<RXC)

#define	PD0 0
#define	PD1 1
#define	PD2 2
#define	PD3 3
#define	PD4 4
#define	PD5 5
#define	PD6 6
#define	PD7 7
#define	PB0 0
#define	PB1 1
#define	PB2 2
#define	PB3 3
#define	PB4 4
#define	PB5 5
#define	PB6 6
#define	PB7 7
#define	PC0 0
#define	PC1 1
#define	PC2 2
#define	PC3 3
#define	PC4 4
#define	PC5 5
#define	PC6 6
#define	PC7 7

#endif
