#ifndef _rcom_INCLUDED_
#define _rcom_INCLUDED_

void Sentralinit(void);
void Siren(unsigned char command);
void Radio(void);
char NewIdent(void);
char Radio_Control(unsigned char nr, volatile char *data);
char Radio_Control_Thermostat(unsigned char nr, volatile char *data);
char Radio_Control_Limits(unsigned char nr, volatile char *data);
char Radio_Get_Status1(unsigned char nr, volatile char *data);
char Radio_Control_Onoff(unsigned char nr, unsigned char onoff);
char Radio_Control_Limonoff(unsigned char nr, unsigned char onoff);
char Radio_Control_Limval(unsigned char nr, unsigned char a, unsigned char b, unsigned char c, unsigned char d, unsigned char e, unsigned char f);
char Radio_Control_Termonoff(unsigned char nr, unsigned char onoff);
char Radio_Control_Termval(unsigned char nr, unsigned char a, unsigned char b, unsigned char c, unsigned char d, unsigned char e, unsigned char f);

extern unsigned char  temp_info[9];      //temperature from wtemp
extern unsigned char  WCNT[9];           //Indication of new temp

#define UNS_8   unsigned char
#define UNS_32  unsigned long int

//*************************************************************************************************************************
//*************************************************************************************************************************
#define RADIO_NEWIDSOFT          8
#define RADIO_TIMEOUT            7
#define RADIO_NEWIDOUTOFSPACE    6
#define RADIO_NEWIDMISSINGPING  5
#define RADIO_ABORT              4
#define RADIO_NEWIDOK            3
#define RADIO_NEWID              2
#define RADIO_NEWIDTIMEOUT      1
#define RADIO_RXOK              0            //Reception on radio ok
#define RADIO_OK                 0
#define RADIO_ERROR              -1          //Feil med init av radio
#define RADIO_RXEMPTY            -2          //No transmission received

//*************************************************************************************************************************
//Subcommands for radio ECO-Starter -> extern
//*************************************************************************************************************************
//Thermostat
#define RSUB_THERMOSTAT_OFF      0
#define RSUB_THERMOSTAT_ON      1
#define RSUB_THERMOSTAT_LOW      2
#define RSUB_THERMOSTAT_HIGH    3
#define RSUB_THERMOSTAT_LOWHIGH  4
//Relay control, or ECO/COMF
#define RSUB_OUTPUT_OFF          0
#define RSUB_OUTPUT_ON          1
#define RSUB_OUTPUT_TOGGLE      2
#define RSUB_OUTPUT_VAL         3   //For div values to be transmitted. IVT T.ex
//Temperature limit alarm
#define RSUB_LIMITS_OFF          0
#define RSUB_LIMITS_ON          1
#define RSUB_LIMITS_LOW          2
#define RSUB_LIMITS_HIGH        3
#define RSUB_LIMITS_LOWHIGH      4


//*************************************************************************************************************************
//Commands from extern -> ECO-Starter  First byte of transmission
//*************************************************************************************************************************
#define RRX_COM_REGISTER  0xD0        //Register new equipment request.
#define RRX_COM_PINGREG    0xD1        //Ping from equ during registering
#define RRX_COM_ALARMCHK  0xD2        //Check alarm status on/off
#define RRX_COM_ALARMCHG  0xD3        //Alarm change (0 = off  1 = on  2 = toggle) With code
#define RRX_COM_ALARMGSM  0xD4        //New alarm from equipment.
#define RRX_COM_TEMP      0xD5        //Temp reading from wireless tempreader. (battery operated)
#define RRX_COM_OK        0xD6        //OK from remote equ after communicatoin.
#define RRX_COM_ONOFF      0xD7        //Request for turning on/off remote switches (0 = off  1 = on  2 = toggle)
#define RRX_COM_ONOFFSTAT  0xD8        //Request if output is on or off. 1 message for all nodes. 64 in all.
#define RRX_COM_STATUS1    0xD9        //Status message return. 1
#define RRX_COM_STATUS2    0xDA        //Status message return. 2
#define RRX_COM_STATUS3    0xDB        //Status message return. 3
#define RRX_COM_PING       0xDC        //Answer to ping
#define RRX_COM_PANEL      0xDD        //Command from wireless panel to turn on or off remote devices.
#define RRX_COM_DIM       0xDE        //Dimmer values from dimmer panel.
#define RRX_COM_ALARMSIMP  0xDF        //Alarm change (0 = off  1 = on  2 = toggle) Without code
#define RRX_COM_PANSTAT   0xE0        //Command from wireless panel. Status for device registered with panel switch.
#define RRX_COM_TEST      0xE1        //Test command for checking equ. Only during production.
#define RRX_COM_TEMPOUT   0xE2        //Outside temperature for distribution.
#define RRX_COM_MOIST    0xE3        //Moisture measurement.


//*************************************************************************************************************************
//Commands from ECO-Starter -> extern  First byte of transmission
//*************************************************************************************************************************
#define RTX_COM_REGANS    0xB0        //Register new equipment answer.
#define RTX_COM_ALARMSTAT  0xB1        //Answer alarmstatus on/off
#define RTX_COM_OUTCHG    0xB2        //Output change (0 = off  1 = on  2 = toggle)
#define RTX_COM_ECOCOMF    0xB3        //Eco and comfort limits for thermostat
#define RTX_COM_TEMPSURV  0xB4        //Tempsurveilance limits
#define RTX_COM_STATUS1    0xB5        //Request status from nodes 1
#define RTX_COM_STATUS2    0xB6        //Request status from nodes 2
#define RTX_COM_STATUS3    0xB7        //Request status from nodes 3
#define RTX_COM_PING      0xB8        //Request status from nodes
#define RTX_COM_ALARMREC  0xB9        //Alarm received acknowledge
#define RTX_COM_SIREN      0xBA        //Siren activate/deactivate
#define RTX_COM_DIM        0xBB        //Dimmer values and on or off.
#define RTX_COM_ACK       0xBC        //Message received ok. ACK back to device. Params can be sent.
#define RTX_COM_SETTINGS  0xBD        //Sending of settings
#define RTX_COM_SETTINGS2 0xBE        //Sending of settings2


//*************************************************************************************************************************
//Type of radioequipment
//*************************************************************************************************************************
#define RTYPE_NODE        0x00        //Node (1 output and 2 inputs possible)
#define RTYPE_TAST        0x01        //Tastatur 0-9 S A
#define RTYPE_SIREN        0x02        //Siren node
#define RTYPE_DETECTOR    0x03        //Wireless detector with battery. (PIR Magnet Fire Water etc)
#define RTYPE_ENERGY      0x04        //Energy reader
#define RTYPE_PANEL        0x05        //Panel for switching on/off or dimming (Will be programmable)
#define RTYPE_EXPOUT      0x06        //Extender in fusebox with x outputs and inputs (GR-1 GR-3 etc)
#define RTYPE_EXPINP      0x07        //Extender in fusebox with x inputs only
#define RTYPE_TEMP        0x08        //Temperature reader (WTEMP with battery)
#define RTYPE_THERM        0x09        //Thermostat  (SI-1 SI-2 NOBØ)
#define RTYPE_DIMMER      0x0A        //Dimmer device
#define RTYPE_IVT          0x0B        //IVT Heat Pump special fo 10C and Normal
#define RTYPE_IVTR        0x0C        //IVT Heat Pump Radio built in pump.
#define RTYPE_400V        0x0D        //400V new Node type
#define RTYPE_OUTTEMP     0x0E        //Outside Wtemp for connection to 400V node. Only one in the system required
#define RTYPE_MOISTURE     0x0F        //Moisture measurement

//*************************************************************************************************************************
//Connected equ and Alarm message if alarm.
//ALARM type transmitted if alarm
//*************************************************************************************************************************
#define RALARMTYPE_NONE          0x00        //No Alarm connected
#define RALARMTYPE_PIR          0x01        //** Burglary
#define RALARMTYPE_FIRE          0x02
#define RALARMTYPE_WATER        0x03
#define RALARMTYPE_MAGNET        0x04        //**
#define RALARMTYPE_GAS          0x05
#define RALARMTYPE_GENERIC      0x06        //Own decision. Generic alarm.
#define RALARMTYPE_WARN          0x07        //OVERTEMP.
#define RALARMTYPE_TEMPHI        0x08
#define RALARMTYPE_TEMPLO        0x09
#define RALARMTYPE_GENPIR        0x0A        //**Own decision. Generic burglary detector
#define RALARMTYPE_TEMP          0x0B
#define RALARMTYPE_BATT          0x0C        //Battery low alarm
#define RALARMTYPE_230VFAIL      0x0D        //230V fail
#define RALARMTYPE_230VOK        0x0F        //230V ok.
#define RALARMTYPE_12VFAIL      0x10        //12V fail
#define RALARMTYPE_12VOK        0x11        //12V ok.
#define RALARMTYPE_CLOCKSTART    0x12        //Clock has not been initialized at startup.
#define RALARMTYPE_CLOCKERR      0x13        //Clock has failed during normal operation
#define RALARMTYPE_CLOCKFAULT    0x14        //Clock hardware error. Only checked during startup.
#define RALARMTYPE_IVTFAIL      0x15        //Error from IVT Heat pump. Decode the error accordingly.
#define RALARMTYPE_MOIST        0x16        //Moisture detector. Battery powered

//Structure for registering new equipment
//Structure from remote equipment.
typedef struct
{
    UNS_8    command;        //The register command RRX_COM_REGISTER
    UNS_8    type;            //Type of equipment. See RTYPE.
    UNS_8    numreg;          //Number of positions this equ requires.
    UNS_8   outputs;        //Number of outputs
    UNS_8    inputs;          //Number of inputs
    UNS_32  model;          //Model number
    UNS_8    majversion;      //Software version major of Radio EQU
    UNS_8    minversion;      //Software version minor of Radio EQU
    UNS_8    battery;        //230V or 12V if 0. Battery operated if 1. Backup battery if 2.
    UNS_8    siren;          //Siren output if 1.
    UNS_8    softreq;        //Software req of central. 0x10 for V1.0
    char    modelname[10];  //model name. Descriptive of what equ it is.
    UNS_8    pad[8];          //Padding up to 32 bytes.
} RADIO_REG_EQUIP;


//Structure for Registered equipment stored in eeprom in central. EQUSTORAGE[xx]
typedef struct 
{
    UNS_32  adr;            //Device address
    UNS_8   subadr;         //Address within equipment if more than 1 output (0-255)
    UNS_8   type;           //Type of equipment according to RTYPE_
    UNS_8   numreg;         //number of positions taken for this device
    UNS_8   outputs;        //Number of outputs
    UNS_8   inputs;         //Number of inputs
    UNS_32  model;          //Model number
    UNS_8   majversion;     //Software version major
    UNS_8   minversion;     //Software version minor
    UNS_8   battery;        //230V or 12V if 0. Battery operated if 1. Backup battery if 2.
    UNS_8   siren;          //Siren output if 1.
    char    name[17];       //Name of device
    char    modelname[10];  //Model Name (node SI1 SI2 etc.)
    UNS_8   intime;         //8 bits for indicating which detectorinput should have first delay before alarm is sent.
    UNS_8   pancont;        //Wireless panel. Which device or group to control. 0xFF = not registered. 00-63 = device 64-68 = group 1-5
    UNS_8   temp[3];        //Temporary variable. Used for temperature surv storing. 0=Hi 1=Lo 2=De/Active
    UNS_8   registerred;    //0 if position is clear. registered otherwise.
} RADIO_REGISTERED;

//Structure for received message from remote equipment.
typedef struct
{
    UNS_8    command;    //
    UNS_8    sub;        //Subadr within device
    UNS_8    pad[26];    //Padding up to 32 bytes.
    UNS_32  fromadr;    //Remote equ adr
} RADIO_RECEIVED_MSG;

//Structure for sending message to remote equipment.
typedef struct
{
    UNS_8    command;    //
    UNS_8    sub;        //Subadr within device
    UNS_8    pad[26];    //Padding up to 32 bytes.
    UNS_32  fromadr;    //Central adr
} RADIO_SEND_MSG;

//Structure for status from Device
typedef struct
{
    UNS_8    command;    //
    UNS_8    subadr;      //If more than 1 output on devicet. Indicate who from. (0-255)
    UNS_8    status1;    //Bit0=Output implemented. Bit1=ON/OFF. Bit2=ECO/Comfort implemented. Bit3=ECO/Comfort active. Bit4=ECO/Comfort status.
                        //Bit5=Dimmer implemented.
    UNS_8    status2;    //Bit0=Plasmacluster Bit1-2=Mode Bit3-4=FanSpeed Bit5=Valid com with pump if 0.
    UNS_8    status3;    //Error code from IVT pump or Outside temp setting for Enode(IVT 400V)

    UNS_8    inptype1;
    UNS_8    inpstat1;    //Tempsurv active if bit0=1.
    UNS_8    inptemp1;		//0xFF indicate no sensor connected or error with sensor
		UNS_8  	inpsurvH1;
		UNS_8  	inpsurvL1;
		UNS_8  	inptermH1;
		UNS_8  	inptermL1;

		UNS_8  	inptype2;
		UNS_8  	inpstat2;
		UNS_8  	inptemp2;
		UNS_8  	inpsurvH2;	//Not used in current version...
		UNS_8  	inpsurvL2;	//Not used in current version...

		UNS_8  	statusnr;		//Which status is returned. Only 1 implemented yet.
		UNS_8		dimval;			//Dimmer value if implemented. 0 = off. 100 = on
		UNS_8		pad[10];		//Padding up to 32 bytes.
		UNS_32  fromadr;		//Central adr
} RADIO_STATUS_EQUIP;


//Alarmmessage from remote equ.
typedef struct
{
		UNS_8		command;		//
		UNS_8  	subadr;			//If more than 1  on equipment. Indicate who from. (0-255)
		UNS_8		input;			//ASCII encoded (0 if only 1 input on equ.  1 if input 1 or 2 if input 2)
		UNS_8		type;				//
		UNS_8		pri;				//
		UNS_8		pad[23];		//Padding up to 32 bytes.
		UNS_32  fromadr;		//Central adr
} RADIO_ALARM_MSG;

#endif
